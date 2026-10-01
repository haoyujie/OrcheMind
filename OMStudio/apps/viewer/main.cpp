// OMStudio 3D 查看器入口
// 用法：omstudio_viewer [entityCount]（默认 2000）
#include "ViewerApp.h"
#include "demo_data.h"
#include "omstudio/Scene.h"
#include "omstudio/Topology.h"
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    int count = 2000;
    if (argc > 1) count = std::atoi(argv[1]);
    if (count < 1) count = 1;

    // 默认从 Clifford 环面构型启动（Tab 切换双曲）
    auto scene = std::make_unique<om::Scene>(om::makeTorusTopology(6));
    om::demo::generate(*scene, count);

    std::cout << "[OMStudio] entities=" << scene->entityCount()
              << " topology=" << scene->topologyName()
              << "  (T:展示/数据 Tab:切换曲面 [:暗 ]:亮 -:慢 =:快 空格:停 左键:选择 右键:平移 滚轮:缩放 F:聚焦 Esc:取消 G:生长)\n";

    Magnum::Platform::GlfwApplication::Configuration conf;
    conf.setTitle("OMStudio 3D Viewer").setSize(Magnum::Vector2i{1280, 800});
    Magnum::Platform::GlfwApplication::GLConfiguration glConf;
    ViewerApp app{Magnum::Platform::GlfwApplication::Arguments{argc, argv}, conf, glConf, *scene};
    return app.exec();
}
