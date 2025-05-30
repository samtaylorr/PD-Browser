#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "ui/UIWindow.h"
#include "ui/UIContext.h"
#include "ClientHTTPSocketHandler.h"
#include <string>
#include <cassert>
#include <iostream>

constexpr const char* project = "PD Browser";

int main(int argc, char* argv[]) {
    try {
        UIWindow window(UIContext::get().windowWidth, UIContext::get().windowHeight, project);
        if (argc >= 2) {
            UIContext::get().address = argv[1];
            ClientHTTPSocketHandler client(UIContext::get().address);
            UIContext::get().path = argv[2];
            client.SendHTTPRequest("GET", UIContext::get().path);
            auto responseOpt = client.ParseHTMLResponse();
            if (responseOpt) {
                window.loadPage(responseOpt->get().html_body);
            } else {
                std::cerr << "Failed to parse HTML response.\n";
            }
            window.loadPage(client.ParseHTMLResponse()->get().html_body);
            window.run();
        }
        else
        {
            throw std::invalid_argument("Usage:\nweb-browser <URL>\nweb-browser <URL> <path>");
        }
        
    }
    catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Exception: %s\n", e.what());
        return 1;
    }
    return 0;
}