#include "VortexField.h"
#include "VortexGeom.h"
#include "omstudio/Entity.h"
#include <Corrade/Containers/ArrayView.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Math/Functions.h>
#include <random>

using namespace Magnum;

namespace {

const char* kPal = R"GLSL(
vec3 palette(float idx) {
    int k = int(mod(idx, 4.0));
    if (k == 0) return vec3(0.16, 0.90, 1.00);
    if (k == 1) return vec3(1.00, 0.24, 0.65);
    if (k == 2) return vec3(0.69, 0.42, 1.00);
    return vec3(1.00, 0.82, 0.35);
}
vec3 vortexAt(float theta, float phi) {
    float R = 2.05 + 0.30 * cos(3.0 * theta);
    float ph = phi + 0.5 * theta;
    float q = R + 0.58 * cos(ph);
    return vec3(q * cos(theta), 0.58 * sin(ph), q * sin(theta));
}
)GLSL";

const char* kPointFrag = R"GLSL(
in vec3 vColor;
out vec4 color;
void main() {
    vec2 uv = gl_PointCoord * 2.0 - 1.0;
    float d = dot(uv, uv);
    if (d > 1.0) discard;
    float a = exp(-d * 3.4);
    color = vec4(vColor * a, a);
}
)GLSL";

} // namespace

VortexField::PointShader::PointShader(bool onVortex) {
    GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex};
    GL::Shader frag{GL::Version::GL330, GL::Shader::Type::Fragment};
    if (onVortex) {
        vert.addSource(std::string(kPal) + R"GLSL(
in vec4 param;
uniform mat4 viewProj;
uniform mat4 spin;
uniform vec3 cameraPos;
uniform float time;
uniform float pointScale;
uniform float gain;
out vec3 vColor;
void main() {
    float th = param.x + param.z * time;
    vec3 p = (spin * vec4(vortexAt(th, param.y), 1.0)).xyz;
    gl_Position = viewProj * vec4(p, 1.0);
    float dist = max(length(p - cameraPos), 0.35);
    gl_PointSize = clamp(pointScale / dist, 1.5, 20.0);
    vColor = palette(param.w) * gain;
}
)GLSL");
        bindAttributeLocation(0, "param");
    } else {
        vert.addSource(R"GLSL(
in vec3 position;
in vec3 color;
uniform mat4 viewProj;
uniform vec3 cameraPos;
uniform float pointScale;
uniform float gain;
out vec3 vColor;
void main() {
    gl_Position = viewProj * vec4(position, 1.0);
    float dist = max(length(position - cameraPos), 0.35);
    gl_PointSize = clamp(pointScale / dist, 1.2, 16.0);
    vColor = color * gain;
}
)GLSL");
        bindAttributeLocation(0, "position");
        bindAttributeLocation(1, "color");
    }
    frag.addSource(kPointFrag);
    CORRADE_INTERNAL_ASSERT_OUTPUT(vert.compile() && frag.compile());
    attachShader(vert);
    attachShader(frag);
    CORRADE_INTERNAL_ASSERT_OUTPUT(link());
    viewProjUniform_ = uniformLocation("viewProj");
    cameraUniform_ = uniformLocation("cameraPos");
    pointScaleUniform_ = uniformLocation("pointScale");
    gainUniform_ = uniformLocation("gain");
    if (onVortex) {
        spinUniform_ = uniformLocation("spin");
        timeUniform_ = uniformLocation("time");
    }
}

VortexField::PointShader& VortexField::PointShader::setViewProj(const Matrix4& m) {
    setUniform(viewProjUniform_, m);
    return *this;
}
VortexField::PointShader& VortexField::PointShader::setSpin(const Matrix4& m) {
    setUniform(spinUniform_, m);
    return *this;
}
VortexField::PointShader& VortexField::PointShader::setCamera(const Vector3& cam) {
    setUniform(cameraUniform_, cam);
    return *this;
}
VortexField::PointShader& VortexField::PointShader::setTime(float t) {
    setUniform(timeUniform_, t);
    return *this;
}
VortexField::PointShader& VortexField::PointShader::setPointScale(float s) {
    setUniform(pointScaleUniform_, s);
    return *this;
}
VortexField::PointShader& VortexField::PointShader::setGain(float g) {
    setUniform(gainUniform_, g);
    return *this;
}

VortexField::LineShader::LineShader(bool trail) {
    GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex};
    GL::Shader frag{GL::Version::GL330, GL::Shader::Type::Fragment};
    if (trail) {
        vert.addSource(std::string(kPal) + R"GLSL(
in vec4 param;
in float age;
uniform mat4 viewProj;
uniform mat4 spin;
uniform float time;
out vec3 vColor;
void main() {
    float th = param.x + param.z * (time - age);
    vec3 p = (spin * vec4(vortexAt(th, param.y), 1.0)).xyz;
    gl_Position = viewProj * vec4(p, 1.0);
    float fade = exp(-age * 3.2);
    vColor = palette(param.w) * fade * 0.85;
}
)GLSL");
        bindAttributeLocation(0, "param");
        bindAttributeLocation(1, "age");
    } else {
        vert.addSource(R"GLSL(
in vec3 position;
in vec3 color;
uniform mat4 viewProj;
uniform mat4 spin;
out vec3 vColor;
void main() {
    gl_Position = viewProj * spin * vec4(position, 1.0);
    vColor = color;
}
)GLSL");
        bindAttributeLocation(0, "position");
        bindAttributeLocation(1, "color");
    }
    frag.addSource(R"GLSL(
in vec3 vColor;
out vec4 color;
void main() {
    color = vec4(vColor, 1.0);
}
)GLSL");
    CORRADE_INTERNAL_ASSERT_OUTPUT(vert.compile() && frag.compile());
    attachShader(vert);
    attachShader(frag);
    CORRADE_INTERNAL_ASSERT_OUTPUT(link());
    viewProjUniform_ = uniformLocation("viewProj");
    spinUniform_ = uniformLocation("spin");
    if (trail) timeUniform_ = uniformLocation("time");
}

VortexField::LineShader& VortexField::LineShader::setViewProj(const Matrix4& m) {
    setUniform(viewProjUniform_, m);
    return *this;
}
VortexField::LineShader& VortexField::LineShader::setSpin(const Matrix4& m) {
    setUniform(spinUniform_, m);
    return *this;
}
VortexField::LineShader& VortexField::LineShader::setTime(float t) {
    setUniform(timeUniform_, t);
    return *this;
}

VortexField::VortexField()
    : starsShader_{false}, particleShader_{true}, nodeShader_{false},
      spokeShader_{false}, trailShader_{true} {
    const float pi = 3.14159265f;
    std::mt19937 rng(20260326u);
    std::uniform_real_distribution<float> ang(0.0f, pi * 2.0f);
    std::uniform_real_distribution<float> uni(0.0f, 1.0f);

    struct PCol { Vector3 p; Vector3 c; };
    std::vector<PCol> stars;
    stars.reserve(360);
    for (int i = 0; i < 360; ++i) {
        float z = uni(rng) * 2.0f - 1.0f;
        float a = ang(rng);
        float r = std::sqrt(std::max(0.0f, 1.0f - z * z)) * (7.5f + uni(rng) * 5.0f);
        float y = z * (7.5f + uni(rng) * 5.0f);
        stars.push_back({{r * std::cos(a), y, r * std::sin(a)},
                         Vector3{0.45f, 0.55f, 0.75f} * (0.15f + uni(rng) * 0.25f)});
    }
    starCount_ = static_cast<int>(stars.size());
    starBuf_.setData(Containers::arrayView(stars), GL::BufferUsage::StaticDraw);
    starMesh_.setCount(starCount_)
        .setPrimitive(GL::MeshPrimitive::Points)
        .addVertexBuffer(starBuf_, 0, GL::Attribute<0, Vector3>{}, GL::Attribute<1, Vector3>{});

    struct Param { Vector4 p; };
    std::vector<Param> particles;
    particles.reserve(4000);
    for (int i = 0; i < 4000; ++i) {
        particles.push_back({Vector4{ang(rng), ang(rng), 0.35f + uni(rng) * 0.9f, float(i % 4)}});
    }
    particleCount_ = static_cast<int>(particles.size());
    particleBuf_.setData(Containers::arrayView(particles), GL::BufferUsage::StaticDraw);
    particleMesh_.setCount(particleCount_)
        .setPrimitive(GL::MeshPrimitive::Points)
        .addVertexBuffer(particleBuf_, 0, GL::Attribute<0, Vector4>{});

    struct TrailVert { Vector4 param; float age; };
    std::vector<TrailVert> trails;
    const int samples = 12;
    const float step = 0.045f;
    trails.reserve(static_cast<size_t>(particleCount_ * (samples - 1) * 2));
    for (const Param& particle : particles) {
        for (int k = 0; k < samples - 1; ++k) {
            trails.push_back({particle.p, k * step});
            trails.push_back({particle.p, (k + 1) * step});
        }
    }
    trailCount_ = static_cast<int>(trails.size());
    trailBuf_.setData(Containers::arrayView(trails), GL::BufferUsage::StaticDraw);
    trailMesh_.setCount(trailCount_)
        .setPrimitive(GL::MeshPrimitive::Lines)
        .addVertexBuffer(trailBuf_, 0, GL::Attribute<0, Vector4>{}, GL::Attribute<1, Float>{});

    std::vector<PCol> spokes;
    spokes.reserve(4000);
    for (int i = 0; i < 2000; ++i) {
        float th = ang(rng), ph = ang(rng);
        Vector3 p = vortexPosition(th, ph);
        Vector3 dir{p.x(), p.y() * 0.35f, p.z()};
        if (dir.dot() < 1e-6f) dir = Vector3::xAxis();
        else dir = dir.normalized();
        Vector3 end = p + dir * (0.75f + uni(rng) * 1.55f);
        Vector3 col = (i % 4 == 0) ? Vector3{0.16f, 0.90f, 1.0f} :
                      (i % 4 == 1) ? Vector3{1.0f, 0.24f, 0.65f} :
                      (i % 4 == 2) ? Vector3{0.69f, 0.42f, 1.0f} :
                                     Vector3{1.0f, 0.82f, 0.35f};
        col = col * (0.18f + uni(rng) * 0.22f);
        spokes.push_back({p, col});
        spokes.push_back({end, col * 0.15f});
    }
    spokeCount_ = static_cast<int>(spokes.size());
    spokeBuf_.setData(Containers::arrayView(spokes), GL::BufferUsage::StaticDraw);
    spokeMesh_.setCount(spokeCount_)
        .setPrimitive(GL::MeshPrimitive::Lines)
        .addVertexBuffer(spokeBuf_, 0, GL::Attribute<0, Vector3>{}, GL::Attribute<1, Vector3>{});

    nodeMesh_.setPrimitive(GL::MeshPrimitive::Points)
        .addVertexBuffer(nodeBuf_, 0, GL::Attribute<0, Vector3>{}, GL::Attribute<1, Vector3>{});
}

void VortexField::drawStars(const Matrix4& viewProj, const Vector3& camera) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
    starsShader_.setViewProj(viewProj).setCamera(camera).setPointScale(18.0f).setGain(0.55f);
    starsShader_.draw(starMesh_);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
}

void VortexField::drawFlow(const Matrix4& viewProj, const Matrix4& spin,
                           const Vector3& camera, float time) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDepthMask(GL_FALSE);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);
    spokeShader_.setViewProj(viewProj).setSpin(spin);
    spokeShader_.draw(spokeMesh_);
    trailShader_.setViewProj(viewProj).setSpin(spin).setTime(time);
    trailShader_.draw(trailMesh_);
    particleShader_.setViewProj(viewProj).setSpin(spin).setCamera(camera).setTime(time)
        .setPointScale(70.0f).setGain(2.4f);
    particleShader_.draw(particleMesh_);
    glDepthMask(GL_TRUE);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                   GL::Renderer::BlendFunction::OneMinusSourceAlpha);
}

void VortexField::drawNodes(const Matrix4& viewProj, const Vector3& camera,
                            const std::vector<om::RenderNode>& nodes, std::uint64_t selected) {
    if (nodes.empty()) return;
    struct PCol { Vector3 p; Vector3 c; };
    std::vector<PCol> pts;
    pts.reserve(nodes.size());
    for (const om::RenderNode& n : nodes) {
        float r, g, b;
        om::rgbToFloats(n.color, r, g, b);
        float gain = n.id == selected ? 2.8f : 1.6f;
        pts.push_back({{n.pos.x, n.pos.y, n.pos.z}, Vector3{r, g, b} * gain});
    }
    nodeBuf_.setData(Containers::arrayView(pts), GL::BufferUsage::DynamicDraw);
    nodeMesh_.setCount(static_cast<Int>(pts.size()));
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDepthMask(GL_FALSE);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);
    nodeShader_.setViewProj(viewProj).setCamera(camera).setPointScale(90.0f).setGain(1.0f);
    nodeShader_.draw(nodeMesh_);
    glDepthMask(GL_TRUE);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                   GL::Renderer::BlendFunction::OneMinusSourceAlpha);
}
