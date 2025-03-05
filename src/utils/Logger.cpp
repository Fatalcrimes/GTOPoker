#include "utils/Logger.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

namespace poker {

// Initialize the singleton instance
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() 
    : minLevel_(Level::INFO), 
      destination_(Destination::CONSOLE) {
    // Private constructor for singleton
}

Logger::~Logger() {
    // Close log file if open
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

void Logger::init(Level minLevel, Destination dest, const std::string& filename) {
    // Lock for thread safety
    std::lock_guard<std::mutex> lock(mutex_);
    
    minLevel_ = minLevel;
    destination_ = dest;
    
    // Close existing log file if open
    if (logFile_.is_open()) {
        logFile_.close();
    }
    
    // Open log file if destination includes file
    if (dest == Destination::FILE || dest == Destination::BOTH) {
        filename_ = filename;
        logFile_.open(filename_, std::ios::out | std::ios::app);
        
        if (!logFile_.is_open()) {
            std::cerr << "ERROR: Failed to open log file: " << filename_ << std::endl;
            // Fallback to console logging
            destination_ = Destination::CONSOLE;
        }
    }
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(Level::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(Level::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(Level::FATAL, message);
}

void Logger::log(Level level, const std::string& message) {
    // Skip logging if level is below minimum
    if (level < minLevel_) {
        return;
    }
    
    // Lock for thread safety
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Format the log message
    std::string formattedMessage = formatLogMessage(level, message);
    
    // Log to console if enabled
    if (destination_ == Destination::CONSOLE || destination_ == Destination::BOTH) {
        // Use appropriate stream based on level
        if (level >= Level::ERROR) {
            std::cerr << formattedMessage << std::endl;
        } else {
            std::cout << formattedMessage << std::endl;
        }
    }
    
    // Log to file if enabled
    if ((destination_ == Destination::FILE || destination_ == Destination::BOTH) 
        && logFile_.is_open()) {
        logFile_ << formattedMessage << std::endl;
        logFile_.flush(); // Ensure it's written immediately
    }
}

void Logger::setLevel(Level level) {
    // Lock for thread safety
    std::lock_guard<std::mutex> lock(mutex_);
    minLevel_ = level;
}

void Logger::setDestination(Destination dest) {
    // Lock for thread safety
    std::lock_guard<std::mutex> lock(mutex_);
    
    destination_ = dest;
    
    // Open log file if necessary
    if ((dest == Destination::FILE || dest == Destination::BOTH) 
        && !logFile_.is_open() && !filename_.empty()) {
        logFile_.open(filename_, std::ios::out | std::ios::app);
    }
}

void Logger::setLogFile(const std::string& filename) {
    // Lock for thread safety
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Close existing log file if open
    if (logFile_.is_open()) {
        logFile_.close();
    }
    
    filename_ = filename;
    
    // Open new log file if destination includes file
    if (destination_ == Destination::FILE || destination_ == Destination::BOTH) {
        logFile_.open(filename_, std::ios::out | std::ios::app);
        
        if (!logFile_.is_open()) {
            std::cerr << "ERROR: Failed to open log file: " << filename_ << std::endl;
            // Fallback to console logging
            destination_ = Destination::CONSOLE;
        }
    }
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR:   return "ERROR";
        case Level::FATAL:   return "FATAL";
        default:             return "UNKNOWN";
    }
}

std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::formatLogMessage(Level level, const std::string& message) const {
    std::stringstream ss;
    ss << "[" << getTimestamp() << "] "
       << "[" << levelToString(level) << "] "
       << message;
    
    return ss.str();
}

// LogStream implementation
LogStream::LogStream(Logger::Level level) : level_(level) {
}

LogStream::~LogStream() {
    // Write the accumulated message to the logger
    Logger::getInstance().log(level_, stream_.str());
}

} // namespace poker