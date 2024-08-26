#include "MainWindow.h"
#include "./ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize COM library
    CoInitialize(nullptr);

    // Get the default playback device
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    IMMDevice* defaultDevice = nullptr;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
    deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &defaultDevice);

    // Get IAudioSessionManager2 for session management
    IAudioSessionManager2* sessionManager = nullptr;
    defaultDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&sessionManager);

    // Get the list of audio sessions
    IAudioSessionEnumerator* sessionEnumerator = nullptr;
    sessionManager->GetSessionEnumerator(&sessionEnumerator);

    int sessionCount;
    sessionEnumerator->GetCount(&sessionCount);

    for (int i = 0; i < sessionCount; i++) {
        IAudioSessionControl* sessionControl = nullptr;
        sessionEnumerator->GetSession(i, &sessionControl);

        IAudioSessionControl2* sessionControl2 = nullptr;
        sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);

        // Get the process ID and name
        DWORD processId;
        sessionControl2->GetProcessId(&processId);

        WCHAR* displayName = nullptr;
        sessionControl2->GetDisplayName(&displayName);

        // Get the volume level for the session
        ISimpleAudioVolume* audioVolume = nullptr;
        sessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&audioVolume);

        float volumeLevel = 0.0f;
        if (audioVolume != nullptr) {
            audioVolume->GetMasterVolume(&volumeLevel);
            audioVolume->Release();
        }

        // If the name is not available, use the process name
        std::wstring appName;
        if (displayName == nullptr || wcslen(displayName) == 0) {
            appName = GetProcessName(processId);
        }
        else {
            appName = displayName;
        }

        // Convert volume level to percentage
        int volumePercentage = static_cast<int>(volumeLevel * 100);

        // Output the application name, process ID, and volume level
        std::wcout << L"Application Name: " << appName
            << L" | Process ID: " << processId
            << L" | Volume: " << volumePercentage << L"%" << std::endl;

        if (displayName) {
            CoTaskMemFree(displayName);
        }
        sessionControl2->Release();
        sessionControl->Release();
    }

    // Clean up
    sessionEnumerator->Release();
    sessionManager->Release();
    defaultDevice->Release();
    deviceEnumerator->Release();
    CoUninitialize();
}

std::wstring MainWindow::GetProcessName(DWORD processID) {
    std::wstring processName = L"<unknown>";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    if (hProcess != nullptr) {
        WCHAR buffer[MAX_PATH];
        if (GetModuleFileNameExW(hProcess, 0, buffer, MAX_PATH)) {
            processName = buffer;
        }
        CloseHandle(hProcess);
    }
    return processName;
}

MainWindow::~MainWindow()
{
    delete ui;
}
