#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <cmath>
#include <chrono>
#include <windows.h>

#define ll long long
#define ld long double
#define vvd vector<vector<ld>>

using namespace std;
using namespace chrono;

CRITICAL_SECTION coutCS;

HANDLE hConsole;
const ll numThreads = 7;

ll threadPriorities[numThreads] = {
        THREAD_PRIORITY_TIME_CRITICAL,
        THREAD_PRIORITY_HIGHEST,
        THREAD_PRIORITY_ABOVE_NORMAL,
        THREAD_PRIORITY_NORMAL,
        THREAD_PRIORITY_BELOW_NORMAL,
        THREAD_PRIORITY_LOWEST,
        THREAD_PRIORITY_IDLE
};

string priorityNames[numThreads] = {
        "TIME_CRITICAL",
        "HIGHEST",
        "ABOVE NORMAL",
        "NORMAL",
        "BELOW NORMAL",
        "LOWEST",
        "IDLE"
};

void UpdateProgressBar(ll index, ll progress)
{
    EnterCriticalSection(&coutCS);

    COORD cursorPosition;
    cursorPosition.X = 0;
    cursorPosition.Y = (SHORT)(index);
    SetConsoleCursorPosition(hConsole, cursorPosition);

    string progressBar = "Поток " + to_string(index + 1) + " прогресс: [";
    ll barWidth = 50;
    ll pos = barWidth * progress / 100;
    for (ll i = 0; i < barWidth; ++i)
    {
        progressBar += (i < pos) ? "=" : " ";
    }
    progressBar += "] " + to_string(progress) + "%";

    cout << progressBar;

    LeaveCriticalSection(&coutCS);
}

ll MultiplyMatrices(const vvd& A, const vvd& B, vvd& C, ll threadIndex)
{
    ll N = A.size();
    ll iterations = 0;
    for (ll i = 0; i < N; ++i)
    {
        for (ll j = 0; j < N; ++j)
        {
            C[i][j] = 0;
            for (ll k = 0; k < N; ++k)
            {
                C[i][j] += A[i][k] * B[k][j];
                ++iterations;
            }
        }

        ll progress = (100 * (i + 1)) / N;
        UpdateProgressBar(threadIndex, min(progress, 100LL));
    }
    return iterations;
}

DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
    ll threadIndex = *(ll*)lpParam;
    SetThreadPriority(GetCurrentThread(), threadPriorities[threadIndex]);

    auto startTime = high_resolution_clock::now();

    vvd A(1200, vector<ld>(1200, 1.0));
    vvd B(1200, vector<ld>(1200, 1.0));
    vvd C(1200, vector<ld>(1200, 0.0));

    ll iterations = MultiplyMatrices(A, B, C, threadIndex);

    auto endTime = high_resolution_clock::now();
    duration<ld> elapsed = endTime - startTime;

    EnterCriticalSection(&coutCS);

    COORD cursorPosition;
    cursorPosition.X = 0;
    cursorPosition.Y = (SHORT)(numThreads + threadIndex);
    SetConsoleCursorPosition(hConsole, cursorPosition);

    cout << "Поток " << threadIndex + 1 << " с приоритетом " << priorityNames[threadIndex]
        << " завершен за " << elapsed.count() << " секунд. "
            << "Итераций: " << iterations << endl;

        LeaveCriticalSection(&coutCS);

        return 0;
}

int main()
{
    InitializeCriticalSection(&coutCS);
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    system("cls");

    HANDLE hThreads[numThreads];
    DWORD threadIDs[numThreads];
    ll threadIndices[numThreads];

    for (ll i = 0; i < numThreads; ++i)
    {
        threadIndices[i] = i;
        hThreads[i] = CreateThread(NULL, 0, ThreadFunction, &threadIndices[i], 0, &threadIDs[i]);
    }

    WaitForMultipleObjects(numThreads, hThreads, TRUE, INFINITE);

    for (ll i = 0; i < numThreads; ++i)
    {
        CloseHandle(hThreads[i]);
    }

    DeleteCriticalSection(&coutCS);

    COORD cursorPosition;
    cursorPosition.X = 0;
    cursorPosition.Y = (SHORT)(2 * numThreads);
    SetConsoleCursorPosition(hConsole, cursorPosition);
    cout << "Все потоки завершены." << endl;

    return 0;
}


