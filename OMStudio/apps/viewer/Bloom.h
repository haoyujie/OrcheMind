// 三维视口的 bloom：场景画进 RGBA16F，四级降采样模糊后叠回默认缓冲。
#pragma once
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Vector2.h>

class Bloom {
public:
    Bloom();

    void resize(const Magnum::Vector2i& viewSize);
    void bindScene();
    void composite(float strength);

private:
    class BlurShader : public Magnum::GL::AbstractShaderProgram {
    public:
        explicit BlurShader();
        void setTexelThreshold(const Magnum::Vector2& texel, float threshold);

    private:
        Magnum::Int texelUniform_ = 0;
        Magnum::Int thresholdUniform_ = 0;
    };

    class CompositeShader : public Magnum::GL::AbstractShaderProgram {
    public:
        explicit CompositeShader();
        void setStrength(float strength);

    private:
        Magnum::Int strengthUniform_ = 0;
    };

    struct Level {
        Magnum::GL::Texture2D a{Magnum::NoCreate};
        Magnum::GL::Texture2D b{Magnum::NoCreate};
        Magnum::GL::Framebuffer fa{Magnum::NoCreate};
        Magnum::GL::Framebuffer fb{Magnum::NoCreate};
        Magnum::Vector2i size{0};
    };

    void allocColor(Magnum::GL::Texture2D& tex, const Magnum::Vector2i& size);
    void attach(Magnum::GL::Framebuffer& fbo, Magnum::GL::Texture2D& tex, const Magnum::Vector2i& size, bool depth);
    void blurPass(Magnum::GL::Framebuffer& dst, const Magnum::Vector2i& size,
                  Magnum::GL::Texture2D& src, const Magnum::Vector2& texel, float threshold);

    Magnum::Vector2i view_{0};
    Magnum::GL::Texture2D sceneColor_{Magnum::NoCreate};
    Magnum::GL::Renderbuffer sceneDepth_{Magnum::NoCreate};
    Magnum::GL::Framebuffer sceneFbo_{Magnum::NoCreate};
    Level level_[4];
    Magnum::GL::Mesh tri_;
    Magnum::GL::Buffer triBuf_;
    BlurShader blur_;
    CompositeShader composite_;
};
