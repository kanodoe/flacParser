#ifndef THREADMANAGER_HPP
#define THREADMANAGER_HPP

#include "Models.hpp"
#include "ILogger.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

class ThreadManager {
public:
    ThreadManager(ILogger& logger, const JobConfig& config);
    ~ThreadManager();
    void Run();
    void Stop();
    bool IsRunning() const;

private:
    void Worker();
    ILogger& m_logger;
    JobConfig m_config;
    std::vector<std::thread> m_workers;
    std::queue<Album> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    std::atomic<bool> m_stop{false};
    std::atomic<bool> m_running{false};

    std::atomic<int> m_detected{0};
    std::atomic<int> m_processed{0};
    std::atomic<int> m_failed{0};
    std::atomic<int> m_generatedCount{0};
    std::atomic<int> m_skippedCount{0};
    std::atomic<int> m_finishedCount{0};
};

#endif // THREADMANAGER_HPP
