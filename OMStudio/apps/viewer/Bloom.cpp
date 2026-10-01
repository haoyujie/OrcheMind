#include "Bloom.h"
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Utility/Debug.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/GL/Version.h>
#include <algorithm>

using namespace Magnum;

namespace {

struct TriVert {
    Vector2 pos;
    Vector2 uv;
};

void prepareColor(GL::Texture2D& tex) {
    tex.setMinificationFilter(GL::SamplerFilter::Linear)
        .setMagnificationFilter(GL::SamplerFilter::Linear)
        .setWrapping(GL::SamplerWrapping::ClampToEdge);
}

} // namespace

Bloom::BlurShader::BlurShader() {
    GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex};
    GL::Shader frag{GL::Version::GL330, GL::Shader::Type::Fragment};
    vert.addSource(R"GLSL(
in vec2 position;
in vec2 uv;
out vec2 vUv;
void main() {
    vUv = uv;
    gl_Position = vec4(position, 0.0, 1.0);
}
)GLSL");
    frag.addSource(R"GLSL(
in vec2 vUv;
uniform sampler2D tex;
uniform vec2 texel;
uniform float threshold;
out vec4 color;
void main() {
    vec3 c = texture(tex, vUv).rgb * 0.227027;
    c += texture(tex, vUv + texel).rgb * 0.1945946;
    c += texture(tex, vUv - texel).rgb * 0.1945946;
    c += texture(tex, vUv + 2.0 * texel).rgb * 0.1216216;
    c += texture(tex, vUv - 2.0 * texel).rgb * 0.1216216;
    c += texture(tex, vUv + 3.0 * texel).rgb * 0.054054;
    c += texture(tex, vUv - 3.0 * texel).rgb * 0.054054;
    if (threshold > 0.0) {
        float lum = max(c.r, max(c.g, c.b));
        c *= smoothstep(threshold - 0.2, threshold + 0.05, lum);
    }
    color = vec4(c, 1.0);
}
)GLSL");
    CORRADE_INTERNAL_ASSERT_OUTPUT(vert.compile() && frag.compile());
    attachShader(vert);
    attachShader(frag);
    bindAttributeLocation(0, "position");
    bindAttributeLocation(1, "uv");
    CORRADE_INTERNAL_ASSERT_OUTPUT(link());
    setUniform(uniformLocation("tex"), 0);
    texelUniform_ = uniformLocation("texel");
    thresholdUniform_ = uniformLocation("threshold");
}

void Bloom::BlurShader::setTexelThreshold(const Vector2& texel, float threshold) {
    setUniform(texelUniform_, texel);
    setUniform(thresholdUniform_, threshold);
}

Bloom::CompositeShader::CompositeShader() {
    GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex};
    GL::Shader frag{GL::Version::GL330, GL::Shader::Type::Fragment};
    vert.addSource(R"GLSL(
in vec2 position;
in vec2 uv;
out vec2 vUv;
void main() {
    vUv = uv;
    gl_Position = vec4(position, 0.0, 1.0);
}
)GLSL");
    frag.addSource(R"GLSL(
in vec2 vUv;
uniform sampler2D sceneTex;
uniform sampler2D b0;
uniform sampler2D b1;
uniform sampler2D b2;
uniform sampler2D b3;
uniform float strength;
out vec4 color;
void main() {
    vec3 scene = texture(sceneTex, vUv).rgb;
    vec3 bloom = texture(b0, vUv).rgb + texture(b1, vUv).rgb
               + texture(b2, vUv).rgb + texture(b3, vUv).rgb;
    color = vec4(scene + bloom * strength, 1.0);
}
)GLSL");
    CORRADE_INTERNAL_ASSERT_OUTPUT(vert.compile() && frag.compile());
    attachShader(vert);
    attachShader(frag);
    bindAttributeLocation(0, "position");
    bindAttributeLocation(1, "uv");
    CORRADE_INTERNAL_ASSERT_OUTPUT(link());
    setUniform(uniformLocation("sceneTex"), 0);
    setUniform(uniformLocation("b0"), 1);
    setUniform(uniformLocation("b1"), 2);
    setUniform(uniformLocation("b2"), 3);
    setUniform(uniformLocation("b3"), 4);
    strengthUniform_ = uniformLocation("strength");
}

void Bloom::CompositeShader::setStrength(float strength) {
    setUniform(strengthUniform_, strength);
}

Bloom::Bloom() {
    TriVert verts[] = {
        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
        {{3.0f, -1.0f}, {2.0f, 0.0f}},
        {{-1.0f, 3.0f}, {0.0f, 2.0f}},
    };
    triBuf_.setData(Containers::arrayView(verts), GL::BufferUsage::StaticDraw);
    tri_.setCount(3)
        .setPrimitive(GL::MeshPrimitive::Triangles)
        .addVertexBuffer(triBuf_, 0, GL::Attribute<0, Vector2>{}, GL::Attribute<1, Vector2>{});
}

void Bloom::allocColor(GL::Texture2D& tex, const Vector2i& size) {
    tex = GL::Texture2D{};
    prepareColor(tex);
    tex.setStorage(1, GL::TextureFormat::RGBA16F, size);
}

void Bloom::attach(GL::Framebuffer& fbo, GL::Texture2D& tex, const Vector2i& size, bool depth) {
    fbo = GL::Framebuffer{{{}, size}};
    fbo.attachTexture(GL::Framebuffer::ColorAttachment{0}, tex, 0)
        .mapForDraw(GL::Framebuffer::ColorAttachment{0});
    if (depth) {
        sceneDepth_ = GL::Renderbuffer{};
        sceneDepth_.setStorage(GL::RenderbufferFormat::DepthComponent16, size);
        fbo.attachRenderbuffer(GL::Framebuffer::BufferAttachment::Depth, sceneDepth_);
    }
    if (fbo.checkStatus(GL::FramebufferTarget::Draw) != GL::Framebuffer::Status::Complete)
        Debug{} << "[OMStudio] bloom framebuffer incomplete" << size.x() << size.y();
}

void Bloom::resize(const Vector2i& viewSize) {
    Vector2i size{std::max(1, viewSize.x()), std::max(1, viewSize.y())};
    if (size == view_) return;
    view_ = size;
    allocColor(sceneColor_, size);
    attach(sceneFbo_, sceneColor_, size, true);
    Vector2i levelSize = size;
    for (int i = 0; i < 4; ++i) {
        levelSize = Vector2i{std::max(1, levelSize.x() / 2), std::max(1, levelSize.y() / 2)};
        level_[i].size = levelSize;
        allocColor(level_[i].a, levelSize);
        allocColor(level_[i].b, levelSize);
        attach(level_[i].fa, level_[i].a, levelSize, false);
        attach(level_[i].fb, level_[i].b, levelSize, false);
    }
}

void Bloom::bindScene() {
    sceneFbo_.bind();
    glViewport(0, 0, view_.x(), view_.y());
    glClearColor(0.020f, 0.027f, 0.043f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Bloom::blurPass(GL::Framebuffer& dst, const Vector2i& size, GL::Texture2D& src,
                     const Vector2& texel, float threshold) {
    dst.bind();
    glViewport(0, 0, size.x(), size.y());
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    src.bind(0);
    blur_.setTexelThreshold(texel, threshold);
    blur_.draw(tri_);
}

void Bloom::composite(float strength) {
    if (view_.x() <= 0) return;
    const Vector2 sceneTexel{1.0f / view_.x(), 0.0f};
    blurPass(level_[0].fb, level_[0].size, sceneColor_, sceneTexel, 0.8f);
    blurPass(level_[0].fa, level_[0].size, level_[0].b, Vector2{0.0f, 1.0f / level_[0].size.y()}, 0.0f);

    GL::Texture2D* src = &level_[0].a;
    Vector2i srcSize = level_[0].size;
    for (int i = 1; i < 4; ++i) {
        blurPass(level_[i].fb, level_[i].size, *src, Vector2{1.0f / srcSize.x(), 0.0f}, 0.0f);
        blurPass(level_[i].fa, level_[i].size, level_[i].b, Vector2{0.0f, 1.0f / level_[i].size.y()}, 0.0f);
        src = &level_[i].a;
        srcSize = level_[i].size;
    }

    GL::defaultFramebuffer.bind();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    sceneColor_.bind(0);
    level_[0].a.bind(1);
    level_[1].a.bind(2);
    level_[2].a.bind(3);
    level_[3].a.bind(4);
    composite_.setStrength(strength);
    composite_.draw(tri_);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                   GL::Renderer::BlendFunction::OneMinusSourceAlpha);
}
