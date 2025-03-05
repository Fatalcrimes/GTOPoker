#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <vector>
#include <memory>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace poker {

/**
 * Logger provides logging functionality with different severity levels
 * and output destinations.
 */
class Logger {
public:
    // Log levels
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL
    };
    
    // Log destination
    enum class Destination {
        CONSOLE,
        FILE,
        BOTH
    };
    
    // Singleton access
    static Logger& getInstance();
    
    // Initialize logger
    void init(Level minLevel = Level::INFO, Destination dest = Destination::CONSOLE, const std::string& filename = "poker_cfr_bot.log");
    
    // Log methods for different levels
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
    
    // Generic log method
    void log(Level level, const std::string& message);
    
    // Set minimum log level
    void setLevel(Level level);
    
    // Set log destination
    void setDestination(Destination dest);
    
    // Set log file
    void setLogFile(const std::string& filename);
    
    // Convert log level to string
    static std::string levelToString(Level level);
    
    // Destructor
    ~Logger();

private:
    // Private constructor for singleton
    Logger();
    
    // Timestamp formatting
    std::string getTimestamp() const;
    
    // Format log message
    std::string formatLogMessage(Level level, const std::string& message) const;
    
    // Data members
    Level minLevel_ = Level::INFO;
    Destination destination_ = Destination::CONSOLE;
    std::string filename_;
    std::ofstream logFile_;
    std::mutex mutex_;
    
    // Prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

// Convenience macros for logging
#define LOG_DEBUG(message) poker::Logger::getInstance().debug(message)
#define LOG_INFO(message) poker::Logger::getInstance().info(message)
#define LOG_WARNING(message) poker::Logger::getInstance().warning(message)
#define LOG_ERROR(message) poker::Logger::getInstance().error(message)
#define LOG_FATAL(message) poker::Logger::getInstance().fatal(message)

// Stream-style log wrapper
class LogStream {
public:
    LogStream(Logger::Level level);
    ~LogStream();
    
    template<typename T>
    LogStream& operator<<(const T& value) {
        stream_ << value;
        return *this;
    }

private:
    Logger::Level level_;
    std::ostringstream stream_;
};

// Stream logging macros
#define DEBUG_LOG poker::LogStream(poker::Logger::Level::DEBUG)
#define INFO_LOG poker::LogStream(poker::Logger::Level::INFO)
#define WARNING_LOG poker::LogStream(poker::Logger::Level::WARNING)
#define ERROR_LOG poker::LogStream(poker::Logger::Level::ERROR)
#define FATAL_LOG poker::LogStream(poker::Logger::Level::FATAL)

} // namespace poker