/// Jack
#ifndef COMMON_BASE_TIMED_STATS_H
#define COMMON_BASE_TIMED_STATS_H

#include <stdint.h>
#include <algorithm>
#include <string>
#include <vector>
#include "mutex.h"
#include "posix_time.h"

// undefine min/max micro on windows platform
#ifdef _WIN32
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif

template <typename T>
struct CountingStatsResult
{
    T last_second_count;
    T last_minute_count;
    T last_hour_count;
    T total_count;
};

template <typename T>
class CountingStats
{
public:
    // in seconds, maintain the records of each second in the past hour.
    static const int kTimeWindowSize = 3600;

    mutable ipcs_common::Mutex m_mutex;
    std::string m_name;
    std::string m_unit;
    int64_t m_start_time;
    int64_t m_last_time;
    T m_total;
    std::vector<T> m_items;

public:
    CountingStats() : m_start_time(-1), m_last_time(-1),
                      m_total(0), m_items(kTimeWindowSize) {}
    CountingStats(const std::string& name, const std::string& unit) :
        m_name(name), m_unit(unit),
        m_start_time(-1), m_last_time(-1),
        m_total(0), m_items(kTimeWindowSize) {}
    ~CountingStats() {}

    void SetDescription(const std::string& name, const std::string& unit)
    {
        m_name = name;
        m_unit = unit;
    }

    T GetTotal() const
    {
        MutexLocker locker(m_mutex);
        return m_total;
    }

    void Increase(int64_t current_time = -1)
    {
        int64_t timestamp = (current_time != -1) ? current_time : ipcs_common::GetTimeStampInMs() / 1000;
        AddInternalCount(T(1), timestamp);
    }

    void Add(T value, int64_t current_time = -1)
    {
        int64_t timestamp = (current_time != -1) ? current_time : ipcs_common::GetTimeStampInMs() / 1000;
        AddInternalCount(value, timestamp);
    }

    // Get counts of recent second/minute/hour and total.
    void GetStatsResult(CountingStatsResult<T>* result, int64_t current_time = -1) const
    {
        int64_t timestamp = (current_time != -1) ? current_time : ipcs_common::GetTimeStampInMs() / 1000;

        MutexLocker locker(m_mutex);
        result->last_second_count = GetRecentStatsInternal(1, timestamp);
        result->last_minute_count = GetRecentStatsInternal(60, timestamp);
        result->last_hour_count = GetRecentStatsInternal(3600, timestamp);
        result->total_count = m_total;
    }

    // Return count of rencent time.
    T GetRecentCount(int seconds, int64_t current_time = -1) const
    {
        int64_t timestamp = (current_time != -1) ? current_time : ipcs_common::GetTimeStampInMs() / 1000;
        if (seconds > 3600)
            seconds = 3600;
        MutexLocker locker(m_mutex);
        return GetRecentStatsInternal(seconds, timestamp);
    }

    T GetMaxCountOfRecent(int seconds, int64_t current_time = -1) const
    {
        int64_t timestamp = (current_time != -1) ? current_time : ipcs_common::GetTimeStampInMs() / 1000;

        MutexLocker locker(&m_mutex);
        int64_t start_time = std::max(timestamp - seconds, m_start_time);
        int64_t end_time = std::min(timestamp - 1, m_last_time);
        if (start_time > m_last_time || end_time < m_start_time)
        {
            return 0;
        }

        T max_count = 0;
        for (int64_t i = start_time; i <= end_time; i++)
        {
            if (m_items[i % kTimeWindowSize] > max_count)
                max_count = m_items[i % kTimeWindowSize];
        }
        return max_count;
    }

    void GetHistoricalItem(std::vector<T>* items, int64_t current_time = -1) const
    {
        int64_t timestamp = (current_time != -1) ? current_time : ipcs_common::GetTimeStampInMs() / 1000;
        items->clear();

        MutexLocker locker(&m_mutex);
        int64_t start_time = std::max(timestamp - kTimeWindowSize, m_start_time);
        int64_t end_time = std::min(timestamp - 1, m_last_time);
        if (start_time > m_last_time || end_time < m_start_time)
        {
            return;
        }

        for (int64_t i = start_time; i <= end_time; i++)
        {
            items->push_back(m_items[i % kTimeWindowSize]);
        }
    }

private:
    void AddInternalCount(T value, int64_t timestamp)
    {
        MutexLocker locker(&m_mutex);
        int pos = timestamp % kTimeWindowSize;
        if (m_start_time < 0) {
            m_start_time = timestamp;
            m_last_time = timestamp;
            m_items[pos] = value;
            m_total = value;
            return;
        }

        if (timestamp < m_last_time) {
            return;
        }

        if (timestamp - m_last_time >= kTimeWindowSize) {
            int64_t new_start_time = timestamp - kTimeWindowSize + 1;
            for (int64_t i = new_start_time; i < timestamp; i++) {
                m_items[i % kTimeWindowSize] = 0;
            }
            m_start_time = new_start_time;
            m_last_time = timestamp;
            m_items[pos] = value;
            m_total += value;
            return;
        }

        if (timestamp == m_last_time) {
            m_items[pos] += value;
            m_total += value;
            return;
        }

        int64_t i = m_last_time + 1;
        while (i < timestamp) {
            m_items[i % kTimeWindowSize] = 0;
            i++;
        }
        m_start_time = std::max(m_start_time, timestamp - kTimeWindowSize + 1);
        m_last_time = timestamp;
        m_items[pos] = value;
        m_total += value;
    }

    T GetRecentStatsInternal(int seconds, int64_t timestamp) const
    {
        if (m_start_time < 0) {
            return 0;
        }

        int64_t start_time = std::max(timestamp - seconds, m_start_time);
        int64_t end_time = std::min(timestamp - 1, m_last_time);
        if (start_time > m_last_time || end_time < m_start_time)
        {
            return 0;
        }

        T sum = 0;
        for (int64_t i = start_time; i <= end_time; i++)
        {
            sum += m_items[i % kTimeWindowSize];
        }
        return sum;
    }
};

template <typename T>
struct PeriodMeanStatsResult {
    T min, max, count, average;
    void Clear()
    {
        min = 0;
        max = 0;
        count = 0;
        average = 0;
    }
};

template <typename T>
struct MeanStatsResult {
    PeriodMeanStatsResult<T> last_second;
    PeriodMeanStatsResult<T> last_minute;
    PeriodMeanStatsResult<T> last_hour;
    PeriodMeanStatsResult<T> total;
};

template <typename T>
class MeanStats
{
public:
    template <typename U>
    struct StatsItem
    {
        StatsItem() : cnt(0), min(0), max(0), sum(0) {}
        U cnt;
        U min;
        U max;
        U sum;
    };

    // in seconds, maintain the records of each second in the past hour.
    static const int kTimeWindowSize = 3600;

    mutable ipcs_common::Mutex m_mutex;
    std::string m_name;
    std::string m_unit;
    int64_t m_start_time;
    int64_t m_last_time;
    StatsItem<T> m_total;
    std::vector<StatsItem<T> > m_items;

public:
    MeanStats() : m_start_time(-1), m_last_time(-1),
                  m_items(kTimeWindowSize) {}
    MeanStats(const std::string& name, const std::string& unit)
        : m_name(name), m_unit(unit),
          m_start_time(-1), m_last_time(-1),
          m_items(kTimeWindowSize) {}
    ~MeanStats() {}

    void SetDescription(const std::string& name, const std::string& unit)
    {
        m_name = name;
        m_unit = unit;
    }

    void Add(T value, int64_t current_time = -1)
    {
        int64_t timestamp = (current_time != -1) ? current_time : ipcs_common::GetTimeStampInMs() / 1000;
        AddInternalValue(value, timestamp);
    }

    void GetStatsResult(MeanStatsResult<T>* result, int64_t current_time = -1) const
    {
        int64_t timestamp = (current_time != -1) ? current_time : ipcs_common::GetTimeStampInMs() / 1000;

        MutexLocker locker(m_mutex);
        GetRecentStatsInternal(1, &result->last_second, timestamp);
        GetRecentStatsInternal(60, &result->last_minute, timestamp);
        GetRecentStatsInternal(3600, &result->last_hour, timestamp);

        result->total.min = m_total.min;
        result->total.max = m_total.max;
        result->total.count = m_total.cnt;
        if (m_total.cnt == 0)
        {
            result->total.average = 0;
        }
        else
        {
            result->total.average = m_total.sum / m_total.cnt;
        }
    }

    // Return count of rencent time.
    void GetRecentStats(int seconds,
                        PeriodMeanStatsResult<T>* result,
                        int64_t current_time = -1) const
    {
        int64_t timestamp = (current_time != -1) ? current_time : ipcs_common::GetTimeStampInMs() / 1000;
        if (seconds > 3600)
            seconds = 3600;
        MutexLocker locker(m_mutex);
        return GetRecentStatsInternal(seconds, result, timestamp);
    }


    void GetHistoricalItem(std::vector<StatsItem<T> >* items, int64_t current_time = -1) const
    {
        int64_t timestamp = (current_time != -1) ? current_time : ipcs_common::GetTimeStampInMs() / 1000;
        items->clear();

        MutexLocker locker(&m_mutex);
        int64_t start_time = std::max(timestamp - kTimeWindowSize, m_start_time);
        int64_t end_time = std::min(timestamp - 1, m_last_time);
        if (start_time > m_last_time || end_time < m_start_time)
        {
            return;
        }

        for (int64_t i = start_time; i <= end_time; i++)
        {
            items->push_back(m_items[i % kTimeWindowSize]);
        }
    }

protected:
    void ClearItem(StatsItem<T>* item)
    {
        item->cnt = 0;
        item->min = 0;
        item->max = 0;
        item->sum = 0;
    }
    void SetItem(StatsItem<T>* item, T value)
    {
        item->cnt = 1;
        item->min = value;
        item->max = value;
        item->sum = value;
    }

    void UpdateItem(StatsItem<T>* item, T value)
    {
        item->cnt++;
        item->min = std::min(value, item->min);
        item->max = std::max(value, item->max);
        item->sum += value;
    }

    void AddInternalValue(T value, int64_t timestamp)
    {
        MutexLocker locker(&m_mutex);
        int pos = timestamp % kTimeWindowSize;
        if (m_start_time < 0) {
            m_start_time = timestamp;
            m_last_time = timestamp;
            SetItem(&m_items[pos], value);
            SetItem(&m_total, value);
            return;
        }

        if (timestamp < m_last_time) {
            return;
        }

        UpdateItem(&m_total, value);

        if (timestamp - m_last_time >= kTimeWindowSize) {
            int64_t new_start_time = timestamp - kTimeWindowSize + 1;
            for (int64_t i = 0; i < kTimeWindowSize; i++) {
                ClearItem(&m_items[i]);
            }
            m_start_time = new_start_time;
            m_last_time = timestamp;
            SetItem(&m_items[pos], value);
            return;
        }

        if (timestamp == m_last_time) {
            UpdateItem(&m_items[pos], value);
            return;
        }

        int64_t i = m_last_time + 1;
        while (i < timestamp) {
            ClearItem(&m_items[i % kTimeWindowSize]);
            i++;
        }
        m_start_time = std::max(m_start_time,
                timestamp - kTimeWindowSize + 1);
        m_last_time = timestamp;
        SetItem(&m_items[pos], value);
    }

    void GetRecentStatsInternal(
        int seconds,
        PeriodMeanStatsResult<T>* result,
        int64_t timestamp) const
    {
        result->Clear();
        if (m_start_time < 0) {
            return;
        }

        int64_t start_time = std::max(timestamp - seconds, m_start_time);
        int64_t end_time = std::min(timestamp - 1, m_last_time);
        if (start_time > m_last_time || end_time < m_start_time)
        {
            return;
        }

        StatsItem<T> section_item = m_items[start_time % kTimeWindowSize];
        for (int64_t i = start_time + 1; i <= end_time; i++)
        {
            const StatsItem<T>* item = &m_items[i % kTimeWindowSize];
            if (item->cnt != 0)
            {
                section_item.cnt += item->cnt;
                section_item.min = std::min(section_item.min, item->min);
                section_item.max = std::max(section_item.max, item->max);
                section_item.sum += item->sum;
            }
        }

        result->min = section_item.min;
        result->max = section_item.max;
        result->count = section_item.cnt;
        if (section_item.cnt == 0)
        {
            result->average = 0;
        }
        else
        {
            result->average = section_item.sum / section_item.cnt;
        }
    }
};

#endif // IPCS_COMMON_BASE_TIMED_STATS_H
