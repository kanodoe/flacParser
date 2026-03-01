#include "../include/AudioProcessor.hpp"
#include "../include/Utils.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <locale>

AudioProcessor::AudioProcessor(ILogger& logger, const JobConfig& config) 
    : m_logger(logger), m_config(config) {}

bool AudioProcessor::ProcessAlbum(const Album& album, std::atomic<int>& generatedCount, std::atomic<int>& skippedCount) {
    // Ensure we handle relative paths correctly even if one has the long path prefix
    auto StripPrefix = [](const std::wstring& p) {
        if (p.find(L"\\\\?\\") == 0) return p.substr(4);
        return p;
    };
    
    fs::path cleanSrcDir = StripPrefix(album.srcDir.wstring());
    fs::path cleanSource = StripPrefix(m_config.source);
    fs::path rel = fs::relative(cleanSrcDir, cleanSource);
    
    std::wstring destBase = Utils::LongPathPrefix(m_config.destination);
    fs::path outDir = fs::path(destBase) / rel;

    fs::path doneFile = Utils::LongPathPrefix((outDir / L".album_done").wstring());
    if (fs::exists(doneFile)) {
        m_logger.Log(L"  Skipping (already done): " + album.cuePath);
        skippedCount += (int)album.tracks.size();
        return true;
    }

    if (!m_config.dryRun) {
        std::error_code ec;
        fs::create_directories(outDir, ec);
    }

    bool albumSuccess = true;
    for (size_t i = 0; i < album.tracks.size(); ++i) {
        const auto& t = album.tracks[i];
        
        int trackNum = 0;
        try { trackNum = std::stoi(t.number); } catch (...) {}
        
        std::wstring filename = Utils::FormatTrackFilename(trackNum, t.title);
        fs::path outFile = Utils::LongPathPrefix((outDir / filename).wstring());

        if (fs::exists(outFile)) {
            skippedCount++;
            continue;
        }

        fs::path audioPath = LocateAudio(album.srcDir, t.audioFile, album.cuePath);
        if (!audioPath.empty()) {
            audioPath = Utils::LongPathPrefix(audioPath.wstring());
        }

        if (audioPath.empty() || !fs::exists(audioPath)) {
            m_logger.Log(L"  ERROR: Audio not found for track " + t.number + L" in " + album.cuePath, true);
            albumSuccess = false;
            continue;
        }

        if (m_config.dryRun) {
            m_logger.Log(L"  [DRY] " + outFile.wstring());
            continue;
        }

        // Format times and command with C locale to ensure dot as decimal separator
        std::basic_stringstream<wchar_t> cmd;
        cmd.imbue(std::locale::classic());
        cmd << std::fixed << std::setprecision(3);

        cmd << L"\"" << m_config.ffmpegPath << L"\" -hide_banner -loglevel error -ss " << t.start;
        
        if (i + 1 < album.tracks.size() && album.tracks[i+1].audioFile == t.audioFile && album.tracks[i+1].start > t.start) {
            double dur = album.tracks[i+1].start - t.start;
            cmd << L" -t " << dur;
        }
        cmd << L" -i \"" << audioPath.wstring() << L"\"";
        
        // Set metadata from CUE
        cmd << L" -vn -c:a flac";
        cmd << L" -metadata title=\"" << Utils::EscapeParam(t.title) << L"\"";
        cmd << L" -metadata artist=\"" << Utils::EscapeParam(t.artist) << L"\"";
        cmd << L" -metadata album=\"" << Utils::EscapeParam(album.title) << L"\"";
        cmd << L" -metadata track=\"" << Utils::EscapeParam(t.number) << L"\"";
        if (!album.genre.empty()) cmd << L" -metadata genre=\"" << Utils::EscapeParam(album.genre) << L"\"";
        if (!album.date.empty()) cmd << L" -metadata date=\"" << Utils::EscapeParam(album.date) << L"\"";

        cmd << L" -f flac -y \"" << outFile.wstring() << L"\"";
        
        int exitCode = 0;
        std::wstring commandLine = cmd.str();

        std::wstring errorOutput = Utils::RunCommandAndCaptureStderr(commandLine, exitCode);

        if (exitCode == 0) {
            generatedCount++;
        } else {
            m_logger.Log(L"  ERROR: ffmpeg failed on track " + t.number + L" in " + album.cuePath, true);
            m_logger.Log(L"  Command: " + commandLine, true);
            if (!errorOutput.empty()) {
                m_logger.Log(L"  Details: " + errorOutput, true);
            }
            albumSuccess = false;
        }
    }

    if (albumSuccess && !m_config.dryRun) {
        std::ofstream done(doneFile);
    }
    return albumSuccess;
}

fs::path AudioProcessor::LocateAudio(const fs::path& srcDir, const std::wstring& cueAudioRef, const std::wstring& cuePath) {
    fs::path p = srcDir / cueAudioRef;
    if (fs::exists(p)) return p;

    std::vector<std::wstring> exts = {L".flac", L".wav", L".ape", L".wv"};
    for (auto& ext : exts) {
        fs::path tmp = p;
        tmp.replace_extension(ext);
        if (fs::exists(tmp)) return tmp;
    }

    fs::path cueP(cuePath);
    std::wstring cueBase = cueP.stem().wstring();
    size_t lastDot = cueBase.find_last_of(L'.');
    if (lastDot != std::wstring::npos) {
        std::wstring lastExt = cueBase.substr(lastDot);
        if (lastExt == L".flac" || lastExt == L".wav" || lastExt == L".ape" || lastExt == L".wv") {
            cueBase = cueBase.substr(0, lastDot);
        }
    }

    for (auto& ext : exts) {
        fs::path tmp = srcDir / (cueBase + ext);
        if (fs::exists(tmp)) return tmp;
    }

    std::vector<fs::path> audioFiles;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(srcDir, ec)) {
        if (ec) break;
        std::wstring ext = entry.path().extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == L".flac" || ext == L".wav" || ext == L".ape" || ext == L".wv") {
            audioFiles.push_back(entry.path());
        }
    }

    if (audioFiles.size() == 1) return audioFiles[0];
    
    if (audioFiles.size() > 1) {
        for (const auto& af : audioFiles) {
            if (af.stem().wstring().find(cueBase) != std::wstring::npos) return af;
        }
        return audioFiles[0];
    }
    
    m_logger.Log(L"  ERROR: Could not find audio file for " + cuePath, true);
    m_logger.Log(L"  Searched for: " + cueAudioRef + L" and variants in " + srcDir.wstring(), true);
    return L"";
}
