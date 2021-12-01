#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <memory>
#include "lib/ini_reader/INIReader.h"

class Settings
{
public:
    static Settings& Instance()
    {
        static Settings s;
        return s;
    }

    int Load(const char*);
    const std::unique_ptr<INIReader>& GetReader() { return reader; }

private:
    Settings() {}
    ~Settings() {}
    Settings(Settings const&) = delete;
    Settings& operator= (Settings const&) = delete;

private:
    std::unique_ptr<INIReader> reader = nullptr;
};

#endif // !_SETTINGS_H