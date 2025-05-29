#ifndef UICONTEXT_H
#define UICONTEXT_H

#include <string>

//TODO: Put renderer inside UIContext

class UIContext
{
public:
    static UIContext& get()
    {
        static UIContext instance;
        return instance;
    }

    // Dimensions
    int windowWidth = 800;
    int windowHeight = 600;
    int scrollbarWidth = 10;

    // Scroll state
    int scrollY = 0;
    int contentHeight = 0;
    
    // Network
    std::string url;

    // Derived
    int getContentWidth() const {
        return windowWidth - scrollbarWidth;
    }

private:
    UIContext() = default;
    ~UIContext() = default;

    UIContext(const UIContext&) = delete;
    UIContext& operator=(const UIContext&) = delete;
};

#endif // UICONTEXT_H
