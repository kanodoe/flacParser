#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

struct Track {
    std::wstring number;
    std::wstring title;
    std::wstring artist;
    double start = 0.0;
    std::wstring audioFile;
};

struct Album {
    std::wstring cuePath;
    std::wstring title;
    std::wstring artist;
    std::wstring genre;
    std::wstring date;
    std::vector<Track> tracks;
    fs::path srcDir;
};

struct JobConfig {
    std::wstring source;
    std::wstring destination;
    int threads;
    bool dryRun;
    bool errorsOnly;
    std::wstring ffmpegPath;
};

#endif // MODELS_HPP
