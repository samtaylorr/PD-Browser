#ifndef UIAPP_H
#define UIAPP_H

#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

class UIApp
{
public:
    UIApp(int width, int height, const std::string& title);

    // RAII: delete copy constructor/assignment
    UIApp(const UIApp&) = delete;
    UIApp& operator=(const UIApp&) = delete;

    ~UIApp();
    void run();
    void loadPage(const std::string& html);

private:
    void cleanup();

    int mWidth;
    int mHeight;
    std::string html;
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
};

#endif // UIAPP_H