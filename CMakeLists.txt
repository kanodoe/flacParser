cmake_minimum_required(VERSION 3.10)
project(flacParser CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find necessary Windows libraries
set(LIBS comdlg32 shlwapi shell32 user32 gdi32 comctl32)

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

# Copy the executable to the root after building
add_custom_command(TARGET flacParser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:flacParser> ${CMAKE_SOURCE_DIR}/flacParser.exe
)
