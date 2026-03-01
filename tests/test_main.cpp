#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "../include/Utils.hpp"
#include "../include/CueParser.hpp"

struct TestCase {
    std::string name;
    std::function<bool()> func;
};

std::vector<TestCase> tests;

void AddTest(const std::string& name, std::function<bool()> func) {
    tests.push_back({name, func});
}

#define ASSERT_TRUE(condition) if (!(condition)) { std::wcerr << L"Assertion failed: " << #condition << std::endl; return false; }
#define ASSERT_EQ(val1, val2) if ((val1) != (val2)) { std::wcerr << L"Assertion failed: " << val1 << L" == " << val2 << std::endl; return false; }

bool TestSanitizeFileName() {
    ASSERT_EQ(Utils::SanitizeFileName(L"normal.flac"), L"normal.flac");
    ASSERT_EQ(Utils::SanitizeFileName(L"invalid/file:name*.flac"), L"invalid_file_name_.flac");
    ASSERT_EQ(Utils::SanitizeFileName(L"control\x01" L"char.flac"), L"control_char.flac");
    return true;
}

bool TestEscapeParam() {
    ASSERT_EQ(Utils::EscapeParam(L"no quotes"), L"no quotes");
    ASSERT_EQ(Utils::EscapeParam(L"with \"quotes\""), L"with \\\"quotes\\\"");
    return true;
}

bool TestFormatTrackFilename() {
    ASSERT_EQ(Utils::FormatTrackFilename(1, L"Title"), L"01 - Title.flac");
    ASSERT_EQ(Utils::FormatTrackFilename(12, L"Very long title that should be truncated eventually if it was much longer than this for real but we will see"), L"12 - Very long title that should be truncated eventually if it was much longer than this for real but we will see.flac");
    
    std::wstring longTitle(300, L'A');
    std::wstring formatted = Utils::FormatTrackFilename(5, longTitle);
    ASSERT_TRUE(formatted.length() <= 210); // 200 + some margin for prefix/ext if implementation allows
    ASSERT_TRUE(formatted.find(L"05 - ") == 0);
    return true;
}

bool TestLongPathPrefix() {
    ASSERT_EQ(Utils::LongPathPrefix(L"C:/short/path"), L"C:\\short\\path");
    std::wstring longPath = L"C:\\";
    for(int i=0; i<25; ++i) longPath += L"verylongfoldername\\";
    std::wstring prefixed = Utils::LongPathPrefix(longPath);
    ASSERT_TRUE(prefixed.find(L"\\\\?\\") == 0);
    return true;
}

bool TestCueParser() {
    std::wstring cueContent = 
        L"PERFORMER \"Artist Name\"\n"
        L"TITLE \"Album Title\"\n"
        L"REM DATE 2023\n"
        L"REM GENRE Rock\n"
        L"FILE \"audio.flac\" WAVE\n"
        L"  TRACK 01 AUDIO\n"
        L"    TITLE \"Track 01\"\n"
        L"    INDEX 01 00:00:00\n"
        L"  TRACK 02 AUDIO\n"
        L"    TITLE \"Track 02\"\n"
        L"    PERFORMER \"Other Artist\"\n"
        L"    INDEX 01 05:30:00\n";
    
    Album album = CueParser::ParseString(cueContent, L"C:\\test.cue");
    ASSERT_EQ(album.artist, L"Artist Name");
    ASSERT_EQ(album.title, L"Album Title");
    ASSERT_EQ(album.date, L"2023");
    ASSERT_EQ(album.genre, L"Rock");
    ASSERT_EQ(album.tracks.size(), (size_t)2);
    
    ASSERT_EQ(album.tracks[0].number, L"01");
    ASSERT_EQ(album.tracks[0].title, L"Track 01");
    ASSERT_EQ(album.tracks[0].artist, L"Artist Name");
    ASSERT_EQ(album.tracks[0].start, 0.0);
    
    ASSERT_EQ(album.tracks[1].number, L"02");
    ASSERT_EQ(album.tracks[1].title, L"Track 02");
    ASSERT_EQ(album.tracks[1].artist, L"Other Artist");
    ASSERT_EQ(album.tracks[1].start, 330.0); // 5*60 + 30
    
    return true;
}

int main() {
    AddTest("SanitizeFileName", TestSanitizeFileName);
    AddTest("EscapeParam", TestEscapeParam);
    AddTest("FormatTrackFilename", TestFormatTrackFilename);
    AddTest("LongPathPrefix", TestLongPathPrefix);
    AddTest("CueParser", TestCueParser);

    int passed = 0;
    for (const auto& test : tests) {
        std::cout << "Running " << test.name << "... ";
        if (test.func()) {
            std::cout << "PASSED" << std::endl;
            passed++;
        } else {
            std::cout << "FAILED" << std::endl;
        }
    }

    std::cout << "\nResults: " << passed << "/" << tests.size() << " tests passed." << std::endl;
    return (passed == (int)tests.size()) ? 0 : 1;
}
