// 用系统字体把 UTF-8 字符串画成纹理，供边上的动词和右侧属性面板使用。
#pragma once
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Vector2.h>
#include <string>
#include <unordered_map>

class TextCache {
public:
    struct Image {
        Magnum::GL::Texture2D* texture = nullptr;
        Magnum::Vector2i pixels{0, 0};
    };

    Image get(const std::string& utf8);

private:
    struct Entry {
        Magnum::GL::Texture2D texture{Magnum::NoCreate};
        Magnum::Vector2i pixels{0, 0};
    };
    std::unordered_map<std::string, Entry> cache_;
};
