#include "SharedMemory.h"
#include <stdexcept>
#include <iostream>

SharedMemory::SharedMemory(const std::wstring& name, size_t size)
    : hMapFile(NULL), pBuf(NULL),
    hReadersMutex(NULL), hWritersMutex(NULL), hNoReadersEvent(NULL),
    hTurnstile(NULL), hWritersCountMutex(NULL),
    activeReaders(0), waitingWriters(0), stopFlag(false), sharedName(name)
{
    hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        (DWORD)size,
        name.c_str()
    );

    if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();
        std::cerr << "CreateFileMappingW failed with error code: " << errorCode << std::endl;
        throw std::runtime_error("Could not create file mapping object.");
    }

    pBuf = MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        size
    );

    if (pBuf == NULL) {
        DWORD errorCode = GetLastError();
        CloseHandle(hMapFile);
        std::cerr << "MapViewOfFile failed with error code: " << errorCode << std::endl;
        throw std::runtime_error("Could not map view of file.");
    }

    hReadersMutex = CreateMutexW(NULL, FALSE, (name + L"_readers_mutex").c_str());
    if (hReadersMutex == NULL || hReadersMutex == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        std::cerr << "CreateMutexW for readers failed with error code: " << errorCode << std::endl;
        throw std::runtime_error("Could not create readers mutex.");
    }

    hWritersMutex = CreateMutexW(NULL, FALSE, (name + L"_writers_mutex").c_str());
    if (hWritersMutex == NULL || hWritersMutex == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();
        CloseHandle(hReadersMutex);
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        std::cerr << "CreateMutexW for writers failed with error code: " << errorCode << std::endl;
        throw std::runtime_error("Could not create writers mutex.");
    }

    hNoReadersEvent = CreateEventW(NULL, TRUE, TRUE, (name + L"_no_readers_event").c_str());
    if (hNoReadersEvent == NULL || hNoReadersEvent == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();
        CloseHandle(hWritersMutex);
        CloseHandle(hReadersMutex);
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        std::cerr << "CreateEventW failed with error code: " << errorCode << std::endl;
        throw std::runtime_error("Could not create no readers event.");
    }

    hTurnstile = CreateSemaphoreW(NULL, 1, 1, (name + L"_turnstile").c_str());
    if (hTurnstile == NULL || hTurnstile == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();
        CloseHandle(hNoReadersEvent);
        CloseHandle(hWritersMutex);
        CloseHandle(hReadersMutex);
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        std::cerr << "CreateSemaphoreW for turnstile failed with error code: " << errorCode << std::endl;
        throw std::runtime_error("Could not create turnstile semaphore.");
    }

    hWritersCountMutex = CreateMutexW(NULL, FALSE, (name + L"_writers_count_mutex").c_str());
    if (hWritersCountMutex == NULL || hWritersCountMutex == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();
        CloseHandle(hTurnstile);
        CloseHandle(hNoReadersEvent);
        CloseHandle(hWritersMutex);
        CloseHandle(hReadersMutex);
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        std::cerr << "CreateMutexW for writers count failed with error code: " << errorCode << std::endl;
        throw std::runtime_error("Could not create writers count mutex.");
    }
}

SharedMemory::~SharedMemory() {
    if (hWritersCountMutex) CloseHandle(hWritersCountMutex);
    if (hTurnstile) CloseHandle(hTurnstile);
    if (hNoReadersEvent) CloseHandle(hNoReadersEvent);
    if (hWritersMutex) CloseHandle(hWritersMutex);
    if (hReadersMutex) CloseHandle(hReadersMutex);
    if (pBuf) UnmapViewOfFile(pBuf);
    if (hMapFile) CloseHandle(hMapFile);
}

void* SharedMemory::getData() {
    return pBuf;
}

HANDLE SharedMemory::getReadersMutex() {
    return hReadersMutex;
}

HANDLE SharedMemory::getWritersMutex() {
    return hWritersMutex;
}

HANDLE SharedMemory::getNoReadersEvent() {
    return hNoReadersEvent;
}

HANDLE SharedMemory::getTurnstile() {
    return hTurnstile;
}

HANDLE SharedMemory::getWritersCountMutex() {
    return hWritersCountMutex;
}

int SharedMemory::getActiveReaders() {
    return activeReaders;
}

void SharedMemory::incrementActiveReaders() {
    ++activeReaders;
}

void SharedMemory::decrementActiveReaders() {
    --activeReaders;
}

void SharedMemory::incrementWaitingWriters() {
    ++waitingWriters;
}

void SharedMemory::decrementWaitingWriters() {
    --waitingWriters;
}

int SharedMemory::getWaitingWriters() {
    return waitingWriters;
}

void SharedMemory::setStopFlag(bool value) {
    stopFlag = value;
}

bool SharedMemory::getStopFlag() {
    return stopFlag;
}

void SharedMemory::incrementReads() {
    ++totalReads;
}

void SharedMemory::incrementWrites() {
    ++totalWrites;
}

void SharedMemory::addReaderWaitTime(long long ms) {
    totalReaderWaitTime += ms;
}

void SharedMemory::addWriterWaitTime(long long ms) {
    totalWriterWaitTime += ms;
}

int SharedMemory::getTotalReads() const {
    return totalReads;
}

int SharedMemory::getTotalWrites() const {
    return totalWrites;
}

long long SharedMemory::getTotalReaderWaitTime() const {
    return totalReaderWaitTime;
}

long long SharedMemory::getTotalWriterWaitTime() const {
    return totalWriterWaitTime;
}

