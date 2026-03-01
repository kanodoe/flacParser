#ifndef ILOGGER_HPP
#define ILOGGER_HPP

#include <string>

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void Log(const std::wstring& message, bool isError = false) = 0;
    virtual void UpdateStats(int detected, int processed, int failed, int generated, int skipped) = 0;
    virtual void SetProgress(int current, int total) = 0;
    virtual bool IsErrorsOnly() const = 0;
};

#endif // ILOGGER_HPP
