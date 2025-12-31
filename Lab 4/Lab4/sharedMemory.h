#pragma once
#include <windows.h>
#include <string>

class SharedMemory {
public:
    SharedMemory(const std::wstring& name, size_t size);
    ~SharedMemory();

    void* getData();

    HANDLE getReadersMutex();
    HANDLE getWritersMutex();
    HANDLE getNoReadersEvent();
    HANDLE getTurnstile();
    HANDLE getWritersCountMutex();

    int getActiveReaders();
    void incrementActiveReaders();
    void decrementActiveReaders();

    void incrementWaitingWriters();
    void decrementWaitingWriters();
    int getWaitingWriters();

    void setStopFlag(bool value);
    bool getStopFlag();

    void incrementReads();
    void incrementWrites();
    void addReaderWaitTime(long long ms);
    void addWriterWaitTime(long long ms);

    int getTotalReads() const;
    int getTotalWrites() const;
    long long getTotalReaderWaitTime() const;
    long long getTotalWriterWaitTime() const;

private:
    HANDLE hMapFile;
    void* pBuf;

    HANDLE hReadersMutex;
    HANDLE hWritersMutex;
    HANDLE hNoReadersEvent;
    HANDLE hTurnstile;
    HANDLE hWritersCountMutex;

    int activeReaders;
    int waitingWriters;

    volatile bool stopFlag;

    std::wstring sharedName;

    int totalReads = 0;
    int totalWrites = 0;
    long long totalReaderWaitTime = 0;
    long long totalWriterWaitTime = 0;
};
