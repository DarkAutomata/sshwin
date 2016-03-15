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

#ifndef __SSHPROTOCOL_H__
#define __SSHPROTOCOL_H__

#include <windows.h>

#include "sshtypes.h"

typedef struct _SSH_STRING SSH_STRING;
typedef struct _SSH_MPINT SSH_MPINT;
typedef struct _SSH_NAMELIST SSH_NAMELIST;
typedef struct _SSH_PACKET SSH_PACKET;
typedef struct _SSH_TRANSPORT_CONTEXT SSH_TRANSPORT_CONTEXT;

// RFC 4250 - 4.1.2
#define SSH_MSG_DISCONNECT                                  1     // [SSH-TRANS]
#define SSH_MSG_IGNORE                                      2     // [SSH-TRANS]
#define SSH_MSG_UNIMPLEMENTED                               3     // [SSH-TRANS]
#define SSH_MSG_DEBUG                                       4     // [SSH-TRANS]
#define SSH_MSG_SERVICE_REQUEST                             5     // [SSH-TRANS]
#define SSH_MSG_SERVICE_ACCEPT                              6     // [SSH-TRANS]
#define SSH_MSG_KEXINIT                                     20    // [SSH-TRANS]
#define SSH_MSG_NEWKEYS                                     21    // [SSH-TRANS]
#define SSH_MSG_USERAUTH_REQUEST                            50    // [SSH-USERAUTH]
#define SSH_MSG_USERAUTH_FAILURE                            51    // [SSH-USERAUTH]
#define SSH_MSG_USERAUTH_SUCCESS                            52    // [SSH-USERAUTH]
#define SSH_MSG_USERAUTH_BANNER                             53    // [SSH-USERAUTH]
#define SSH_MSG_GLOBAL_REQUEST                              80    // [SSH-CONNECT]
#define SSH_MSG_REQUEST_SUCCESS                             81    // [SSH-CONNECT]
#define SSH_MSG_REQUEST_FAILURE                             82    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_OPEN                                90    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION                   91    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_OPEN_FAILURE                        92    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_WINDOW_ADJUST                       93    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_DATA                                94    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_EXTENDED_DATA                       95    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_EOF                                 96    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_CLOSE                               97    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_REQUEST                             98    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_SUCCESS                             99    // [SSH-CONNECT]
#define SSH_MSG_CHANNEL_FAILURE                             100   // [SSH-CONNECT]

// RFC 4250 - 4.1.2
#define SSH_DISCONNECT_HOST_NOT_ALLOWED_TO_CONNECT          1
#define SSH_DISCONNECT_PROTOCOL_ERROR                       2
#define SSH_DISCONNECT_KEY_EXCHANGE_FAILED                  3
#define SSH_DISCONNECT_RESERVED                             4
#define SSH_DISCONNECT_MAC_ERROR                            5
#define SSH_DISCONNECT_COMPRESSION_ERROR                    6
#define SSH_DISCONNECT_SERVICE_NOT_AVAILABLE                7
#define SSH_DISCONNECT_PROTOCOL_VERSION_NOT_SUPPORTED       8
#define SSH_DISCONNECT_HOST_KEY_NOT_VERIFIABLE              9
#define SSH_DISCONNECT_CONNECTION_LOST                      10
#define SSH_DISCONNECT_BY_APPLICATION                       11
#define SSH_DISCONNECT_TOO_MANY_CONNECTIONS                 12
#define SSH_DISCONNECT_AUTH_CANCELLED_BY_USER               13
#define SSH_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE       14
#define SSH_DISCONNECT_ILLEGAL_USER_NAME                    15

// RFC 4250 - 4.1.3
#define SSH_OPEN_ADMINISTRATIVELY_PROHIBITED                1
#define SSH_OPEN_CONNECT_FAILED                             2
#define SSH_OPEN_UNKNOWN_CHANNEL_TYPE                       3
#define SSH_OPEN_RESOURCE_SHORTAGE                          4

// RFC 4250 - 4.4.2
#define SSH_EXTENDED_DATA_STDERR                            1


// RFC 4253 - 6.1
#define SSH_MAX_PACKET_SIZE                                 (256*1024)

struct _SSH_STRING
{
    uint32 Length;
    uint8* pData;
};

struct _SSH_MPINT
{
    uint32 Length;
    uint8* pData;
};

struct _SSH_NAMELIST
{
    uint32 Length;
    uint8* pData;
    uint32 NameCount;
    uint8** pNames;
};

struct _SSH_PKT_MSG_KEXINIT
{
    uint8 Command;
    uint8 Cookie[16];
    SSH_NAMELIST KexAlgorithms;
    SSH_NAMELIST ServerHostKeyAlgorithms;
    SSH_NAMELIST EncryptionAlgorithmsClient;
    SSH_NAMELIST EncryptionAlgorithmsServer;
    SSH_NAMELIST MacAlgorithmsClient;
    SSH_NAMELIST MacAlgorithmsServer;
    SSH_NAMELIST CompressionAlgorithmsClient;
    SSH_NAMELIST CompressionAlgorithmsServer;
    SSH_NAMELIST LanguagesClient;
    SSH_NAMELIST LanguagesServer;
    uint8 FirstKexPacketFollows;
    uint32 Reserved;
};

struct _SSH_PACKET
{
    uint32 Length;
    uint8* pData;
};

typedef int (*SSH_TRANSPORT_SEND_FUNC) (
    _In_ SSH_TRANSPORT_CONTEXT* pContext,
    _In_reads_bytes_(SendCount) const void* pBuffer,
    _In_ int SendCount
    );

typedef int (*SSH_TRANSPORT_RECV_FUNC) (
    _In_ SSH_TRANSPORT_CONTEXT* pContext,
    _Out_writes_bytes_to_(RecvMax, return) void* pBuffer,
    _In_ int RecvCount
    );
        
struct _SSH_TRANSPORT_CONTEXT
{
    size_t StructureSize;
    const char* TransportName;
    
    SSH_TRANSPORT_SEND_FUNC SendFunc;
    SSH_TRANSPORT_RECV_FUNC RecvFunc;
    
    char ConnectString[256];
    char* pRecvPacket;
    uint32 RecvPacketMax;
    uint32 RecvPacketLen;
};

int
SshTransRunConnection(
    _In_ SSH_TRANSPORT_CONTEXT* pContext
    );

int
SshTransRecvConnectString(
    _In_ SSH_TRANSPORT_CONTEXT* pContext
    );

int
SshTransRecvPacket(
    _In_ SSH_TRANSPORT_CONTEXT* pContext
    );

int
SshTransSendPacket(
    _In_ SSH_TRANSPORT_CONTEXT* pContext,
    _In_reads_bytes_(SendCount) const void* pBuffer,
    _In_ int SendCount
    );

uint32
SshTransDataAsU32(
    _In_reads_(4) char* pData
    );

#endif __SSHPROTOCOL_H__

