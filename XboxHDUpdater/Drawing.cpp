#include "Drawing.h"
#include "Resources.h"
#include "TerminalBuffer.h"
#include <map>

#define SSFN_IMPLEMENTATION
#define SFFN_MAXLINES 8192
#define SSFN_memcmp memcmp
#define SSFN_memset memset
#define SSFN_realloc realloc
#define SSFN_free free
#include "ssfn.h"

typedef struct {
    int x;
    int y;
    int width;
    int height;
} recti;

typedef struct {
    float x;
    float y;
    float width;
    float height;
} rectf;

#define FONT_TEXTURE_DIMENSION 1024

/* One vertex for terminal batch: XYZ + diffuse + UV (matches D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1) */
struct terminal_vertex_t {
    float x, y, z;
    DWORD diffuse;
    float u, v;
};

#define TERMINAL_MAX_VERTS (TerminalBuffer::Cols * TerminalBuffer::Rows * 6)

namespace
{
	ssfn_t* mFontContext = NULL;
    LPDIRECT3DDEVICE8 mD3dDevice;
    static DWORD s_bufferWidth;
    static DWORD s_bufferHeight;

    std::map<uint32_t, recti> charMap;

    int texture_width;
    int texture_height;
    D3DTexture* font_texture;

    static terminal_vertex_t s_terminalVerts[TERMINAL_MAX_VERTS];
}

void Drawing::SetD3dDevice(LPDIRECT3DDEVICE8 d3dDevice) {
    mD3dDevice = d3dDevice;
}

LPDIRECT3DDEVICE8 Drawing::GetD3dDevice() {
    return mD3dDevice;
}

void Drawing::SetBufferWidth(DWORD width) {
    s_bufferWidth = width;
}

void Drawing::SetBufferHeight(DWORD height) {
    s_bufferHeight = height;
}

DWORD Drawing::GetBufferWidth() {
    return s_bufferWidth;
}

DWORD Drawing::GetBufferHeight() {
    return s_bufferHeight;
}

void Drawing::Swizzle(const void* src, const uint32_t& depth, const uint32_t& width, const uint32_t& height, void* dest)
{
  for (UINT y = 0; y < height; y++)
  {
    UINT sy = 0;
    if (y < width)
    {
      for (int bit = 0; bit < 16; bit++)
        sy |= ((y >> bit) & 1) << (2*bit);
      sy <<= 1; // y counts twice
    }
    else
    {
      UINT y_mask = y % width;
      for (int bit = 0; bit < 16; bit++)
        sy |= ((y_mask >> bit) & 1) << (2*bit);
      sy <<= 1; // y counts twice
      sy += (y / width) * width * width;
    }
    BYTE *s = (BYTE *)src + y * width * depth;
    for (UINT x = 0; x < width; x++)
    {
      UINT sx = 0;
      if (x < height * 2)
      {
        for (int bit = 0; bit < 16; bit++)
          sx |= ((x >> bit) & 1) << (2*bit);
      }
      else
      {
        int x_mask = x % (2*height);
        for (int bit = 0; bit < 16; bit++)
          sx |= ((x_mask >> bit) & 1) << (2*bit);
        sx += (x / (2 * height)) * 2 * height * height;
      }
      BYTE *d = (BYTE *)dest + (sx + sy)*depth;
      for (unsigned int i = 0; i < depth; ++i)
        *d++ = *s++;
    }
  }
}

void Drawing::CreateImage(uint8_t* imageData, D3DFORMAT format, int width, int height)
{
	if (FAILED(D3DXCreateTexture(mD3dDevice, width, height, 1, 0, format, D3DPOOL_DEFAULT, &font_texture)))
	{
		return;
	}

	D3DSURFACE_DESC surfaceDesc;
	font_texture->GetLevelDesc(0, &surfaceDesc);

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(font_texture->LockRect(0, &lockedRect, NULL, 0)))
	{
		uint8_t* tempBuffer = (uint8_t*)malloc(surfaceDesc.Size);
		memset(tempBuffer, 0, surfaceDesc.Size);
		uint8_t* src = imageData;
		uint8_t* dst = tempBuffer;
		for (int32_t y = 0; y < height; y++)
		{
			memcpy(dst, src, width * 4);
			src += width * 4;
			dst += surfaceDesc.Width * 4;
		}
		Swizzle(tempBuffer, 4, surfaceDesc.Width, surfaceDesc.Height, lockedRect.pBits);
		free(tempBuffer);
		font_texture->UnlockRect(0);
	}
}

void Drawing::GenerateBitmapFont()
{
	if (mFontContext != NULL)
	{
        return;
    }

	mFontContext = (ssfn_t*)malloc(sizeof(ssfn_t)); 
    if (mFontContext == NULL)
    {
        return;
    }
	memset(mFontContext, 0, sizeof(ssfn_t));

	if (ssfn_load(mFontContext, &terminal_sfn[0]) != 0)
    {
        return;
    }

	ssfn_select(mFontContext, SSFN_FAMILY_ANY, "CascadiaCode", SSFN_STYLE_REGULAR, 32);

	int textureWidth = FONT_TEXTURE_DIMENSION;
	int textureHeight = FONT_TEXTURE_DIMENSION; 
	uint32_t* imageData = (uint32_t*)malloc(textureWidth * textureHeight * 4);
    if (imageData == NULL)
    {
        return;
    }

	memset(imageData, 0, textureWidth * textureHeight * 4);  

	int x = 2;
	int y = 2;

	const char* charsToEncode =
		" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
		//"\xE2\x94\x80\xE2\x94\x81\xE2\x94\x82\xE2\x94\x83\xE2\x94\x84\xE2\x94\x85\xE2\x94\x86\xE2\x94\x87\xE2\x94\x88\xE2\x94\x89\xE2\x94\x8A\xE2\x94\x8B\xE2\x94\x8C\xE2\x94\x8D\xE2\x94\x8E\xE2\x94\x8F"
		//"\xE2\x94\x90\xE2\x94\x91\xE2\x94\x92\xE2\x94\x93\xE2\x94\x94\xE2\x94\x95\xE2\x94\x96\xE2\x94\x97\xE2\x94\x98\xE2\x94\x99\xE2\x94\x9A\xE2\x94\x9B\xE2\x94\x9C\xE2\x94\x9D\xE2\x94\x9E\xE2\x94\x9F"
		//"\xE2\x94\xA0\xE2\x94\xA1\xE2\x94\xA2\xE2\x94\xA3\xE2\x94\xA4\xE2\x94\xA5\xE2\x94\xA6\xE2\x94\xA7\xE2\x94\xA8\xE2\x94\xA9\xE2\x94\xAA\xE2\x94\xAB\xE2\x94\xAC\xE2\x94\xAD\xE2\x94\xAE\xE2\x94\xAF"
		//"\xE2\x94\xB0\xE2\x94\xB1\xE2\x94\xB2\xE2\x94\xB3\xE2\x94\xB4\xE2\x94\xB5\xE2\x94\xB6\xE2\x94\xB7\xE2\x94\xB8\xE2\x94\xB9\xE2\x94\xBA\xE2\x94\xBB\xE2\x94\xBC\xE2\x94\xBD\xE2\x94\xBE\xE2\x94\xBF"
		//"\xE2\x95\x80\xE2\x95\x81\xE2\x95\x82\xE2\x95\x83\xE2\x95\x84\xE2\x95\x85\xE2\x95\x86\xE2\x95\x87\xE2\x95\x88\xE2\x95\x89\xE2\x95\x8A\xE2\x95\x8B\xE2\x95\x8C\xE2\x95\x8D\xE2\x95\x8E\xE2\x95\x8F"
		//"\xE2\x95\x90\xE2\x95\x91\xE2\x95\x92\xE2\x95\x93\xE2\x95\x94\xE2\x95\x95\xE2\x95\x96\xE2\x95\x97\xE2\x95\x98\xE2\x95\x99\xE2\x95\x9A\xE2\x95\x9B\xE2\x95\x9C\xE2\x95\x9D\xE2\x95\x9E\xE2\x95\x9F"
		//"\xE2\x95\xA0\xE2\x95\xA1\xE2\x95\xA2\xE2\x95\xA3\xE2\x95\xA4\xE2\x95\xA5\xE2\x95\xA6\xE2\x95\xA7\xE2\x95\xA8\xE2\x95\xA9\xE2\x95\xAA\xE2\x95\xAB\xE2\x95\xAC\xE2\x95\xAD\xE2\x95\xAE\xE2\x95\xAF"
		//"\xE2\x95\xB0\xE2\x95\xB1\xE2\x95\xB2\xE2\x95\xB3\xE2\x95\xB4\xE2\x95\xB5\xE2\x95\xB6\xE2\x95\xB7\xE2\x95\xB8\xE2\x95\xB9\xE2\x95\xBA\xE2\x95\xBB\xE2\x95\xBC\xE2\x95\xBD\xE2\x95\xBE\xE2\x95\xBF";
	
    char* currentCharPos = const_cast<char*>(charsToEncode);
	while(*currentCharPos)
	{	
		char* nextCharPos = currentCharPos;
		uint32_t unicode = ssfn_utf8(&nextCharPos);

		int32_t length = nextCharPos - currentCharPos;
		char* currentChar = (char*)malloc(length + 1);
		memcpy(currentChar, currentCharPos, length);
		currentChar[length] = 0;

		currentCharPos = nextCharPos;

        int bounds_x;
        int bounds_y;
        int bounds_width;
        int bounds_height;
        int ret = ssfn_bbox(mFontContext, currentChar, &bounds_width, &bounds_height, &bounds_x, &bounds_y);
		if (ret != 0)
		{
			continue;
		}

		if ((x + bounds_width + 2) > textureWidth)
		{
			x = 2;
			y = y + bounds_height + 2;
		}

		recti* rect = &charMap[unicode];
		rect->x = x;
		rect->y = y;
		rect->width = bounds_width;
		rect->height = bounds_height;

		ssfn_buf_t buffer; 
		memset(&buffer, 0, sizeof(buffer));
		buffer.ptr = (uint8_t*)imageData;
		buffer.x = x + bounds_x;
		buffer.y = y + bounds_y;
		buffer.w = textureWidth;
		buffer.h = textureHeight;
		buffer.p = textureWidth * 4;
		buffer.bg = 0;                                  /* transparent so texture has proper alpha mask */
		buffer.fg = 0xffffffff;                         /* white; SSFN writes coverage into alpha */   

		ssfn_render(mFontContext, &buffer, currentChar);

		x = x + bounds_width + 2;   
		free(currentChar);
	}

	CreateImage((uint8_t*)imageData, D3DFMT_A8R8G8B8, textureWidth, textureHeight);
	free(imageData);
}

void Drawing::Init()
{
    GenerateBitmapFont();
}

void Drawing::DrawTerminal(const char* buffer, uint32_t color)
{
    const int cellW = Drawing::GetBufferWidth() / TerminalBuffer::Cols;
    const int cellH = Drawing::GetBufferHeight() / TerminalBuffer::Rows;
    const float bufH = (float)Drawing::GetBufferHeight();
    const float invDim = 1.0f / (float)FONT_TEXTURE_DIMENSION;

    terminal_vertex_t* v = s_terminalVerts;
    int nVerts = 0;

    for (int row = 0; row < TerminalBuffer::Rows; row++)
    {
        for (int col = 0; col < TerminalBuffer::Cols; col++)
        {
            char c = buffer[row * TerminalBuffer::Cols + col];
            std::map<uint32_t, recti>::iterator it = charMap.find((uint32_t)(unsigned char)c);
            if (it == charMap.end())
            {
                continue;
            }

            const recti& r = it->second;
            float u0 = r.x * invDim;
            float v0 = r.y * invDim;
            float u1 = (r.x + r.width) * invDim;
            float v1 = (r.y + r.height) * invDim;

            float px = (float)(col * cellW) - 0.5f;
            float py = bufH - ((float)(row  * cellH)) - 0.5f;
            float pz = 0.0f;
            float fw = (float)cellW;
            float fh = (float)cellH;

            v[0].x = px + fw;
            v[0].y = py + fh;
            v[0].z = pz;
            v[0].diffuse = color;
            v[0].u = u1;
            v[0].v = v0;
            v[1].x = px + fw;
            v[1].y = py;
            v[1].z = pz;
            v[1].diffuse = color;
            v[1].u = u1;
            v[1].v = v1;
            v[2].x = px;
            v[2].y = py;
            v[2].z = pz;
            v[2].diffuse = color;
            v[2].u = u0;
            v[2].v = v1;
            v[3].x = px + fw;
            v[3].y = py + fh;
            v[3].z = pz;
            v[3].diffuse = color;
            v[3].u = u1;
            v[3].v = v0;
            v[4].x = px;
            v[4].y = py;
            v[4].z = pz;
            v[4].diffuse = color;
            v[4].u = u0;
            v[4].v = v1;
            v[5].x = px;
            v[5].y = py + fh;
            v[5].z = pz;
            v[5].diffuse = color;
            v[5].u = u0;
            v[5].v = v0;
            v += 6;
            nVerts += 6;
        }
    }

    if (nVerts == 0)
    {
        return;
    }

    mD3dDevice->BeginScene();
    mD3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0xff000000, 1.0f, 0L);

    mD3dDevice->SetTexture(0, font_texture);
    mD3dDevice->DrawPrimitiveUP(
        D3DPT_TRIANGLELIST,
        nVerts / 3,
        s_terminalVerts,
        sizeof(terminal_vertex_t));

    mD3dDevice->EndScene();
	mD3dDevice->Present(NULL, NULL, NULL, NULL);
}
