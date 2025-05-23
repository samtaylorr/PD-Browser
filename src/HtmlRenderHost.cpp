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
        // Optionally trim whitespace here if needed
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
    if (italic) {
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

int HtmlRenderHost::text_width( const char* text, litehtml::uint_ptr hFont )
{
    TTF_Font* font = (TTF_Font*)hFont;
    
    if(!font) {
        // std::cout << "[text_width](" << text << ") font: null" << std::endl;
        return 0;
    }
    return TTF_GetFontSize(font);
}

void HtmlRenderHost::draw_text( litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos )
{
    SDL_Color sdlcolor={color.red, color.green, color.blue, color.alpha};
    SDL_Surface *info;
    TTF_Font* font = (TTF_Font*)hFont;

    if(!(info=TTF_RenderText_Blended(font, text, 0, sdlcolor))) {
        //handle error here, perhaps print SDL_GetError at least
    } else {
        // fixme - use baseline correctly

        SDL_Texture *texture = SDL_CreateTextureFromSurface(m_renderer, info);
        
        SDL_FRect src = {
            0.0f,
            0.0f,
            static_cast<float>(info->w),
            static_cast<float>(info->h)
        };

        SDL_FRect dst = {
            static_cast<float>(pos.x),
            static_cast<float>(pos.y - static_cast<int>(pos.height * 0.5)),
            static_cast<float>(info->w),
            static_cast<float>(info->h)
        };


        SDL_RenderTexture(m_renderer, texture, &src, &dst);
        SDL_DestroyTexture(texture);
    }
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
    return "sans-serif";
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