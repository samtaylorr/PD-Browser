#ifndef WINSOCKINIT_H
#define WINSOCKINIT_H

struct WinsockInit {
    WinsockInit() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            throw std::runtime_error("WSAStartup failed");
    }
    ~WinsockInit() noexcept { WSACleanup(); }

    // Deletes the copying and moving operations to insure there is only one of this resource.
    WinsockInit(const WinsockInit&) = delete;
    WinsockInit& operator=(const WinsockInit&) = delete;
    WinsockInit(WinsockInit&&) = delete;
    WinsockInit& operator=(WinsockInit&&) = delete;

private:
    WSADATA wsaData;
};

struct SocketRAII {
    SOCKET sock = INVALID_SOCKET;

    SocketRAII() = default;
    explicit SocketRAII(SOCKET s) : sock(s) {}

    SocketRAII(const SocketRAII&) = delete;
    SocketRAII& operator=(const SocketRAII&) = delete;

    SocketRAII(SocketRAII&& other) noexcept : sock(other.sock) {
        other.sock = INVALID_SOCKET;
    }
    SocketRAII& operator=(SocketRAII&& other) noexcept {
        if (this != &other) {
            close();
            sock = other.sock;
            other.sock = INVALID_SOCKET;
        }
        return *this;
    }

    bool operator==(SOCKET other) const {
        return sock == other;
    }

    ~SocketRAII() {
        close();
    }

    void close() {
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
            sock = INVALID_SOCKET;
        }
    }

    operator SOCKET() const { return sock; }
};

#endif