// OMStudio 3D 查看器：Magnum 渲染层
// 职责：实例化节点渲染 + 批量线段渲染 + 视锥剔除/LOD 收集 + 射线拾取 +
//      轨道相机 + 选中聚焦动画 + 拓扑切换 + "生长新对象"演示
#pragma once
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/Shaders/Flat.h>
#include "SurfaceMesh.h"
#include "TextCache.h"
#include <vector>

namespace om {
class Scene;
struct RenderNode;
struct RenderEdge;
} // namespace om

class ViewerApp : public Magnum::Platform::GlfwApplication {
public:
    explicit ViewerApp(const Arguments& args, const Configuration& conf,
                       const GLConfiguration& glConf, om::Scene& scene);

    void drawEvent() override;
    void viewportEvent(ViewportEvent& event) override;
    void mousePressEvent(MouseEvent& event) override;
    void mouseReleaseEvent(MouseEvent& event) override;
    void mouseMoveEvent(MouseMoveEvent& event) override;
    void mouseScrollEvent(MouseScrollEvent& event) override;
    void keyPressEvent(KeyEvent& event) override;

private:
    void updateCamera(int viewWidth, int viewHeight);
    void cameraBasis(Magnum::Vector3& cam, Magnum::Vector3& right, Magnum::Vector3& up) const;
    void applySurface(SurfaceKind kind);
    uint64_t pickId(const Magnum::Vector2i& pos, int viewWidth) const;
    bool handleHudClick(const Magnum::Vector2i& pos);
    void renderSurface();
    void renderNodes(const std::vector<om::RenderNode>& nodes);
    void renderEdges(const std::vector<om::RenderEdge>& edges);
    void renderLabels(const std::vector<om::RenderEdge>& edges, const Magnum::Vector3& right, const Magnum::Vector3& up);
    void renderHud();
    void drawSolid(const Magnum::Vector3* quad, const Magnum::Color4& color);
    void drawText(float x, float y, const std::string& utf8, float scale = 1.0f);

    om::Scene& scene_;

    SurfaceKind surface_ = SurfaceKind::Torus;
    bool positionsLocked_ = true;

    // 轨道相机
    float theta_ = 0.6f, phi_ = 0.35f, dist_ = 10.0f;
    Magnum::Vector3 target_{0.0f, 0.0f, 0.0f};
    bool focusAnim_ = false;

    // GPU 资源
    Magnum::GL::Mesh nodeMesh_, edgeMesh_, quadMesh_;
    Magnum::GL::Buffer edgeBuf_, quadBuf_;
    Magnum::Shaders::Flat3D flat_;
    Magnum::Shaders::Flat3D flatTex_{Magnum::Shaders::Flat3D::Flag::Textured};
    Magnum::Matrix4 viewProj_;
    SurfaceGpu torusSurf_, hyperSurf_, innerSurf_;
    TextCache text_;

    // 输入状态
    bool rotating_ = false, panning_ = false, draggingObject_ = false, moved_ = false, uiPress_ = false;
    Magnum::Vector2i lastMouse_{}, pressPos_{};
    uint64_t pressHit_ = 0;
};
