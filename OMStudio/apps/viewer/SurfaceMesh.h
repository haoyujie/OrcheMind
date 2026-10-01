// 流形曲面：环面 / 双曲面 / 内环面。给节点提供远近参照。
#pragma once
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Mesh.h>

enum class SurfaceKind { Torus, Hyperboloid, InnerTorus };

struct SurfaceGpu {
    Magnum::GL::Buffer vertices;
    Magnum::GL::Buffer indices;
    Magnum::GL::Mesh mesh;
};

void buildSurface(SurfaceGpu& gpu, SurfaceKind kind);
