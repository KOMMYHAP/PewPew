#include "sample_counter.h"

template <class Sample>
SampleCounter<Sample>::SampleCounter(int32_t samplesLimit)
    : _samples(static_cast<std::size_t>(samplesLimit), Sample{})
{
}

template <class Sample>
void SampleCounter<Sample>::Reset()
{
    _currentSample = 0;
    _availableSamplesCount = 0;
    for (Sample &sample : _samples)
    {
        sample = Sample{};
    }
}

template <class Sample>
void SampleCounter<Sample>::AddSample(Sample sample)
{
    _samples[_currentSample] = sample;
    _currentSample += 1;
    if (_currentSample == _samples.capacity())
    {
        _currentSample = 0;
    }
    _availableSamplesCount = std::min<int32_t>(_availableSamplesCount + 1, _samples.capacity());
}

template <class Sample>
Sample SampleCounter<Sample>::CalcMedian() const
{
    if (IsEmpty())
    {
        return {};
    }

    auto samples = _samples;
    std::sort(samples.begin(), samples.begin() + _availableSamplesCount);

    // Take less value if the samples count is even
    const uint16_t indexOffset = (_availableSamplesCount % 2 != 0) ? 0 : 1;
    return samples[_availableSamplesCount / 2 - indexOffset];
}

template <class Sample>
Sample SampleCounter<Sample>::CalcAverage() const
{
    if (IsEmpty())
    {
        return {};
    }

    const Sample total = std::accumulate(_samples.begin(), _samples.begin() + _availableSamplesCount, Sample{});
    return static_cast<Sample>(std::round(static_cast<float>(total) / _availableSamplesCount));
}

template class SampleCounter<int32_t>;
template class SampleCounter<int64_t>;
template class SampleCounter<float>;