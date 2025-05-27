#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")

    using SocketType = SOCKET;
    constexpr SocketType INVALID_SOCKET_TYPE = INVALID_SOCKET;

    struct WinsockInit {
        WinsockInit() {
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
                throw std::runtime_error("WSAStartup failed");
        }
        ~WinsockInit() noexcept {
            WSACleanup();
        }

        WinsockInit(const WinsockInit&) = delete;
        WinsockInit& operator=(const WinsockInit&) = delete;
        WinsockInit(WinsockInit&&) = delete;
        WinsockInit& operator=(WinsockInit&&) = delete;

    private:
        WSADATA wsaData;
    };

    inline int CloseSocket(SocketType s) {
        return closesocket(s);
    }

#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <cstring>
    #include <stdexcept>

    using SocketType = int;
    constexpr SocketType INVALID_SOCKET_TYPE = -1;

    struct WinsockInit {
        WinsockInit() = default;
        ~WinsockInit() noexcept = default;

        WinsockInit(const WinsockInit&) = delete;
        WinsockInit& operator=(const WinsockInit&) = delete;
        WinsockInit(WinsockInit&&) = delete;
        WinsockInit& operator=(WinsockInit&&) = delete;
    };

    inline int CloseSocket(SocketType s) {
        return close(s);
    }

#endif

struct SocketRAII {
    SocketType sock = INVALID_SOCKET_TYPE;

    SocketRAII() = default;
    explicit SocketRAII(SocketType s) : sock(s) {}

    SocketRAII(const SocketRAII&) = delete;
    SocketRAII& operator=(const SocketRAII&) = delete;

    SocketRAII(SocketRAII&& other) noexcept : sock(other.sock) {
        other.sock = INVALID_SOCKET_TYPE;
    }

    SocketRAII& operator=(SocketRAII&& other) noexcept {
        if (this != &other) {
            close();
            sock = other.sock;
            other.sock = INVALID_SOCKET_TYPE;
        }
        return *this;
    }

    bool operator==(SocketType other) const {
        return sock == other;
    }

    ~SocketRAII() {
        close();
    }

    void close() {
        if (sock != INVALID_SOCKET_TYPE) {
            CloseSocket(sock);
            sock = INVALID_SOCKET_TYPE;
        }
    }

    operator SocketType() const {
        return sock;
    }
};

#endif // SOCKET_UTILS_H