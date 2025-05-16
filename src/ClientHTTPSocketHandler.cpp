#include "ClientHTTPSocketHandler.h"
#include "util/Parser.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cassert>

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
            return 1;
        }
        std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> resultGuard(result, freeaddrinfo);

        SocketRAII connectSocket; // SocketRAII with default INVALID_SOCKET

        for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
            SocketRAII tempSocket(::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol));
            if (tempSocket == INVALID_SOCKET) continue;

            if (connectWithTimeout(tempSocket, ptr->ai_addr, 5000)) {
                connectSocket = std::move(tempSocket);  // transfer ownership here
                break;  // stop after successful connect
            } else {
                std::cerr << "Connection timed out or failed.\n";
            }
        }

        if (connectSocket == INVALID_SOCKET) {
            std::cerr << "Unable to connect to server!\n";
            return 1;
        }

        std::string sendbuf = method + " " + path + " HTTP/1.1\r\nHost: " + address + "\r\nConnection: close\r\n\r\n";
        iResult = send(connectSocket, sendbuf.c_str(), static_cast<int>(sendbuf.size()), 0);
        if (iResult == SOCKET_ERROR) {
            std::cerr << "send failed with error: " << WSAGetLastError() << "\n";
            return 1;
        }

        if (shutdown(connectSocket, SD_SEND) == SOCKET_ERROR) {
            std::cerr << "shutdown failed with error: " << WSAGetLastError() << "\n";
            return 1;
        }

        this->resetDataBuffer();

        std::vector<char> recvbuf(DEFAULT_BUFLEN);
        setsockopt(connectSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
        do {
            iResult = recv(connectSocket, recvbuf.data(), (int)recvbuf.size(), 0);
            if (iResult > 0) {
                this->recvWrite(iResult, recvbuf.data());
            } else if (iResult == 0) {
                break;
            } else {
                if (WSAGetLastError() == WSAETIMEDOUT) {
                    std::cerr << "recv timed out.\n";
                }
                else
                {
                    std::cerr << "recv failed with error: " << WSAGetLastError() << "\n";
                }
                break;
            }
        } while (iResult > 0);

        if (auto response = ParseHTMLResponse()) {
            auto range = response->get().headers.equal_range("date");
            for (auto it = range.first; it != range.second; ++it)
                std::cout << "Date: " << it->second << "\n";
        }  else {
            std::cerr << "Failed to parse HTTP response.\n";
            return 1;
        }

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
        std::string body = response.substr(pos + (response[pos] == '\r' ? 4 : 2)); // skip separator length
        auto parsedHeaders = HeaderParser::parseHeaders(headers);
        for (auto& [key, value] : parsedHeaders) this->httpResponse->headers.emplace(key, value);
        httpResponse->html_body = body;
        return *httpResponse;
    }
    else {
        std::cerr << "No header/body separator found! First 100 bytes: "
              << response.substr(0, 100) << "\n";
        return std::nullopt;
    }
}

const HttpResponse& ClientHTTPSocketHandler::GetResponse() const {
    assert(httpResponse.has_value() && "HttpResponse is not available");
    return *httpResponse;
}