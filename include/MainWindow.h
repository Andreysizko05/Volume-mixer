#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <iostream>
#include <psapi.h>  // for gettting name through path
#include <endpointvolume.h>
#include <vector>
#include <qdebug.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Method to set system volume
    void SetSystemVolume(int volumePercentage);

    // Method to set application volume
    void SetApplicationVolume(const std::wstring& appIdentifier, int volumePercentage);

private:
    Ui::MainWindow *ui;

    IAudioEndpointVolume* endpointVolume = nullptr;
    IAudioSessionManager2* sessionManager = nullptr;
    std::vector<ISimpleAudioVolume*> audioVolumes;

    std::wstring GetProcessName(DWORD processID);
    std::wstring GetFileNameFromPath(const std::wstring& path);
};
#endif // MAINWINDOW_H
