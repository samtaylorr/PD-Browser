#ifndef UISCROLLBAR_H
#define UISCROLLBAR_H

#include <SDL3/SDL.h>

class UIScrollbar {
public:
    UIScrollbar();

    void update(int contentHeight, int viewHeight);
    void handle_wheel_event(const SDL_Event& e);
    void render(SDL_Renderer* renderer, int viewWidth, int viewHeight);

    int get_scroll_y() const;

private:
    int mScrollY;
    int mContentHeight;
    int mViewHeight;
};

#endif // UISCROLLBAR_H
