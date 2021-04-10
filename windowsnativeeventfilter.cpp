#include "windowsnativeeventfilter.h"
#include <tchar.h>
#include <Dbt.h>
#include <setupapi.h>
#include <QDebug>

#pragma comment (lib, "Kernel32.lib")
#pragma comment (lib, "User32.lib")

#define THRD_MESSAGE_EXIT WM_USER + 1
const _TCHAR CLASS_NAME[] = _T("Sample Window Class");

WindowsNativeEventFilter *instance = nullptr;

static const GUID GUID_DEVINTERFACE_LIST[] =
{
    // GUID_DEVINTERFACE_USB_DEVICE
    { 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },
    // GUID_DEVINTERFACE_DISK
    { 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },
    // GUID_DEVINTERFACE_HID,
    { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },
    // GUID_NDIS_LAN_CLASS
    { 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } }
    //// GUID_DEVINTERFACE_COMPORT
    //{ 0x86e0d1e0, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },
    //// GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
    //{ 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },
    //// GUID_DEVINTERFACE_PARALLEL
    //{ 0x97F76EF0, 0xF883, 0x11D0, { 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C } },
    //// GUID_DEVINTERFACE_PARCLASS
    //{ 0x811FC6A5, 0xF728, 0x11D0, { 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1 } }
};

WindowsNativeEventFilter::WindowsNativeEventFilter(QObject *parent)
    : QObject(parent)
{
    instance = this;
    DWORD iThread;
    HANDLE hThread = CreateThread(NULL, 0, ThrdFunc, NULL, 0, &iThread);
    if (hThread == NULL) {
        qDebug() << "error";
    }
}

WindowsNativeEventFilter::~WindowsNativeEventFilter()
{
    PostThreadMessage(iThread, THRD_MESSAGE_EXIT, 0, 0);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
}

static DWORD ThrdFunc(LPVOID lpParam)
{
    (void)lpParam;
    if (0 == instance->MyRegisterClass())
        return -1;

    if (!instance->CreateMessageOnlyWindow())
        return -1;

    instance->RegisterDeviceNotify();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == THRD_MESSAGE_EXIT)
        {
            qDebug() << "worker receive the exiting Message...";
            return 0;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void WindowsNativeEventFilter::RegisterDeviceNotify()
{
    HDEVNOTIFY hDevNotify;
    for (int i = 0; i < sizeof(GUID_DEVINTERFACE_LIST) / sizeof(GUID); i++)
    {
        DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
        ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
        NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
        NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
        hDevNotify = RegisterDeviceNotification(hWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
    }
}

bool WindowsNativeEventFilter::CreateMessageOnlyWindow()
{
    hWnd = CreateWindowEx(0, CLASS_NAME, _T(""), WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL,       // Parent window
                          NULL,       // Menu
                          GetModuleHandle(NULL),  // Instance handle
                          NULL        // Additional application data
                          );

    return hWnd != NULL;
}

ATOM WindowsNativeEventFilter::MyRegisterClass()
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    return RegisterClass(&wc);
}

static LRESULT CALLBACK  WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if((message == WM_DEVICECHANGE) && (((UINT)wParam == DBT_DEVICEARRIVAL) || ((UINT)wParam == DBT_DEVICEREMOVECOMPLETE))){
        qDebug() << "On device change";
        emit instance->onDeviceChange();
        return true;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
