#include "ThrashTheCache.h"

#include <algorithm>
#include <chrono>
#include <numeric> 
#include <imgui.h>

using Clock = std::chrono::high_resolution_clock;

static float AverageTrimmed(std::vector<long long> samples)
{
    if (samples.empty()) return 0.f;
    if (samples.size() <= 2)
    {
        const long long sum = std::accumulate(samples.begin(), samples.end(), 0LL);
        return static_cast<float>(sum) / static_cast<float>(samples.size());
    }

    std::sort(samples.begin(), samples.end());
    const long long sum = std::accumulate(samples.begin() + 1, samples.end() - 1, 0LL);
    return static_cast<float>(sum) / static_cast<float>(samples.size() - 2);
}

template<typename TBuffer, typename TOp>
ThrashTheCache::ExerciseResults ThrashTheCache::RunBenchmark(int bufferSize, int numSamples, TBuffer buffer, TOp op)
{
    ExerciseResults res;

    for (int step = 1; step <= 1024; step *= 2)
    {
        std::vector<long long> samples;
        samples.reserve(numSamples);

        for (int s = 0; s < numSamples; ++s)
        {
            const auto start = Clock::now();
            for (int i = 0; i < bufferSize; i += step)
                op(buffer, i);
            const auto end = Clock::now();

            samples.push_back(
                std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        res.stepSizes.push_back(step);
        res.timings_us.push_back(AverageTrimmed(std::move(samples)));
    }

    return res;
}

ThrashTheCache::ExerciseResults ThrashTheCache::RunExercise1(int numSamples)
{
    std::vector<int> buffer(INT_BUFFER_SIZE, 1);
    return RunBenchmark(INT_BUFFER_SIZE, numSamples, std::move(buffer),
        [](std::vector<int>& buf, int i) { buf[i] *= 2; });
}

ThrashTheCache::ExerciseResults ThrashTheCache::RunExercise2(int numSamples)
{
    std::vector<GameObject3D> buffer(GO_BUFFER_SIZE);
    for (int i = 0; i < GO_BUFFER_SIZE; ++i) buffer[i].ID = i;
    return RunBenchmark(GO_BUFFER_SIZE, numSamples, std::move(buffer),
        [](std::vector<GameObject3D>& buf, int i) { buf[i].ID *= 2; });
}

ThrashTheCache::ExerciseResults ThrashTheCache::RunExercise2Alt(int numSamples)
{
    std::vector<GameObject3DAlt> hotBuffer(GO_BUFFER_SIZE);
    std::vector<CacheTransform> coldBuffer(GO_BUFFER_SIZE);

    for (int i = 0; i < GO_BUFFER_SIZE; ++i)
    {
        hotBuffer[i].ID = i;
        hotBuffer[i].pTransform = &coldBuffer[i];
    }
    return RunBenchmark(GO_BUFFER_SIZE, numSamples, hotBuffer,
        [](std::vector<GameObject3DAlt>& buf, int i) { buf[i].ID *= 2; });
}

void ThrashTheCache::DrawResults(const ExerciseResults& res, unsigned int color, const char* tableId)
{
    if (res.timings_us.empty()) return;

    float maxVal = *std::max_element(res.timings_us.begin(), res.timings_us.end());
    if (maxVal <= 0.f) maxVal = 1.f;

    constexpr ImVec2 plotSize{ 380.f, 140.f };
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float  bottomY = origin.y + plotSize.y;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(origin, { origin.x + plotSize.x, bottomY }, IM_COL32(20, 20, 20, 255));
    dl->AddRect(origin, { origin.x + plotSize.x, bottomY }, IM_COL32(100, 100, 100, 255));

    const int   n = static_cast<int>(res.timings_us.size());
    const float barW = plotSize.x / static_cast<float>(n);

    for (int i = 0; i < n; ++i)
    {
        const float barH = (res.timings_us[i] / maxVal) * plotSize.y;
        const float x0 = origin.x + i * barW + 1.f;
        const float x1 = origin.x + (i + 1) * barW - 1.f;

        dl->AddRectFilled({ x0, bottomY - barH }, { x1, bottomY }, color);

        if (i % 2 == 0)
        {
            char label[8];
            snprintf(label, sizeof(label), "%d", res.stepSizes[i]);
            dl->AddText({ x0, bottomY + 2.f }, IM_COL32(180, 180, 180, 255), label);
        }
    }

    ImGui::Dummy({ plotSize.x, plotSize.y + 18.f });

    ImGui::Spacing();
    if (ImGui::BeginTable(tableId, 2,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Step size");
        ImGui::TableSetupColumn("Avg time (us)");
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(res.stepSizes.size()); ++i)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%d", res.stepSizes[i]);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f", res.timings_us[i]);
        }

        ImGui::EndTable();
    }
}

void ThrashTheCache::DrawCombinedResults(const ExerciseResults& resA, unsigned int colorA, const ExerciseResults& resB, unsigned int colorB, const char* tableId)
{
    if (resA.timings_us.empty() || resB.timings_us.empty()) return;

    const float maxA = *std::max_element(resA.timings_us.begin(), resA.timings_us.end());
    const float maxB = *std::max_element(resB.timings_us.begin(), resB.timings_us.end());
    float maxVal = std::max(maxA, maxB);
    if (maxVal <= 0.f) maxVal = 1.f;

    constexpr ImVec2 plotSize{ 380.f, 140.f };
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const float  bottomY = origin.y + plotSize.y;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(origin, { origin.x + plotSize.x, bottomY }, IM_COL32(20, 20, 20, 255));
    dl->AddRect(origin, { origin.x + plotSize.x, bottomY }, IM_COL32(100, 100, 100, 255));

    const int   n = static_cast<int>(resA.timings_us.size());
    const float barW = plotSize.x / static_cast<float>(n);

    for (int i = 0; i < n; ++i)
    {
        const float halfW = barW * 0.5f;
        const float xBase = origin.x + i * barW;

        const float barHA = (resA.timings_us[i] / maxVal) * plotSize.y;
        dl->AddRectFilled({ xBase + 1.f,         bottomY - barHA },
            { xBase + halfW - 1.f,  bottomY }, colorA);

        if (i < static_cast<int>(resB.timings_us.size()))
        {
            const float barHB = (resB.timings_us[i] / maxVal) * plotSize.y;
            dl->AddRectFilled({ xBase + halfW + 1.f, bottomY - barHB },
                { xBase + barW - 1.f,  bottomY }, colorB);
        }

        if (i % 2 == 0)
        {
            char label[8];
            snprintf(label, sizeof(label), "%d", resA.stepSizes[i]);
            dl->AddText({ xBase + 1.f, bottomY + 2.f }, IM_COL32(180, 180, 180, 255), label);
        }
    }

    ImGui::Dummy({ plotSize.x, plotSize.y + 18.f });

    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(colorA), "GameObject3D");
    ImGui::SameLine();
    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(colorB), "  GameObject3DAlt");

    ImGui::Spacing();
    if (ImGui::BeginTable(tableId, 3,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Step size");
        ImGui::TableSetupColumn("GO3D (us)");
        ImGui::TableSetupColumn("GO3DAlt (us)");
        ImGui::TableHeadersRow();

        for (int i = 0; i < n; ++i)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%d", resA.stepSizes[i]);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f", resA.timings_us[i]);
            ImGui::TableSetColumnIndex(2);
            if (i < static_cast<int>(resB.timings_us.size()))
                ImGui::Text("%.2f", resB.timings_us[i]);
        }

        ImGui::EndTable();
    }
}

void ThrashTheCache::SamplesInput(const char* id, int& samples)
{
    char buf[32];

    snprintf(buf, sizeof(buf), "##val_%s", id);
    ImGui::SetNextItemWidth(50.f);
    ImGui::InputInt(buf, &samples, 0, 0);
    samples = std::clamp(samples, MIN_SAMPLES, MAX_SAMPLES);

    snprintf(buf, sizeof(buf), "+##plus_%s", id);
    ImGui::SameLine();
    if (ImGui::Button(buf) && samples < MAX_SAMPLES) ++samples;

    snprintf(buf, sizeof(buf), "-##minus_%s", id);
    ImGui::SameLine();
    if (ImGui::Button(buf) && samples > MIN_SAMPLES) --samples;

    ImGui::SameLine();
    ImGui::Text("# samples");
}

void ThrashTheCache::Render() const
{
    if (ImGui::CollapsingHeader("Exercise 1", ImGuiTreeNodeFlags_DefaultOpen))
    {
        SamplesInput("ex1", m_ex1.numSamples);

        if (ImGui::Button("Thrash the cache with int"))
        {
            m_ex1.results = RunExercise1(m_ex1.numSamples);
            m_ex1.hasResults = true;
        }

        if (m_ex1.hasResults)
        {
            ImGui::Spacing();
            DrawResults(m_ex1.results, IM_COL32(51, 140, 255, 255), "table_ex1");
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Exercise 2", ImGuiTreeNodeFlags_DefaultOpen))
    {
        SamplesInput("ex2", m_ex2.numSamples);

        if (ImGui::Button("Thrash the cache with GameObject3D"))
        {
            m_ex2.results = RunExercise2(m_ex2.numSamples);
            m_ex2.hasResults = true;
        }

        if (ImGui::Button("Thrash the cache with GameObject3DAlt"))
        {
            m_ex2Alt.results = RunExercise2Alt(m_ex2.numSamples);
            m_ex2Alt.hasResults = true;
        }

        if (m_ex2.hasResults)
        {
            ImGui::Spacing();
            ImGui::Text("GameObject3D");
            DrawResults(m_ex2.results, IM_COL32(255, 102, 51, 255), "table_ex2");
        }

        if (m_ex2Alt.hasResults)
        {
            ImGui::Spacing();
            ImGui::Text("GameObject3DAlt");
            DrawResults(m_ex2Alt.results, IM_COL32(51, 255, 102, 255), "table_ex2alt");
        }

        if (m_ex2.hasResults && m_ex2Alt.hasResults)
        {
            ImGui::Spacing();
            ImGui::Text("Combined");
            DrawCombinedResults(
                m_ex2.results, IM_COL32(255, 102, 51, 255),
                m_ex2Alt.results, IM_COL32(51, 255, 102, 255),
                "table_ex2combined");
        }
    }
}