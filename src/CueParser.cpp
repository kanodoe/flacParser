#include "../include/CueParser.hpp"
#include "../include/Utils.hpp"
#include <sstream>
#include <regex>

std::vector<Album> CueParser::ParseRecursive(const std::wstring& rootPath) {
    std::vector<Album> albums;
    std::error_code ec;
    
    // Convert to absolute path and normalize before adding prefix
    fs::path absRoot = fs::absolute(rootPath, ec);
    if (ec) absRoot = rootPath;
    
    std::wstring sourcePath = Utils::LongPathPrefix(absRoot.wstring());

    for (auto it = fs::recursive_directory_iterator(sourcePath, fs::directory_options::skip_permission_denied, ec); 
         it != fs::end(it); it.increment(ec)) {
        if (ec) {
            ec.clear();
            continue;
        }
        try {
            if (it->path().extension() == L".cue") {
                Album album = ParseSingleCue(it->path());
                if (!album.tracks.empty()) {
                    albums.push_back(album);
                }
            }
        } catch (...) {}
    }
    return albums;
}

Album CueParser::ParseSingleCue(const fs::path& path) {
    Album album;
    album.cuePath = path.wstring();
    album.srcDir = path.parent_path();

    std::wstring content = Utils::ReadFileAsWide(path);
    if (content.empty()) return album;

    std::wstringstream ss(content);
    std::wstring line;
    std::wstring currentAudio;
    Track* currentTrack = nullptr;

    std::wregex reFile(LR"(FILE\s+\"([^\"]+)\")");
    std::wregex reFileNoQuotes(LR"(FILE\s+(.+)\s+\w+)");
    std::wregex reTrack(LR"(TRACK\s+(\d+)\s+\w+)");
    std::wregex reTitle(LR"(TITLE\s+\"?(.+?)\"?$)");
    std::wregex rePerformer(LR"(PERFORMER\s+\"?(.+?)\"?$)");
    std::wregex reDate(LR"(REM\s+DATE\s+(\d+))");
    std::wregex reGenre(LR"(REM\s+GENRE\s+\"?(.+?)\"?$)");
    std::wregex reIndex(LR"(INDEX\s+01\s+(\d+):(\d+):(\d+))");

    while (std::getline(ss, line)) {
        line.erase(0, line.find_first_not_of(L" \t\r\n"));
        line.erase(line.find_last_not_of(L" \t\r\n") + 1);

        std::wsmatch matches;
        if (std::regex_search(line, matches, reFile)) {
            currentAudio = matches[1].str();
        } else if (std::regex_search(line, matches, reFileNoQuotes)) {
            currentAudio = matches[1].str();
        } else if (std::regex_search(line, matches, reTrack)) {
            album.tracks.push_back({});
            currentTrack = &album.tracks.back();
            currentTrack->number = matches[1].str();
            currentTrack->audioFile = currentAudio;
            currentTrack->title = L"Track " + currentTrack->number;
            currentTrack->artist = album.artist; 
        } else if (std::regex_search(line, matches, reTitle)) {
            if (currentTrack) currentTrack->title = matches[1].str();
            else album.title = matches[1].str();
        } else if (std::regex_search(line, matches, rePerformer)) {
            if (currentTrack) currentTrack->artist = matches[1].str();
            else album.artist = matches[1].str();
        } else if (std::regex_search(line, matches, reDate)) {
            album.date = matches[1].str();
        } else if (std::regex_search(line, matches, reGenre)) {
            album.genre = matches[1].str();
        } else if (currentTrack && std::regex_search(line, matches, reIndex)) {
            try {
                int mm = std::stoi(matches[1].str());
                int ss_idx = std::stoi(matches[2].str());
                int ff = std::stoi(matches[3].str());
                currentTrack->start = mm * 60 + ss_idx + ff / 75.0;
            } catch (...) {}
        }
    }
    return album;
}
