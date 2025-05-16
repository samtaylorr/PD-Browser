#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "sdl/UIApp.h"
#include "ClientHTTPSocketHandler.h"
#include <string>

constexpr int kScreenWidth{ 1280 };
constexpr int kScreenHeight{ 720 };

int main(int argc, char* argv[]) {
    try {
        UIApp app(kScreenWidth, kScreenHeight, "PD Browser");
        if (argc >= 2) {
            ClientHTTPSocketHandler client(argv[1]);
            client.SendHTTPRequest("GET", "/index.html");
        }
        app.run();
    } 
    catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Exception: %s\n", e.what());
        return 1;
    }
    return 0;
}