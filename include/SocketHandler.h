#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <vector>
#include <stdexcept>
#include <string_view>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <fcntl.h>
    #include <errno.h>
#endif

#include "raii/RAIISockets.h"

class SocketHandler {
private:
    int totalReceived = 0;
    int bufferCapacity = 0;
    std::vector<char> buffer;

public:
    static constexpr size_t DEFAULT_BUFLEN = 512;
    static constexpr int timeoutMs = 5000;

    std::string_view getDataBuffer() const {
        return std::string_view(buffer.data(), totalReceived);
    }

    void recvWrite(int iResult, const char* recvbuf) {
        if (iResult <= 0) return;
        if (bufferCapacity < totalReceived + iResult) {
            bufferCapacity = totalReceived + iResult;
            buffer.resize(bufferCapacity);
        }
        std::memcpy(buffer.data() + totalReceived, recvbuf, iResult);
        totalReceived += iResult;
    }

    void resetDataBuffer() {
        totalReceived = 0;
        buffer.clear();
        bufferCapacity = 0;
    }

    int getTotalReceived() const {
        return totalReceived;
    }

    bool connectWithTimeout(SocketRAII& tempSocket, sockaddr* addr, int timeoutMs) {
#ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(tempSocket, FIONBIO, &mode);

        int result = connect(tempSocket, addr, sizeof(sockaddr));
        if (result == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
            return false;
        }

        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(tempSocket, &writeSet);

        TIMEVAL timeout{};
        timeout.tv_sec = timeoutMs / 1000;
        timeout.tv_usec = (timeoutMs % 1000) * 1000;

        result = select(0, nullptr, &writeSet, nullptr, &timeout);
        if (result > 0 && FD_ISSET(tempSocket, &writeSet)) {
            mode = 0;
            ioctlsocket(tempSocket, FIONBIO, &mode);
            return true;
        }

        return false;
#else
        int flags = fcntl(tempSocket, F_GETFL, 0);
        fcntl(tempSocket, F_SETFL, flags | O_NONBLOCK);

        int result = connect(tempSocket, addr, sizeof(sockaddr));
        if (result == -1 && errno != EINPROGRESS) {
            return false;
        }

        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(tempSocket, &writeSet);

        struct timeval timeout;
        timeout.tv_sec = timeoutMs / 1000;
        timeout.tv_usec = (timeoutMs % 1000) * 1000;

        result = select(tempSocket + 1, nullptr, &writeSet, nullptr, &timeout);
        if (result > 0 && FD_ISSET(tempSocket, &writeSet)) {
            int so_error;
            socklen_t len = sizeof(so_error);
            getsockopt(tempSocket, SOL_SOCKET, SO_ERROR, &so_error, &len);
            if (so_error == 0) {
                fcntl(tempSocket, F_SETFL, flags); // restore
                return true;
            }
        }

        return false;
#endif
    }
};

#endif // SOCKETHANDLER_H