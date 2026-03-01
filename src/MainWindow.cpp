#include "../include/MainWindow.hpp"
#include "../include/ThreadManager.hpp"
#include "../include/Utils.hpp"
#include <shlobj.h>
#include <commdlg.h>
#include <fstream>
#include <thread>
#include <algorithm>

#define ID_BTN_SOURCE 101
#define ID_BTN_DESTINATION 102
#define ID_BTN_START 103
#define ID_BTN_CLEAR 104
#define ID_BTN_EXPORT 105
#define ID_BTN_FFMPEG 106
#define ID_EDIT_SOURCE 201
#define ID_EDIT_DESTINATION 202
#define ID_EDIT_LOG 203
#define ID_CHK_DRYRUN 204
#define ID_EDIT_THREADS 205
#define ID_CHK_ERRORS_ONLY 206
#define ID_STATIC_STATS 207
#define ID_PROGRESS 208
#define ID_EDIT_FFMPEG 209

MainWindow::MainWindow() : m_hWnd(NULL), hEditSource(NULL), hEditDestination(NULL), hEditLog(NULL), 
                           hChkDryRun(NULL), hEditThreads(NULL), hChkErrorsOnly(NULL), hStaticStats(NULL), 
                           hProgress(NULL), hBtnStart(NULL), hEditFFmpeg(NULL) {}

bool MainWindow::Create(HINSTANCE hInstance, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, NULL, LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), NULL, L"FlacParserGUI", NULL };
    RegisterClassExW(&wc);

    m_hWnd = CreateWindowExW(0, L"FlacParserGUI", L"FLAC CUE Splitter Pro", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 550, 600, NULL, NULL, hInstance, this);
    if (!m_hWnd) return false;

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    return true;
}

int MainWindow::MessageLoop() {
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

void MainWindow::Log(const std::wstring& message, bool isError) {
    if (IsErrorsOnly() && !isError) return;
    std::lock_guard<std::mutex> lock(m_logMutex);
    std::wstring text = message + L"\r\n";
    int len = GetWindowTextLengthW(hEditLog);
    SendMessageW(hEditLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageW(hEditLog, EM_REPLACESEL, 0, (LPARAM)text.c_str());
}

void MainWindow::UpdateStats(int detected, int processed, int failed, int generated, int skipped) {
    wchar_t buffer[256];
    swprintf(buffer, 256, L"Detected: %d | Processed: %d | Failed: %d | Generated: %d | Skipped: %d", 
            detected, processed, failed, generated, skipped);
    SetWindowTextW(hStaticStats, buffer);
}

void MainWindow::SetProgress(int current, int total) {
    if (total > 0) {
        int pos = (int)((current / (float)total) * 100);
        SendMessage(hProgress, PBM_SETPOS, pos, 0);
    } else {
        SendMessage(hProgress, PBM_SETPOS, 0, 0);
    }
}

bool MainWindow::IsErrorsOnly() const {
    return SendMessage(hChkErrorsOnly, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = NULL;
    if (msg == WM_NCCREATE) {
        pThis = static_cast<MainWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
        if (pThis) {
            pThis->m_hWnd = hWnd;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        }
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis) return pThis->HandleMessage(msg, wParam, lParam);
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

LRESULT MainWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        CreateWindowW(L"STATIC", L"Source:", WS_VISIBLE | WS_CHILD, 10, 15, 75, 20, m_hWnd, NULL, NULL, NULL);
        hEditSource = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 90, 12, 350, 25, m_hWnd, (HMENU)ID_EDIT_SOURCE, NULL, NULL);
        CreateWindowW(L"BUTTON", L"...", WS_VISIBLE | WS_CHILD, 450, 12, 30, 25, m_hWnd, (HMENU)ID_BTN_SOURCE, NULL, NULL);

        CreateWindowW(L"STATIC", L"Destination:", WS_VISIBLE | WS_CHILD, 10, 55, 80, 20, m_hWnd, NULL, NULL, NULL);
        hEditDestination = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 90, 52, 350, 25, m_hWnd, (HMENU)ID_EDIT_DESTINATION, NULL, NULL);
        CreateWindowW(L"BUTTON", L"...", WS_VISIBLE | WS_CHILD, 450, 52, 30, 25, m_hWnd, (HMENU)ID_BTN_DESTINATION, NULL, NULL);

        CreateWindowW(L"STATIC", L"FFmpeg:", WS_VISIBLE | WS_CHILD, 10, 95, 80, 20, m_hWnd, NULL, NULL, NULL);
        hEditFFmpeg = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 90, 92, 350, 25, m_hWnd, (HMENU)ID_EDIT_FFMPEG, NULL, NULL);
        CreateWindowW(L"BUTTON", L"...", WS_VISIBLE | WS_CHILD, 450, 92, 30, 25, m_hWnd, (HMENU)ID_BTN_FFMPEG, NULL, NULL);

        CreateWindowW(L"STATIC", L"Threads:", WS_VISIBLE | WS_CHILD, 10, 135, 70, 20, m_hWnd, NULL, NULL, NULL);
        hEditThreads = CreateWindowW(L"EDIT", L"4", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 90, 132, 40, 25, m_hWnd, (HMENU)ID_EDIT_THREADS, NULL, NULL);
        hChkDryRun = CreateWindowW(L"BUTTON", L"Dry Run (Simulation)", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 150, 132, 150, 25, m_hWnd, (HMENU)ID_CHK_DRYRUN, NULL, NULL);
        hBtnStart = CreateWindowW(L"BUTTON", L"START PROCESS", WS_VISIBLE | WS_CHILD, 330, 132, 150, 30, m_hWnd, (HMENU)ID_BTN_START, NULL, NULL);

        hChkErrorsOnly = CreateWindowW(L"BUTTON", L"Errors only", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 170, 100, 25, m_hWnd, (HMENU)ID_CHK_ERRORS_ONLY, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Clear Log", WS_VISIBLE | WS_CHILD, 270, 170, 100, 25, m_hWnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Export Log", WS_VISIBLE | WS_CHILD, 380, 170, 100, 25, m_hWnd, (HMENU)ID_BTN_EXPORT, NULL, NULL);

        hEditLog = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 10, 205, 470, 250, m_hWnd, (HMENU)ID_EDIT_LOG, NULL, NULL);
        
        hProgress = CreateWindowExW(0, PROGRESS_CLASSW, NULL, WS_VISIBLE | WS_CHILD | PBS_SMOOTH, 10, 465, 470, 20, m_hWnd, (HMENU)ID_PROGRESS, NULL, NULL);
        hStaticStats = CreateWindowW(L"STATIC", L"Detected: 0 | Processed: 0 | Failed: 0 | Generated: 0 | Skipped: 0", WS_VISIBLE | WS_CHILD, 10, 495, 470, 20, m_hWnd, (HMENU)ID_STATIC_STATS, NULL, NULL);
        
        SetWindowTextW(hEditFFmpeg, Utils::LocateFFmpeg().c_str());
        break;
    }
    case WM_SIZE:
        OnSize(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_SOURCE: OnBrowseSource(); break;
        case ID_BTN_DESTINATION: OnBrowseDestination(); break;
        case ID_BTN_FFMPEG: OnBrowseFFmpeg(); break;
        case ID_BTN_CLEAR: OnClearLog(); break;
        case ID_BTN_EXPORT: OnExportLog(); break;
        case ID_BTN_START: OnStart(); break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_APP + 1:
        EnableWindow(hBtnStart, TRUE);
        break;
    default:
        return DefWindowProcW(m_hWnd, msg, wParam, lParam);
    }
    return 0;
}

void MainWindow::OnSize(int width, int height) {
    if (!hEditLog) return;
    int margin = 10;
    int col2 = 90;
    int btnW = 30;
    int startBtnW = 150;

    SetWindowPos(hEditSource, NULL, col2, 12, std::max(50, width - col2 - btnW - margin*2), 25, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(m_hWnd, ID_BTN_SOURCE), NULL, width - btnW - margin, 12, btnW, 25, SWP_NOZORDER);
    
    SetWindowPos(hEditDestination, NULL, col2, 52, std::max(50, width - col2 - btnW - margin*2), 25, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(m_hWnd, ID_BTN_DESTINATION), NULL, width - btnW - margin, 52, btnW, 25, SWP_NOZORDER);

    SetWindowPos(hEditFFmpeg, NULL, col2, 92, std::max(50, width - col2 - btnW - margin*2), 25, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(m_hWnd, ID_BTN_FFMPEG), NULL, width - btnW - margin, 92, btnW, 25, SWP_NOZORDER);

    SetWindowPos(hBtnStart, NULL, width - startBtnW - margin, 132, startBtnW, 30, SWP_NOZORDER);

    SetWindowPos(GetDlgItem(m_hWnd, ID_BTN_EXPORT), NULL, width - 100 - margin, 170, 100, 25, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(m_hWnd, ID_BTN_CLEAR), NULL, width - 210 - margin, 170, 100, 25, SWP_NOZORDER);

    int logTop = 205;
    int statsH = 20;
    int progressH = 20;
    int logH = height - logTop - progressH - statsH - margin * 4;
    if (logH < 50) logH = 50;

    SetWindowPos(hEditLog, NULL, margin, logTop, width - margin*2, logH, SWP_NOZORDER);
    SetWindowPos(hProgress, NULL, margin, logTop + logH + margin, width - margin*2, progressH, SWP_NOZORDER);
    SetWindowPos(hStaticStats, NULL, margin, logTop + logH + progressH + margin*2, width - margin*2, statsH, SWP_NOZORDER);
}

void MainWindow::OnBrowseSource() {
    wchar_t path[MAX_PATH];
    BROWSEINFOW bi = { 0 };
    bi.hwndOwner = m_hWnd;
    bi.lpszTitle = L"Select source folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl && SHGetPathFromIDListW(pidl, path)) {
        SetWindowTextW(hEditSource, path);
        CoTaskMemFree(pidl);
    }
}

void MainWindow::OnBrowseDestination() {
    wchar_t path[MAX_PATH];
    BROWSEINFOW bi = { 0 };
    bi.hwndOwner = m_hWnd;
    bi.lpszTitle = L"Select destination folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl && SHGetPathFromIDListW(pidl, path)) {
        SetWindowTextW(hEditDestination, path);
        CoTaskMemFree(pidl);
    }
}

void MainWindow::OnBrowseFFmpeg() {
    wchar_t filename[MAX_PATH] = L"ffmpeg.exe";
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameW(&ofn)) {
        SetWindowTextW(hEditFFmpeg, ofn.lpstrFile);
    }
}

void MainWindow::OnClearLog() {
    SetWindowTextW(hEditLog, L"");
}

void MainWindow::OnExportLog() {
    wchar_t filename[MAX_PATH] = L"log_split.txt";
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    if (GetSaveFileNameW(&ofn)) {
        int len = GetWindowTextLengthW(hEditLog);
        std::wstring text(len, L'\0');
        GetWindowTextW(hEditLog, &text[0], len + 1);
        std::ofstream out(ofn.lpstrFile, std::ios::binary);
        if (out) {
            unsigned char bom[] = {0xEF, 0xBB, 0xBF};
            out.write((char*)bom, 3);
            int u8len = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), NULL, 0, NULL, NULL);
            if (u8len > 0) {
                std::string u8(u8len, 0);
                WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), &u8[0], u8len, NULL, NULL);
                out.write(u8.data(), u8.size());
            }
            out.close();
            MessageBoxW(m_hWnd, L"Log exported successfully", L"Info", MB_OK);
        }
    }
}

void MainWindow::OnStart() {
    JobConfig config;
    wchar_t buf[MAX_PATH];
    GetWindowTextW(hEditSource, buf, MAX_PATH); config.source = buf;
    GetWindowTextW(hEditDestination, buf, MAX_PATH); config.destination = buf;
    GetWindowTextW(hEditFFmpeg, buf, MAX_PATH); config.ffmpegPath = buf;
    GetWindowTextW(hEditThreads, buf, 10); config.threads = _wtoi(buf);
    config.dryRun = (SendMessage(hChkDryRun, BM_GETCHECK, 0, 0) == BST_CHECKED);
    config.errorsOnly = (SendMessage(hChkErrorsOnly, BM_GETCHECK, 0, 0) == BST_CHECKED);

    if (config.ffmpegPath.empty()) {
        MessageBoxW(m_hWnd, L"Please select the path to ffmpeg.exe", L"ffmpeg Missing", MB_ICONERROR);
        return;
    }
    Log(L"Using ffmpeg at: " + config.ffmpegPath);

    if (config.source.empty() || config.destination.empty()) {
        MessageBoxW(m_hWnd, L"Select source and destination", L"Error", MB_ICONERROR);
        return;
    }

    // Validate ffmpeg exists (now we always have a path or we returned early)
    if (!fs::exists(config.ffmpegPath)) {
        std::wstring msg = L"ffmpeg.exe not found at: " + config.ffmpegPath + L"\n\nPlease ensure it exists or place it in the application folder.";
        MessageBoxW(m_hWnd, msg.c_str(), L"ffmpeg not found", MB_ICONERROR);
        return;
    }

    EnableWindow(hBtnStart, FALSE);
    std::thread([this, config]() {
        ThreadManager manager(*this, config);
        manager.Run();
        PostMessage(m_hWnd, WM_APP + 1, 0, 0);
    }).detach();
}
