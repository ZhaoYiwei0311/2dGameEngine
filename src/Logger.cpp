#include "Logger.h"
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>

std::vector<LogEntry> Logger::messages;

std::string CurrentDateTimeToString() {
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string output(30, '\0');
    std::strftime(&output[0], output.size(), "%d-%b-%Y %H:%M:%S", std::localtime(&now));
    return output;
}

void Logger::Log(const std::string& message) {
    LogEntry LogEntry;
    LogEntry.type = LOG_INFO;
    LogEntry.message = "Log: [" + CurrentDateTimeToString() + "]: " + message;
    std::cout << "\x1B[32m" << LogEntry.message << "\033[0m" << std::endl;

    messages.push_back(LogEntry);
}

void Logger::Err(const std::string& message) {
    LogEntry LogEntry;
    LogEntry.type = LOG_ERROR;
    LogEntry.message = "Err: [" + CurrentDateTimeToString() + "]: " + message;
    std::cout << "\x1B[91m" << LogEntry.message << "\033[0m" << std::endl;

    messages.push_back(LogEntry);
}