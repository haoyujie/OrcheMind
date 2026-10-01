// 展示模式的星点、辐条、流光拖尾和粒子。拖尾位置由相位在着色器里算出。
#pragma once
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Matrix4.h>
#include <cstdint>
#include <vector>

namespace om {
struct RenderNode;
}

class VortexField {
public:
    VortexField();

    void drawStars(const Magnum::Matrix4& viewProj, const Magnum::Vector3& camera);
    void drawFlow(const Magnum::Matrix4& viewProj, const Magnum::Matrix4& spin,
                  const Magnum::Vector3& camera, float time);
    void drawNodes(const Magnum::Matrix4& viewProj, const Magnum::Vector3& camera,
                   const std::vector<om::RenderNode>& nodes, std::uint64_t selected);

private:
    class PointShader : public Magnum::GL::AbstractShaderProgram {
    public:
        explicit PointShader(bool onVortex);
        PointShader& setViewProj(const Magnum::Matrix4& m);
        PointShader& setSpin(const Magnum::Matrix4& m);
        PointShader& setCamera(const Magnum::Vector3& cam);
        PointShader& setTime(float t);
        PointShader& setPointScale(float s);
        PointShader& setGain(float g);

    private:
        Magnum::Int viewProjUniform_ = 0;
        Magnum::Int spinUniform_ = 0;
        Magnum::Int cameraUniform_ = 0;
        Magnum::Int timeUniform_ = 0;
        Magnum::Int pointScaleUniform_ = 0;
        Magnum::Int gainUniform_ = 0;
    };

    class LineShader : public Magnum::GL::AbstractShaderProgram {
    public:
        explicit LineShader(bool trail);
        LineShader& setViewProj(const Magnum::Matrix4& m);
        LineShader& setSpin(const Magnum::Matrix4& m);
        LineShader& setTime(float t);

    private:
        Magnum::Int viewProjUniform_ = 0;
        Magnum::Int spinUniform_ = 0;
        Magnum::Int timeUniform_ = 0;
    };

    PointShader starsShader_;
    PointShader particleShader_;
    PointShader nodeShader_;
    LineShader spokeShader_;
    LineShader trailShader_;

    Magnum::GL::Buffer starBuf_, particleBuf_, trailBuf_, spokeBuf_, nodeBuf_;
    Magnum::GL::Mesh starMesh_, particleMesh_, trailMesh_, spokeMesh_, nodeMesh_;
    int starCount_ = 0;
    int particleCount_ = 0;
    int trailCount_ = 0;
    int spokeCount_ = 0;
};
