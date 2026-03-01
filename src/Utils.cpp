#include "../include/Utils.hpp"
#include <windows.h>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cwchar>

namespace Utils {

std::wstring SanitizeFileName(std::wstring name) {
    std::wstring invalid = L"\\/:*?\"<>|";
    for (wchar_t &c : name) {
        if (invalid.find(c) != std::wstring::npos || (unsigned int)c < 32) c = L'_';
    }
    return name;
}

std::wstring EscapeParam(const std::wstring& p) {
    std::wstring escaped;
    for (wchar_t c : p) {
        if (c == L'\"') escaped += L"\\\"";
        else escaped += c;
    }
    return escaped;
}

std::wstring ReadFileAsWide(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return L"";
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (content.empty()) return L"";

    if (content.size() >= 2 && (unsigned char)content[0] == 0xFF && (unsigned char)content[1] == 0xFE) {
        std::wstring wstr((content.size() - 2) / 2, 0);
        std::memcpy(&wstr[0], content.data() + 2, wstr.size() * 2);
        return wstr;
    }

    int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, content.data(), (int)content.size(), NULL, 0);
    if (wlen > 0) {
        std::wstring wstr(wlen, 0);
        MultiByteToWideChar(CP_UTF8, 0, content.data(), (int)content.size(), &wstr[0], wlen);
        if (!wstr.empty() && (unsigned short)wstr[0] == 0xFEFF) wstr.erase(0, 1);
        return wstr;
    }

    wlen = MultiByteToWideChar(CP_ACP, 0, content.data(), (int)content.size(), NULL, 0);
    if (wlen > 0) {
        std::wstring wstr(wlen, 0);
        MultiByteToWideChar(CP_ACP, 0, content.data(), (int)content.size(), &wstr[0], wlen);
        return wstr;
    }
    
    return L"";
}

std::wstring GetLastErrorAsString() {
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) return L"";
    LPWSTR messageBuffer = nullptr;
    size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
    std::wstring message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

std::wstring RunCommandAndCaptureStderr(const std::wstring& cmd, int& exitCode) {
    HANDLE hStdErrRead = NULL;
    HANDLE hStdErrWrite = NULL;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hStdErrRead, &hStdErrWrite, &saAttr, 0)) {
        exitCode = -1;
        return L"Failed to create pipe: " + GetLastErrorAsString();
    }
    if (!SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0)) {
        exitCode = -1;
        return L"Failed to set handle information: " + GetLastErrorAsString();
    }

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hStdErrWrite;
    si.hStdOutput = hStdErrWrite;
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

    std::vector<wchar_t> cmdBuffer(cmd.begin(), cmd.end());
    cmdBuffer.push_back(0);

    if (!CreateProcessW(NULL, cmdBuffer.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hStdErrRead);
        CloseHandle(hStdErrWrite);
        exitCode = -1;
        return L"CreateProcess failed: " + GetLastErrorAsString() + L"\nCommand: " + cmd;
    }

    CloseHandle(hStdErrWrite);

    std::string outputA;
    char buffer[4096];
    DWORD dwRead;
    while (ReadFile(hStdErrRead, buffer, sizeof(buffer) - 1, &dwRead, NULL) && dwRead > 0) {
        buffer[dwRead] = 0;
        outputA.append(buffer, dwRead);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD dwExitCode;
    GetExitCodeProcess(pi.hProcess, &dwExitCode);
    exitCode = (int)dwExitCode;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hStdErrRead);

    int wlen = MultiByteToWideChar(CP_UTF8, 0, outputA.data(), (int)outputA.size(), NULL, 0);
    if (wlen > 0) {
        std::wstring wstr(wlen, 0);
        MultiByteToWideChar(CP_UTF8, 0, outputA.data(), (int)outputA.size(), &wstr[0], wlen);
        return wstr;
    }
    wlen = MultiByteToWideChar(CP_ACP, 0, outputA.data(), (int)outputA.size(), NULL, 0);
    if (wlen > 0) {
        std::wstring wstr(wlen, 0);
        MultiByteToWideChar(CP_ACP, 0, outputA.data(), (int)outputA.size(), &wstr[0], wlen);
        return wstr;
    }

    return L"";
}

std::wstring LocateFFmpeg() {
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) > 0) {
        fs::path p(exePath);
        fs::path parent = p.parent_path();
        
        // Try same folder
        fs::path ffmpeg = parent / L"ffmpeg.exe";
        if (fs::exists(ffmpeg)) return ffmpeg.wstring();
        
        // Try parent folder (e.g. if running from cmake-build-debug)
        ffmpeg = parent.parent_path() / L"ffmpeg.exe";
        if (fs::exists(ffmpeg)) return ffmpeg.wstring();

        // Try external_script folder
        ffmpeg = parent.parent_path() / L"external_script" / L"ffmpeg.exe";
        if (fs::exists(ffmpeg)) return ffmpeg.wstring();
    }
    
    // Try to find in PATH using Windows API
    wchar_t foundPath[MAX_PATH];
    wchar_t* filePart;
    if (SearchPathW(NULL, L"ffmpeg.exe", NULL, MAX_PATH, foundPath, &filePart) > 0) {
        return foundPath;
    }
    
    return L""; // Empty means not found
}

std::wstring FormatTrackFilename(int trackNum, const std::wstring& title, const std::wstring& extension) {
    std::wstring safeTitle = SanitizeFileName(title);
    
    wchar_t numBuf[32];
    swprintf(numBuf, 32, L"%02d - ", trackNum);
    std::wstring prefix = numBuf;
    
    const size_t maxFilenameLen = 200;
    size_t fixedLen = prefix.length() + extension.length();
    
    if (fixedLen + safeTitle.length() > maxFilenameLen) {
        size_t allowedTitleLen = (maxFilenameLen > fixedLen) ? (maxFilenameLen - fixedLen) : 0;
        if (allowedTitleLen > 0) {
            safeTitle = safeTitle.substr(0, allowedTitleLen);
            while (!safeTitle.empty() && safeTitle.back() == L' ') safeTitle.pop_back();
        } else {
            safeTitle = L"";
        }
    }
    
    return prefix + safeTitle + extension;
}

std::wstring LongPathPrefix(const std::wstring& path) {
    if (path.empty()) return path;
    
    // Normalize to backslashes
    std::wstring normalized = path;
    std::replace(normalized.begin(), normalized.end(), L'/', L'\\');

    if (normalized.length() > 240 && normalized.find(L"\\\\?\\") == std::wstring::npos) {
        if (normalized.length() >= 2 && normalized[1] == L':') {
            return L"\\\\?\\" + normalized;
        }
        // For UNC paths: \\?\UNC\server\share
        if (normalized.length() >= 2 && normalized[0] == L'\\' && normalized[1] == L'\\') {
            return L"\\\\?\\UNC" + normalized.substr(1);
        }
    }
    return normalized;
}

} // namespace Utils
