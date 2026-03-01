#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <mutex>
#include "ILogger.hpp"
#include "Models.hpp"

class MainWindow : public ILogger {
public:
    MainWindow();
    bool Create(HINSTANCE hInstance, int nCmdShow);
    int MessageLoop();

    void Log(const std::wstring& message, bool isError = false) override;
    void UpdateStats(int detected, int processed, int failed, int generated, int skipped) override;
    void SetProgress(int current, int total) override;
    bool IsErrorsOnly() const override;

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void OnSize(int width, int height);
    void OnStart();
    void OnBrowseSource();
    void OnBrowseDestination();
    void OnBrowseFFmpeg();
    void OnClearLog();
    void OnExportLog();

    HWND m_hWnd;
    HWND hEditSource, hEditDestination, hEditLog, hChkDryRun, hEditThreads, hChkErrorsOnly, hStaticStats, hProgress, hBtnStart, hEditFFmpeg;
    std::mutex m_logMutex;
};

#endif // MAINWINDOW_HPP
