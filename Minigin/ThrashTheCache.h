#pragma once
#include <vector>

class ThrashTheCache
{
public:
    void Render() const;

private:
    struct CacheTransform
    {
        float matrix[16] = {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1
        };
    };

    struct GameObject3D
    {
        CacheTransform transform{};
        int ID{};
    };

    struct GameObject3DAlt
    {
        int ID{};
        CacheTransform* pTransform{ nullptr };
    };

    struct ExerciseResults
    {
        std::vector<int> stepSizes;
        std::vector<float> timings_us;
    };

    struct ExerciseState
    {
        int numSamples{ 100 };
        ExerciseResults results{};
        bool hasResults{ false };
    };

    static constexpr int INT_BUFFER_SIZE = 1 << 26;
    static constexpr int GO_BUFFER_SIZE = (INT_BUFFER_SIZE * static_cast<int>(sizeof(int))) / static_cast<int>(sizeof(GameObject3D));
    static constexpr int MIN_SAMPLES = 1;
    static constexpr int MAX_SAMPLES = 100;

    template<typename TBuffer, typename TOp>
    static ExerciseResults RunBenchmark(int bufferSize, int numSamples, TBuffer buffer, TOp op);

    static ExerciseResults RunExercise1(int numSamples);
    static ExerciseResults RunExercise2(int numSamples);
    static ExerciseResults RunExercise2Alt(int numSamples);

    static void DrawResults(const ExerciseResults& res, unsigned int color, const char* tableId);
    static void SamplesInput(const char* id, int& samples);

    mutable ExerciseState m_ex1{};
    mutable ExerciseState m_ex2{};
    mutable ExerciseState m_ex2Alt{};
};