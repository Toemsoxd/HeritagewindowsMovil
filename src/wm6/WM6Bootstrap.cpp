#include <windows.h>
#include <d3dm.h>

static const TCHAR kClassName[] = TEXT("OptiCraftWM6Bootstrap");

static IDirect3DMobile* g_d3dm = 0;
static IDirect3DMobileDevice* g_device = 0;
static HWND g_window = 0;

static LRESULT CALLBACK Wm6WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static HWND CreateWm6Window(HINSTANCE instance)
{
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Wm6WndProc;
    wc.hInstance = instance;
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = kClassName;

    if (!RegisterClass(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return 0;

    return CreateWindow(
        kClassName,
        TEXT("OptiCraft Heritage WM6"),
        WS_VISIBLE,
        0, 0, 320, 240,
        0, 0, instance, 0);
}

static void ShowHrError(HWND owner, LPCTSTR operation, HRESULT hr)
{
    TCHAR buffer[256];
    wsprintf(
        buffer,
        TEXT("%s failed.\r\nHRESULT=0x%08lx"),
        operation,
        (unsigned long)hr);
    MessageBox(owner, buffer, TEXT("OptiCraft WM6"), MB_OK | MB_ICONERROR);
}

static bool InitD3DM(HWND hwnd)
{
    g_d3dm = Direct3DMobileCreate(D3DM_SDK_VERSION);
    if (!g_d3dm) {
        MessageBox(
            hwnd,
            TEXT("Direct3DMobileCreate returned NULL.\r\n")
            TEXT("No usable Direct3D Mobile runtime was exposed by the phone."),
            TEXT("OptiCraft WM6"),
            MB_OK | MB_ICONERROR);
        return false;
    }

    D3DMCAPS caps;
    ZeroMemory(&caps, sizeof(caps));

    HRESULT hr = g_d3dm->GetDeviceCaps(
        0,
        D3DMDEVTYPE_DEFAULT,
        &caps);
    if (FAILED(hr)) {
        ShowHrError(hwnd, TEXT("GetDeviceCaps"), hr);
        return false;
    }

    D3DMPRESENT_PARAMETERS pp;
    ZeroMemory(&pp, sizeof(pp));

    pp.BackBufferWidth = 320;
    pp.BackBufferHeight = 240;
    pp.BackBufferFormat = D3DMFMT_R5G6B5;
    pp.BackBufferCount = 1;
    pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pp.SwapEffect = D3DMSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = hwnd;
    pp.Windowed = TRUE;
    pp.EnableAutoDepthStencil = FALSE;

    hr = g_d3dm->CreateDevice(
        0,
        D3DMDEVTYPE_DEFAULT,
        hwnd,
        0,
        &pp,
        &g_device);

    if (FAILED(hr)) {
        ShowHrError(hwnd, TEXT("CreateDevice"), hr);
        return false;
    }

    return true;
}

static void ShutdownD3DM()
{
    if (g_device) {
        g_device->Release();
        g_device = 0;
    }

    if (g_d3dm) {
        g_d3dm->Release();
        g_d3dm = 0;
    }
}

static bool RenderFrame(unsigned long frame)
{
    // Deliberately keep the first hardware milestone tiny:
    // clear -> EndScene -> Present. Once this works on the S730, the same
    // device becomes the base for the real terrain renderer.
    unsigned long phase = (frame >> 3) & 255UL;
    unsigned long r = phase;
    unsigned long g = 255UL - phase;
    D3DMCOLOR color = D3DMCOLOR_ARGB(255, r, g, 32);

    HRESULT hr = g_device->Clear(
        0,
        0,
        D3DMCLEAR_TARGET,
        color,
        1.0f,
        0);

    if (FAILED(hr))
        return false;

    hr = g_device->BeginScene();
    if (FAILED(hr))
        return false;

    hr = g_device->EndScene();
    if (FAILED(hr))
        return false;

    hr = g_device->Present(0, 0, 0, 0);
    return SUCCEEDED(hr);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPTSTR, int)
{
    g_window = CreateWm6Window(instance);
    if (!g_window)
        return 10;

    if (!InitD3DM(g_window)) {
        ShutdownD3DM();
        return 20;
    }

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    unsigned long frame = 0;

    for (;;) {
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                ShutdownD3DM();
                return 0;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!RenderFrame(frame++)) {
            MessageBox(
                g_window,
                TEXT("The D3DM frame failed.\r\n")
                TEXT("The device was created, but rendering/presentation failed."),
                TEXT("OptiCraft WM6"),
                MB_OK | MB_ICONERROR);
            ShutdownD3DM();
            return 30;
        }

        // Do not Sleep() here. D3DM presentation/VSync is allowed to pace the
        // first hardware test. If the driver runs uncapped, the next renderer
        // stage will add an explicit frame limiter.
    }
}
