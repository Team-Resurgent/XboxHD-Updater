#include <xtl.h>
#include <stdio.h>
#include <string>
#include <math.h>
#include <stdlib.h>

#include "Drawing.h"
#include "Debug.h"
#include "InputManager.h"
#include "External.h"
#include "Resources.h"
#include "TerminalBuffer.h"
#include "HDHelper.h"
#include "HDPlusHelper.h"
#include "String.h"
#include "Utility.h"
#include "ssfn.h"

#include <xgraphics.h>

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

typedef struct {
    DWORD dwWidth;
    DWORD dwHeight;
    BOOL  fProgressive;
    BOOL  fWideScreen;
	DWORD dwFreq;
} DISPLAY_MODE;

DISPLAY_MODE displayModes[] =
{
	// HDTV Progressive Modes
    {  1280,    720,    TRUE,   TRUE,  60 },         // 1280x720 progressive 16x9
	// EDTV Progressive Modes
    {   720,    480,    TRUE,   TRUE,  60 },         // 720x480 progressive 16x9
    {   640,    480,    TRUE,   TRUE,  60 },         // 640x480 progressive 16x9
    {   720,    480,    TRUE,   FALSE, 60 },         // 720x480 progressive 4x3
    {   640,    480,    TRUE,   FALSE, 60 },         // 640x480 progressive 4x3
	// SDTV PAL-50 Interlaced Modes
    {   720,    480,    FALSE,  TRUE,  50 },         // 720x480 interlaced 16x9 50Hz
    {   640,    480,    FALSE,  TRUE,  50 },         // 640x480 interlaced 16x9 50Hz
    {   720,    480,    FALSE,  FALSE, 50 },         // 720x480 interlaced 4x3  50Hz
    {   640,    480,    FALSE,  FALSE, 50 },         // 640x480 interlaced 4x3  50Hz
	// SDTV NTSC / PAL-60 Interlaced Modes
    {   720,    480,    FALSE,  TRUE,  60 },         // 720x480 interlaced 16x9
    {   640,    480,    FALSE,  TRUE,  60 },         // 640x480 interlaced 16x9
    {   720,    480,    FALSE,  FALSE, 60 },         // 720x480 interlaced 4x3
    {   640,    480,    FALSE,  FALSE, 60 },         // 640x480 interlaced 4x3
};

#define NUM_MODES (sizeof(displayModes) / sizeof(displayModes[0]))

static bool WaitButton(ControllerButton controllerButton)
{
	while (true)
	{
		InputManager::ProcessController();
        if (InputManager::ControllerPressed(ControllerB, -1))
		{
			return true;
		}
        else if (InputManager::ControllerPressed(controllerButton, -1))
		{
			return false;
		}
		Sleep(100);
	}
}

bool SupportsMode(DISPLAY_MODE mode, DWORD dwVideoStandard, DWORD dwVideoFlags)
{
    if (mode.dwFreq == 60 && !(dwVideoFlags & XC_VIDEO_FLAGS_PAL_60Hz) && (dwVideoStandard == XC_VIDEO_STANDARD_PAL_I))
    {
        return false;
    }
    if (mode.dwFreq == 50 && (dwVideoStandard != XC_VIDEO_STANDARD_PAL_I))
    {
        return false;
    }
    if (mode.dwHeight == 480 && mode.fWideScreen && !(dwVideoFlags & XC_VIDEO_FLAGS_WIDESCREEN ))
    {
        return false;
    }
    if (mode.dwHeight == 480 && mode.fProgressive && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_480p))
    {
        return false;
    }
    if (mode.dwHeight == 720 && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_720p))
    {
        return false;
    }
    if (mode.dwHeight == 1080 && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_1080i))
    {
        return false;
    }
    return true;
}

bool CreateDevice()
{
	uint32_t videoFlags = XGetVideoFlags();
	uint32_t videoStandard = XGetVideoStandard();
	uint32_t currentMode;
    for (currentMode = 0; currentMode < NUM_MODES-1; currentMode++)
    {
        if (SupportsMode(displayModes[currentMode], videoStandard, videoFlags))
        {
            break;
        }
    } 

	LPDIRECT3D8 d3d = Direct3DCreate8(D3D_SDK_VERSION);
    if(d3d == NULL)
	{
		Debug::Print("Failed to create d3d\n");
        return false;
	}

	Drawing::SetBufferWidth(displayModes[currentMode].dwWidth);
	Drawing::SetBufferHeight(displayModes[currentMode].dwHeight);

	D3DPRESENT_PARAMETERS params; 
    ZeroMemory(&params, sizeof(params));
	params.BackBufferWidth = displayModes[currentMode].dwWidth;
    params.BackBufferHeight = displayModes[currentMode].dwHeight;
	params.Flags = displayModes[currentMode].fProgressive ? D3DPRESENTFLAG_PROGRESSIVE : D3DPRESENTFLAG_INTERLACED;
    params.Flags |= displayModes[currentMode].fWideScreen ? D3DPRESENTFLAG_WIDESCREEN : 0;
    params.FullScreen_RefreshRateInHz = displayModes[currentMode].dwFreq;
	params.BackBufferFormat = D3DFMT_X8R8G8B8;
    params.BackBufferCount = 1;
    params.EnableAutoDepthStencil = FALSE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	LPDIRECT3DDEVICE8 d3dDevice;
    if (FAILED(d3d->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &d3dDevice)))
	{
		Debug::Print("Failed to create device\n");
        return false;
	}
	Drawing::SetD3dDevice(d3dDevice);
    
    Drawing::GetD3dDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	Drawing::GetD3dDevice()->SetVertexShader(D3DFVF_CUSTOMVERTEX);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	Drawing::GetD3dDevice()->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity(&matIdentity);
	D3DXMATRIX matProjection;
	D3DXMatrixOrthoOffCenterLH(&matProjection, 0, (float)Drawing::GetBufferWidth(), 0, (float)Drawing::GetBufferHeight(), 1.0f, 800.0f);
	Drawing::GetD3dDevice()->SetTransform(D3DTS_VIEW, &matIdentity);
	Drawing::GetD3dDevice()->SetTransform(D3DTS_WORLD, &matIdentity);
	Drawing::GetD3dDevice()->SetTransform(D3DTS_PROJECTION, &matProjection);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	Drawing::GetD3dDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	Drawing::GetD3dDevice()->BeginScene();
	Drawing::GetD3dDevice()->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0xff000000, 1.0f, 0L);
	Drawing::GetD3dDevice()->EndScene();
	Drawing::GetD3dDevice()->Present(NULL, NULL, NULL, NULL);

	return true;
}


static void InitTerminalBuffer()
{
    TerminalBuffer::Clear();

    TerminalBuffer::SetCursor(0, 0);
    TerminalBuffer::Write("X-HD Firmware Flasher\n");
    TerminalBuffer::Write("=====================\n");
    TerminalBuffer::Write("\n");

    TerminalBuffer::Write("Encoder: ");
    Utility::EncoderEnum encoder = Utility::GetEncoder();
    if (encoder == Utility::EncoderConexant) {
        TerminalBuffer::Write("Conexant\n");
    } else if (encoder == Utility::EncoderFocus) {
        TerminalBuffer::Write("Focus\n");
    } else if (encoder == Utility::EncoderXcalibur) {
        TerminalBuffer::Write("Xcalibur\n");
    }
}

static void XHDProcess()
{
    TerminalBuffer::Write("HDMI Device: XHD\n");

    uint8_t currentMode = HDHelper::GetMode();
    TerminalBuffer::Write("Current Mode: %s\n", currentMode == I2C_HDMI_MODE_APPLICATION ? "Application" : "Bootloader");

    if (currentMode == I2C_HDMI_MODE_APPLICATION)
    {
        TerminalBuffer::Write("Current Firmware: ");
        uint32_t version = HDHelper::ReadVersion();
        if (version == 0xFFFFFFFF)
        {
            TerminalBuffer::Write("Not detected\n");
            return;
        }
        else
        {
            char line[50];
            _snprintf(line, sizeof(line), "%u.%u.%u\n",
                (unsigned)((version >> 24) & 0xFF),
                (unsigned)((version >> 16) & 0xFF),
                (unsigned)((version >> 8) & 0xFF));
            TerminalBuffer::Write(line);
        }
    }

    TerminalBuffer::Write("Firmware To Write: ");
    uint32_t firmwareSize = 0;
    uint8_t* firmwareData = Utility::LoadFirmware(&firmwareSize);
    if (firmwareData == NULL)
    {
        TerminalBuffer::Write("Not Found\n\n");
        return;
    }
    else
    {
        TerminalBuffer::Write("Found\n\n");
    }

    TerminalBuffer::Write("Note: screen will flash when changing X-HD mode.\n\n");

    TerminalBuffer::Write("Press A to Flash / B Exit\n");
    bool exit = WaitButton(ControllerA);
    if (exit)
    {
        HalReturnToFirmware(2);
    }

    TerminalBuffer::Write("Press X to Confirm / B Exit\n\n");
    exit = WaitButton(ControllerX);
    if (exit)
    {
        HalReturnToFirmware(2);
    }

    if (currentMode == I2C_HDMI_MODE_APPLICATION)
    {
        TerminalBuffer::Write("Entering Bootloader Mode: ");
        HDHelper::ChangeMode(I2C_HDMI_MODE_BOOTLOADER);
        TerminalBuffer::Write("Done\n");
    }

    if (HDHelper::FlashApplication(firmwareData, firmwareSize) == false)
    {
        return;
    }
    
    TerminalBuffer::Write("Entering Application Mode: ");
    HDHelper::ChangeMode(I2C_HDMI_MODE_APPLICATION);
    TerminalBuffer::Write("Done\n");

    TerminalBuffer::Write("New Firmware: ");
    uint32_t version = HDHelper::ReadVersion();
    if (version == 0xFFFFFFFF)
    {
        TerminalBuffer::Write("Not detected\n");
    }
    else
    {
        char line[50];
        _snprintf(line, sizeof(line), "%u.%u.%u\n",
            (unsigned)((version >> 24) & 0xFF),
            (unsigned)((version >> 16) & 0xFF),
            (unsigned)((version >> 8) & 0xFF));
        TerminalBuffer::Write(line);
    }

    Sleep(3000);
    TerminalBuffer::Write("\nRebooting...");
    Utility::Reboot();
}

static void HDPlusProcess()
{
    TerminalBuffer::Write("HDMI Device: HDPlus\n");

    uint8_t currentMode = HDPlusHelper::GetMode();
    TerminalBuffer::Write("Current Mode: %s\n", currentMode == HDPLUS_II2C_HDMI_MODE_APPLICATION ? "Application" : "Bootloader");

    if (currentMode == HDPLUS_II2C_HDMI_MODE_APPLICATION)
    {
        TerminalBuffer::Write("Current Firmware: ");
        uint32_t version = HDPlusHelper::ReadVersion();
        if (version == 0xFFFFFFFF)
        {
            TerminalBuffer::Write("Not detected\n");
            return;
        }
        else
        {
            char line[50];
            _snprintf(line, sizeof(line), "%u.%u.%u\n",
                (unsigned)((version >> 24) & 0xFF),
                (unsigned)((version >> 16) & 0xFF),
                (unsigned)((version >> 8) & 0xFF));
            TerminalBuffer::Write(line);
        }
    }

    TerminalBuffer::Write("Firmware To Write: ");
    uint32_t firmwareSize = 0;
    uint8_t* firmwareData = Utility::LoadFirmware(&firmwareSize);
    if (firmwareData == NULL)
    {
        TerminalBuffer::Write("Not Found\n\n");
        return;
    }
    else
    {
        TerminalBuffer::Write("Found\n\n");
    }

    TerminalBuffer::Write("Note: screen will flash when changing HDPlus mode.\n\n");

    TerminalBuffer::Write("Press A to Flash / B Exit\n");
    bool exit = WaitButton(ControllerA);
    if (exit)
    {
        HalReturnToFirmware(2);
    }

    TerminalBuffer::Write("Press X to Confirm / B Exit\n\n");
    exit = WaitButton(ControllerX);
    if (exit)
    {
        HalReturnToFirmware(2);
    }

    if (currentMode == HDPLUS_II2C_HDMI_MODE_APPLICATION)
    {
        TerminalBuffer::Write("Entering Bootloader Mode: ");
        HDPlusHelper::ChangeMode(HDPLUS_II2C_HDMI_MODE_BOOTLOADER | HDPLUS_II2C_HDMI_MODE_ENABLE_FLASH, HDPLUS_II2C_HDMI_MODE_BOOTLOADER);
        TerminalBuffer::Write("Done\n");
    }

    if (HDPlusHelper::FlashApplication(firmwareData, firmwareSize) == false)
    {
        return;
    }

    TerminalBuffer::Write("Entering Application Mode: ");
    HDPlusHelper::ChangeMode(HDPLUS_II2C_HDMI_MODE_APPLICATION | HDPLUS_II2C_HDMI_MODE_ENABLE_APPLICATION, HDPLUS_II2C_HDMI_MODE_ENABLE_APPLICATION);
    TerminalBuffer::Write("Done\n");

    if (HDHelper::FlashBootloader(firmwareData, firmwareSize) == false)
    {
        return;
    }

    TerminalBuffer::Write("New Firmware: ");
    uint32_t version = HDHelper::ReadVersion();
    if (version == 0xFFFFFFFF)
    {
        TerminalBuffer::Write("Not detected\n");
    }
    else
    {
        char line[50];
        _snprintf(line, sizeof(line), "%u.%u.%u\n",
            (unsigned)((version >> 24) & 0xFF),
            (unsigned)((version >> 16) & 0xFF),
            (unsigned)((version >> 8) & 0xFF));
        TerminalBuffer::Write(line);
    }

    Sleep(3000);
    TerminalBuffer::Write("\nRebooting...");
    Utility::Reboot();
}

void __cdecl main()
{
	Debug::Print("Welcome to Xbox HD Updater\n");

	bool deviceCreated = CreateDevice();

	Drawing::GenerateBitmapFont();

    InputManager::Init();
	InitTerminalBuffer();

    Utility::HdmiDeviceEnum hdmiDevice = Utility::GetHdmiDevice();
    if (hdmiDevice == Utility::HdmiDeviceXHD) {
        XHDProcess();
    } else if (hdmiDevice == Utility::HdmiDeviceHDPlus) {
        HDPlusProcess();
    } else {
        TerminalBuffer::Write("HDMI Device: Not detected\n");
    }
    
    TerminalBuffer::Write("\nPress A to Exit");
    WaitButton(ControllerA);
    HalReturnToFirmware(2);
}
