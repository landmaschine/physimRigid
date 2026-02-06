#pragma once

#include <chrono>

enum class TimeUnit {
  Minutes,
  Seconds,
  Milliseconds,
  Microseconds,
  Nanoseconds
};

using min = std::integral_constant<TimeUnit, TimeUnit::Minutes>;
using s   = std::integral_constant<TimeUnit, TimeUnit::Seconds>;
using ms  = std::integral_constant<TimeUnit, TimeUnit::Milliseconds>;
using us  = std::integral_constant<TimeUnit, TimeUnit::Microseconds>;
using ns  = std::integral_constant<TimeUnit, TimeUnit::Nanoseconds>;

class Timer {
public:
  Timer() = default;
  ~Timer() = default;

  void start() {
    m_start = std::chrono::high_resolution_clock::now();
  }

  template <typename Unit = ms>
  float stop() const {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = end - m_start;

    if constexpr (Unit::value == TimeUnit::Minutes) {
      return std::chrono::duration<float, std::ratio<60>>(duration).count();
    } else if constexpr (Unit::value == TimeUnit::Seconds) {
      return std::chrono::duration<float>(duration).count();
    } else if constexpr (Unit::value == TimeUnit::Milliseconds) {
      return std::chrono::duration<float, std::milli>(duration).count();
    } else if constexpr (Unit::value == TimeUnit::Microseconds) {
      return std::chrono::duration<float, std::micro>(duration).count();
    } else if constexpr (Unit::value == TimeUnit::Nanoseconds) {
      return std::chrono::duration<float, std::nano>(duration).count();
    }
  }

  template <typename Unit = ms>
  float elapsed() const {
    return stop<Unit>();
  }

private:
  std::chrono::high_resolution_clock::time_point m_start{};
};
