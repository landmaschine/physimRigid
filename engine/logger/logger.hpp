#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>

enum class LogLevel {
  NORMAL,
  LOOP,
  WARNING,
  ERROR,
  CRITICAL
};

#define LOG(...) Logger(LogLevel::NORMAL, __FILE__, __LINE__)(__VA_ARGS__)
#define WARLOG(...)  Logger(LogLevel::WARNING, __FILE__, __LINE__)(__VA_ARGS__);
#define ERRLOG(...)  Logger(LogLevel::ERROR, __FILE__, __LINE__)(__VA_ARGS__);
#define CRITLOG(...) Logger(LogLevel::CRITICAL, __FILE__, __LINE__)(__VA_ARGS__);
#define LOOPLOG(...) Logger(LogLevel::LOOP, __FILE__, __LINE__)(__VA_ARGS__);

class Logger {
public:
  Logger(LogLevel level, const char* file, int line);
  ~Logger();

  template<typename T>
  Logger& operator<<(const T& msg) {
    stream << msg;
    return *this;
  }

  template<typename... Args>
  void operator()(Args&&... args) {
    std::ostringstream oss;
    (oss << ... << std::forward<Args>(args));
    stream << oss.str();
  }

private:
  LogLevel level;
  std::ostringstream stream;
  const char* file;
  int line;
  std::string getCurrentTime() const;
  std::string getColorCode() const;
  std::string levelToString() const;
  static std::mutex logMutex;
  static std::ofstream logFile;
  static void initLogFile();
};