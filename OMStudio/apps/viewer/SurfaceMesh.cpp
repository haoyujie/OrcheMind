#include "SurfaceMesh.h"
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Shaders/Generic.h>
#include <cmath>
#include <vector>

using namespace Magnum;

namespace {

void upload(SurfaceGpu& gpu, const std::vector<Vector3>& pos, const std::vector<UnsignedInt>& idx) {
    gpu.vertices.setData(pos, GL::BufferUsage::StaticDraw);
    gpu.indices.setData(idx, GL::BufferUsage::StaticDraw);
    gpu.mesh = GL::Mesh{};
    gpu.mesh.setCount(static_cast<Int>(idx.size()))
        .setPrimitive(GL::MeshPrimitive::Triangles)
        .addVertexBuffer(gpu.vertices, 0, Shaders::Generic3D::Position{})
        .setIndexBuffer(gpu.indices, 0, GL::MeshIndexType::UnsignedInt);
}

void addQuad(std::vector<UnsignedInt>& idx, int a, int b, int c, int d) {
    idx.push_back(a); idx.push_back(b); idx.push_back(c);
    idx.push_back(a); idx.push_back(c); idx.push_back(d);
}

void buildTorus(std::vector<Vector3>& pos, std::vector<UnsignedInt>& idx,
                float major, float tube, int rings, int segs) {
    const float pi = 3.14159265f;
    for (int i = 0; i <= rings; ++i) {
        float th = 2.0f * pi * i / rings;
        for (int j = 0; j <= segs; ++j) {
            float ph = 2.0f * pi * j / segs;
            float cr = major + tube * std::cos(ph);
            pos.push_back({cr * std::cos(th), tube * std::sin(ph), cr * std::sin(th)});
        }
    }
    int stride = segs + 1;
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segs; ++j) {
            int a = i * stride + j;
            addQuad(idx, a, a + 1, a + stride + 1, a + stride);
        }
    }
}

void buildHyperboloid(std::vector<Vector3>& pos, std::vector<UnsignedInt>& idx) {
    const float pi = 3.14159265f;
    const int rings = 40, segs = 28;
    const float a = 1.35f, c = 1.6f;
    for (int i = 0; i <= rings; ++i) {
        float u = 2.0f * pi * i / rings;
        for (int j = 0; j <= segs; ++j) {
            float v = -1.15f + 2.30f * j / segs;
            float ch = std::cosh(v);
            pos.push_back({a * ch * std::cos(u), c * std::sinh(v), a * ch * std::sin(u)});
        }
    }
    int stride = segs + 1;
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segs; ++j) {
            int q = i * stride + j;
            addQuad(idx, q, q + 1, q + stride + 1, q + stride);
        }
    }
}

} // namespace

void buildSurface(SurfaceGpu& gpu, SurfaceKind kind) {
    std::vector<Vector3> pos;
    std::vector<UnsignedInt> idx;
    if (kind == SurfaceKind::Hyperboloid) {
        buildHyperboloid(pos, idx);
    } else if (kind == SurfaceKind::InnerTorus) {
        buildTorus(pos, idx, 1.15f, 0.48f, 40, 20);
    } else {
        buildTorus(pos, idx, 2.2f, 1.0f, 48, 24);
    }
    upload(gpu, pos, idx);
}
