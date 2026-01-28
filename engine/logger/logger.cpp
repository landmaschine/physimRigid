#include "logger.hpp"

#include <iostream>

std::mutex Logger::logMutex;
std::ofstream Logger::logFile;

static const char* extractFileName(const char* path) {
  const char* slash = strrchr(path, '/');
  return slash ? slash + 1 : path;
}

Logger::Logger(LogLevel lvl, const char* srcFile, int srcLine) 
  : level(lvl), file(extractFileName(srcFile)), line(srcLine) {
    initLogFile();
}

Logger::~Logger() {
  std::lock_guard<std::mutex> lock(logMutex);
  std::string msg = stream.str();
  std::string timestamp = getCurrentTime();
  std::string color = getColorCode();
  std::string levelStr = levelToString();

  std::cout << color
            << "[" << levelStr << "] "
            << "{" << file << ":" << line << "} "
            << msg << "\033[0m" << std::endl;
  
  if(level != LogLevel::LOOP) {
    logFile << "[" << timestamp << "]"
            << "[" << levelStr << "] "
            << "{" << file << ":" << line << "} "
            << msg << std::endl;
  }
}

std::string Logger::getCurrentTime() const {
  std::time_t now = std::time(nullptr);
  char buf[64];
  std::tm timeinfo;
  
#ifdef __EMSCRIPTEN__
  localtime_r(&now, &timeinfo);
#else
  localtime_s(&timeinfo, &now);
#endif
  
  std::strftime(buf, sizeof(buf), "%F %T", &timeinfo);
  return buf;
}

std::string Logger::getColorCode() const {
  switch(level) {
    case LogLevel::NORMAL:   return "\033[1;32m";
    case LogLevel::LOOP:     return "\033[1;32m";
    case LogLevel::WARNING:  return "\033[1;33m";
    case LogLevel::ERROR:    return "\033[1;31m";
    case LogLevel::CRITICAL: return "\033[1;41m";
    default: return "\033[0m";
  }
}

std::string Logger::levelToString() const {
  switch(level) {
    case LogLevel::NORMAL:   return "LOG";
    case LogLevel::LOOP:     return "LOOP";
    case LogLevel::WARNING:  return "WARNING";
    case LogLevel::ERROR:    return "ERROR";
    case LogLevel::CRITICAL: return "CRITICAL";
    default: return "UNKNOWN";
  }
}

void Logger::initLogFile() {
  if(!logFile.is_open()) {
    logFile.open("log.txt", std::ios::app);
  }
}