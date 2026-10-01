#include "GlowShader.h"
#include <Corrade/Utility/Debug.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>

using namespace Magnum;

GlowShader::GlowShader() {
    GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex};
    GL::Shader frag{GL::Version::GL330, GL::Shader::Type::Fragment};
    vert.addSource(R"GLSL(
in vec3 position;
in vec3 normal;
uniform mat4 viewProj;
uniform mat4 model;
uniform vec3 cameraPos;
out vec3 vN;
out vec3 vW;
out vec3 vV;
void main() {
    vec4 w = model * vec4(position, 1.0);
    vW = w.xyz;
    vN = mat3(model) * normal;
    vV = cameraPos - w.xyz;
    gl_Position = viewProj * w;
}
)GLSL");
    frag.addSource(R"GLSL(
in vec3 vN;
in vec3 vW;
in vec3 vV;
uniform float time;
uniform float gain;
out vec4 color;
void main() {
    vec3 N = normalize(vN);
    vec3 V = normalize(vV);
    if (dot(N, V) < 0.0) N = -N;
    float fres = pow(1.0 - max(dot(N, V), 0.0), 3.0);
    float ang = atan(vW.z, vW.x);
    float stripe = 0.5 + 0.5 * sin(ang * 3.0 + vW.y * 6.0 + time * 0.7);
    float pulse = 0.82 + 0.18 * sin(time * 2.4 + ang * 5.0);
    vec3 cyan = vec3(0.16, 0.90, 1.0);
    vec3 magenta = vec3(1.0, 0.24, 0.65);
    vec3 purple = vec3(0.69, 0.42, 1.0);
    vec3 neon = mix(mix(cyan, purple, stripe), magenta, 0.35 + 0.25 * sin(ang * 6.0));
    vec3 rgb = neon * (0.05 + fres * 1.6) * pulse * gain;
    float alpha = 0.06 + fres * 0.72;
    color = vec4(rgb, alpha);
}
)GLSL");
    CORRADE_INTERNAL_ASSERT_OUTPUT(vert.compile() && frag.compile());
    attachShader(vert);
    attachShader(frag);
    bindAttributeLocation(0, "position");
    bindAttributeLocation(1, "normal");
    CORRADE_INTERNAL_ASSERT_OUTPUT(link());
    viewProjUniform_ = uniformLocation("viewProj");
    modelUniform_ = uniformLocation("model");
    cameraUniform_ = uniformLocation("cameraPos");
    timeUniform_ = uniformLocation("time");
    gainUniform_ = uniformLocation("gain");
}

GlowShader& GlowShader::setViewProj(const Matrix4& m) {
    setUniform(viewProjUniform_, m);
    return *this;
}
GlowShader& GlowShader::setModel(const Matrix4& m) {
    setUniform(modelUniform_, m);
    return *this;
}
GlowShader& GlowShader::setCamera(const Vector3& cam) {
    setUniform(cameraUniform_, cam);
    return *this;
}
GlowShader& GlowShader::setTime(float t) {
    setUniform(timeUniform_, t);
    return *this;
}
GlowShader& GlowShader::setGain(float g) {
    setUniform(gainUniform_, g);
    return *this;
}
