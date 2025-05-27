#include "HtmlRenderHost.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

using namespace litehtml;

// HtmlRenderHost.cpp
litehtml::uint_ptr HtmlRenderHost::create_font(
    const litehtml::font_description& descr,
    const litehtml::document* doc,
    litehtml::font_metrics* fm)
{
    int size = descr.size;
    std::string fontName = "Roboto-Medium";
    if (!descr.family.empty()) {
        size_t pos = descr.family.find(',');
        fontName = (pos == std::string::npos) ? descr.family : descr.family.substr(0, pos);
    }

    std::string fontPath = "../data/fonts/" + fontName + ".ttf";

    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), size);
    if (font == nullptr) {
        std::cout << "[create_font] can't load ttf: " << fontName << std::endl;
        std::cout << "TTF_OpenFont: " << SDL_GetError() << std::endl;
        return 0;
    }

    int ttfStyle = TTF_STYLE_NORMAL;

    bool italic = (descr.style == litehtml::font_style_italic);
    if (italic) {std::string fontPath = "../data/fonts/" + fontName + ".ttf";

    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), size);
    if (font == nullptr) {
        std::cout << "[create_font] can't load ttf: " << fontName << std::endl;
        std::cout << "TTF_OpenFont: " << SDL_GetError() << std::endl;
        return 0;
    }

    int ttfStyle = TTF_STYLE_NORMAL;
        ttfStyle |= TTF_STYLE_ITALIC;
    }

    int decoration = descr.decoration_line;
    const int font_decoration_underline = 1;
    const int font_decoration_linethrough = 2;

    if ((decoration & font_decoration_linethrough) != 0) {
        ttfStyle |= TTF_STYLE_STRIKETHROUGH;
    }
    if ((decoration & font_decoration_underline) != 0) {
        ttfStyle |= TTF_STYLE_UNDERLINE;
    }

    TTF_SetFontStyle(font, ttfStyle);

    if (fm) {
        fm->ascent    = TTF_GetFontAscent(font);
        fm->descent   = TTF_GetFontDescent(font);
        fm->height    = TTF_GetFontHeight(font);
        fm->x_height  = get_default_font_size();
        fm->draw_spaces = italic || (decoration != 0);
    }

    return (litehtml::uint_ptr)font;
}


void HtmlRenderHost::delete_font( litehtml::uint_ptr hFont )
{
    // todo fix segfault
    return;

    TTF_Font* font = (TTF_Font*)hFont;

    if(font) {
        TTF_CloseFont( font );
    }
}

int HtmlRenderHost::text_width(const char* text, litehtml::uint_ptr hFont)
{
    TTF_Font* font = (TTF_Font*)hFont;
    if (!font) {
        return 0;
    }

    TTF_Text* ttfText = TTF_CreateText(NULL, font, text, strlen(text));
    if (!ttfText) {
        std::cerr << "TTF_CreateText failed: " << SDL_GetError() << std::endl;
        return 0;
    }

    int w = 0, h = 0;
    if (TTF_GetTextSize(ttfText, &w, &h)) {
        TTF_DestroyText(ttfText);
        return w;
    } else {
        TTF_DestroyText(ttfText);
        return 0;
    }
}

void HtmlRenderHost::draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
    if (!m_renderer) return;
    if (!text) return;

    TTF_Font* font = (TTF_Font*)hFont;
    if (!font) return;

    SDL_Color sdlcolor = {
        (Uint8)color.red,
        (Uint8)color.green,
        (Uint8)color.blue,
        (color.alpha != 0) ? (Uint8)color.alpha : (Uint8)255
    };

    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, 0, sdlcolor);
    if (!textSurface) {
        std::cerr << "TTF_RenderText_Blended failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
    SDL_DestroySurface(textSurface);
    if (!textTexture) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_FRect dstRect;
    dstRect.x = static_cast<float>(pos.x);
    dstRect.y = static_cast<float>(pos.y);

    float w = 0.f, h = 0.f;
    if (SDL_GetTextureSize(textTexture, &w, &h)) {
        dstRect.w = w;
        dstRect.h = h;
    } else {
        std::cerr << "SDL_GetTextureSize failed: " << SDL_GetError() << std::endl;
        // fallback to pos width/height or zero
        dstRect.w = pos.width > 0 ? static_cast<float>(pos.width) : 0.f;
        dstRect.h = pos.height > 0 ? static_cast<float>(pos.height) : 0.f;
    }

    SDL_RenderTexture(m_renderer, textTexture, nullptr, &dstRect);
    SDL_DestroyTexture(textTexture);
}

int HtmlRenderHost::pt_to_px( int pt ) const
{
    return pt; // (int) round(pt * 125.0 / 72.0);
}

int HtmlRenderHost::get_default_font_size() const
{
    return 16;
}

void HtmlRenderHost::draw_list_marker( litehtml::uint_ptr hdc, const litehtml::list_marker& marker )
{
}

void HtmlRenderHost::load_image( const char* src, const char* baseurl, bool redraw_on_ready )
{
    std::cout << "#loadImage " << src << "\n";
}

void HtmlRenderHost::get_image_size( const char* src, const char* baseurl, litehtml::size& sz )
{

}

void HtmlRenderHost::draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color)
{
    SDL_FRect fillRect = {
        static_cast<float>(layer.clip_box.x),
        static_cast<float>(layer.clip_box.y),
        static_cast<float>(layer.clip_box.width),
        static_cast<float>(layer.clip_box.height)
    };

    SDL_Color sdlColor = {
        color.red,
        color.green,
        color.blue,
        color.alpha
    };
    SDL_SetRenderDrawColor(m_renderer, sdlColor.r, sdlColor.g, sdlColor.b, sdlColor.a);
    SDL_RenderFillRect(m_renderer, &fillRect);
}

void HtmlRenderHost::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    if (borders.top.width != 0 && borders.top.style > litehtml::border_style_hidden) {
        SDL_FRect fillRect = {
            static_cast<float>(draw_pos.x),
            static_cast<float>(draw_pos.y),
            static_cast<float>(draw_pos.width),
            static_cast<float>(draw_pos.height)
        };
        SDL_SetRenderDrawColor(m_renderer, borders.top.color.red, borders.top.color.green, borders.top.color.blue, borders.top.color.alpha);
        SDL_RenderRect(m_renderer, &fillRect);
    }
}

const char* HtmlRenderHost::get_default_font_name() const
{
    return "Roboto-Medium";
}


litehtml::element::ptr	HtmlRenderHost::create_element( const char* tag_name,
														const litehtml::string_map& attributes,
														const std::shared_ptr<litehtml::document>& doc)
{
    return 0;
}

void HtmlRenderHost::get_media_features(litehtml::media_features& media) const
{
    litehtml::position client;
    get_viewport(client);
    media.type      = litehtml::media_type_screen;
    media.width     = client.width;
    media.height    = client.height;
    media.device_width  = 512;
    media.device_height = 512;
    media.color     = 8;
    media.monochrome  = 0;
    media.color_index = 256;
    media.resolution  = 96;
}

void HtmlRenderHost::get_language(litehtml::string& language, litehtml::string& culture) const
{
    language = "en";
    culture = "";
}

void HtmlRenderHost::set_renderer(SDL_Renderer* renderer) {
    m_renderer = renderer;
}