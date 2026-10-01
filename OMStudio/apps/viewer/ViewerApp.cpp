#include "ViewerApp.h"
#include <Corrade/Utility/Debug.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/OpenGL.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Functions.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Trade/MeshData.h>
#include "omstudio/Scene.h"
#include "omstudio/Topology.h"
#include <Magnum/GL/TextureFormat.h>
#include <Corrade/Containers/ArrayView.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <random>
#include <sstream>
#include <string>

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

} // namespace

namespace {

constexpr int kPanelW = 300;

struct HudButton {
    int x, y, w, h;
    int action; // 1 环面 2 双曲面 3 内环面 4 锁定
};

std::vector<HudButton> hudButtons(const Vector2i& ws) {
    int x0 = ws.x() - kPanelW + 16;
    int bw = 84, bh = 28, gap = 8;
    return {
        {x0, 44, bw, bh, 1},
        {x0 + bw + gap, 44, bw, bh, 2},
        {x0, 80, bw, bh, 3},
        {x0 + bw + gap, 80, bw, bh, 5},
        {x0, 124, 176, bh, 4},
    };
}

Vector3 px(float x, float y, const Vector2i& ws) {
    return {(x / std::max(1, ws.x())) * 2.0f - 1.0f,
            1.0f - (y / std::max(1, ws.y())) * 2.0f, 0.0f};
}

Vector2i toFb(const Vector2i& p, const Vector2i& win, const Vector2i& fb) {
    return {p.x() * fb.x() / std::max(1, win.x()), p.y() * fb.y() / std::max(1, win.y())};
}

struct Vert {
    Vector3 p;
    Vector2 uv;
};

const char* surfaceName(SurfaceKind k) {
    if (k == SurfaceKind::Hyperboloid) return "双曲面";
    if (k == SurfaceKind::InnerTorus) return "内环面";
    if (k == SurfaceKind::TwistedTorus) return "涡旋环";
    return "环面";
}

} // namespace

ViewerApp::ViewerApp(const Arguments& args, const Configuration& conf,
                     const GLConfiguration& glConf, om::Scene& scene)
    : Platform::GlfwApplication{args, conf, glConf}, scene_(scene) {

    nodeMesh_ = MeshTools::compile(Primitives::uvSphereSolid(8, 12));
    edgeMesh_.setPrimitive(GL::MeshPrimitive::Lines)
        .addVertexBuffer(edgeBuf_, 0, Shaders::Flat3D::Position{});
    quadMesh_.setPrimitive(GL::MeshPrimitive::Triangles)
        .addVertexBuffer(quadBuf_, 0, Shaders::Flat3D::Position{}, Shaders::Flat3D::TextureCoordinates{});

    buildSurface(torusSurf_, SurfaceKind::Torus);
    buildSurface(hyperSurf_, SurfaceKind::Hyperboloid);
    buildSurface(innerSurf_, SurfaceKind::InnerTorus);
    buildSurface(twistSurf_, SurfaceKind::TwistedTorus);

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

void ViewerApp::updateCamera(int viewWidth, int viewHeight) {
    Vector3 camPos, right, up;
    cameraBasis(camPos, right, up);
    float aspect = (float)viewWidth / std::max(1, viewHeight);
    Matrix4 proj = Matrix4::perspectiveProjection(Rad(Deg(45.0f)), aspect, 0.05f, 200.0f);
    // Magnum::lookAt 返回的是相机在世界中的姿态，视图矩阵是它的逆。
    Matrix4 view = Matrix4::lookAt(camPos, target_, Vector3::yAxis()).invertedRigid();
    viewProj_ = proj * view;
}

void ViewerApp::cameraBasis(Vector3& cam, Vector3& right, Vector3& up) const {
    float cy = std::cos(phi_);
    cam = target_ + dist_ * Vector3{cy * std::sin(theta_), std::sin(phi_), cy * std::cos(theta_)};
    Vector3 fwd = (target_ - cam).normalized();
    right = Math::cross(fwd, Vector3::yAxis());
    if (right.dot() < 1e-8f) right = Vector3::xAxis();
    else right = right.normalized();
    up = Math::cross(right, fwd).normalized();
}

void ViewerApp::applySurface(SurfaceKind kind) {
    surface_ = kind;
    // 涡旋环只是展示网格，不改核心投影，节点留在当前拓扑上。
    if (kind != SurfaceKind::TwistedTorus) {
        const int dim = scene_.topology().dim();
        const uint64_t sel = scene_.selected();
        if (kind == SurfaceKind::Hyperboloid)
            scene_.setTopology(om::makeHyperbolicTopology(dim));
        else if (kind == SurfaceKind::InnerTorus)
            scene_.setTopology(om::makeTorusTopology(dim, 1.15f, 0.48f, 0.22f, 0.16f));
        else
            scene_.setTopology(om::makeTorusTopology(dim, 2.2f, 1.0f, 0.35f, 0.25f));
        if (sel) scene_.highlight(sel);
    }
    focusAnim_ = scene_.selected() != 0;
    Debug{} << "[OMStudio] surface ->" << surfaceName(kind);
}

void ViewerApp::drawEvent() {
    const Vector2i fb = framebufferSize();
    const int viewW = std::max(1, fb.x() - kPanelW);
    const auto now = std::chrono::steady_clock::now();
    if (clock0_ == std::chrono::steady_clock::time_point{}) clock0_ = now;
    showTime_ = std::chrono::duration<float>(now - clock0_).count();
    // 约 12 圈/分钟。
    spin_ = Matrix4::rotationY(Rad(showTime_ * 1.256637f));

    if (focusAnim_) {
        if (scene_.selected()) {
            Vector3 goal = toMagVec(scene_.centerPos());
            target_ = Math::lerp(target_, goal, 0.12f);
            if ((target_ - goal).length() < 0.02f) { target_ = goal; focusAnim_ = false; }
        } else {
            target_ = Math::lerp(target_, Vector3{}, 0.12f);
            if (target_.length() < 0.02f) { target_ = Vector3{}; focusAnim_ = false; }
        }
    }

    updateCamera(viewW, fb.y());
    Vector3 camPos, right, up;
    cameraBasis(camPos, right, up);

    om::ViewFrustum frustum = om::Mat4::extractFrustum(toOmMat4(viewProj_));
    std::vector<om::RenderNode> nodes;
    std::vector<om::RenderEdge> edges;
    scene_.collectVisible(frustum, toOmVec(camPos), 22.0f, 0.30f, nodes, edges);

    if (showcase_) {
        bloom_.resize({viewW, fb.y()});
        bloom_.bindScene();
        GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
        vortex_.drawStars(viewProj_, camPos);
        renderSurface();
        if (surface_ == SurfaceKind::TwistedTorus)
            vortex_.drawFlow(viewProj_, spin_, camPos, showTime_);
        vortex_.drawNodes(viewProj_, camPos, nodes, scene_.selected());
        glViewport(0, 0, fb.x(), fb.y());
        GL::defaultFramebuffer.bind();
        glClearColor(0.020f, 0.027f, 0.043f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, viewW, fb.y());
        bloom_.composite(0.72f);
    } else {
        // 部分 Intel 驱动上 Magnum 的 DSA clear 清不掉深度。
        glViewport(0, 0, fb.x(), fb.y());
        glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, viewW, fb.y());
        renderSurface();
        renderEdges(edges);
        renderNodes(nodes);
        renderLabels(edges, right, up);
    }

    glViewport(0, 0, fb.x(), fb.y());
    renderHud();

    swapBuffers();
    if (showcase_ || focusAnim_ || rotating_ || panning_ || draggingObject_) redraw();
}

void ViewerApp::drawSolid(const Vector3* quad, const Color4& color) {
    const Vector2 uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    Vert v[6] = {
        {quad[0], uv[0]}, {quad[1], uv[1]}, {quad[2], uv[2]},
        {quad[0], uv[0]}, {quad[2], uv[2]}, {quad[3], uv[3]},
    };
    quadBuf_.setData(Containers::arrayView(v), GL::BufferUsage::DynamicDraw);
    quadMesh_.setCount(6);
    flat_.setTransformationProjectionMatrix(Matrix4{})
        .setColor(color);
    flat_.draw(quadMesh_);
}

void ViewerApp::drawText(float x, float y, const std::string& utf8, float scale) {
    if (utf8.empty()) return;
    TextCache::Image img = text_.get(utf8);
    if (!img.texture || img.pixels.x() <= 0) return;
    const Vector2i fb = framebufferSize();
    const float w = img.pixels.x() * scale;
    const float h = img.pixels.y() * scale;
    // 纹理第 0 行是字形顶部，对应 v=0。
    Vector3 q[4] = {px(x, y, fb), px(x + w, y, fb), px(x + w, y + h, fb), px(x, y + h, fb)};
    Vector2 uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    Vert v[6] = {
        {q[0], uv[0]}, {q[1], uv[1]}, {q[2], uv[2]},
        {q[0], uv[0]}, {q[2], uv[2]}, {q[3], uv[3]},
    };
    quadBuf_.setData(Containers::arrayView(v), GL::BufferUsage::DynamicDraw);
    quadMesh_.setCount(6);
    flatTex_.setTransformationProjectionMatrix(Matrix4{})
        .bindTexture(*img.texture)
        .setColor(Color4{1.0f});
    flatTex_.draw(quadMesh_);
}

void ViewerApp::renderSurface() {
    SurfaceGpu* gpu = &torusSurf_;
    Color4 color{0.35f, 0.55f, 0.75f, 0.38f};
    if (surface_ == SurfaceKind::Hyperboloid) {
        gpu = &hyperSurf_;
        color = Color4{0.55f, 0.38f, 0.68f, 0.36f};
    } else if (surface_ == SurfaceKind::InnerTorus) {
        gpu = &innerSurf_;
        color = Color4{0.28f, 0.62f, 0.48f, 0.42f};
    } else if (surface_ == SurfaceKind::TwistedTorus) {
        gpu = &twistSurf_;
        color = Color4{0.16f, 0.55f, 0.85f, 0.45f};
    }
    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 2.0f);
    if (showcase_) {
        Vector3 cam, right, up;
        cameraBasis(cam, right, up);
        const Matrix4 model = (surface_ == SurfaceKind::TwistedTorus) ? spin_ : Matrix4{};
        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                       GL::Renderer::BlendFunction::OneMinusSourceAlpha);
        glow_.setViewProj(viewProj_).setModel(model).setCamera(cam).setTime(showTime_);
        glow_.draw(gpu->mesh);
    } else {
        flat_.setTransformationProjectionMatrix(viewProj_).setColor(color);
        flat_.draw(gpu->mesh);
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void ViewerApp::renderNodes(const std::vector<om::RenderNode>& nodes) {
    const uint64_t sel = scene_.selected();
    for (const auto& n : nodes) {
        float r, g, b;
        om::rgbToFloats(n.color, r, g, b);
        float scale = n.radius;
        if (n.id == sel) {
            scale *= 1.45f;
            r = std::min(1.0f, r + 0.35f);
            g = std::min(1.0f, g + 0.35f);
            b = std::min(1.0f, b + 0.25f);
        }
        const Matrix4 model = Matrix4::translation(Vector3{n.pos.x, n.pos.y, n.pos.z}) *
                              Matrix4::scaling(Vector3{scale});
        flat_.setTransformationProjectionMatrix(viewProj_ * model)
            .setColor(Color4{r, g, b, 1.0f});
        flat_.draw(nodeMesh_);
    }
}

void ViewerApp::renderEdges(const std::vector<om::RenderEdge>& edges) {
    if (edges.empty()) return;
    std::vector<Vector3> pts;
    pts.reserve(edges.size() * 2);
    for (const auto& e : edges) {
        pts.emplace_back(e.pa.x, e.pa.y, e.pa.z);
        pts.emplace_back(e.pb.x, e.pb.y, e.pb.z);
    }
    edgeBuf_.setData(Containers::ArrayView<const Vector3>{pts.data(), pts.size()},
                     GL::BufferUsage::DynamicDraw);
    edgeMesh_.setCount(static_cast<Int>(pts.size()));
    flat_.setTransformationProjectionMatrix(viewProj_)
        .setColor(Color4{0.72f, 0.80f, 0.92f, 0.85f});
    flat_.draw(edgeMesh_);
}

void ViewerApp::renderLabels(const std::vector<om::RenderEdge>& edges, const Vector3& right, const Vector3& up) {
    if (edges.empty()) return;
    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    glDepthMask(GL_FALSE);
    const float s = 0.00105f * std::max(dist_, 3.0f);
    for (const auto& e : edges) {
        char weight[32];
        std::snprintf(weight, sizeof(weight), "%.2f", e.strength);
        std::string text = e.verb.empty() ? "关联" : e.verb;
        text += " ";
        text += weight;
        TextCache::Image img = text_.get(text);
        if (!img.texture) continue;
        const float w = img.pixels.x() * s;
        const float h = img.pixels.y() * s;
        Vector3 mid{(e.pa.x + e.pb.x) * 0.5f, (e.pa.y + e.pb.y) * 0.5f, (e.pa.z + e.pb.z) * 0.5f};
        mid += up * (h * 0.65f);
        Vector3 q[4] = {
            mid - right * (w * 0.5f) + up * (h * 0.5f),
            mid + right * (w * 0.5f) + up * (h * 0.5f),
            mid + right * (w * 0.5f) - up * (h * 0.5f),
            mid - right * (w * 0.5f) - up * (h * 0.5f),
        };
        Vector3 plate[4] = {
            mid - right * (w * 0.55f) + up * (h * 0.62f),
            mid + right * (w * 0.55f) + up * (h * 0.62f),
            mid + right * (w * 0.55f) - up * (h * 0.55f),
            mid - right * (w * 0.55f) - up * (h * 0.55f),
        };
        Vector2 uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        Vert back[6] = {
            {plate[0], uv[0]}, {plate[1], uv[1]}, {plate[2], uv[2]},
            {plate[0], uv[0]}, {plate[2], uv[2]}, {plate[3], uv[3]},
        };
        quadBuf_.setData(Containers::arrayView(back), GL::BufferUsage::DynamicDraw);
        quadMesh_.setCount(6);
        flat_.setTransformationProjectionMatrix(viewProj_)
            .setColor(Color4{0.04f, 0.05f, 0.07f, 0.72f});
        flat_.draw(quadMesh_);
        Vert v[6] = {
            {q[0], uv[0]}, {q[1], uv[1]}, {q[2], uv[2]},
            {q[0], uv[0]}, {q[2], uv[2]}, {q[3], uv[3]},
        };
        quadBuf_.setData(Containers::arrayView(v), GL::BufferUsage::DynamicDraw);
        quadMesh_.setCount(6);
        flatTex_.setTransformationProjectionMatrix(viewProj_)
            .bindTexture(*img.texture)
            .setColor(Color4{1.0f, 1.0f, 1.0f, 0.95f});
        flatTex_.draw(quadMesh_);
    }
    glDepthMask(GL_TRUE);
}

void ViewerApp::renderHud() {
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
    glDepthMask(GL_FALSE);

    const Vector2i fb = framebufferSize();
    const float x0 = static_cast<float>(fb.x() - kPanelW);
    Vector3 panel[4] = {
        px(x0, 0, fb), px((float)fb.x(), 0, fb),
        px((float)fb.x(), (float)fb.y(), fb), px(x0, (float)fb.y(), fb)};
    drawSolid(panel, Color4{0.07f, 0.08f, 0.10f, 0.94f});

    drawText(x0 + 16, 12, "曲面", 1.0f);
    for (const HudButton& b : hudButtons(fb)) {
        bool on = false;
        std::string caption;
        Color4 bg{0.18f, 0.20f, 0.24f, 1.0f};
        if (b.action == 1) { caption = "环面"; on = surface_ == SurfaceKind::Torus; }
        else if (b.action == 2) { caption = "双曲面"; on = surface_ == SurfaceKind::Hyperboloid; }
        else if (b.action == 3) { caption = "内环面"; on = surface_ == SurfaceKind::InnerTorus; }
        else if (b.action == 5) { caption = "涡旋环"; on = surface_ == SurfaceKind::TwistedTorus; }
        else {
            caption = positionsLocked_ ? "位置已锁定" : "位置可拖动";
            on = positionsLocked_;
            if (on) bg = Color4{0.45f, 0.32f, 0.12f, 1.0f};
        }
        if (on && b.action != 4) bg = Color4{0.22f, 0.40f, 0.58f, 1.0f};
        Vector3 q[4] = {
            px((float)b.x, (float)b.y, fb),
            px((float)(b.x + b.w), (float)b.y, fb),
            px((float)(b.x + b.w), (float)(b.y + b.h), fb),
            px((float)b.x, (float)(b.y + b.h), fb)};
        drawSolid(q, bg);
        drawText((float)b.x + 10, (float)b.y + 4, caption, 0.85f);
    }
    drawText(x0 + 16, 160, positionsLocked_ ? "解锁后可拖动选中对象" : "拖动选中对象以改位置", 0.8f);
    drawText(x0 + 16, 184, showcase_ ? "展示模式   T 切数据" : "数据模式   T 切展示", 0.8f);

    float y = 216;
    const om::Entity* e = scene_.find(scene_.selected());
    if (!e) {
        drawText(x0 + 16, y, "未选择对象", 1.0f);
        drawText(x0 + 16, y + 28, "左键点选球体", 0.85f);
    } else {
        drawText(x0 + 16, y, "对象", 1.0f);
        y += 28;
        drawText(x0 + 16, y, "名称  " + e->label, 0.85f);
        y += 24;
        drawText(x0 + 16, y, "编号  " + std::to_string(e->id), 0.85f);
        y += 24;
        std::string parent = "无";
        if (const om::Entity* p = scene_.find(e->parentId)) parent = p->label;
        drawText(x0 + 16, y, "父对象  " + parent, 0.85f);
        y += 24;
        char posbuf[96];
        std::snprintf(posbuf, sizeof(posbuf), "位置  %.2f  %.2f  %.2f", e->pos3.x, e->pos3.y, e->pos3.z);
        drawText(x0 + 16, y, posbuf, 0.85f);
        y += 24;
        drawText(x0 + 16, y, e->pinned ? "钉住  是" : "钉住  否", 0.85f);
        y += 24;
        std::ostringstream dim;
        dim.setf(std::ios::fixed);
        dim.precision(2);
        dim << "高维";
        for (float c : e->highDim) dim << "  " << c;
        drawText(x0 + 16, y, dim.str(), 0.75f);
        y += 36;
        drawText(x0 + 16, y, "关联", 1.0f);
        y += 28;
        int shown = 0;
        for (const om::Relation& rel : scene_.relations()) {
            if (rel.a != e->id && rel.b != e->id) continue;
            const uint64_t otherId = rel.a == e->id ? rel.b : rel.a;
            const om::Entity* other = scene_.find(otherId);
            char line[160];
            std::snprintf(line, sizeof(line), "%s  →  %s  %.2f",
                          rel.verb.empty() ? "关联" : rel.verb.c_str(),
                          other ? other->label.c_str() : "?",
                          rel.strength);
            drawText(x0 + 16, y, line, 0.75f);
            y += 22;
            if (++shown >= 12 || y > fb.y() - 28) break;
        }
        if (shown == 0) drawText(x0 + 16, y, "无", 0.85f);
    }

    glDepthMask(GL_TRUE);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
}

uint64_t ViewerApp::pickId(const Vector2i& pos, int viewWidth) const {
    if (pos.x() >= viewWidth) return 0;
    float ndcX = (float)pos.x() / std::max(1, viewWidth) * 2.0f - 1.0f;
    float ndcY = 1.0f - (float)pos.y() / std::max(1, framebufferSize().y()) * 2.0f;
    const Matrix4 inv = viewProj_.inverted();
    Vector4 n4 = inv * Vector4{ndcX, ndcY, -1.0f, 1.0f};
    Vector3 nearP = n4.xyz() / n4.w();
    Vector4 f4 = inv * Vector4{ndcX, ndcY, 1.0f, 1.0f};
    Vector3 farP = f4.xyz() / f4.w();
    Vector3 dir = (farP - nearP).normalized();
    om::Ray ray{{nearP.x(), nearP.y(), nearP.z()}, {dir.x(), dir.y(), dir.z()}};
    uint64_t hit = scene_.pick(ray);
    if (hit) return hit;
    // 球体在屏幕上很小，射线没打中时用屏幕距离再捞一次。
    const float limit = 22.0f * 22.0f;
    float best = limit;
    const float viewH = static_cast<float>(std::max(1, framebufferSize().y()));
    for (const om::Entity& e : scene_.entities()) {
        Vector4 clip = viewProj_ * Vector4{e.pos3.x, e.pos3.y, e.pos3.z, 1.0f};
        if (clip.w() <= 0.05f) continue;
        Vector3 ndc = clip.xyz() / clip.w();
        if (ndc.z() < -1.0f || ndc.z() > 1.0f) continue;
        float sx = (ndc.x() * 0.5f + 0.5f) * static_cast<float>(viewWidth);
        float sy = (1.0f - (ndc.y() * 0.5f + 0.5f)) * viewH;
        float dx = sx - static_cast<float>(pos.x());
        float dy = sy - static_cast<float>(pos.y());
        float d2 = dx * dx + dy * dy;
        if (d2 < best) { best = d2; hit = e.id; }
    }
    return hit;
}

bool ViewerApp::handleHudClick(const Vector2i& pos) {
    const Vector2i fb = framebufferSize();
    if (pos.x() < fb.x() - kPanelW) return false;
    for (const HudButton& b : hudButtons(fb)) {
        if (pos.x() < b.x || pos.y() < b.y || pos.x() >= b.x + b.w || pos.y() >= b.y + b.h) continue;
        if (b.action == 1) applySurface(SurfaceKind::Torus);
        else if (b.action == 2) applySurface(SurfaceKind::Hyperboloid);
        else if (b.action == 3) applySurface(SurfaceKind::InnerTorus);
        else if (b.action == 5) applySurface(SurfaceKind::TwistedTorus);
        else positionsLocked_ = !positionsLocked_;
        redraw();
        return true;
    }
    return true; // 点在面板空白处，不让它转相机
}

void ViewerApp::mousePressEvent(MouseEvent& event) {
    const Vector2i fb = framebufferSize();
    const Vector2i pos = toFb(event.position(), windowSize(), fb);
    lastMouse_ = event.position();
    pressPos_ = event.position();
    moved_ = false;
    uiPress_ = false;
    if (event.button() == MouseEvent::Button::Left) {
        if (handleHudClick(pos)) { uiPress_ = true; return; }
        const int viewW = std::max(1, fb.x() - kPanelW);
        pressHit_ = pickId(pos, viewW);
        draggingObject_ = !positionsLocked_ && pressHit_ != 0;
        rotating_ = positionsLocked_ || pressHit_ == 0;
        if (pressHit_) {
            scene_.highlight(pressHit_);
            focusAnim_ = true;
            const om::Entity* e = scene_.find(pressHit_);
            Debug{} << "[OMStudio] selected id=" << static_cast<unsigned long long>(e->id)
                    << " label=" << e->label.c_str();
        }
        redraw();
    } else if (event.button() == MouseEvent::Button::Right) {
        panning_ = true;
    }
}

void ViewerApp::mouseReleaseEvent(MouseEvent& event) {
    if (event.button() == MouseEvent::Button::Left && !uiPress_ && !moved_ && pressHit_ == 0)
        scene_.highlight(0);
    rotating_ = false;
    panning_ = false;
    draggingObject_ = false;
    uiPress_ = false;
    redraw();
}

void ViewerApp::mouseMoveEvent(MouseMoveEvent& event) {
    Vector2i fromPress = event.position() - pressPos_;
    if (fromPress.x() * fromPress.x() + fromPress.y() * fromPress.y() > 16) moved_ = true;
    Vector2i delta = event.position() - lastMouse_;
    lastMouse_ = event.position();
    if (!moved_) return;

    const float scale = 0.005f;
    if (draggingObject_ && pressHit_ && !positionsLocked_) {
        focusAnim_ = false;
        Vector3 cam, right, up;
        cameraBasis(cam, right, up);
        const float s = dist_ * 0.0022f;
        const om::Entity* ent = scene_.find(pressHit_);
        if (!ent) return;
        om::Vec3 p = ent->pos3;
        const float dx = delta.x() * s;
        const float dy = -delta.y() * s;
        p.x += right.x() * dx + up.x() * dy;
        p.y += right.y() * dx + up.y() * dy;
        p.z += right.z() * dx + up.z() * dy;
        scene_.pinPosition(pressHit_, p);
        redraw();
    } else if (rotating_) {
        theta_ -= delta.x() * scale;
        phi_ = Math::clamp(phi_ + delta.y() * scale, -1.40f, 1.40f);
        redraw();
    } else if (panning_) {
        Vector3 cam, right, up;
        cameraBasis(cam, right, up);
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
        if (surface_ == SurfaceKind::Torus) applySurface(SurfaceKind::Hyperboloid);
        else if (surface_ == SurfaceKind::Hyperboloid) applySurface(SurfaceKind::InnerTorus);
        else if (surface_ == SurfaceKind::InnerTorus) applySurface(SurfaceKind::TwistedTorus);
        else applySurface(SurfaceKind::Torus);
        redraw();
    } else if (event.key() == K::T) {
        showcase_ = !showcase_;
        Debug{} << "[OMStudio]" << (showcase_ ? "showcase" : "data");
        redraw();
    } else if (event.key() == K::F) {
        if (scene_.selected()) focusAnim_ = true;
    } else if (event.key() == K::Esc) {
        scene_.highlight(0);
        focusAnim_ = true;
        Debug{} << "[OMStudio] selection cleared";
        redraw();
    } else if (event.key() == K::G) {
        uint64_t sel = scene_.selected();
        if (!sel) { Debug{} << "[OMStudio] G: nothing selected"; return; }
        const om::Entity* pe = scene_.find(sel);
        const om::VecN parentDim = pe->highDim;
        const uint64_t parentId = pe->id;
        const float parentRadius = pe->radius;
        std::mt19937 rng(12345u + (unsigned)sel);
        std::normal_distribution<float> nd(0.0f, 1.0f);
        om::VecN v(6);
        for (int d = 0; d < 6; ++d) v[d] = parentDim[d] * 0.70f + nd(rng) * 0.30f;
        v[4] += 0.2f;
        om::normalizeInPlace(v);
        om::Entity child;
        child.highDim = v;
        child.radius = parentRadius * 0.72f;
        child.color = 0xFFD25A;
        child.parentId = parentId;
        child.label = "morpheme_grown";
        uint64_t id = scene_.addEntity(child);
        scene_.setRelation(parentId, id, 0.85f, "生长");
        scene_.highlight(id);
        focusAnim_ = true;
        Debug{} << "[OMStudio] grown new entity id=" << static_cast<unsigned long long>(id)
                << " under" << static_cast<unsigned long long>(sel);
        redraw();
    } else if (event.key() == K::C) {
        uint64_t sel = scene_.selected();
        if (!sel) return;
        const om::Entity* e = scene_.find(sel);
        Debug{} << "[OMStudio] entity id=" << static_cast<unsigned long long>(e->id)
                << " label=" << e->label.c_str()
                << " parent=" << static_cast<unsigned long long>(e->parentId)
                << " radius=" << e->radius;
    }
}
