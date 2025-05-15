#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <vector>
#include <stdexcept>

#include "raii/RAIISockets.h"

class SocketHandler
{
private:
    int totalReceived;
    int bufferCapacity;
    std::vector<char> buffer;
public:
    static constexpr size_t DEFAULT_BUFLEN = 512;
    static constexpr DWORD timeoutMs = 5000;
    std::string_view getDataBuffer() const;
    void recvWrite(int iResult, const char* recvbuf);
    void resetDataBuffer();
    bool connectWithTimeout(SocketRAII& tempSocket, sockaddr* addr, int timeoutMs);
    int getTotalReceived() const;
};

#endif