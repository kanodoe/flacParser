#ifndef CUEPARSER_HPP
#define CUEPARSER_HPP

#include "Models.hpp"
#include <vector>
#include <string>

class CueParser {
public:
    static std::vector<Album> ParseRecursive(const std::wstring& rootPath);
private:
    static Album ParseSingleCue(const fs::path& path);
};

#endif // CUEPARSER_HPP
