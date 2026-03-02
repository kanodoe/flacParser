# FLAC CUE Splitter Pro

**High-performance Windows desktop tool to split FLAC albums using CUE sheets. Features multithreading, full metadata preservation, Unicode/Long Path support, and a resizable native UI. Automates the entire pipeline from detection to tagging with FFmpeg integration. Built in C++ for speed and efficiency. Licensed under GNU GPL v3.**

A native Windows desktop application (C++) designed to automate the splitting of large FLAC audio files using their associated `.cue` sheet.

## Downloads

You can download the precompiled standalone executable from the `binary/` folder in this repository.

- [**Download flacParser.exe**](./binary/flacParser.exe) ([SHA256 Checksum](./binary/flacParser.exe.sha256))

### Verification (v1.5.1)
To ensure the integrity of the downloaded file, you can verify its SHA256 checksum:
- **Checksum**: `fbb92282cce7dc31c96989f5a34137ba66fbbd9ba7ee3829161a3df7ee3d98bc`
- **Verification Command (Windows)**: `certutil -hashfile flacParser.exe SHA256`

*Note: Make sure to have `ffmpeg.exe` available as per the Requirements section.*

## Main Features

- **Intuitive Graphical Interface**: Source and destination folder selection using native Windows dialogs.
- **Multithreaded Processing**: Leverages all your CPU cores to process multiple albums in parallel.
- **FFmpeg Integration**: Automatic or manual selection of `ffmpeg.exe` with real-time log output.
- **Intelligent Audio Detection**: Smartly searches for the audio file associated with the `.cue` (supports `.flac`, `.wav`, `.ape`, `.wv`).
- **Metadata Preservation**: Extracts Artist, Album, Title, Date, and Genre directly from the `.cue` and injects them into the output files.
- **CUE-based Filename Generation**: Automatically generates files with the format `TrackNum - Title.flac` based on CUE info.
- **Incremental Mode**: Uses hidden `.album_done` files to skip already processed albums and speed up future runs.
- **Unicode & Long Path Support**: Correctly handles files with special characters (Chinese, Cyrillic, etc.) and paths exceeding the 260-character Windows limit.
- **Resizable Interface**: Modern UI that adapts to any window size with a real-time progress bar.

## Requirements

- Windows 10/11.
- `ffmpeg.exe` (can be placed in the application directory, project root, or anywhere in your system PATH).

## How to Use

1. If the program doesn't auto-detect `ffmpeg.exe`, select it manually using the **...** button in the **FFmpeg** row.
2. Select the **Source** folder containing your CUE/FLAC files.
3. Select the **Destination** folder where you want to save the split tracks.
4. Adjust the number of **Threads** based on your processor power.
5. Click **START PROCESS**.
6. Use **Dry Run** to simulate the process without generating real files (planned output will be shown in the log).
7. Upon successful completion, a hidden `.album_done` file is created in the destination folder to prevent re-processing.

## Development & Testing

This project includes a suite of unit tests to ensure the reliability of the core logic (`Utils`, `CueParser`).

### Running Tests
To compile and run the tests, use CMake:
```powershell
cmake --build cmake-build-debug --target flacParserTests
.\cmake-build-debug\flacParserTests.exe
```

## Credits

Developed in C++ using Win32 API and FFmpeg.

## License

This project is licensed under the **GNU General Public License v3.0**.
