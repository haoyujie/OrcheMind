# MSYS2 安装的是 glfw3（目标名 glfw），Magnum 2020.06 要求 GLFW::GLFW。
include(CMakeFindDependencyMacro)
find_dependency(glfw3 CONFIG)
if(NOT TARGET GLFW::GLFW)
  add_library(GLFW::GLFW INTERFACE IMPORTED)
  set_property(TARGET GLFW::GLFW PROPERTY INTERFACE_LINK_LIBRARIES glfw)
endif()
