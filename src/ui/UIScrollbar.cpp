#include "ui/UIScrollbar.h"
#include "ui/UIContext.h"
#include <algorithm>

UIScrollbar::UIScrollbar()
    : mScrollY(0), mContentHeight(0), mViewHeight(0) {}

void UIScrollbar::update(int contentHeight, int viewHeight) {
    mContentHeight = contentHeight;
    mViewHeight = viewHeight;
    mScrollY = std::clamp(mScrollY, 0, std::max(0, mContentHeight - mViewHeight));
}

void UIScrollbar::handle_wheel_event(const SDL_Event& e) {
    if (e.type == SDL_EVENT_MOUSE_WHEEL) {
        mScrollY -= e.wheel.y * 30;
        mScrollY = std::clamp(mScrollY, 0, std::max(0, mContentHeight - mViewHeight));
    }
}

int UIScrollbar::get_scroll_y() const {
    return mScrollY;
}

void UIScrollbar::render(SDL_Renderer* renderer, int viewWidth, int viewHeight) {
    const int scrollbarX = static_cast<int>(UIContext::get().getContentWidth());
    const int scrollbarWidth = viewWidth - scrollbarX;

    SDL_FRect scrollbarTrack = {
        static_cast<float>(scrollbarX),
        0.0f,
        static_cast<float>(scrollbarWidth),
        static_cast<float>(viewHeight)
    };

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &scrollbarTrack);

    // Prevent divide-by-zero
    if (mContentHeight <= 0 || mViewHeight <= 0) return;

    float viewRatio = static_cast<float>(mViewHeight) / static_cast<float>(mContentHeight);
    float handleHeight = viewRatio * viewHeight;
    float handleY = (static_cast<float>(mScrollY) / static_cast<float>(mContentHeight)) * viewHeight;

    SDL_FRect scrollbarHandle = {
        static_cast<float>(scrollbarX),
        handleY,
        static_cast<float>(scrollbarWidth),
        handleHeight
    };

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &scrollbarHandle);
}