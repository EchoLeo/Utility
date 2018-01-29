#pragma once
//--------------------------------------------------------------------------------------------------
// This class represents a simple memory pool. This memory pool is split into chunks of block, which
// include a pointer point to the next block and a data block of memory that used to save the data. 
// However, the pointer is not necessary, because we can use part of the data block to store the
// pointer as if the data block is big enought(4 bytes) to store a pointer. To do this, we can change
// the value of CHUNK_HEADER_SIZE(in MemoryPool.cpp) to zero. 
// 
// The memory is a 2-d array of memory of blocks. Each row contains (m_numChunks) blocks, each block 
// each block's size is represent by m_chunkSize.
//
// If there is no free memory and allow to resize the memory pool(represent by member m_allowResize),
// we will grow memory pool with a new row.
//
// After you create a MemoryPool object you must initialize it (via the Init() function) with a chunk
// size and the number of chuncks you want created. 
//
// Look for more details in this article :  
//--------------------------------------------------------------------------------------------------
#include <string>

class MemoryPool
{
    // the memory is a 2-d array of memory of blocks
    unsigned char** m_ppRawMemoryArray;
    // the mumber of row in the memory pool array
    unsigned int m_memArraySize;
    // the size of each chunk and number of chunks each row, respectively
    unsigned int m_chunkSize, m_numChunks;

    // the head of the free memory blocks linked list
    unsigned char *m_pHead;

    // when memory pool is fills up, true if we can resize memory pool
    bool m_allowResize;

    // tracking variables we only care about for debug
#ifdef _DEBUG
    std::string m_memPoolName;
    // record alloc info
    unsigned long m_allocPeak, m_numAllocs;
#endif

public:
    MemoryPool();
    ~MemoryPool();
    MemoryPool(const MemoryPool& memPool) = delete;
    bool Init(unsigned int chunkSize, unsigned int numChunks);
    void Destroy();

    void* Alloc();
    void Free(void* pMem);
    unsigned int GetChunkSize() const { return m_chunkSize; }
    
    void SetAllowResize(bool allowResize) { m_allowResize = allowResize; }

#ifdef _DEBUG
    void SetMemoryPoolName(const char* name) { m_memPoolName = name; }
    std::string GetMemoryPoolName() const { return m_memPoolName; }
#else
    void SetMemoryPoolName(const char* name) {  }
    const char* GetMemoryPoolName() const { return "<No MemoryPoolName>" } 
#endif

private:
    // Reset the internal vars
    void Reset();

    // internal memory allocation helpers
    bool GrowMemoryArray();
    unsigned char* AllocateNewMemoryBlock();

    //internal linked list management
    unsigned char* GetNext(unsigned char* pBlock);
    void SetNext(unsigned char* pBlockToChange, unsigned char* pNewNext);
};