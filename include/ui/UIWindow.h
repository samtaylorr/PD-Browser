#ifndef UIWINDOW_H
#define UIWINDOW_H

#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "ui/UIScrollbar.h"

class UIWindow
{
public:
    UIWindow(int width, int height, const std::string& title);

    // RAII: delete copy constructor/assignment
    UIWindow(const UIWindow&) = delete;
    UIWindow& operator=(const UIWindow&) = delete;

    ~UIWindow();
    void run();
    void loadPage(const std::string& html);

private:
    void cleanup();

    int mWidth;
    int mHeight;
    int mScrollY = 0;
    std::string html;
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    UIScrollbar mScrollbar;
};

#endif // UIWINDOW_H