#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "sdl/UIApp.h"
#include "ClientHTTPSocketHandler.h"
#include <string>
#include <cassert>
#include <iostream>

constexpr int kScreenWidth{ 600 };
constexpr int kScreenHeight{ 400 };
constexpr const char* project = "PD Browser";

int main(int argc, char* argv[]) {
    try {
        UIApp app(kScreenWidth, kScreenHeight, project);
        if (argc >= 2) {
            ClientHTTPSocketHandler client(argv[1]);
            client.SendHTTPRequest("GET", "/");
            auto responseOpt = client.ParseHTMLResponse();
            if (responseOpt) {
                app.loadPage(responseOpt->get().html_body);
            } else {
                std::cerr << "Failed to parse HTML response.\n";
            }
            app.loadPage(client.ParseHTMLResponse()->get().html_body);
            app.run();
        }
        else
        {
            throw std::invalid_argument("Usage: web-browser <URL>");
        }
        
    }
    catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Exception: %s\n", e.what());
        return 1;
    }
    return 0;
}