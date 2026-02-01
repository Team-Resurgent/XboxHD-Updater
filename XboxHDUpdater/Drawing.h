#pragma once

#include "External.h"

#include <string>
#include <map>

class Drawing
{
public:
    static void SetD3dDevice(LPDIRECT3DDEVICE8 d3dDevice);
    static LPDIRECT3DDEVICE8 GetD3dDevice();

    static void SetBufferWidth(DWORD width);
    static void SetBufferHeight(DWORD height);
    static DWORD GetBufferWidth();
    static DWORD GetBufferHeight();

    static void Swizzle(const void* src, const uint32_t& depth, const uint32_t& width, const uint32_t& height, void* dest);
    static void CreateImage(uint8_t* imageData, D3DFORMAT format, int width, int height);
    static void GenerateBitmapFont();
    static void Init();
    static void DrawTerminal(const char* buffer, uint32_t color);
};
