#include "MemoryPool.h"

const static size_t CHUNK_HEADER_SIZE = sizeof(unsigned char*);

MemoryPool::MemoryPool()
{
    Reset();
}

void MemoryPool::Reset()
{
    m_ppRawMemoryArray = nullptr;
    m_memArraySize = 0;
    m_chunkSize = 0;
    m_numChunks = 0;
    m_pHead = nullptr;
    m_allowResize = true;

#ifdef _DEBUG
    m_allocPeak = 0;
    m_numAllocs = 0;
#endif
}

MemoryPool::~MemoryPool()
{
    Destroy();
}

void MemoryPool::Destroy()
{
#ifdef _DEBUG
    // record the number chunks to the logger.
#endif

    for (size_t i = 0; i < m_memArraySize; ++i)
    {
        free(m_ppRawMemoryArray[i]);
    }
    free(m_ppRawMemoryArray);

    Reset();
}

bool MemoryPool::Init(unsigned int chunkSize, unsigned int numChunks)
{
    if (m_ppRawMemoryArray)
        Destroy();

    m_chunkSize = chunkSize;
    m_numChunks = numChunks;

    return GrowMemoryArray();
}

bool MemoryPool::GrowMemoryArray()
{
#ifdef _DEBUG
    //std::string str("Growing memory pool : [" + GetMemoryPoolName() + " : " + m_chunkSize + "] = " + (m_memArraySize + 1) + "\n");
    // output the str to logger.
#endif

    size_t allocationSize = sizeof(unsigned char*) * (m_memArraySize + 1);
    unsigned char** ppNewMemArray = (unsigned char**)malloc(allocationSize);

    if (!ppNewMemArray)
        return false;
    
    for (size_t i = 0; i < m_memArraySize; ++i)
        ppNewMemArray[i] = m_ppRawMemoryArray[i];
    
    ppNewMemArray[m_memArraySize] = AllocateNewMemoryBlock();

    if (m_pHead)
    {
        unsigned char* pCurr = m_pHead;
        unsigned char* pNext = GetNext(m_pHead);
        while (pNext)
        {
            pCurr = pNext;
            pNext = GetNext(pCurr);
        }
        SetNext(pCurr, ppNewMemArray[m_memArraySize]);
    }
    else
    {
        m_pHead = ppNewMemArray[m_memArraySize];
    }

    if (m_ppRawMemoryArray)
        free(m_ppRawMemoryArray);

    m_ppRawMemoryArray = ppNewMemArray;
    ++m_memArraySize;

    return true;
}

unsigned char* MemoryPool::AllocateNewMemoryBlock()
{
    size_t blockSize = m_chunkSize + CHUNK_HEADER_SIZE;
    size_t trueSize = blockSize * m_numChunks;

    unsigned char* pNewMem = (unsigned char*) malloc(trueSize);
    if (!pNewMem)
        return nullptr;
    
    unsigned char* pEnd = pNewMem + trueSize;
    unsigned char* pCurr = pNewMem;

    while (pCurr < pEnd)
    {
        unsigned char* pNext = pCurr + blockSize;

        unsigned char** ppChunkHeader = (unsigned char**)pCurr;
        ppChunkHeader[0] = (pNext < pEnd ? pNext : nullptr);

        pCurr += blockSize;
    }
    return pNewMem;
}

unsigned char* MemoryPool::GetNext(unsigned char* pBlock)
{
    unsigned char** ppChunkHeader = (unsigned char**)pBlock;
    return ppChunkHeader[0];
}

void MemoryPool::SetNext(unsigned char* pBlockToChange, unsigned char* pNewNext)
{
    unsigned char** ppChunkHeader = (unsigned char**)pBlockToChange;
    ppChunkHeader[0] = pNewNext;
}

void* MemoryPool::Alloc()
{
    if (!m_pHead && (!m_allowResize || !GrowMemoryArray()) )
    {
        return nullptr;
    }

#ifdef _DEBUG
    ++m_numAllocs;
    m_allowResize = m_numAllocs > m_allocPeak ? m_numAllocs : m_allocPeak;
#endif
    unsigned char* pRet = m_pHead;
    m_pHead = GetNext(m_pHead);
    return pRet + CHUNK_HEADER_SIZE;
}

void MemoryPool::Free(void* pMem)
{
    if (pMem)
    {
        unsigned char* pBlock = ((unsigned char*)pMem) - CHUNK_HEADER_SIZE;
        SetNext(pBlock, m_pHead);
        m_pHead = pBlock;
#ifdef _DEBUG
    --m_numAllocs;
#endif
    }
}