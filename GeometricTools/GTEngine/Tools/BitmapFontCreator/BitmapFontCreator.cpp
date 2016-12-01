// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2015
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.0.0 (2015/09/23)

#include <windows.h>
#include <string>

std::wstring const gNameSpace = L"gte";
std::wstring const gMacroPrefix = L"GTE";
std::wstring const gFilePrefix = L"Gte";
LONG gWindowWidth = 1024;
LONG gWindowHeight = 256;


bool CreateHeaderFile(std::wstring const& fontName, int weight, int size,
    DWORD italic)
{
    size_t const maxCharacters = 1024;
    wchar_t temp[maxCharacters];
    swprintf(temp, maxCharacters, L"Font%sW%dH%d", fontName.c_str(), weight,
        size);
    std::wstring className(temp);
    if (italic)
    {
        className += L"I";
    }

    std::wstring fileName = gFilePrefix + className + L".h";
    FILE* output = nullptr;
    errno_t result = _wfopen_s(&output, fileName.c_str(), L"wt");
    if (result != 0 || !output)
    {
        return false;
    }

    wchar_t const* cn = className.c_str();
    fwprintf(output, L"// Geometric Tools LLC, Redmond WA 98052\n");
    fwprintf(output, L"// Copyright (c) 1998-2014\n");
    fwprintf(output, L"// Distributed under the Boost Software License, Version 1.0.\n");
    fwprintf(output, L"// http://www.boost.org/LICENSE_1_0.txt\n");
    fwprintf(output, L"// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt\n");
    fwprintf(output, L"// File Version: automatically generated\n");
    fwprintf(output, L"\n");
    fwprintf(output, L"#pragma once\n");
    fwprintf(output, L"\n");
    fwprintf(output, L"#include \"%sFont.h\"\n", gFilePrefix.c_str());
    fwprintf(output, L"\n");
    fwprintf(output, L"namespace %s\n", gNameSpace.c_str());
    fwprintf(output, L"{\n");
    fwprintf(output, L"\n");
    fwprintf(output, L"class %s_IMPEXP %s : public Font\n", gMacroPrefix.c_str(), cn);
    fwprintf(output, L"{\n");
    fwprintf(output, L"public:\n");
    fwprintf(output, L"    virtual ~%s();\n", cn);
    fwprintf(output, L"    %s(int maxMessageLength);\n", cn);
    fwprintf(output, L"\n");
    fwprintf(output, L"private:\n");
    fwprintf(output, L"    static int msWidth;\n");
    fwprintf(output, L"    static int msHeight;\n");
    fwprintf(output, L"    static unsigned char msTexels[];\n");
    fwprintf(output, L"    static float msCharacterData[];\n");
    fwprintf(output, L"};\n");
    fwprintf(output, L"\n");
    fwprintf(output, L"}\n");

    fclose(output);
    return true;
}

bool CreateSourceFile(std::wstring const& fontName, int weight, int size,
    DWORD italic, int width, int height, unsigned char const* texels,
    float const* characterData)
{
    size_t const maxCharacters = 1024;
    wchar_t temp[maxCharacters];
    swprintf(temp, maxCharacters, L"Font%sW%dH%d", fontName.c_str(), weight,
        size);
    std::wstring className(temp);
    if (italic)
    {
        className += L"I";
    }

    std::wstring fileName = gFilePrefix + className;
    std::wstring cppFileName = fileName + L".cpp";
    FILE* output = nullptr;
    errno_t result = _wfopen_s(&output, cppFileName.c_str(), L"wt");
    if (result != 0 || !output)
    {
        return false;
    }

    wchar_t const* cn = className.c_str();
    int const numTexels = width*height;
    fwprintf(output, L"// Geometric Tools LLC, Redmond WA 98052\n");
    fwprintf(output, L"// Copyright (c) 1998-2014\n");
    fwprintf(output, L"// Distributed under the Boost Software License, Version 1.0.\n");
    fwprintf(output, L"// http://www.boost.org/LICENSE_1_0.txt\n");
    fwprintf(output, L"// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt\n");
    fwprintf(output, L"// File Version: automatically generated\n");
    fwprintf(output, L"\n");
    fwprintf(output, L"#include \"%s.h\"\n", fileName.c_str());
    fwprintf(output, L"using namespace %s;\n", gNameSpace.c_str());
    fwprintf(output, L"\n");
    fwprintf(output, L"\n");
    fwprintf(output, L"%s::~%s()\n", cn, cn);
    fwprintf(output, L"{\n");
    fwprintf(output, L"}\n");
    fwprintf(output, L"\n");
    fwprintf(output, L"%s::%s(int maxMessageLength)\n", cn, cn);
    fwprintf(output, L"    :\n");
    fwprintf(output, L"    Font(msWidth, msHeight, (char const*)msTexels, msCharacterData,\n");
    fwprintf(output, L"        maxMessageLength)\n");
    fwprintf(output, L"{\n");
    fwprintf(output, L"}\n");
    fwprintf(output, L"\n");
    fwprintf(output, L"\n");
    fwprintf(output, L"int %s::msWidth = %d;\n", cn, width);
    fwprintf(output, L"int %s::msHeight = %d;\n", cn, height);
    fwprintf(output, L"\n");
    fwprintf(output, L"unsigned char %s::msTexels[%d] =\n", cn, numTexels);
    fwprintf(output, L"{\n");
    int numPerRow = 16, numPerRowM1 = numPerRow - 1;
    for (int i = 0; i < numTexels; ++i, ++texels)
    {
        fwprintf(output, L"%3d, ", *texels);
        if ((i % numPerRow) == numPerRowM1)
        {
            fwprintf(output, L"\n");
        }
    }
    fwprintf(output, L"\n");
    fwprintf(output, L"};\n");
    fwprintf(output, L"\n");
    fwprintf(output, L"float %s::msCharacterData[257] =\n", cn);
    fwprintf(output, L"{\n");
    numPerRow = 8;
    numPerRowM1 = numPerRow - 1;
    for (int i = 0; i < 257; ++i, ++characterData)
    {
        fwprintf(output, L"%ff, ", *characterData);
        if ((i % numPerRow) == numPerRowM1)
        {
            fwprintf(output, L"\n");
        }
    }
    fwprintf(output, L"\n");
    fwprintf(output, L"};\n");

    fclose(output);
    return true;
}

void CreateFontData(HDC hDC, int& width, int& height, unsigned char*& texels,
    float characterData[257])
{
    // Get the height of the font.
    TEXTMETRIC metric;
    GetTextMetrics(hDC, &metric);
    height = (int)metric.tmHeight;

    // Compute the width needed for all 256 characters, taking into account
    // the extra pixel we place at the left side of each character's image.
    width = 0;
    for (int i = 0; i < 256; i++)
    {
        int charWidth;
        GetCharWidth32(hDC, i, i, &charWidth);
        width += (charWidth + 1);
    }

    texels = new unsigned char[width*height];

    // The character data stores textures coordinates in the x-direction.
    float const dx = 1.0f/width;

    int start = 0;
    characterData[0] = 0.5f*dx;
    RECT r;
    r.left = 0;
    r.top = 0;
    r.right = 4*metric.tmHeight;
    r.bottom = 2*metric.tmHeight;
    for (int i = 0; i < 256; ++i)
    {
        wchar_t msg[256];
        wsprintf(msg, L"character %d", i);
        TextOut(hDC, 768, 3*height, msg, static_cast<int>(wcslen(msg)));

        // Determine how many bytes wide the character will be.
        int charWidth;
        GetCharWidth32(hDC, i, i, &charWidth);

        // Place a blank pixel at the start of each character.
        ++charWidth; 
        int end = start + charWidth;
        characterData[i+1] = (end + 0.5f)*dx;

        // Clear the background.
        FillRect(hDC, &r, nullptr);

        // Draw the character.
        wchar_t cChar = static_cast<wchar_t>(i);
        TextOut(hDC, 1, 0, &cChar, 1);

        // Grab the character's pixel data from the screen and store it in the 
        // appropriate texels.
        for (int row = 0; row < height; ++row)
        {
            for (int col = 0; col < charWidth; ++ col)
            {
                COLORREF kColor = GetPixel(hDC, col, height - row);
                texels[width*row + start + col] = GetRValue(kColor);
            }
        }

        start += charWidth;
    }
}

void ProcessFont(HWND hWnd, std::wstring const& fontName, int weight,
    int size, DWORD italic)
{
    HDC hDC = GetDC(hWnd);
    SetTextColor(hDC, RGB(0, 0, 0));
    SetBkColor(hDC, RGB(255, 255, 255));

    // Create the font.
    HFONT hFont = CreateFont(size, 0, 0, 0, weight, italic, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, VARIABLE_PITCH, fontName.c_str());

    HGDIOBJ oldFont = SelectObject(hDC, hFont);

    RECT r;
    r.left = r.top = 0;
    r.right = gWindowWidth - 1;
    r.bottom = gWindowHeight - 1;
    FillRect(hDC, &r, nullptr);
    wchar_t msg[256];
    wsprintf(msg, L"Rendering %s, weight = %d, size = %d, italics = %d",
        fontName.c_str(), weight, size, italic);
    TextOut(hDC, 8, 3*size, msg, (int)wcslen(msg));

    int width, height;
    unsigned char* texels;
    float characterData[257];
    CreateFontData(hDC, width, height, texels, characterData);

    CreateHeaderFile(fontName, weight, size, italic);
    CreateSourceFile(fontName, weight, size, italic, width, height, texels,
        characterData);

    delete[] texels;

    SelectObject(hDC, oldFont);
    DeleteObject(hFont);
    ReleaseDC(hWnd, hDC);
}

int wmain(int, wchar_t const*[])
{
    wchar_t* className = L"BitmapFontCreator";
    WNDCLASS wc;
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = DefWindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = nullptr;
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = className;
    wc.lpszMenuName  = nullptr;
    RegisterClass(&wc);

    HWND hWnd = CreateWindow(className, className, WS_OVERLAPPEDWINDOW,
        0, 0, gWindowWidth, gWindowHeight, nullptr, nullptr, nullptr,
        nullptr);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    std::wstring fontName = L"Arial";
    int weight = FW_SEMIBOLD;
    int size = 36;
    DWORD italic = 0;
    ProcessFont(hWnd, fontName, weight, size, italic);

    DestroyWindow(hWnd);
    UnregisterClass(className, nullptr);
    return 0;
}

