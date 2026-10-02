#include <windows.h>
#include <d3dm.h>

static const TCHAR kClassName[] = TEXT("OptiCraftWM6D3DMProbe");

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static HWND createProbeWindow(HINSTANCE instance)
{
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = kClassName;

    if (!RegisterClass(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return NULL;

    return CreateWindow(
        kClassName,
        TEXT("OptiCraft D3DM Probe"),
        WS_VISIBLE,
        0, 0, 320, 240,
        NULL, NULL, instance, NULL);
}

static void logCaps(const D3DMCAPS& caps)
{
    TCHAR buffer[512];
    wsprintf(
        buffer,
        TEXT("D3DM OK\\r\\nAdapter=%lu\\r\\nDevCaps=0x%08lx\\r\\nRasterCaps=0x%08lx\\r\\n")
        TEXT("VertexProcessingCaps=0x%08lx\\r\\nMaxTexture=%lux%lu\\r\\nMaxStages=%lu\\r\\n")
        TEXT("MaxTextures=%lu\\r\\nMaxPrimitives=%lu"),
        (unsigned long)caps.AdapterOrdinal,
        (unsigned long)caps.DevCaps,
        (unsigned long)caps.RasterCaps,
        (unsigned long)caps.VertexProcessingCaps,
        (unsigned long)caps.MaxTextureWidth,
        (unsigned long)caps.MaxTextureHeight,
        (unsigned long)caps.MaxTextureBlendStages,
        (unsigned long)caps.MaxSimultaneousTextures,
        (unsigned long)caps.MaxPrimitiveCount);
    MessageBox(NULL, buffer, TEXT("OptiCraft WM6"), MB_OK);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPTSTR, int)
{
    HWND hwnd = createProbeWindow(instance);
    if (!hwnd)
        return 10;

    IDirect3DMobile* d3dm = Direct3DMobileCreate(D3DM_SDK_VERSION);
    if (!d3dm) {
        MessageBox(
            hwnd,
            TEXT("Direct3DMobileCreate failed.\\r\\n")
            TEXT("The WM6 SDK may be installed correctly, but this device does not expose a usable D3DM runtime."),
            TEXT("OptiCraft WM6"),
            MB_OK | MB_ICONERROR);
        return 20;
    }

    D3DMCAPS caps;
    ZeroMemory(&caps, sizeof(caps));
    HRESULT hr = d3dm->GetDeviceCaps(0, D3DMDEVTYPE_DEFAULT, &caps);
    if (FAILED(hr)) {
        TCHAR buffer[128];
        wsprintf(buffer, TEXT("GetDeviceCaps failed: 0x%08lx"), (unsigned long)hr);
        MessageBox(hwnd, buffer, TEXT("OptiCraft WM6"), MB_OK | MB_ICONERROR);
        d3dm->Release();
        return 30;
    }

    logCaps(caps);

    // Do not create the real game device yet. The first milestone is to prove
    // that the phone exposes D3DM and inspect its driver capabilities without
    // committing the renderer to a particular back-buffer/depth format.
    d3dm->Release();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
