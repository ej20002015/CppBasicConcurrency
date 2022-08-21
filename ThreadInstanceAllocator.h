#include <cstdint>
#include <vector>

#pragma once

class ThreadInstanceAllocator
{
public:

    ThreadInstanceAllocator() = delete;
    ThreadInstanceAllocator(const uint64_t problemSize);
    ThreadInstanceAllocator(const uint64_t problemSize, const uint32_t threadCount);

    uint64_t getNextOffset();
    
    void reset();

private:

    std::vector<uint64_t> m_problemSizeOffsets;
    uint32_t m_currentOffsetsIndex = 0;
};