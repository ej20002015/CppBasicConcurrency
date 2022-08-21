#include "ThreadInstanceAllocator.h"

#include <thread>
#include <vector>

ThreadInstanceAllocator::ThreadInstanceAllocator(const uint64_t problemSize)
    : ThreadInstanceAllocator(problemSize, std::thread::hardware_concurrency()) {} 

ThreadInstanceAllocator::ThreadInstanceAllocator(const uint64_t problemSize, const uint32_t threadCount)
    : m_problemSizeOffsets(((threadCount - 1) * 2) + 2)
{
    const uint64_t standardInstanceCountPerThread = problemSize / static_cast<uint64_t>(threadCount);
    const uint64_t leftOverInstanceCount = problemSize % static_cast<uint64_t>(threadCount);

    std::vector<uint64_t> instancesPerThread(threadCount);
    std::fill(instancesPerThread.begin(), instancesPerThread.end(), standardInstanceCountPerThread);

    // Distribute the left over instances amongst the threads

    for (uint64_t i = 0, j = 0; i < leftOverInstanceCount; i++, j = (j + 1) % static_cast<uint64_t>(threadCount))
        instancesPerThread[j]++;

    m_problemSizeOffsets.front() = 0;
    
    uint64_t accumulatedNumber = instancesPerThread[0];
    for (uint32_t i = 0, j = 1; i < threadCount - 1; i++, j += 2)
    {
        m_problemSizeOffsets[j] = accumulatedNumber;
        m_problemSizeOffsets[j + 1] = accumulatedNumber;
        accumulatedNumber += instancesPerThread[i + 1];
    }

    m_problemSizeOffsets.back() = problemSize;
}

uint64_t ThreadInstanceAllocator::getNextOffset()
{
    uint32_t nextOffset = m_problemSizeOffsets[m_currentOffsetsIndex];
    m_currentOffsetsIndex = (m_currentOffsetsIndex + 1) % m_problemSizeOffsets.size();
    return nextOffset;
}

void ThreadInstanceAllocator::reset()
{
    m_currentOffsetsIndex = 0;
}