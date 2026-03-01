#include "include/MainWindow.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MainWindow window;
    if (!window.Create(hInstance, nCmdShow)) {
        return -1;
    }
    return window.MessageLoop();
}
