// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <GTEnginePCH.h>
#include <LowLevel/GteLogger.h>
#include <Applications/GteEnvironment.h>
#include <cstdarg>
#include <cstdlib>
#include <io.h>  // For _access_s.  This must change for Linux and Mac OS X.
using namespace gte;


Environment::~Environment ()
{
}

Environment::Environment ()
{
}

std::string Environment::GetVariable (std::string const& name)
{
    size_t size;
    getenv_s(&size, nullptr, 0, name.c_str());
    if (size > 0)
    {
        char* tmpvar = new char[size];
        errno_t result = getenv_s(&size, tmpvar, size, name.c_str());
        std::string var = (result == 0 ? std::string(tmpvar) : "");
        delete[] tmpvar;
        return var;
    }
    else
    {
        return "";
    }
}

bool Environment::SplitPath (std::string const& path, std::string& drive,
    std::string& directory, std::string& name, std::string& extension)
{
    char driveBuffer[_MAX_DRIVE];
    char directoryBuffer[_MAX_DIR];
    char nameBuffer[_MAX_FNAME];
    char extensionBuffer[_MAX_EXT];
    errno_t result = _splitpath_s(path.c_str(), driveBuffer, directoryBuffer,
        nameBuffer, extensionBuffer);
    if (result == 0)
    {
        drive = std::string(driveBuffer);
        directory = std::string(directoryBuffer);
        name = std::string(nameBuffer);
        extension = std::string(extensionBuffer);
        return true;
    }

    drive = "";
    directory = "";
    name = "";
    extension = "";
    LogError("SplitPath(" + path + ") failed.");
    return false;
}

bool Environment::FullPath (std::string const& relativePath,
    std::string& fullPath)
{
    char fullPathBuffer[_MAX_PATH];
    if (_fullpath(fullPathBuffer, relativePath.c_str(), _MAX_PATH))
    {
        fullPath = std::string(fullPathBuffer);
        return true;
    }

    LogError("FullPath" + relativePath + ") failed.");
    return false;
}

int Environment::GetNumDirectories () const
{
    return static_cast<int>(mDirectories.size());
}

std::string Environment::Get (int i) const
{
    if (0 <= i && i < static_cast<int>(mDirectories.size()))
    {
        return mDirectories[i];
    }

    LogError("Invalid index.");
    return "";
}

bool Environment::Insert (std::string const& directory)
{
    if (directory.size() > 0)
    {
        for (auto& d : mDirectories)
        {
            if (directory == d
            ||  directory + "/" == d
            ||  directory + "\\" == d)
            {
                return false;
            }
        }

        // Ensure all directories are terminated with a slash.
        char lastChar = directory[directory.size() - 1];
        if (lastChar == '\\' || lastChar == '/')
        {
            mDirectories.push_back(directory);
        }
        else
        {
            mDirectories.push_back(directory + "/");
        }
        return true;
    }

    LogError("Insert expects non-empty inputs.");
    return false;
}

bool Environment::Remove (std::string const& directory)
{
    auto iter = mDirectories.begin(), end = mDirectories.end();
    for (/**/; iter != end; ++iter)
    {
        if (directory == *iter)
        {
            mDirectories.erase(iter);
            return true;
        }
    }
    return false;
}

void Environment::RemoveAll ()
{
    mDirectories.clear();
}

std::string Environment::GetPath (std::string const& name, FileMode mode)
    const
{
    for (auto const& directory : mDirectories)
    {
        std::string decorated = directory + name;

        errno_t result = 0;
        if (mode == FM_EXISTS)
        {
            result = _access_s(decorated.c_str(), 0);
        }
        else if (mode == FM_READABLE)
        {
            result = _access_s(decorated.c_str(), 2);
            if (result != 0)
            {
                result = _access_s(decorated.c_str(), 6);
            }
        }
        else
        {
            result = _access_s(decorated.c_str(), 4);
            if (result != 0)
            {
                result = _access_s(decorated.c_str(), 6);
            }
        }

        if (result == 0)
        {
            return decorated;
        }
    }
    return "";
}

std::string Environment::CreateString(char const* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    return CreateStringFromArgs(format, arguments);
}

std::string Environment::CreateStringFromArgs(char const* format,
    va_list arguments)
{
    if (format)
    {
        // Get the output string length not including the null terminator.
        int length = _vscprintf(format, arguments);
        if (length > 0)
        {
            // Create the string from the arguments.  The std::string will
            // have the extra space for the null terminator.
            std::string result;
            result.resize(length);
            int numWritten = _vsnprintf_s(&result[0], length + 1, length,
                format, arguments);
            if (numWritten == length)
            {
                return result;
            }

            LogError("Mismatch of counts in _vsnprintf_s.");
        }
        else if (length < 0)
        {
            // If the parameter validation failed and any user-defined handler
            // did not terminate the application, the returned length is -1.
            // Return a string that indicates this failure.
            LogError("Parameter validation failed.");
        }
        else
        {
            LogError("Unexpected error.");
        }
    }
    else
    {
        // A null format pointer will cause _vscprintf to assert in Debug mode
        // but crash in Release mode using the default invalid parameter
        // handler.
        LogError("Expecting nonnull format string.");
    }

    return "";
}

std::wstring Environment::CreateString(wchar_t const* format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    return CreateStringFromArgs(format, arguments);
}

std::wstring Environment::CreateStringFromArgs(wchar_t const* format,
    va_list arguments)
{
    if (format)
    {
        // Get the output string length not including the null terminator.
        int length = _vscwprintf(format, arguments);
        if (length > 0)
        {
            // Create the string from the arguments.  The std::string will
            // have the extra space for the null terminator.
            std::wstring result;
            result.resize(length);
            int numWritten = _vsnwprintf_s(&result[0], length + 1, length,
                format, arguments);
            if (numWritten == length)
            {
                return result;
            }

            LogError("Mismatch of counts in _vsnwprintf_s.");
        }
        else if (length < 0)
        {
            // If the parameter validation failed and any user-defined handler
            // did not terminate the application, the returned length is -1.
            // Return a string that indicates this failure.
            LogError("Parameter validation failed.");
        }
        else
        {
            LogError("Unexpected error.");
        }
    }
    else
    {
        // A null format pointer will cause _vscprintf to assert in Debug mode
        // but crash in Release mode using the default invalid parameter
        // handler.
        LogError("Expecting nonnull format string.");
    }

    return L"";
}

std::wstring Environment::Convert(std::string const& input)
{
    std::wstring output = L"";
    for (int i = 0; i < (int)input.length(); ++i)
    {
        output += (wchar_t)input[i];
    }
    return output;
}

std::string Environment::Convert(std::wstring const& input)
{
    std::string output = "";
    for (int i = 0; i < (int)input.length(); ++i)
    {
        output += (char)input[i];
    }
    return output;
}

