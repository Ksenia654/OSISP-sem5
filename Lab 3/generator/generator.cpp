#include <windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>


const char* pipeName = "\\\\.\\pipe\\my_pipe";

int main() {
    HANDLE hPipe = CreateNamedPipeA(
        pipeName,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 
        1,        
        4096,     
        4096,    
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateNamedPipe failed: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Ожидание подключения клиента..." << std::endl;
    BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (!connected) {
        std::cerr << "ConnectNamedPipe failed: " << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return 1;
    }
    std::cout << "Клиент подключен." << std::endl;

    for (int i = 0; i < 16; ++i) {
        std::string job = "Заявка " + std::to_string(i);
        DWORD written = 0;
        if (!WriteFile(hPipe, job.c_str(), static_cast<DWORD>(job.size() + 1), &written, NULL)) {
            std::cerr << "WriteFile (job) failed: " << GetLastError() << std::endl;
            break;
        }
        std::cout << "Сгенерировано: " << job << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        char buffer[1024];
        DWORD bytesRead = 0;
        if (!ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            std::cerr << "ReadFile (result) failed: " << GetLastError() << std::endl;
            break;
        }
        buffer[bytesRead] = '\0';
        std::cout << "Результат: " << buffer << std::endl;
    }

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    return 0;
}


