#include "sdl/UIApp.h"
#include "HtmlRenderHost.h"
#include <stdexcept>
#include <memory>
#include <iostream>

UIApp::UIApp(int width, int height, const std::string& title)
    : mWidth(width), mHeight(height), mWindow(nullptr), mRenderer(nullptr)
{
    if( !SDL_Init( SDL_INIT_VIDEO ) ) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    if( !TTF_Init() ) {
        throw std::runtime_error(std::string("TTF_Init failed: ") + SDL_GetError());
    }

    mWindow = SDL_CreateWindow(title.c_str(), width, height, 0);
    if (!mWindow) {
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateWindow failSDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNCed: ") + SDL_GetError());
    }

    mRenderer = SDL_CreateRenderer(mWindow, NULL);
    if (!mRenderer) {
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
    }
}

UIApp::~UIApp() { cleanup(); }

void UIApp::cleanup() {
    if (mWindow) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }

    if (mRenderer) {
        SDL_DestroyRenderer(mRenderer);
        mRenderer = nullptr;
    }

    TTF_Quit();
    SDL_Quit();
}

void UIApp::run() {
    bool quit = false;
    SDL_Event e;
    SDL_zero(e);

    std::unique_ptr<HtmlRenderHost> htmlRenderer = std::make_unique<HtmlRenderHost>();
    htmlRenderer->set_renderer(mRenderer);
    litehtml::document::ptr doc = litehtml::document::createFromString(html, htmlRenderer.get());
    doc->render(mWidth);

    // Simple test for draw_text later
    litehtml::font_description fontDesc;
    fontDesc.family = "Roboto-Regular";
    fontDesc.size = 14;
    fontDesc.weight = 400;
    litehtml::font_metrics metrics;
    litehtml::font_metrics* fm = &metrics;
    litehtml::uint_ptr yourFontHandle = htmlRenderer->create_font(fontDesc, doc.get(), fm);

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        // Clear screen with white
        SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
        SDL_RenderClear(mRenderer);

        doc->draw(reinterpret_cast<litehtml::uint_ptr> (htmlRenderer.get()), 0, 0, nullptr);

        // We call here, but doc->draw() should be doing it instead (?)
        //htmlRenderer->draw_text(0, "Hello World", yourFontHandle, {255, 0, 0, 255}, {10, 10, 100, 20});

        // Present the rendered frame
        SDL_RenderPresent(mRenderer);
    }
}

void UIApp::loadPage(const std::string& html)
{
    this->html = html;
}