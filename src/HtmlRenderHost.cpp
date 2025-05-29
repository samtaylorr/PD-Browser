#include "HtmlRenderHost.h"
#include "ClientHTTPSocketHandler.h"
#include "ui/UIContext.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <unordered_map>
#include <regex>
#include <litehtml.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_IOStream.h>

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

    std::string fontPath = "data/fonts/" + fontName + ".ttf";
    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), size);
    if (!font) {
        std::cerr << "Failed to load font: " << SDL_GetError() << std::endl;
        return (litehtml::uint_ptr)0;
    }

    int ttfStyle = TTF_STYLE_NORMAL;

    bool italic = (descr.style == litehtml::font_style_italic);
    if (italic) {
        ttfStyle |= TTF_STYLE_ITALIC;    // Only affects local copy
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
    if (!mRenderer) return;
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

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
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

    SDL_RenderTexture(mRenderer, textTexture, nullptr, &dstRect);
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

void HtmlRenderHost::load_image(const char* src, const char* baseurl, bool redraw_on_ready)
{
    std::string imageUrl = resolve_url(src, UIContext::get().url);
    std::cout << imageUrl << std::endl;
    // Check cache
    if (imageCache.find(imageUrl) != imageCache.end()) {
        if (redraw_on_ready && mRedrawCallback) mRedrawCallback();
        return;
    }

    // Use your HTTP handler to fetch the image
    ClientHTTPSocketHandler handler(imageUrl);  // You'll need to split domain from path
    if (handler.SendHTTPRequest("GET", extract_path(imageUrl)) == 0) {
        auto response = handler.ParseHTMLResponse();  // Could rename to ParseHTTPResponse()
        if (response && response->get().html_body.size()) {
            const std::string& data = response->get().html_body;

            // Use SDL_RWFromMem to read image from memory
            SDL_IOStream* rw = SDL_IOFromConstMem(data.data(), static_cast<int>(data.size()));
            if (!rw) {
                std::cerr << "SDL_IOFromConstMem failed: " << SDL_GetError() << "\n";
                return;
            }

            SDL_Surface* surface = IMG_Load_IO(rw, 1);
            if (!surface) {
                std::cerr << "IMG_Load_IO failed: " << SDL_GetError() << "\n";
                return;
            }

            SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
            SDL_DestroySurface(surface);
            if (!texture) {
                std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
                return;
            }

            imageCache[imageUrl] = texture;

            if (redraw_on_ready && mRedrawCallback) {
                mRedrawCallback();
            }
        }
    }
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
    SDL_SetRenderDrawColor(mRenderer, sdlColor.r, sdlColor.g, sdlColor.b, sdlColor.a);
    SDL_RenderFillRect(mRenderer, &fillRect);
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
        SDL_SetRenderDrawColor(mRenderer, borders.top.color.red, borders.top.color.green, borders.top.color.blue, borders.top.color.alpha);
        SDL_RenderRect(mRenderer, &fillRect);
    }
}

const char* HtmlRenderHost::get_default_font_name() const
{
    return "Arial";
}

litehtml::element::ptr HtmlRenderHost::create_element(
    const char* tag_name,
    const litehtml::string_map& attributes,
    const std::shared_ptr<litehtml::document>& doc)
{
    return nullptr;
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
    mRenderer = renderer;
}

void HtmlRenderHost::get_viewport(litehtml::position& viewport) const {
    if (mRenderer) {
        int w, h;
        SDL_GetCurrentRenderOutputSize(mRenderer, &w, &h);
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<int>(UIContext::get().getContentWidth());
        viewport.height = h;
    } else {
        viewport = litehtml::position(0, 0, 600, 400); // fallback
    }
}

void HtmlRenderHost::transform_text(litehtml::string& text, litehtml::text_transform tt) {
    switch(tt) {
        case litehtml::text_transform_capitalize:
            if (!text.empty()) text[0] = std::toupper(text[0]);
            break;
        case litehtml::text_transform_uppercase:
            std::transform(text.begin(), text.end(), text.begin(), ::toupper);
            break;
        case litehtml::text_transform_lowercase:
            std::transform(text.begin(), text.end(), text.begin(), ::tolower);
            break;
        default:
            break;
    }
}

// Prevents CSS lookup failure
void HtmlRenderHost::import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl)
{
    text.clear();
}

void HtmlRenderHost::split_text(const char* text,
    const std::function<void(const char*)>& on_word,
    const std::function<void(const char*)>& on_space)
{
    std::string str(text);
    std::string token;
    for (char ch : str) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!token.empty()) {
                on_word(token.c_str());
                token.clear();
            }
            std::string space(1, ch);
            on_space(space.c_str());
        } else {
            token += ch;
        }
    }

    if (!token.empty()) {
        on_word(token.c_str());
    }
}

std::string HtmlRenderHost::resolve_url(const std::string& src, const std::string& baseurl)
{
    if (src.find("http://") == 0 || src.find("https://") == 0) {
        return src;  // Absolute
    } else {
        // Basic base resolution, e.g., https://example.com/assets/
        if (!baseurl.empty() && baseurl.back() == '/' && src.front() != '/') {
            return baseurl + src;
        } else if (!baseurl.empty()) {
            return baseurl + "/" + src;
        } else {
            return src;  // fallback
        }
    }
}

std::string HtmlRenderHost::extract_path(const std::string& url) {
    auto scheme_end = url.find("://");
    if (scheme_end == std::string::npos) return "/";

    auto path_start = url.find('/', scheme_end + 3);
    return (path_start != std::string::npos) ? url.substr(path_start) : "/";
}