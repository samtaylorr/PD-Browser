#define WIN32_LEAN_AND_MEAN
#include "ClientHTTPSocketHandler.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int __cdecl main(int argc, char **argv) 
{
    // Validate the parameters
    if (argc >= 2) {
        ClientHTTPSocketHandler client(argv[1]);
        return client.SendHTTPRequest("GET", "/index.html");
    }
}