#include "ui/UIWindow.h"
#include "ui/UIContext.h"
#include "HtmlRenderHost.h"
#include <stdexcept>
#include <memory>
#include <iostream>

UIWindow::UIWindow(int width, int height, const std::string& title)
    : mWidth(width), mHeight(height), mWindow(nullptr), mRenderer(nullptr)
{
    if( !SDL_Init( SDL_INIT_VIDEO ) ) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    if( !TTF_Init() ) {
        throw std::runtime_error(std::string("TTF_Init failed: ") + SDL_GetError());
    }

    mWindow = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_RESIZABLE);
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

UIWindow::~UIWindow() { cleanup(); }

void UIWindow::cleanup() {
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

void UIWindow::run() {
    bool quit = false;
    SDL_Event e;
    SDL_zero(e);

    std::unique_ptr<HtmlRenderHost> htmlRenderer = std::make_unique<HtmlRenderHost>();
    htmlRenderer->set_renderer(mRenderer);
    litehtml::document::ptr doc = litehtml::document::createFromString(html, htmlRenderer.get());
    doc->render(UIContext::get().getContentWidth());
    mScrollbar.update(doc->height(), UIContext::get().windowHeight);

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                UIContext::get().windowWidth = e.window.data1;
                UIContext::get().windowHeight = e.window.data2;
                doc->render(static_cast<int>(UIContext::get().getContentWidth()));
                mScrollbar.update(doc->height(), UIContext::get().windowHeight);
            }

            mScrollbar.handle_wheel_event(e);
        }

        // Clear screen with white
        SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
        SDL_RenderClear(mRenderer);

        doc->draw(reinterpret_cast<litehtml::uint_ptr>(htmlRenderer.get()), 0, -mScrollbar.get_scroll_y(), nullptr);
        mScrollbar.render(mRenderer, UIContext::get().windowWidth, UIContext::get().windowHeight);

        // Present the rendered frame
        SDL_RenderPresent(mRenderer);
    }
}

void UIWindow::loadPage(const std::string& html) { this->html = html; }