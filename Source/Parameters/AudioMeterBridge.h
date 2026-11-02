#pragma once

#include <atomic>

namespace ferruxe
{

/** Immutable UI meter frame read at 30 Hz on the message thread. */
struct MeterSnapshot
{
    float inputPeakLinear  = 0.0f;
    float outputPeakLinear = 0.0f;
    float gainReductionDb  = 0.0f;
    bool  ceilingClipped   = false;
};

/**
 * Lock-free bridge between the real-time Audio Thread (producer) and the UI Thread (consumer).
 * Equivalent to C# Interlocked peak-hold operations—never blocks or allocates memory.
 */
class AudioMeterBridge
{
public:
    void publishFromAudioThread (float inPeak, float outPeak, float grDb, bool clipped) noexcept
    {
        updateAtomicMax (inputPeak, inPeak);
        updateAtomicMax (outputPeak, outPeak);
        updateAtomicMax (gainReductionDb, grDb);

        if (clipped)
            ceilingClipped.store (true, std::memory_order_relaxed);
    }

    [[nodiscard]] MeterSnapshot consumeForUiThread() noexcept
    {
        return MeterSnapshot
        {
            inputPeak.exchange (0.0f, std::memory_order_relaxed),
            outputPeak.exchange (0.0f, std::memory_order_relaxed),
            gainReductionDb.exchange (0.0f, std::memory_order_relaxed),
            ceilingClipped.exchange (false, std::memory_order_relaxed)
        };
    }

private:
    static void updateAtomicMax (std::atomic<float>& target, float candidate) noexcept
    {
        float current = target.load (std::memory_order_relaxed);
        while (candidate > current
               && ! target.compare_exchange_weak (current, candidate, std::memory_order_relaxed))
        {
        }
    }

    std::atomic<float> inputPeak       { 0.0f };
    std::atomic<float> outputPeak      { 0.0f };
    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<bool>  ceilingClipped  { false };
};

} // namespace ferruxe
