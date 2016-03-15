/*
Copyright (c) 2016, Jonathan Ward
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "sshtransport.h"

int InitWinsock(
    );

int FreeWinsock(
    );

void Usage(
    );

// TODO: Move these to their own files.
typedef struct _SSH_TCP_TRANSPORT_CONTEXT
{
    SSH_TRANSPORT_CONTEXT Base;
    SOCKET Socket;
} SSH_TCP_TRANSPORT_CONTEXT;

int SshTcpSend(
    _In_ SSH_TRANSPORT_CONTEXT* pContext,
    _In_reads_bytes_(SendCount) const void* pBuffer,
    _In_ int SendCount
    );

int SshTcpRecv(
    _In_ SSH_TRANSPORT_CONTEXT* pContext,
    _Out_writes_bytes_to_(RecvMax, return) void* pBuffer,
    _In_ int RecvCount
    );

int main(
    int argc,
    char* argv[])
{
    SOCKET sock = INVALID_SOCKET;
    struct addrinfo* pAddrLookup = NULL;
    struct addrinfo addressHints = {0};
    struct addrinfo* pAddrUse = NULL;
    SSH_TCP_TRANSPORT_CONTEXT tcpContext = {0};
    
    DWORD dwResult = 0;
    int result = 0;
    
    InitWinsock();
    
    if (argc < 2)
    {
        Usage();
        result = -1;
        goto Cleanup;
    }
    
    addressHints.ai_family = AF_UNSPEC;
    addressHints.ai_socktype = SOCK_STREAM;
    addressHints.ai_protocol = IPPROTO_TCP;
    
    dwResult = getaddrinfo(argv[1], NULL, &addressHints, &pAddrLookup);
    if (dwResult != ERROR_SUCCESS)
    {
        fprintf(stdout, "getaddrinfo failed: %08X\n", dwResult);
        result = -1;
        goto Cleanup;
    }
    else
    {
        struct addrinfo* pAddr;
        
        for (pAddr = pAddrLookup; pAddr != NULL; pAddr = pAddr->ai_next)
        {
            if (pAddr->ai_family == AF_INET)
            {
                pAddrUse = pAddr;
                ((struct sockaddr_in*) pAddrUse->ai_addr)->sin_port = htons(22);
                break;
            }
            else if (pAddr->ai_family == AF_INET6)
            {
                pAddrUse = pAddr;
                ((struct sockaddr_in6*) pAddrUse->ai_addr)->sin6_port = htons(22);
            }
        }
    }
    
    sock = socket(pAddrUse->ai_family, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stdout, "Socket failed\n");
        result = -1;
        goto Cleanup;
    }
    
    // Connect to the address.
    result = connect(sock, pAddrUse->ai_addr, (int)pAddrUse->ai_addrlen);
    if (result != 0)
    {
        fprintf(stdout, "Connecet failed\n");
        goto Cleanup;
    }
    
    tcpContext.Base.StructureSize = sizeof(tcpContext);
    tcpContext.Base.TransportName = "tcpip";
    tcpContext.Base.SendFunc = SshTcpSend;
    tcpContext.Base.RecvFunc = SshTcpRecv;
    tcpContext.Socket = sock;
    
    SshTransRunConnection(&tcpContext.Base);
    
Cleanup:
    if (sock)
    {
        closesocket(sock);
    }
    
    if (pAddrLookup)
    {
        freeaddrinfo(pAddrLookup);
    }
    
    FreeWinsock();
    
    return 0;
}

int InitWinsock(
    )
{
    int result;
    WSADATA winsockData = {0};
    
    result = WSAStartup(MAKEWORD(2,2), &winsockData);
    if (result != NO_ERROR)
    {
        return 0;
    }
    
    return -1;
}

int FreeWinsock(
    )
{
    WSACleanup();
    
    return -1;
}

void Usage(
    )
{
    fprintf(stdout, "USAGE: ssh.exe user@hostname\n");
}

int SshTcpSend(
    _In_ SSH_TRANSPORT_CONTEXT* pContext_,
    _In_reads_bytes_(SendCount) const void* pBuffer,
    _In_ int SendCount
    )
{
    SSH_TCP_TRANSPORT_CONTEXT* pContext = (SSH_TCP_TRANSPORT_CONTEXT*) pContext_;
    
    return send(pContext->Socket, pBuffer, SendCount, 0);
}

int SshTcpRecv(
    _In_ SSH_TRANSPORT_CONTEXT* pContext_,
    _Out_writes_bytes_to_(RecvMax, return) void* pBuffer,
    _In_ int RecvCount
    )
{
    SSH_TCP_TRANSPORT_CONTEXT* pContext = (SSH_TCP_TRANSPORT_CONTEXT*) pContext_;
    
    return recv(pContext->Socket, pBuffer, RecvCount, 0);
}


