#ifndef WINDOWSNATIVEEVENTFILTER_H
#define WINDOWSNATIVEEVENTFILTER_H

#include <QObject>
#include <Windows.h>

class WindowsNativeEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit WindowsNativeEventFilter(QObject *parent = nullptr);
    ~WindowsNativeEventFilter();
signals:
    void onDeviceChange();
private:
    void RegisterDeviceNotify();
    bool CreateMessageOnlyWindow();
    ATOM MyRegisterClass();
private:
    DWORD iThread;
    HANDLE hThread;
    HWND hWnd;

};

#endif // WINDOWSNATIVEEVENTFILTER_H
