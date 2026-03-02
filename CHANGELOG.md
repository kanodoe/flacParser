## Changelog

### v1.5.1 (Current)
- **Portabilidad**: Ahora el ejecutable se enlaza estáticamente con las librerías de MinGW (`libwinpthread`, `libgcc`, `libstdc++`), lo que permite ejecutarlo en otros computadores sin necesidad de instalar DLLs adicionales.
- **Distribución**: Binario actualizado en la carpeta `binary/` para su descarga directa.
- **Seguridad**: Se ha incluido un checksum SHA256 para verificar la integridad del ejecutable.

### v1.5.0
- **Testing Suite**: Added unit tests for `Utils` and `CueParser` to ensure code quality and robustness.
- **Refactoring**: Improved code structure to facilitate automated testing of CUE parsing logic.
- **Documentation**: Updated README with binary download links and testing instructions.

### v1.4.0
- **Robustness**: Fixed processing failures for long filenames by implementing intelligent truncation (max 200 chars).
- **Format Support**: Added explicit `-f flac` to FFmpeg commands to ensure output format consistency.
- **Error Handling**: Improved error reporting when FFmpeg fails due to complex metadata.

### v1.3.0
- **Manual FFmpeg Selection**: Added UI controls to browse and select `ffmpeg.exe` manually.
- **Dynamic Layout**: Improved window resizing logic to keep all controls visible.
- **Advanced Diagnostics**: Log now shows the exact FFmpeg command executed on failure.

### v1.2.0
- **FFmpeg Localization**: Improved search logic using `SearchPathW` and PATH environment variable.
- **Locale Independence**: Forced period (`.`) as decimal separator for FFmpeg parameters, fixing syntax errors on systems with non-English locales (e.g., Spanish).

### v1.1.0
- **Unicode & Long Paths**: Full migration to Wide-string API (`\\?\` prefix) to handle deep folder structures and special characters.
- **Metadata Fixes**: Improved CUE parsing to correctly handle UTF-8/ANSI encodings and escape complex titles.
- **Incremental Logic**: Implementation of `.album_done` tracking.

### v1.0.0
- Initial release with basic GUI, multi-threading, and CUE/FLAC splitting logic.
