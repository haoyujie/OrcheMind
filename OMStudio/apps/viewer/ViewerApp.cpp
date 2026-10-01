#include "ViewerApp.h"
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Math/Functions.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Debug.h>
#include "omstudio/Scene.h"
#include "omstudio/Topology.h"
#include <cstring>
#include <random>

using namespace Magnum;
using namespace Magnum::Math::Literals;

namespace {

// Magnum Matrix4 → om::Mat4（两者均列主序）
om::Mat4 toOmMat4(const Matrix4& m) {
    om::Mat4 r{};
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row)
            r.v[c * 4 + row] = m[c][row];
    return r;
}

om::Vec3 toOmVec(const Vector3& v) { return {v.x(), v.y(), v.z()}; }
Vector3 toMagVec(const om::Vec3& v) { return {v.x, v.y, v.z}; }

std::unique_ptr<GL::Shader> makeProgram(const char* vsSrc, const char* fsSrc) {
    GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex};
    vert.addSource(vsSrc);
    vert.compile();
    GL::Shader frag{GL::Version::GL330, GL::Shader::Type::Fragment};
    frag.addSource(fsSrc);
    frag.compile();
    auto prog = std::make_unique<GL::Shader>(GL::Version::GL330);
    prog->attachShaders({vert, frag});
    prog->link();
    return prog;
}

const char* kNodeVs = R"(
#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec3 aIPos;
layout(location=3) in vec3 aIColor;
layout(location=4) in float aIRadius;
uniform mat4 uViewProj;
out vec3 vColor;
out vec3 vNormal;
void main() {
    vColor = aIColor;
    vNormal = aNormal;
    vec4 world = vec4(aIPos + aPosition * aIRadius, 1.0);
    gl_Position = uViewProj * world;
}
)";

const char* kNodeFs = R"(
#version 330 core
in vec3 vColor;
in vec3 vNormal;
out vec4 fragColor;
uniform vec3 uLightDir;
void main() {
    float d = max(dot(normalize(vNormal), normalize(uLightDir)), 0.0);
    float a = 0.32 + 0.68 * d;
    fragColor = vec4(vColor * a, 1.0);
}
)";

const char* kEdgeVs = R"(
#version 330 core
layout(location=0) in vec3 aPosition;
uniform mat4 uViewProj;
void main() {
    gl_Position = uViewProj * vec4(aPosition, 1.0);
}
)";

const char* kEdgeFs = R"(
#version 330 core
out vec4 fragColor;
uniform vec4 uColor;
void main() {
    fragColor = uColor;
}
)";

} // namespace

ViewerApp::ViewerApp(const Arguments& args, const Configuration& conf,
                     const GLConfiguration& glConf, om::Scene& scene)
    : Platform::Application{args, conf, glConf}, scene_(scene) {

    // ---- 节点：单位球 + 实例属性（顶点0/1，实例2/3/4）----
    // GLSL 中以 layout(location=N) 显式声明，无需 bindAttributeLocation
    nodeShader_ = makeProgram(kNodeVs, kNodeFs);
    nodeVpLoc_ = nodeShader_->uniformLocation("uViewProj");
    nodeLightLoc_ = nodeShader_->uniformLocation("uLightDir");

    const auto sphere = Primitives::uvSphereSolid(1.0f, 12, 16);
    nodeVerts_.setData(MeshTools::interleave(sphere.positions(0), sphere.normals(0)),
                       GL::BufferUsage::StaticDraw);
    nodeMesh_.setCount((GLint)sphere.positions(0).size())
        .addVertexBuffer(nodeVerts_, 0,
                         GL::Attribute<3, Float, 0>{},  // aPosition
                         GL::Attribute<3, Float, 1>{})  // aNormal
        .addVertexBufferInstanced(nodeInstBuf_, 1, (GLsizei)sizeof(InstanceData), 0,
                                  InstPosition{}, InstColor{}, InstRadius{});

    // ---- 边：线段 ----
    edgeShader_ = makeProgram(kEdgeVs, kEdgeFs);
    edgeVpLoc_ = edgeShader_->uniformLocation("uViewProj");
    edgeColorLoc_ = edgeShader_->uniformLocation("uColor");
    edgeMesh_.setPrimitive(GL::MeshPrimitive::Lines)
        .addVertexBuffer(edgeBuf_, 0, GL::Attribute<3, Float, 0>{});

    // ---- 全局 GL 状态 ----
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                   GL::Renderer::BlendFunction::OneMinusSourceAlpha);
    GL::Renderer::setClearColor(Color4{0.10f, 0.11f, 0.13f, 1.0f});
}

void ViewerApp::viewportEvent(ViewportEvent& event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});
    redraw();
}

void ViewerApp::updateCamera() {
    float cy = std::cos(phi_);
    Vector3 camPos = target_ + dist_ * Vector3{cy * std::sin(theta_), std::sin(phi_), cy * std::cos(theta_)};
    const Vector2i size = windowSize();
    float aspect = (float)size.x() / std::max(1, size.y());
    Matrix4 proj = Matrix4::perspectiveProjection(Rad(Deg(45.0f)), aspect, 0.05f, 200.0f);
    Matrix4 view = Matrix4::lookAt(camPos, target_, Vector3::yAxis());
    viewProj_ = proj * view;
}

void ViewerApp::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    updateCamera();

    // 聚焦动画：target 平滑移向选中对象
    if (focusAnim_) {
        if (scene_.selected()) {
            Vector3 goal = toMagVec(scene_.centerPos());
            target_ = Math::lerp(target_, goal, 0.08f);
            if ((target_ - goal).length() < 0.02f) { target_ = goal; focusAnim_ = false; }
        } else {
            target_ = Math::lerp(target_, Vector3{}, 0.08f);
            if (target_.length() < 0.02f) { target_ = Vector3{}; focusAnim_ = false; }
        }
    }

    // 相机位置（世界空间）
    float cy = std::cos(phi_);
    Vector3 camPos = target_ + dist_ * Vector3{cy * std::sin(theta_), std::sin(phi_), cy * std::cos(theta_)};

    // 视锥剔除 + LOD + 边强度过滤（核心层，Octree 加速）
    om::ViewFrustum frustum = om::Mat4::extractFrustum(toOmMat4(viewProj_));
    std::vector<om::RenderNode> nodes;
    std::vector<om::RenderEdge> edges;
    scene_.collectVisible(frustum, toOmVec(camPos), 18.0f, 0.30f, nodes, edges);

    renderEdges(edges);   // 先画边（半透明）
    renderNodes(nodes);   // 再画节点

    swapBuffers();
    if (focusAnim_ || rotating_ || panning_) redraw();
}

void ViewerApp::renderNodes(const std::vector<om::RenderNode>& nodes) {
    std::vector<InstanceData> inst;
    inst.reserve(nodes.size());
    for (const auto& n : nodes) {
        float r, g, b;
        om::rgbToFloats(n.color, r, g, b);
        inst.push_back({Vector3{n.pos.x, n.pos.y, n.pos.z}, Color3{r, g, b}, n.radius});
    }
    nodeInstBuf_.setData(Containers::ArrayView<const InstanceData>{inst.data(), inst.size()},
                         GL::BufferUsage::DynamicDraw);
    nodeShader_->use();
    nodeShader_->setUniform(nodeVpLoc_, viewProj_);
    nodeShader_->setUniform(nodeLightLoc_, Vector3{0.35f, 0.45f, 0.80f}.normalized());
    nodeMesh_.setInstanceCount((GLint)inst.size()).draw();
}

void ViewerApp::renderEdges(const std::vector<om::RenderEdge>& edges) {
    std::vector<Vector3> pts;
    pts.reserve(edges.size() * 2);
    for (const auto& e : edges) {
        pts.emplace_back(e.pa.x, e.pa.y, e.pa.z);
        pts.emplace_back(e.pb.x, e.pb.y, e.pb.z);
    }
    edgeBuf_.setData(Containers::ArrayView<const Vector3>{pts.data(), pts.size()},
                     GL::BufferUsage::DynamicDraw);
    edgeShader_->use();
    edgeShader_->setUniform(edgeVpLoc_, viewProj_);
    edgeShader_->setUniform(edgeColorLoc_, Color4{0.55f, 0.62f, 0.78f, 0.30f});
    edgeMesh_.setCount((GLint)pts.size()).draw();
}

void ViewerApp::pickAt(const Vector2i& pos) {
    const Vector2i size = windowSize();
    float ndcX = (float)pos.x() / std::max(1, size.x()) * 2.0f - 1.0f;
    float ndcY = 1.0f - (float)pos.y() / std::max(1, size.y()) * 2.0f;

    const Matrix4 inv = viewProj_.inverted();
    Vector4 n4 = inv * Vector4{ndcX, ndcY, -1.0f, 1.0f};
    Vector3 nearP = n4.xyz() / n4.w();
    Vector4 f4 = inv * Vector4{ndcX, ndcY, 1.0f, 1.0f};
    Vector3 farP = f4.xyz() / f4.w();
    Vector3 dir = (farP - nearP).normalized();

    om::Ray ray{{nearP.x(), nearP.y(), nearP.z()},
                {dir.x(), dir.y(), dir.z()}};
    uint64_t hit = scene_.pick(ray); // Octree 候选过滤 + 精确球相交
    if (hit) {
        scene_.select(hit); // 选中并重投影聚焦
        focusAnim_ = true;
        const om::Entity* e = scene_.find(hit);
        Debug{} << "[OMStudio] selected id=" << e->id << " label=" << e->label
                << " topology=" << scene_.topologyName();
        Debug{} << "  dims=" << e->highDim;
    } else {
        Debug{} << "[OMStudio] pick: miss";
    }
    redraw();
}

void ViewerApp::mousePressEvent(MouseEvent& event) {
    lastMouse_ = event.position();
    if (event.button() == MouseEvent::Button::Left) {
        rotating_ = true;
        pickAt(event.position());
    } else if (event.button() == MouseEvent::Button::Right) {
        panning_ = true;
    }
}

void ViewerApp::mouseReleaseEvent(MouseEvent&) {
    rotating_ = false;
    panning_ = false;
}

void ViewerApp::mouseMoveEvent(MouseMoveEvent& event) {
    Vector2i delta = event.position() - lastMouse_;
    lastMouse_ = event.position();
    const float scale = 0.005f;
    if (rotating_) {
        theta_ -= delta.x() * scale;
        phi_ = Math::clamp(phi_ + delta.y() * scale, -1.40f, 1.40f);
        redraw();
    } else if (panning_) {
        float cy = std::cos(phi_);
        Vector3 camPos = target_ + dist_ * Vector3{cy * std::sin(theta_), std::sin(phi_), cy * std::cos(theta_)};
        Vector3 fwd = (target_ - camPos).normalized();
        Vector3 right = Math::cross(fwd, Vector3::yAxis()).normalized();
        Vector3 up = Math::cross(right, fwd);
        target_ += (-right * (float)delta.x() + up * (float)delta.y()) * (dist_ * 0.0015f);
        redraw();
    }
}

void ViewerApp::mouseScrollEvent(MouseScrollEvent& event) {
    dist_ *= std::exp(-event.offset().y() * 0.15f);
    dist_ = Math::clamp(dist_, 0.4f, 60.0f);
    redraw();
}

void ViewerApp::keyPressEvent(KeyEvent& event) {
    using K = KeyEvent::Key;
    if (event.key() == K::Tab) {
        // 运行时切换构型：环面 ↔ 双曲
        if (std::strcmp(scene_.topologyName(), "clifford-torus") == 0)
            scene_.setTopology(om::makeHyperbolicTopology(scene_.topology().dim()));
        else
            scene_.setTopology(om::makeTorusTopology(scene_.topology().dim()));
        if (scene_.selected()) scene_.select(scene_.selected()); // 保持聚焦中心
        Debug{} << "[OMStudio] topology ->" << scene_.topologyName();
        redraw();
    } else if (event.key() == K::F) {
        if (scene_.selected()) focusAnim_ = true;
    } else if (event.key() == K::Escape) {
        scene_.clearSelection();
        focusAnim_ = true;
        Debug{} << "[OMStudio] selection cleared";
        redraw();
    } else if (event.key() == K::G) {
        // 演示"基于选中对象生长新对象"：C# 编辑器侧做同样的事
        uint64_t sel = scene_.selected();
        if (!sel) { Debug{} << "[OMStudio] G: nothing selected"; return; }
        const om::Entity* pe = scene_.find(sel);
        std::mt19937 rng(12345u + (unsigned)sel);
        std::normal_distribution<float> nd(0.0f, 1.0f);
        om::VecN v(6);
        for (int d = 0; d < 6; ++d) v[d] = pe->highDim[d] * 0.70f + nd(rng) * 0.30f;
        v[4] += 0.2f;
        om::normalizeInPlace(v);
        om::Entity child;
        child.highDim = v;
        child.radius = pe->radius * 0.72f;
        child.color = 0xFFD25A;
        child.parentId = pe->id;
        child.label = "morpheme_grown";
        uint64_t id = scene_.addEntity(child);
        scene_.setRelation(pe->id, id, 0.85f);
        scene_.select(id);
        focusAnim_ = true;
        Debug{} << "[OMStudio] grown new entity id=" << id << " under" << sel;
        redraw();
    } else if (event.key() == K::C) {
        uint64_t sel = scene_.selected();
        if (!sel) return;
        const om::Entity* e = scene_.find(sel);
        Debug{} << "[OMStudio] entity id=" << e->id << " label=" << e->label
                << " parent=" << e->parentId << " radius=" << e->radius
                << " pos3=" << e->pos3;
    }
}
