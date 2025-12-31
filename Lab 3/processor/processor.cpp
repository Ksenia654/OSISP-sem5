#include <windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

const char* pipeName = "\\\\.\\pipe\\my_pipe";

std::string processRequest(const std::string& data) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return "Заявка \"" + data + "\" имеет длину: " + std::to_string(data.size());
}

int main() {
    HANDLE hPipe = CreateFileA(
        pipeName,
        GENERIC_READ | GENERIC_WRITE, 
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateFile failed: " << GetLastError() << std::endl;
        return 1;
    }

    DWORD mode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(hPipe, &mode, NULL, NULL)) {
        std::cerr << "SetNamedPipeHandleState failed: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return 1;
    }

    char buffer[1024];
    DWORD bytesRead = 0;
    while (true) {
        BOOL ok = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (!ok) {
            DWORD err = GetLastError();
            if (err == ERROR_BROKEN_PIPE) {
                break;
            }
            std::cerr << "ReadFile (job) failed: " << err << std::endl;
            break;
        }
        buffer[bytesRead] = '\0';
        std::string job(buffer);

        std::string result = processRequest(job);
        DWORD written = 0;
        if (!WriteFile(hPipe, result.c_str(), static_cast<DWORD>(result.size() + 1), &written, NULL)) {
            std::cerr << "WriteFile (result) failed: " << GetLastError() << std::endl;
            break;
        }

        std::cout << "Обработано: " << job << std::endl;
    }

    CloseHandle(hPipe);
    return 0;
}
