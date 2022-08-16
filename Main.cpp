#include <iostream>
#include <array>
#include <algorithm>
#include <span>
#include <thread>
#include <chrono>

#include "ThreadInstanceAllocator.h"

static void printFirstNElements(std::span<uint32_t> elements, uint32_t n, std::string_view containerName)
{
    uint32_t bound = (n > elements.size()) ? elements.size() : n;

    std::cout << "First " << bound << " elements of " << containerName << ":" << std::endl;

    uint32_t i;
    for (i = 0; i < bound - 1; i++)
        std::cout << elements[i] << ", ";

    std::cout << elements[i] << "\n" << std::endl;
}

static void performVectorAddition(std::span<const uint32_t> vectorASpan, std::span<const uint32_t> vectorBSpan, std::span<uint32_t> vectorCSpan)
{
    std::generate(vectorCSpan.begin(), vectorCSpan.end(), [=, i = uint32_t(0)] () mutable
    {
        uint32_t value = vectorASpan[i] + vectorBSpan[i];
        i++;
        return value;
    });
}

int main(int argc, char** argv)
{
    if (argc < 3)
        std::cout << "USAGE: ./ConcurrencyBenchmark [PROBLEM_SIZE: INT] [VECTOR_OUTPUT_LENGTH: INT]" << std::endl;

    static const uint32_t VECTOR_SIZE = atoi(argv[1]);
    const uint32_t n = atoi(argv[2]);

    std::vector<uint32_t> vectorA(VECTOR_SIZE), vectorB(VECTOR_SIZE), vectorC(VECTOR_SIZE), vectorD(VECTOR_SIZE);

    auto genIncrementingValues = [i = std::uint32_t(0)] () mutable { return ++i; };  

    std::generate(vectorA.begin(), vectorA.end(), genIncrementingValues);
    std::generate(vectorB.begin(), vectorB.end(), genIncrementingValues);


    printFirstNElements(std::span(vectorA), n, "vectorA");
    printFirstNElements(std::span(vectorB), n, "vectorB");

    std::cout << "-------------------\n" << std::endl;

    {
        auto start = std::chrono::high_resolution_clock::now();

        performVectorAddition(std::span(vectorA), std::span(vectorB), std::span(vectorC));

        auto end = std::chrono::high_resolution_clock::now();

        printFirstNElements(std::span(vectorC), n, "vectorC");

        std::cout << "Time with 1 thread: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;
    }

    std::cout << "-------------------\n" << std::endl;

    {
        auto start = std::chrono::high_resolution_clock::now();

        {
            ThreadInstanceAllocator allocator(VECTOR_SIZE);
            uint32_t hardwareThreadCount = std::jthread::hardware_concurrency();
            std::vector<std::jthread> threads(std::jthread::hardware_concurrency());

            for (uint32_t i = 0; i < hardwareThreadCount - 1; i++)
            {
                uint32_t threadStartOffset = allocator.getNextOffset();
                uint32_t threadEndOffset = allocator.getNextOffset();
                threads[i] = std::jthread
                (
                    performVectorAddition,
                    std::span<uint32_t>(vectorA.begin() + threadStartOffset, vectorA.begin() + threadEndOffset),
                    std::span<uint32_t>(vectorB.begin() + threadStartOffset, vectorB.begin() + threadEndOffset),
                    std::span<uint32_t>(vectorD.begin() + threadStartOffset, vectorD.begin() + threadEndOffset)
                );
            }
            
            uint32_t threadStartOffset = allocator.getNextOffset();
            uint32_t threadEndOffset = allocator.getNextOffset();

            performVectorAddition
            (
                std::span<uint32_t>(vectorA.begin() + threadStartOffset, vectorA.begin() + threadEndOffset),
                std::span<uint32_t>(vectorB.begin() + threadStartOffset, vectorB.begin() + threadEndOffset),
                std::span<uint32_t>(vectorD.begin() + threadStartOffset, vectorD.begin() + threadEndOffset)
            );
        }

        auto end = std::chrono::high_resolution_clock::now();

        printFirstNElements(std::span(vectorD), n, "vectorD");

        std::cout << "Time with 4 threads: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;
    }
    

    return 0;
}