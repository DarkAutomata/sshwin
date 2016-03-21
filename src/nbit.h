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

#ifndef __NBIT_H__
#define __NBIT_H__

#include "sshtypes.h"

#define NBIT_RESULT_FAILED(x)       ((x) & 0x80000000)
#define NBIT_RESULT_SUCCEEDED(x)    (! NBIT_RESULT_FAILED(x))

#define NBIT_RESULT_SUCCESS         0x00000000
#define NBIT_RESULT_EQUAL           0x00000001
#define NBIT_RESULT_GREATER         0x00000002
#define NBIT_RESULT_LESS            0x00000003

#define NBIT_RESULT_NO_MEMORY       0x80000001
#define NBIT_RESULT_INVALID_RANGE   0x80000002
#define NBIT_RESULT_BITCOUNT_DIFF   0x80000003
#define NBIT_RESULT_OVERFLOW        0x80000004

typedef uint32 NBIT_RESULT;
typedef struct _NBIT NBIT;
typedef struct _NBIT_CTX NBIT_CTX;

NBIT*
NbitAlloc(
    uint32 BitCount
    );

void
NbitFree(
    _In_ NBIT* pVal
    );

NBIT_CTX*
NbitCtxAlloc(
    uint32 BitCount
    );

void
NbitCtxFree(
    _In_ NBIT_CTX* pContext
    );

NBIT_RESULT
NbitCtxSetModBase(
    _Inout_ NBIT_CTX* pContext,
    _In_opt_ const NBIT* pBase
    );

NBIT*
NbitCopy(
    _In_ const NBIT* pVal,
    _In_ uint32 BitCount
    );

void
NbitZero(
    _Inout_ NBIT* pVal
    );

NBIT_RESULT
NbitAssign(
    _In_ const NBIT* pVal,
    _Inout_ NBIT* pOut
    );

uint32
NbitBitCount(
    _In_ const NBIT* pVal
    );

NBIT_RESULT
NbitTest(
    _In_ const NBIT* pVal0,
    _In_ const NBIT* pVal1
    );

uint32
NbitMsb(
    _In_ const NBIT* pVal
    );

int
NbitIsNeg(
    _In_ const NBIT* pVal
    );

void
NbitNeg(
    _Inout_ NBIT* pVal
    );

void
NbitLs1(
    _Inout_ NBIT* pVal
    );

void
NbitRs1(
    _Inout_ NBIT* pVal
    );

NBIT_RESULT
NbitSetBit(
    _Inout_ NBIT* pVal,
    _In_ uint32 BitIndex
    );

NBIT_RESULT
NbitClrBit(
    _Inout_ NBIT* pVal,
    _In_ uint32 BitIndex
    );

NBIT_RESULT
NbitAdd(
    _In_ NBIT_CTX* pContext,
    _Inout_ NBIT* pVal,
    _In_ const NBIT* pAdd
    );

NBIT_RESULT
NbitSub(
    _In_ NBIT_CTX* pContext,
    _Inout_ NBIT* pVal,
    _In_ const NBIT* pSub
    );

NBIT_RESULT
NbitMul(
    _In_ NBIT_CTX* pContext,
    _Inout_ NBIT* pVal,
    _In_ const NBIT* pMultiplier
    );

NBIT_RESULT
NbitDiv(
    _In_ NBIT_CTX* pContext,
    _Inout_ NBIT* pVal,
    _In_ const NBIT* pDivisor,
    _Inout_opt_ NBIT* pRemainder
    );

#endif  // __NBIT_H__

