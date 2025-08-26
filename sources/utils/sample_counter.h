#pragma once

template <class Sample>
class SampleCounter
{
  public:
    explicit SampleCounter(int32_t samplesLimit);

    void AddSample(Sample sample);
    void Reset();

    bool IsEmpty() const { return _availableSamplesCount == 0; }
    bool IsFull() const { return _availableSamplesCount == _samples.capacity(); }

    Sample CalcAverage() const;
    Sample CalcMedian() const;

  private:
    std::vector<Sample> _samples;
    int32_t _availableSamplesCount{0};
    int32_t _currentSample{0};
};

using Int32SampleCounter = SampleCounter<int32_t>;
using Int64SampleCounter = SampleCounter<int64_t>;
using FloatSampleCounter = SampleCounter<float>;

