#include "Settings.h"
#include "Helper.h"
#include <string>

int Settings::Load(const char* file_name)
{
    reader = std::make_unique<INIReader>(file_name);

    if (reader->ParseError() < 0)
    {
        Helper::WriteAppLog("Can't load " + std::string(file_name));
        return -1;
    }

    return 1;
}