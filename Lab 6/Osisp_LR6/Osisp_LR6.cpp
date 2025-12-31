#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

void WriteRegHeader(std::ofstream& outFile)
{
    outFile << "Windows Registry Editor Version 5.00" << std::endl << std::endl;
}



std::string GetValueTypeString(DWORD type)
{
    switch (type)
    {
    case REG_SZ: return "REG_SZ";
    case REG_DWORD: return "REG_DWORD";
    case REG_BINARY: return "REG_BINARY";
    case REG_EXPAND_SZ: return "REG_EXPAND_SZ";
    case REG_MULTI_SZ: return "REG_MULTI_SZ";
    default: return "UNKNOWN";
    }
}

std::string ConvertBinaryToHex(const BYTE* data, DWORD size)
{
    std::ostringstream oss;
    for (DWORD i = 0; i < size; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
        if (i < size - 1)
        {
            oss << " ";
        }
    }

    return oss.str();
}

void SearchRegistryValue(HKEY hKey, std::ofstream& outFile)
{
    char valueName[256];
    DWORD valueNameSize;
    DWORD index = 0;

    while (true)
    {
        valueNameSize = sizeof(valueName);
        DWORD type;
        BYTE value[256];
        DWORD valueSize = sizeof(value);

        if (RegEnumValueA(hKey, index, valueName, &valueNameSize, NULL, &type, value, &valueSize) != ERROR_SUCCESS)
        {
            break;
        }

        std::string valueStr;
        if (type == REG_SZ || type == REG_EXPAND_SZ)
        {
            valueStr.assign((char*)value, valueSize);
        }
        else if (type == REG_DWORD)
        {
            DWORD intValue = *((DWORD*)value);
            valueStr = std::to_string(intValue);
        }
        else if (type == REG_BINARY)
        {
            valueStr = ConvertBinaryToHex(value, valueSize);
        }
        else
        {
            valueStr = "<unsupported type>";
        }

        std::cout << "  Found Value: \"" << valueName << "\" = \"" << valueStr << "\"" << std::endl;
        outFile << "\"" << valueName << "\"=" << GetValueTypeString(type) << ":" << valueStr << std::endl;

        index++;
    }
}

void SearchRegistryValueByName(HKEY hKey, std::ofstream& outFile, const std::string& searchValueName, const std::string& fullKeyPath)
{
    char valueName[256];
    DWORD valueNameSize;
    DWORD index = 0;

    while (true)
    {
        valueNameSize = sizeof(valueName);
        DWORD type;
        BYTE value[256];
        DWORD valueSize = sizeof(value);

        if (RegEnumValueA(hKey, index, valueName, &valueNameSize, NULL, &type, value, &valueSize) != ERROR_SUCCESS)
        {
            break;
        }

        if (searchValueName == valueName)
        {
            std::string valueStr;
            if (type == REG_SZ || type == REG_EXPAND_SZ)
            {
                valueStr.assign((char*)value, valueSize);
            }
            else if (type == REG_DWORD)
            {
                DWORD intValue = *((DWORD*)value);
                valueStr = std::to_string(intValue);
            }
            else if (type == REG_BINARY)
            {
                valueStr = ConvertBinaryToHex(value, valueSize);
            }
            else
            {
                valueStr = "<unsupported type>";
            }

            std::cout << "  Found Value by Name: \"" << valueName << "\" = \"" << valueStr << "\"" << std::endl;
            outFile << "\"" << valueName << "\"=" << GetValueTypeString(type) << ":" << valueStr << std::endl;
        }

        index++;
    }
}

void SearchRegistryKey(HKEY hKey, const std::string& pattern, std::ofstream& outFile, const std::string& searchValueName) {
    char keyName[256];
    DWORD keyNameSize;
    DWORD index = 0;

    while (true)
    {
        keyNameSize = sizeof(keyName);
        if (RegEnumKeyExA(hKey, index, keyName, &keyNameSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
        {
            break;
        }

        std::string fullKeyPath = pattern + "\\" + keyName;
        std::cout << "Found Key: " << fullKeyPath << std::endl;
        outFile << "[" << fullKeyPath << "]" << std::endl;


        HKEY subKey;
        if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &subKey) == ERROR_SUCCESS)
        {
            if (!searchValueName.empty())
            {
                SearchRegistryValueByName(subKey, outFile, searchValueName, fullKeyPath);
            }
            else
            {
                SearchRegistryValue(subKey, outFile);
            }

            SearchRegistryKey(subKey, fullKeyPath, outFile, searchValueName);
            RegCloseKey(subKey);
        }

        index++;
    }
}


void SearchRegistry(HKEY hKeyRoot, const std::string& pattern, const std::string& searchValueName)
{
    std::ofstream outFile("registry_output.reg");
    if (!outFile)
    {
        std::cerr << "Failed to create output file." << std::endl;
        return;
    }

    WriteRegHeader(outFile);
    SearchRegistryKey(hKeyRoot, pattern, outFile, searchValueName);
    outFile.close();
}

int main()
{
    std::string registryPath;
    std::string searchValueName;

    std::cout << "Enter the Registry Path: ";
    std::getline(std::cin, registryPath);

    std::cout << "Enter a value name to search for (leave blank for all values): ";
    std::getline(std::cin, searchValueName);

    HKEY hKeyRoot;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, registryPath.c_str(), 0, KEY_READ, &hKeyRoot) == ERROR_SUCCESS)
    {
        SearchRegistry(hKeyRoot, registryPath, searchValueName);
        RegCloseKey(hKeyRoot);
    }
    else
    {
        std::cerr << "Failed to open registry key." << std::endl;
    }

    return 0;
}