#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace Utils {
    std::wstring SanitizeFileName(std::wstring name);
    std::wstring EscapeParam(const std::wstring& p);
    std::wstring ReadFileAsWide(const fs::path& path);
    std::wstring GetLastErrorAsString();
    std::wstring RunCommandAndCaptureStderr(const std::wstring& cmd, int& exitCode);
    std::wstring LocateFFmpeg();
    std::wstring FormatTrackFilename(int trackNum, const std::wstring& title, const std::wstring& extension = L".flac");
    std::wstring LongPathPrefix(const std::wstring& path);
}

#endif // UTILS_HPP
