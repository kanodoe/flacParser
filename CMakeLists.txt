cmake_minimum_required(VERSION 3.10)
project(flacParser CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find necessary Windows libraries
set(LIBS comdlg32 shlwapi shell32 user32 gdi32 comctl32)

# Ensure static linking for MinGW to avoid missing DLL errors (libwinpthread, libgcc, libstdc++)
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

set(SOURCES
    main.cpp
    src/Utils.cpp
    src/CueParser.cpp
    src/AudioProcessor.cpp
    src/ThreadManager.cpp
    src/MainWindow.cpp
)

add_executable(flacParser WIN32 ${SOURCES})
target_include_directories(flacParser PRIVATE include)
target_link_libraries(flacParser PRIVATE ${LIBS})

# Tests
set(TEST_SOURCES
    tests/test_main.cpp
    src/Utils.cpp
    src/CueParser.cpp
)
add_executable(flacParserTests ${TEST_SOURCES})
target_include_directories(flacParserTests PRIVATE include)
target_link_libraries(flacParserTests PRIVATE comdlg32 shlwapi user32)

# Copy the executable to the root after building
add_custom_command(TARGET flacParser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:flacParser> ${CMAKE_SOURCE_DIR}/flacParser.exe
)
