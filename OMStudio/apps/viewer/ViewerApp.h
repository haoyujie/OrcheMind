// OMStudio 3D 查看器：Magnum 渲染层
// 职责：实例化节点渲染 + 批量线段渲染 + 视锥剔除/LOD 收集 + 射线拾取 +
//      轨道相机 + 选中聚焦动画 + 拓扑切换 + "生长新对象"演示
#pragma once
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Platform/Application.h>
#include <memory>
#include <vector>

namespace om {
class Scene;
struct RenderNode;
struct RenderEdge;
} // namespace om

class ViewerApp : public Magnum::Platform::Application {
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
    struct InstanceData {
        Magnum::Vector3 pos;
        Magnum::Color3 color;
        float radius;
    };

    // 实例属性（location 2/3/4，与球顶点 0/1 区分）
    using InstPosition = Magnum::GL::Attribute<3, Magnum::Float, 2>;
    using InstColor = Magnum::GL::Attribute<3, Magnum::Float, 3>;
    using InstRadius = Magnum::GL::Attribute<1, Magnum::Float, 4>;

    void updateCamera();
    void pickAt(const Magnum::Vector2i& pos);
    void renderNodes(const std::vector<om::RenderNode>& nodes);
    void renderEdges(const std::vector<om::RenderEdge>& edges);

    om::Scene& scene_;

    // 轨道相机
    float theta_ = 0.6f, phi_ = 0.35f, dist_ = 6.0f;
    Magnum::Vector3 target_{0.0f, 0.0f, 0.0f};
    bool focusAnim_ = false;

    // GPU 资源
    Magnum::GL::Mesh nodeMesh_, edgeMesh_;
    Magnum::GL::Buffer nodeVerts_, nodeInstBuf_, edgeBuf_;
    std::unique_ptr<Magnum::GL::Shader> nodeShader_, edgeShader_;
    Magnum::GL::Int nodeVpLoc_, nodeLightLoc_, edgeVpLoc_, edgeColorLoc_;
    Magnum::Matrix4 viewProj_;

    // 输入状态
    bool rotating_ = false, panning_ = false;
    Magnum::Vector2i lastMouse_;
};
