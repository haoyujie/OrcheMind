#include "SurfaceMesh.h"
#include "VortexGeom.h"
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Vector3.h>
#include <cmath>
#include <vector>

using namespace Magnum;

namespace {

void upload(SurfaceGpu& gpu, const std::vector<Vector3>& pos, const std::vector<Vector3>& nrm,
            const std::vector<UnsignedInt>& idx) {
    gpu.vertices.setData(pos, GL::BufferUsage::StaticDraw);
    gpu.normals.setData(nrm, GL::BufferUsage::StaticDraw);
    gpu.indices.setData(idx, GL::BufferUsage::StaticDraw);
    gpu.mesh = GL::Mesh{};
    gpu.mesh.setCount(static_cast<Int>(idx.size()))
        .setPrimitive(GL::MeshPrimitive::Triangles)
        .addVertexBuffer(gpu.vertices, 0, GL::Attribute<0, Vector3>{})
        .addVertexBuffer(gpu.normals, 0, GL::Attribute<1, Vector3>{})
        .setIndexBuffer(gpu.indices, 0, GL::MeshIndexType::UnsignedInt);
}

void addQuad(std::vector<UnsignedInt>& idx, int a, int b, int c, int d) {
    idx.push_back(a); idx.push_back(b); idx.push_back(c);
    idx.push_back(a); idx.push_back(c); idx.push_back(d);
}

void buildTorus(std::vector<Vector3>& pos, std::vector<Vector3>& nrm, std::vector<UnsignedInt>& idx,
                float major, float tube, int rings, int segs) {
    const float pi = 3.14159265f;
    for (int i = 0; i <= rings; ++i) {
        float th = 2.0f * pi * i / rings;
        for (int j = 0; j <= segs; ++j) {
            float ph = 2.0f * pi * j / segs;
            float cr = major + tube * std::cos(ph);
            float ct = std::cos(th), st = std::sin(th);
            pos.push_back({cr * ct, tube * std::sin(ph), cr * st});
            nrm.push_back({std::cos(ph) * ct, std::sin(ph), std::cos(ph) * st});
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

void buildHyperboloid(std::vector<Vector3>& pos, std::vector<Vector3>& nrm, std::vector<UnsignedInt>& idx) {
    const float pi = 3.14159265f;
    const int rings = 40, segs = 28;
    const float a = 1.35f, c = 1.6f;
    for (int i = 0; i <= rings; ++i) {
        float u = 2.0f * pi * i / rings;
        for (int j = 0; j <= segs; ++j) {
            float v = -1.15f + 2.30f * j / segs;
            float ch = std::cosh(v), sh = std::sinh(v);
            float cu = std::cos(u), su = std::sin(u);
            pos.push_back({a * ch * cu, c * sh, a * ch * su});
            Vector3 du{-a * ch * su, 0.0f, a * ch * cu};
            Vector3 dv{a * sh * cu, c * ch, a * sh * su};
            Vector3 n = Math::cross(du, dv);
            float len = n.length();
            nrm.push_back(len > 1e-6f ? n / len : Vector3{cu, 0.0f, su});
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

void buildTwisted(std::vector<Vector3>& pos, std::vector<Vector3>& nrm, std::vector<UnsignedInt>& idx) {
    const float pi = 3.14159265f;
    const int rings = 96, segs = 36;
    for (int i = 0; i <= rings; ++i) {
        float th = 2.0f * pi * i / rings;
        for (int j = 0; j <= segs; ++j) {
            float ph = 2.0f * pi * j / segs;
            pos.push_back(vortexPosition(th, ph));
            nrm.push_back(vortexNormal(th, ph));
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

void buildSurface(SurfaceGpu& gpu, SurfaceKind kind) {
    std::vector<Vector3> pos;
    std::vector<Vector3> nrm;
    std::vector<UnsignedInt> idx;
    if (kind == SurfaceKind::Hyperboloid) {
        buildHyperboloid(pos, nrm, idx);
    } else if (kind == SurfaceKind::InnerTorus) {
        buildTorus(pos, nrm, idx, 1.15f, 0.48f, 40, 20);
    } else if (kind == SurfaceKind::TwistedTorus) {
        buildTwisted(pos, nrm, idx);
    } else {
        buildTorus(pos, nrm, idx, 2.2f, 1.0f, 48, 24);
    }
    upload(gpu, pos, nrm, idx);
}
