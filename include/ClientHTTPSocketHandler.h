#ifndef CLIENTHTTPSOCKETHANDLER_H
#define CLIENTHTTPSOCKETHANDLER_H

#include "SocketHandler.h"
#include "raii/RAIISockets.h"
#include <memory>
#include <string>
#include <map>
#include <optional>
#include <functional>

// Structure for holding HTTP headers and HTML body content
struct HttpResponse {
    std::multimap<std::string, std::string> headers;
    std::string html_body;  // Decoded response body
};

class ClientHTTPSocketHandler : public SocketHandler
{
public:
    explicit ClientHTTPSocketHandler(std::string addr);

    // Sends an HTTP request (default method = GET, default path = /index.html)
    int SendHTTPRequest(const std::string& method = "GET", const std::string& path = "/index.html");

    // Parses the received HTTP response into headers and body
    std::optional<std::reference_wrapper<HttpResponse>> ParseHTMLResponse();

private:
    std::string address;
    WinsockInit wsaInit;
    SocketRAII socket;
    std::vector<char> dataBuffer;
    std::optional<HttpResponse> httpResponse;
};

#endif // CLIENTHTTPSOCKETHANDLER_H
