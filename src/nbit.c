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
#include "nbit.h"

#define NBIT_ELEMENT_BITS       32
#define NBIT_LOWER_BIT          1
#define NBIT_UPPER_BIT          (NBIT_LOWER_BIT << (NBIT_ELEMENT_BITS-1))

typedef uint32 NBIT_ELEMENT;

struct _NBIT
{
    uint32 BitCount;
    NBIT_ELEMENT Bits[1];
};

#define NBIT_CTX_MUL_TMP_COUNT  2

struct _NBIT_CTX
{
    uint32 BitCount;
    NBIT* pModBase;
    NBIT* pMulTmp[NBIT_CTX_MUL_TMP_COUNT];
};

// ----------------------------------------------------------------------------
// Private functions
// ----------------------------------------------------------------------------
void
NbitAdjustForBase(
    _In_ NBIT_CTX* pContext,
    _Inout_ NBIT* pValue
    );

NBIT*
NbitAlloc(
    uint32 BitCount
    )
{
    NBIT* pResult;
    
    if (BitCount & (NBIT_ELEMENT_BITS-1))
    {
        BitCount += (NBIT_ELEMENT_BITS-1);
    }
    
    BitCount &= (~(NBIT_ELEMENT_BITS-1));
    
    pResult = (NBIT*) malloc(
            sizeof(*pResult) +
            (sizeof(NBIT_ELEMENT) * (BitCount / NBIT_ELEMENT_BITS)));
    
    if (pResult)
    {
        uint32 i;
        uint32 elementCount;
        
        pResult->BitCount = BitCount;
        
        elementCount = pResult->BitCount / NBIT_ELEMENT_BITS;
        
        for (i = 0; i < elementCount; i++)
        {
            pResult->Bits[i] = 0;
        }
    }
    
    return pResult;
}

void
NbitFree(
    _In_ NBIT* pVal
    )
{
    free(pVal);
}

NBIT_CTX*
NbitCtxAlloc(
    _In_ uint32 BitCount
    )
{
    NBIT_RESULT status = NBIT_RESULT_SUCCESS;
    NBIT_CTX* pContext = NULL;
    uint32 i;
    
    pContext = (NBIT_CTX*) malloc(sizeof(*pContext));
    if (! pContext)
    {
        status = NBIT_RESULT_NO_MEMORY;
        goto Cleanup;
    }
    
    memset(pContext, 0, sizeof(*pContext));
    
    pContext->BitCount = BitCount;
    
    for (i = 0; i < NBIT_CTX_MUL_TMP_COUNT; i++)
    {
        pContext->pMulTmp[i] = NbitAlloc(BitCount);
        
        if (! pContext->pMulTmp[i])
        {
            status = NBIT_RESULT_NO_MEMORY;
            goto Cleanup;
        }
    }
    
Cleanup:
    
    if (status != NBIT_RESULT_SUCCESS)
    {
        if (pContext)
        {
            NbitCtxFree(pContext);
        }
    }
    
    return pContext;
}

void
NbitCtxFree(
    _In_ NBIT_CTX* pContext
    )
{
    uint32 i;
    
    for (i = 0; i < NBIT_CTX_MUL_TMP_COUNT; i++)
    {
        NbitFree(pContext->pMulTmp[i]);
        pContext->pMulTmp[i] = NULL;
    }
    
    if (pContext->pModBase)
    {
        NbitFree(pContext->pModBase);
        pContext->pModBase = NULL;
    }
    
    free(pContext);
}

NBIT_RESULT
NbitCtxSetModBase(
    _Inout_ NBIT_CTX* pContext,
    _In_opt_ const NBIT* pBase
    )
{
    if (pContext->pModBase)
    {
        NbitFree(pContext->pModBase);
        pContext->pModBase = NULL;
    }
    
    if (pBase)
    {
        if (pBase->BitCount != pContext->BitCount)
        {
            return NBIT_RESULT_BITCOUNT_DIFF;
        }
        
        pContext->pModBase = NbitCopy(pBase, pContext->BitCount);
        if (! pContext->pModBase)
        {
            return NBIT_RESULT_NO_MEMORY;
        }
    }
    
    return NBIT_RESULT_SUCCESS;
}

NBIT*
NbitCopy(
    _In_ const NBIT* pVal,
    _In_ uint32 BitCount
    )
{
    NBIT* pResult;
    
    if (pVal->BitCount > BitCount)
    {
        return NULL;
    }
    
    pResult = (NBIT*) malloc(
            sizeof(*pResult) +
            (sizeof(NBIT_ELEMENT) * (BitCount / NBIT_ELEMENT_BITS)));
    
    if (pResult)
    {
        uint32 i;
        uint32 elementCount;
        NBIT_ELEMENT valSign;
        
        pResult->BitCount = pVal->BitCount;
        
        elementCount = pVal->BitCount / NBIT_ELEMENT_BITS;
        
        for (i = 0; i < elementCount; i++)
        {
            pResult->Bits[i] = pVal->Bits[i];
        }
        
        elementCount = pResult->BitCount / NBIT_ELEMENT_BITS;
        
        // Sign extend.
        if (pVal->Bits[pVal->BitCount / NBIT_ELEMENT_BITS] & NBIT_UPPER_BIT)
        {
            valSign = ~((NBIT_ELEMENT) 0);
        }
        else
        {
            valSign = 0;
        }
        
        for (; i < elementCount; i++)
        {
            pResult->Bits[i] = valSign;
        }
    }
    
    return pResult;
}

void
NbitZero(
    _Inout_ NBIT* pVal
    )
{
    uint32 i;
    uint32 elementCount;
    
    elementCount = pVal->BitCount / NBIT_ELEMENT_BITS;
    
    for (i = 0; i < elementCount; i++)
    {
        pVal->Bits[i] = 0;
    }
}

NBIT_RESULT
NbitAssign(
    _In_ const NBIT* pVal,
    _Inout_ NBIT* pOut
    )
{
    uint32 i;
    uint32 elementCount;
    
    if (pVal->BitCount != pOut->BitCount)
    {
        return NBIT_RESULT_BITCOUNT_DIFF;
    }
    
    elementCount = pVal->BitCount / NBIT_ELEMENT_BITS;
    
    for (i = 0; i < elementCount; i++)
    {
        pOut->Bits[i] = pVal->Bits[i];
    }
    
    return NBIT_RESULT_SUCCESS;
}

uint32
NbitBitCount(
    _In_ const NBIT* pVal
    )
{
    return pVal->BitCount;
}

NBIT_RESULT
NbitTest(
    _In_ const NBIT* pVal0,
    _In_ const NBIT* pVal1
    )
{
    NBIT_RESULT result = NBIT_RESULT_EQUAL;
    uint32 i;
    int val0Neg = 0;
    int val1Neg = 0;
    
    if (pVal0->BitCount != pVal1->BitCount)
    {
        return NBIT_RESULT_BITCOUNT_DIFF;
    }
    
    val0Neg = NbitIsNeg(pVal0);
    val1Neg = NbitIsNeg(pVal1);
    
    if ((val0Neg) && (!val1Neg))
    {
        return NBIT_RESULT_LESS;
    }
    
    if ((!val0Neg) && (val1Neg))
    {
        return NBIT_RESULT_GREATER;
    }
    
    i = pVal0->BitCount;
    
    do
    {
        i--;
        
        if (pVal0->Bits[i] > pVal1->Bits[i])
        {
            result = NBIT_RESULT_GREATER;
            break;
        }
        else if (pVal0->Bits[i] < pVal1->Bits[i])
        {
            result = NBIT_RESULT_GREATER;
            break;
        }
    } while (i > 0);
    
    // Swap the evaluation if both are negative.
    if ((val0Neg) && (val1Neg))
    {
        if (result == NBIT_RESULT_GREATER)
        {
            result = NBIT_RESULT_LESS;
        }
        else if (result == NBIT_RESULT_LESS)
        {
            result = NBIT_RESULT_GREATER;
        }
    }
    
    return result;
}

uint32
NbitMsb(
    _In_ const NBIT* pVal
    )
{
    uint32 testVal;
    uint32 i;
    uint32 bitIndex;
    
    if (NbitIsNeg(pVal))
    {
        testVal = ~((NBIT_ELEMENT)(0));
    }
    else
    {
        testVal = 0;
    }
    
    i = (pVal->BitCount / NBIT_ELEMENT_BITS);
    
    do
    {
        i--;
        
        if (pVal->Bits[i] != testVal)
        {
            break;
        }
    } while (i > 0);
    
    // Look at the highest bit index first.
    bitIndex = NBIT_ELEMENT_BITS;
    
    do
    {
        bitIndex--;
        
        if (((pVal->Bits[i] >> bitIndex) & NBIT_LOWER_BIT) != (testVal & NBIT_LOWER_BIT))
        {
            break;
        }
    } while (bitIndex > 0);
    
    return (i * NBIT_ELEMENT_BITS) + bitIndex;
}

int
NbitIsNeg(
    _In_ const NBIT* pVal
    )
{
    if (pVal->Bits[(pVal->BitCount / NBIT_ELEMENT_BITS)-1] & NBIT_UPPER_BIT)
    {
        return 1;
    }
    
    return 0;
}

void
NbitNeg(
    _Inout_ NBIT* pVal
    )
{
    uint32 i;
    uint32 elementCount;
    
    NBIT_ELEMENT carry = 1;
    
    elementCount = pVal->BitCount / NBIT_ELEMENT_BITS;
    
    // Invert + 1.
    for (i = 0; i < elementCount; i++)
    {
        pVal->Bits[i] = ~pVal->Bits[i];
        
        if (carry)
        {
            pVal->Bits[i] ++;
            
            if (pVal->Bits[i] != 0)
            {
                carry = 0;
            }
        }
    }
}

void
NbitLs1(
    _Inout_ NBIT* pVal
    )
{
    uint32 i;
    uint32 elementCount;
    
    NBIT_ELEMENT shift = 0;
    
    elementCount = pVal->BitCount / NBIT_ELEMENT_BITS;
    
    for (i = 0; i < elementCount; i++)
    {
        shift |= pVal->Bits[i] & NBIT_UPPER_BIT;
        
        pVal->Bits[i] = 
            (pVal->Bits[i] << 1) |
            (shift & NBIT_LOWER_BIT);
        
        shift >>= NBIT_ELEMENT_BITS-1;
    }
}

void
NbitRs1(
    _Inout_ NBIT* pVal
    )
{
    uint32 i;
    
    NBIT_ELEMENT shift = 0;
    
    i = pVal->BitCount / NBIT_ELEMENT_BITS;
    
    do
    {
        i--;
        
        shift |= pVal->Bits[i] & NBIT_LOWER_BIT;
        
        pVal->Bits[i] =
            (pVal->Bits[i] >> 1) |
            (shift & NBIT_UPPER_BIT);
        
        shift <<= NBIT_ELEMENT_BITS-1;
    } while (i > 0);
}

NBIT_RESULT
NbitSetBit(
    _Inout_ NBIT* pVal,
    _In_ uint32 BitIndex
    )
{
    uint32 elementIndex;
    
    if (BitIndex >= pVal->BitCount)
    {
        return NBIT_RESULT_INVALID_RANGE;
    }
    
    elementIndex = BitIndex / NBIT_ELEMENT_BITS;
    BitIndex = BitIndex & (NBIT_ELEMENT_BITS-1);
    
    pVal->Bits[elementIndex] |= (1 << BitIndex);
    
    return NBIT_RESULT_SUCCESS;
}

NBIT_RESULT
NbitClrBit(
    _Inout_ NBIT* pVal,
    _In_ uint32 BitIndex
    )
{
    uint32 elementIndex;
    
    if (BitIndex >= pVal->BitCount)
    {
        return NBIT_RESULT_INVALID_RANGE;
    }
    
    elementIndex = BitIndex / NBIT_ELEMENT_BITS;
    BitIndex = BitIndex & (NBIT_ELEMENT_BITS-1);
    
    pVal->Bits[elementIndex] &= ~(1 << BitIndex);
    
    return NBIT_RESULT_SUCCESS;
}

NBIT_RESULT
NbitAdd(
    _In_ NBIT_CTX* pContext,
    _Inout_ NBIT* pVal,
    _In_ const NBIT* pAdd
    )
{
    uint32 i;
    uint32 elementCount;
    uint32 carry = 0;
    
    UNREFERENCED_PARAMETER(pContext);
    
    if (pVal->BitCount != pAdd->BitCount)
    {
        return NBIT_RESULT_BITCOUNT_DIFF;
    }
    
    elementCount = pVal->BitCount / NBIT_ELEMENT_BITS;
    
    for (i = 0; i < elementCount; i++)
    {
        NBIT_ELEMENT preVal;
        
        if (carry)
        {
            pVal->Bits[i]++;
            
            if (pVal->Bits[i] != 0)
            {
                carry = 0;
            }
        }
        
        preVal = pVal->Bits[i];
        
        pVal->Bits[i] += pAdd->Bits[i];
        
        if (pVal->Bits[i] < preVal)
        {
            carry = 1;
        }
    }
    
    return NBIT_RESULT_SUCCESS;
}

NBIT_RESULT
NbitSub(
    _In_ NBIT_CTX* pContext,
    _Inout_ NBIT* pVal,
    _In_ const NBIT* pSub
    )
{
    uint32 elementCount;
    uint32 borrow = 0;
    uint32 i;
    
    UNREFERENCED_PARAMETER(pContext);
    
    if (pVal->BitCount != pSub->BitCount)
    {
        return NBIT_RESULT_BITCOUNT_DIFF;
    }
    
    elementCount = pVal->BitCount / NBIT_ELEMENT_BITS;
    
    for (i = 0; i < elementCount; i++)
    {
        NBIT_ELEMENT preVal;
        
        if (borrow)
        {
            if (pVal->Bits[i] != 0)
            {
                borrow = 0;
            }
            
            pVal->Bits[i]--;
        }
        
        preVal = pVal->Bits[i];
        
        pVal->Bits[i] -= pSub->Bits[i];
        
        if (pVal->Bits[i] > preVal)
        {
            borrow = 1;
        }
    }
    
    return NBIT_RESULT_SUCCESS;
}

NBIT_RESULT
NbitMul(
    _In_ NBIT_CTX* pContext,
    _In_ NBIT* pVal,
    _In_ const NBIT* pMultiplier
    )
{
    NBIT_RESULT result = NBIT_RESULT_SUCCESS;
    uint32 i;
    int val0Neg = 0;
    int val1Neg = 0;
    uint32 elementCount;
    
    if ((pVal->BitCount != pMultiplier->BitCount) ||
        (pVal->BitCount != pContext->BitCount))
    {
        return NBIT_RESULT_BITCOUNT_DIFF;
    }
    
    // Use the context to make copies of Val0 and Val1.
    NbitAssign(pVal, pContext->pMulTmp[0]);
    if (NbitIsNeg(pContext->pMulTmp[0]))
    {
        val0Neg = 1;
        NbitNeg(pContext->pMulTmp[0]);
    }
    
    NbitAssign(pMultiplier, pContext->pMulTmp[1]);
    if (NbitIsNeg(pContext->pMulTmp[1]))
    {
        val1Neg = 1;
        NbitNeg(pContext->pMulTmp[1]);
    }
    
    NbitZero(pVal);
    
    elementCount = pContext->pMulTmp[0]->BitCount / NBIT_ELEMENT_BITS;
    
    // Iterate over the bits of pMulTmp[0].  For every set bit add the value
    // of pMulTmp[1] to pResult.  For each iteration shift pMulTmp[1] left by
    // 1.
    for (i = 0; i < elementCount; i++)
    {
        uint32 j;
        
        for (j = 0; j < NBIT_ELEMENT_BITS; j++)
        {
            if (pContext->pMulTmp[0]->Bits[i] & (1 << j))
            {
                // Check for overflow.
                if (pContext->pMulTmp[1]->Bits[elementCount-1] & NBIT_UPPER_BIT)
                {
                    result = NBIT_RESULT_OVERFLOW;
                    goto Cleanup;
                }
                
                // Add pMulTmp[1] to pResult.
                NbitAdd(pContext, pVal, pContext->pMulTmp[1]);
                
                // Check for overflow.
                if (pVal->Bits[elementCount-1] & NBIT_UPPER_BIT)
                {
                    result = NBIT_RESULT_OVERFLOW;
                    goto Cleanup;
                }
            }
            
            // Shift pMulTmp[1] left by one.
            NbitLs1(pContext->pMulTmp[1]);
        }
    }
    
    // Set appropriate sign.
    if (val0Neg != val1Neg)
    {
        NbitNeg(pVal);
    }
    
Cleanup:
    
    return result;
}

NBIT_RESULT
NbitDiv(
    _In_ NBIT_CTX* pContext,
    _Inout_ NBIT* pVal,
    _In_ const NBIT* pDivisor,
    _Inout_opt_ NBIT* pRemainder
    )
{
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(pVal);
    UNREFERENCED_PARAMETER(pDivisor);
    UNREFERENCED_PARAMETER(pRemainder);
    
    return NBIT_RESULT_OVERFLOW;
}

