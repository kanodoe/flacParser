#ifndef AUDIOPROCESSOR_HPP
#define AUDIOPROCESSOR_HPP

#include "Models.hpp"
#include "ILogger.hpp"
#include <atomic>

class AudioProcessor {
public:
    AudioProcessor(ILogger& logger, const JobConfig& config);
    bool ProcessAlbum(const Album& album, std::atomic<int>& generatedCount, std::atomic<int>& skippedCount);
private:
    fs::path LocateAudio(const fs::path& srcDir, const std::wstring& cueAudioRef, const std::wstring& cuePath);
    ILogger& m_logger;
    const JobConfig& m_config;
};

#endif // AUDIOPROCESSOR_HPP
