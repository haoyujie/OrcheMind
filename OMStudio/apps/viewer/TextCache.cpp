#include "TextCache.h"
#include <Corrade/Containers/ArrayView.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/ImageView.h>
#include <Magnum/PixelFormat.h>
#include <windows.h>
#include <vector>

using namespace Magnum;

namespace {

std::wstring utf8ToWide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    if (n <= 0) return {};
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), w.data(), n);
    return w;
}

} // namespace

TextCache::Image TextCache::get(const std::string& utf8) {
    auto found = cache_.find(utf8);
    if (found != cache_.end()) return {&found->second.texture, found->second.pixels};

    std::wstring text = utf8ToWide(utf8.empty() ? " " : utf8);
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    HFONT font = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                             DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");
    HGDIOBJ oldFont = SelectObject(dc, font);
    RECT rc{0, 0, 0, 0};
    DrawTextW(dc, text.c_str(), -1, &rc, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
    int w = std::max(1L, rc.right + 8);
    int h = std::max(1L, rc.bottom + 6);

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = w;
    info.bmiHeader.biHeight = -h; // 自上而下
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ oldBmp = SelectObject(dc, bmp);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(235, 240, 248));
    RECT box{4, 2, w, h};
    DrawTextW(dc, text.c_str(), -1, &box, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE);

    std::vector<char> rgba(static_cast<size_t>(w * h * 4));
    const auto* bgra = static_cast<const unsigned char*>(bits);
    for (int i = 0; i < w * h; ++i) {
        unsigned char b = bgra[i * 4 + 0];
        unsigned char g = bgra[i * 4 + 1];
        unsigned char r = bgra[i * 4 + 2];
        int lum = (r + g + b) / 3;
        rgba[static_cast<size_t>(i) * 4 + 0] = static_cast<char>(236);
        rgba[static_cast<size_t>(i) * 4 + 1] = static_cast<char>(240);
        rgba[static_cast<size_t>(i) * 4 + 2] = static_cast<char>(246);
        rgba[static_cast<size_t>(i) * 4 + 3] = static_cast<char>(lum);
    }

    Entry entry;
    entry.pixels = {w, h};
    entry.texture = GL::Texture2D{};
    entry.texture.setMagnificationFilter(GL::SamplerFilter::Linear)
        .setMinificationFilter(GL::SamplerFilter::Linear)
        .setWrapping(GL::SamplerWrapping::ClampToEdge)
        .setStorage(1, GL::TextureFormat::RGBA8, entry.pixels)
        .setSubImage(0, {}, ImageView2D{PixelFormat::RGBA8Unorm, entry.pixels,
                                       Corrade::Containers::ArrayView<const void>{rgba.data(), rgba.size()}});

    SelectObject(dc, oldBmp);
    SelectObject(dc, oldFont);
    DeleteObject(bmp);
    DeleteObject(font);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);

    auto inserted = cache_.emplace(utf8, std::move(entry));
    return {&inserted.first->second.texture, inserted.first->second.pixels};
}
