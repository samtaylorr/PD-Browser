
#ifndef CLIENTHTTPSOCKETHANDLER_H
#define CLIENTHTTPSOCKETHANDLER_H
#include "SocketHandler.h"
#include "raii/RAIISockets.h"

#include <memory>
#include <string>
#include <map>

// Holds common HTTP headers and body
typedef struct {
    std::multimap<std::string, std::string> headers;
    std::string html_body;  // Decoded body
} HttpResponse;

class ClientHTTPSocketHandler : public SocketHandler
{
public:
    explicit ClientHTTPSocketHandler(std::string addr)
        : address(std::move(addr)), wsaInit(), socket() {}
    int SendHTTPRequest(const std::string& method, const std::string& path);
    HttpResponse* ParseHTMLResponse();

private:
    std::string address;
    WinsockInit wsaInit;
    SocketRAII socket;
    std::vector<char> dataBuffer;
    std::unique_ptr<HttpResponse> httpResponse;
};

#endif