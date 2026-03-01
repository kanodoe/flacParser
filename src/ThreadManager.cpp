#include "../include/ThreadManager.hpp"
#include "../include/CueParser.hpp"
#include "../include/AudioProcessor.hpp"
#include <chrono>

ThreadManager::ThreadManager(ILogger& logger, const JobConfig& config)
    : m_logger(logger), m_config(config) {}

ThreadManager::~ThreadManager() {
    Stop();
}

void ThreadManager::Run() {
    m_running = true;
    m_stop = false;
    m_detected = 0;
    m_processed = 0;
    m_failed = 0;
    m_generatedCount = 0;
    m_skippedCount = 0;
    m_finishedCount = 0;

    m_logger.Log(L"Scanning for CUE files in: " + m_config.source);
    auto albums = CueParser::ParseRecursive(m_config.source);
    m_detected = (int)albums.size();
    
    m_logger.Log(L"Found " + std::to_wstring(m_detected) + L" CUE files.");
    m_logger.UpdateStats(m_detected, 0, 0, 0, 0);
    m_logger.SetProgress(0, m_detected);

    if (m_detected == 0) {
        m_logger.Log(L"No CUE files found in source directory.");
        m_running = false;
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        for (const auto& a : albums) {
            m_tasks.push(a);
        }
    }

    int threadCount = std::min(m_config.threads, (int)m_detected.load());
    if (threadCount <= 0) threadCount = 1;

    for (int i = 0; i < threadCount; ++i) {
        m_workers.emplace_back(&ThreadManager::Worker, this);
    }

    for (auto& w : m_workers) {
        if (w.joinable()) w.join();
    }
    
    m_logger.Log(L"Process finished.");
    m_running = false;
}

void ThreadManager::Stop() {
    m_stop = true;
    m_condition.notify_all();
    // No podemos hacer join aquí si Stop() es llamado desde el hilo que Run() está esperando.
    // Pero Run() hace join de los workers.
}

bool ThreadManager::IsRunning() const {
    return m_running;
}

void ThreadManager::Worker() {
    AudioProcessor processor(m_logger, m_config);
    while (!m_stop) {
        Album album;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            if (m_tasks.empty()) break;
            album = std::move(m_tasks.front());
            m_tasks.pop();
        }

        m_logger.Log(L"Processing: " + album.cuePath);
        bool success = processor.ProcessAlbum(album, m_generatedCount, m_skippedCount);
        if (success) m_processed++;
        else m_failed++;

        m_finishedCount++;
        m_logger.UpdateStats(m_detected, m_processed, m_failed, m_generatedCount, m_skippedCount);
        m_logger.SetProgress(m_finishedCount, m_detected);
    }
}
