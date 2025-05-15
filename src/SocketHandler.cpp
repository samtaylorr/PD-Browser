#include "SocketHandler.h"
#include <iostream>

void SocketHandler::resetDataBuffer()
{
    buffer.clear();
    buffer.reserve(512); // reserve capacity upfront
    totalReceived = 0;
}

void SocketHandler::recvWrite(int iResult, const char* recvbuf)
{
    if (iResult <= 0) return;

    // Append the received data to the vector buffer
    buffer.insert(buffer.end(), recvbuf, recvbuf + iResult);

    // Update totalReceived if you want to keep it (optional)
    totalReceived = static_cast<int>(buffer.size());
}

std::string_view SocketHandler::getDataBuffer() const
{
    return buffer.data();
}

int SocketHandler::getTotalReceived() const
{
    return totalReceived;
}

bool SocketHandler::connectWithTimeout(SocketRAII& tempSocket, sockaddr* addr, int timeoutMs)
{
    // Set socket to non-blocking
    u_long nonBlocking = 1;
    ioctlsocket(tempSocket.sock, FIONBIO, &nonBlocking);

    int result = connect(tempSocket.sock, addr, static_cast<int>(sizeof(sockaddr_in))); // careful here

    if (result == SOCKET_ERROR) {
        int lastErr = WSAGetLastError();
        if (lastErr != WSAEWOULDBLOCK && lastErr != WSAEINPROGRESS) {
            // Immediate failure
            return false;
        }

        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(tempSocket.sock, &writeSet);

        timeval timeout{};
        timeout.tv_sec = timeoutMs / 1000;
        timeout.tv_usec = (timeoutMs % 1000) * 1000;

        result = select(0, nullptr, &writeSet, nullptr, &timeout);
        if (result <= 0) {
            // Timeout or select error
            return false;
        }

        // Check for socket error after select says writable
        int optVal = 0;
        int optLen = sizeof(optVal);
        if (getsockopt(tempSocket.sock, SOL_SOCKET, SO_ERROR, (char*)&optVal, &optLen) == SOCKET_ERROR) {
            return false;
        }
        if (optVal != 0) {
            // Connect failed
            return false;
        }
    } else if (result != 0) {
        // Unexpected error
        return false;
    }

    // Connected successfully: set back to blocking mode
    u_long blocking = 0;
    ioctlsocket(tempSocket.sock, FIONBIO, &blocking);

    return true;
}