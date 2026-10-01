// 曲面发光：菲涅尔边缘光，输出可超过 1 以便 bloom 抓住高光。
#pragma once
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Math/Vector3.h>

class GlowShader : public Magnum::GL::AbstractShaderProgram {
public:
    explicit GlowShader();

    GlowShader& setViewProj(const Magnum::Matrix4& m);
    GlowShader& setModel(const Magnum::Matrix4& m);
    GlowShader& setCamera(const Magnum::Vector3& cam);
    GlowShader& setTime(float t);
    GlowShader& setGain(float g);

private:
    Magnum::Int viewProjUniform_ = 0;
    Magnum::Int modelUniform_ = 0;
    Magnum::Int cameraUniform_ = 0;
    Magnum::Int timeUniform_ = 0;
    Magnum::Int gainUniform_ = 0;
};
