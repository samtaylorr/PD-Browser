#include "ClientHTTPSocketHandler.h"
#include "util/HeaderParser.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
    #define GET_LAST_ERROR WSAGetLastError()
    #define SOCKET_ERROR_CODE SOCKET_ERROR
    #define SHUTDOWN_SEND SD_SEND
#else
    #include <sys/time.h>
    #include <errno.h>
    #define GET_LAST_ERROR errno
    #define SOCKET_ERROR_CODE -1
    #define SHUTDOWN_SEND SHUT_WR
    #include <netinet/in.h>
    #include <netinet/tcp.h>
#endif

ClientHTTPSocketHandler::ClientHTTPSocketHandler(std::string addr)
    : address(std::move(addr)), wsaInit(), socket() {}

int ClientHTTPSocketHandler::SendHTTPRequest(const std::string& method, const std::string& path)
{
    try {
        struct addrinfo *result = nullptr, *ptr = nullptr, hints{};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        int iResult = getaddrinfo(address.c_str(), "80", &hints, &result);
        if (iResult != 0) {
            std::cerr << "getaddrinfo failed with error: " << iResult << "\n";
            return 1;std::cout << address.c_str() << std::endl;
        }
        std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> resultGuard(result, freeaddrinfo);

        SocketRAII connectSocket;

        for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
            SocketRAII tempSocket(::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol));
            if (tempSocket == INVALID_SOCKET_TYPE) continue;

            if (connectWithTimeout(tempSocket, ptr->ai_addr, 5000)) {
                connectSocket = std::move(tempSocket);
                break;
            }
        }

        if (connectSocket == INVALID_SOCKET_TYPE) {
            std::cerr << "Unable to connect to server!\n";
            return 1;
        }

        std::string sendbuf = method + " " + path + " HTTP/1.1\r\nHost: " + address + "\r\nConnection: close\r\n\r\n";
        iResult = send(connectSocket, sendbuf.c_str(), static_cast<int>(sendbuf.size()), 0);
        if (iResult == SOCKET_ERROR_CODE) {
            std::cerr << "send failed with error: " << GET_LAST_ERROR << "\n";
            return 1;
        }

        if (shutdown(connectSocket, SHUTDOWN_SEND) == SOCKET_ERROR_CODE) {
            std::cerr << "shutdown failed with error: " << GET_LAST_ERROR << "\n";
            return 1;
        }

        this->resetDataBuffer();

        std::vector<char> recvbuf(DEFAULT_BUFLEN);
        #ifdef _WIN32
        setsockopt(connectSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
        #else
        struct timeval tv { timeoutMs / 1000, (timeoutMs % 1000) * 1000 };
        setsockopt(connectSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        #endif

        do {
            iResult = recv(connectSocket, recvbuf.data(), static_cast<int>(recvbuf.size()), 0);
            if (iResult > 0) {
                this->recvWrite(iResult, recvbuf.data());
            } else if (iResult == 0) {
                break;
            } else {
                std::cerr << "recv failed with error: " << GET_LAST_ERROR << "\n";
                break;
            }
        } while (iResult > 0);

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
}

std::optional<std::reference_wrapper<HttpResponse>> ClientHTTPSocketHandler::ParseHTMLResponse()
{
    httpResponse.emplace();
    std::string_view response_view = this->getDataBuffer();
    std::string response(response_view.substr(0, this->getTotalReceived()));

    size_t pos = response.find("\r\n\r\n");
    if (pos == std::string::npos) pos = response.find("\n\n");

    if (pos != std::string::npos) {
        std::string headers = response.substr(0, pos);
        std::string body = response.substr(pos + (response[pos] == '\r' ? 4 : 2));
        auto parsedHeaders = HeaderParser::parseHeaders(headers);
        for (auto& [key, value] : parsedHeaders) {
            this->httpResponse->headers.emplace(key, value);
        }
        httpResponse->html_body = body;
    } else {
        // No headers found: treat entire response as body
        httpResponse->html_body = response;
    }
    return *httpResponse;
}