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

    // Get the overall system volume
    defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&endpointVolume);

    float systemVolumeLevel = 0.0f;
    if (endpointVolume != nullptr) {
        endpointVolume->GetMasterVolumeLevelScalar(&systemVolumeLevel);
    }

    // Convert system volume level to percentage
    int systemVolumePercentage = static_cast<int>(systemVolumeLevel * 100);

    // Output the overall system volume
    std::wcout << L"System Volume: " << systemVolumePercentage << L"%" << std::endl;

    // Get IAudioSessionManager2 for session management
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

        // Get the display name
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
        std::wcout << L"Application Name: " << GetFileNameFromPath(appName)
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
    SetSystemVolume(53);
    SetApplicationVolume(L"steam.exe", 70);
}










MainWindow::~MainWindow()
{
    if (endpointVolume) {
        endpointVolume->Release();
    }
    if (sessionManager) {
        sessionManager->Release();
    }
    CoUninitialize();

    delete ui;
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

std::wstring MainWindow::GetFileNameFromPath(const std::wstring& path)
{
    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

void MainWindow::SetSystemVolume(int volumePercentage)
{
    if (endpointVolume) {
        float volumeLevel = static_cast<float>(volumePercentage) / 100.0f;
        endpointVolume->SetMasterVolumeLevelScalar(volumeLevel, nullptr);
    }
}

void MainWindow::SetApplicationVolume(const std::wstring& appIdentifier, int volumePercentage)
{
    IAudioSessionEnumerator* sessionEnumerator = nullptr;
    sessionManager->GetSessionEnumerator(&sessionEnumerator);

    int sessionCount;
    sessionEnumerator->GetCount(&sessionCount);

    for (int i = 0; i < sessionCount; ++i) {
        IAudioSessionControl* sessionControl = nullptr;
        IAudioSessionControl2* sessionControl2 = nullptr;

        sessionEnumerator->GetSession(i, &sessionControl);
        sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);

        DWORD processId;
        sessionControl2->GetProcessId(&processId);

        // Get the display name
        WCHAR* displayName = nullptr;
        sessionControl2->GetDisplayName(&displayName);

        // Get the volume level for the session
        ISimpleAudioVolume* audioVolume = nullptr;
        sessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&audioVolume);

        // If the name is not available, use the process name
        std::wstring appName;
        if (displayName == nullptr || wcslen(displayName) == 0) {
            appName = GetProcessName(processId);
        }
        else {
            appName = displayName;
        }

        // If the process name or path matches the provided identifier
        if (appName == appIdentifier || GetFileNameFromPath(appName) == appIdentifier) {
            float volumeLevel = static_cast<float>(volumePercentage) / 100.0f;
            if (audioVolume != nullptr) {
                audioVolume->SetMasterVolume(volumeLevel, nullptr);
            }
            break;
        }

        // Clean up
        if (audioVolume) {
            audioVolume->Release();
        }
        if (displayName) {
            CoTaskMemFree(displayName);
        }
        sessionControl2->Release();
        sessionControl->Release();
    }

    sessionEnumerator->Release();
}