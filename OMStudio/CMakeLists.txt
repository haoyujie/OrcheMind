cmake_minimum_required(VERSION 3.16)
project(OMStudio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(OMSTUDIO_BUILD_VIEWER "Build Magnum 3D viewer demo" ON)
option(OMSTUDIO_BUILD_SHARED "Build C API shared library (for C# P/Invoke)" ON)
option(OMSTUDIO_BUILD_TESTS "Build core self-tests" ON)

# ---------------- 核心静态库（无第三方依赖，C++17） ----------------
add_library(omstudio_core STATIC
  src/Scene.cpp
  src/Octree.cpp
  src/TorusTopology.cpp
  src/HyperbolicTopology.cpp
)
target_include_directories(omstudio_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(MSVC)
  target_compile_options(omstudio_core PRIVATE /W4)
else()
  target_compile_options(omstudio_core PRIVATE -Wall -Wextra)
endif()

# ---------------- C API 动态库（omstudio.dll，C# P/Invoke 用） ----------------
if(OMSTUDIO_BUILD_SHARED)
  add_library(omstudio SHARED src/OmStudio.cpp)
  target_compile_definitions(omstudio PRIVATE OMSTUDIO_EXPORTS)
  target_link_libraries(omstudio PRIVATE omstudio_core)
  target_include_directories(omstudio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

# ---------------- 核心自测 ----------------
if(OMSTUDIO_BUILD_TESTS)
  enable_testing()
  add_executable(omstudio_core_test tests/core_test.cpp)
  target_link_libraries(omstudio_core_test PRIVATE omstudio_core)
  # 自测含 C API 冒烟。动态库存在时直接链 omstudio.dll；否则把 OmStudio.cpp 编进测试。
  if(OMSTUDIO_BUILD_SHARED)
    target_link_libraries(omstudio_core_test PRIVATE omstudio)
  else()
    target_sources(omstudio_core_test PRIVATE src/OmStudio.cpp)
    target_compile_definitions(omstudio_core_test PRIVATE OMSTUDIO_EXPORTS)
  endif()
  add_test(NAME core_test COMMAND omstudio_core_test)
endif()

# ---------------- Magnum 三维查看器 ----------------
if(OMSTUDIO_BUILD_VIEWER)
  # 让 find_package(GLFW) 命中 cmake/GLFWConfig.cmake（MSYS2 的 glfw3 别名）
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
  find_package(Magnum REQUIRED COMPONENTS GL MeshTools Primitives Shaders GlfwApplication)
  # Application 后端由 vcpkg 提供（glfwapplication 或 sdl2application）
  if(NOT TARGET Magnum::Application)
    message(FATAL_ERROR "Magnum::Application 未找到。请安装 magnum[gl,glfwapplication] 或 magnum[gl,sdl2application]")
  endif()

  add_executable(omstudio_viewer
    apps/viewer/main.cpp
    apps/viewer/ViewerApp.cpp
    apps/viewer/demo_data.cpp
    apps/viewer/SurfaceMesh.cpp
    apps/viewer/TextCache.cpp
  )
  target_link_libraries(omstudio_viewer PRIVATE
    omstudio_core
    Magnum::Application
    Magnum::GL
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::Shaders
  )
  if(WIN32)
    target_link_libraries(omstudio_viewer PRIVATE gdi32)
  endif()
  if(NOT MSVC)
    target_compile_options(omstudio_viewer PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
  endif()
endif()
