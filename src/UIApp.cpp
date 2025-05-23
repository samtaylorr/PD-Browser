#include "sdl/UIApp.h"
#include "HtmlRenderHost.h"
#include <stdexcept>
#include <memory>
#include <iostream>

UIApp::UIApp(int width, int height, const std::string& title)
    : mWidth(width), mHeight(height), mWindow(nullptr), mScreenSurface(nullptr), mHelloWorld(nullptr)
{
    if( !SDL_Init( SDL_INIT_VIDEO ) ) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    mWindow = SDL_CreateWindow(title.c_str(), width, height, 0);
    if (!mWindow) {
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    }

    mScreenSurface = SDL_GetWindowSurface(mWindow);
    if (!mScreenSurface) {
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_GetWindowSurface failed: ") + SDL_GetError());
    }
}

UIApp::~UIApp() { cleanup(); }

void UIApp::cleanup() {
    if (mHelloWorld) {
        SDL_DestroySurface(mHelloWorld);
        mHelloWorld = nullptr;
    }
    if (mWindow) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }
    SDL_Quit();
}

void UIApp::run() {
    bool quit = false;
    SDL_Event e;
    SDL_zero(e);

    // Create HTML renderer
    std::unique_ptr<HtmlRenderHost> renderer = std::make_unique<HtmlRenderHost>();
    litehtml::document::ptr doc = litehtml::document::createFromString(html, renderer.get());
    doc->render(mWidth);  // Layout with desired width
    doc->draw(reinterpret_cast<litehtml::uint_ptr>(renderer.get()), 0, 0, nullptr);
    std::cout << html << std::endl;
    renderer->draw_text(reinterpret_cast<litehtml::uint_ptr>(renderer.get()), "TEST", 1, {255,255,255,255}, {10,10,50,20});

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        SDL_FillSurfaceRect(mScreenSurface, nullptr, SDL_MapSurfaceRGB(mScreenSurface, 0xFF, 0xFF, 0xFF));
        SDL_BlitSurface(mHelloWorld, nullptr, mScreenSurface, nullptr);
        SDL_UpdateWindowSurface(mWindow);
    }
}

void UIApp::loadPage(const std::string& html)
{
    this->html = html;
}