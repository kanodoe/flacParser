#ifndef CUEPARSER_HPP
#define CUEPARSER_HPP

#include "Models.hpp"
#include <vector>
#include <string>

class CueParser {
public:
    static std::vector<Album> ParseRecursive(const std::wstring& rootPath);
    static Album ParseString(const std::wstring& content, const fs::path& cuePath);
private:
    static Album ParseSingleCue(const fs::path& path);
};

#endif // CUEPARSER_HPP
