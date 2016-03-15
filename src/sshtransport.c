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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sshtransport.h"

#define SSH_TRANSPORT_CONNECT_STRING        "SSH-2.0-winssh_0.1 A Work in progress.\r\n"
#define SSH_TRANSPORT_CONNECT_STRING_LEN    ((int)(strlen(SSH_TRANSPORT_CONNECT_STRING)))

int
SshTransRunConnection(
    _In_ SSH_TRANSPORT_CONTEXT* pContext
    )
{
    int result = 0;
    
    // Send our connection string.
    result = pContext->SendFunc(
            pContext,
            SSH_TRANSPORT_CONNECT_STRING,
            SSH_TRANSPORT_CONNECT_STRING_LEN);
    
    if (result != SSH_TRANSPORT_CONNECT_STRING_LEN)
    {
        fprintf(stderr, "CONN: Send Connection String Failed\n");
        goto Cleanup;
    }
    
    // Get connection string from remote.
    result = SshTransRecvConnectString(pContext);
    if (result <= 0)
    {
        fprintf(stderr, "CONN: Get Connection String Failed\n");
        goto Cleanup;
    }
    
    fprintf(stderr, "VERSION: %s\n", pContext->ConnectString);
    
    // Get first protocol packet.
    result = SshTransRecvPacket(pContext);
    
    // Send first protocol packet.
    
Cleanup:
    
    return result;
}

int
SshTransRecvConnectString(
    _In_ SSH_TRANSPORT_CONTEXT* pContext
    )
{
    int count = 0;
    int found = 0;
    
    while ((! found) && (count < sizeof(pContext->ConnectString)))
    {
        int readCount = pContext->RecvFunc(pContext, &pContext->ConnectString[count], 1);
        
        if (readCount == 1)
        {
            count++;
            
            if (count >= 2)
            {
                if ((pContext->ConnectString[count-2] == '\r') &&
                    (pContext->ConnectString[count-1] == '\n'))
                {
                    pContext->ConnectString[count-2] = 0;
                    found = 1;
                }
            }
        }
        else
        {
            return -1;
        }
    }
    
    return count;
}

int
SshTransRecvPacket(
    _In_ SSH_TRANSPORT_CONTEXT* pContext
    )
{
    int result = 0;
    uint32 packetSize = 0;
    
    pContext->RecvPacketLen = 0;
    
    for (;;)
    {
        int readCount;
        
        if ((pContext->RecvPacketLen + 4) >= pContext->RecvPacketMax)
        {
            // More packet please.
            int newSize;
            char* pNewBuffer;
            
            newSize = pContext->RecvPacketMax * 2;
            if (newSize < 4096)
            {
                newSize = 4096;
            }
            
            pNewBuffer = (char*) malloc(newSize);
            
            if ((pContext->RecvPacketMax > 0) &&
                (pContext->pRecvPacket))
            {
                memcpy(pNewBuffer, pContext->pRecvPacket, pContext->RecvPacketMax);
            }
            
            if (pContext->pRecvPacket)
            {
                free(pContext->pRecvPacket);
            }
            
            pContext->pRecvPacket = pNewBuffer;
            pContext->RecvPacketMax = newSize;
        }
        
        // Read packet size first.
        if (pContext->RecvPacketLen == 0)
        {
            readCount = pContext->RecvFunc(pContext, &pContext->pRecvPacket[0], 4);
            
            if (readCount != 4)
            {
                result = -1;
                break;
            }
            
            packetSize = SshTransDataAsU32(pContext->pRecvPacket);
            pContext->RecvPacketLen += readCount;
            
            if (packetSize > SSH_MAX_PACKET_SIZE)
            {
                result = -1;
                break;
            }
        }
        else
        {
            int requestCount = (int)(packetSize - pContext->RecvPacketLen);
            
            readCount = pContext->RecvFunc(
                    pContext,
                    &pContext->pRecvPacket[pContext->RecvPacketLen],
                    requestCount);
            
            if (readCount <= 0)
            {
                result = -1;
                break;
            }
            
            pContext->RecvPacketLen += readCount;
        }
        
        if (packetSize == pContext->RecvPacketLen)
        {
            result = (int) pContext->RecvPacketLen;
            break;
        }
    }
    
    return result;
}

int
SshTransSendPacket(
    _In_ SSH_TRANSPORT_CONTEXT* pContext,
    _In_reads_bytes_(SendCount) const void* pBuffer,
    _In_ int SendCount
    )
{
    return pContext->SendFunc(pContext, pBuffer, SendCount);
}

uint32
SshTransDataAsU32(
    _In_reads_(4) char* pData
    )
{
    uint32 result = 0;
    // TODO: hton properly.
    
    result =  (((uint32) pData[3]) <<  0) & 0x000000FF;
    result |= (((uint32) pData[2]) <<  8) & 0x0000FF00;
    result |= (((uint32) pData[1]) << 16) & 0x00FF0000;
    result |= (((uint32) pData[0]) << 24) & 0xFF000000;
    
    return result;
}


