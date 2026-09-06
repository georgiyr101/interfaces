#include "BatteryMonitor.h"
using namespace std;

#ifndef GUID_BATTERY_DISCHARGE_LEVEL_CRITICAL
DEFINE_GUID(GUID_BATTERY_DISCHARGE_LEVEL_CRITICAL,
    0x6374100d, 0xee3b, 0x431c, 0x8b, 0x2f, 0x31, 0x01, 0x21, 0x19, 0xf6, 0x77);
#endif

void getPowerStatus() {
	SYSTEM_POWER_STATUS sps;

	if (GetSystemPowerStatus(&sps)) {
        if (sps.ACLineStatus == 1) {
            cout << "Тип питания: Сеть\n";
        }
        else if (sps.ACLineStatus == 0) {
            cout << "Тип питания: Аккумулятор\n";
        }
        else {
            cout << "Тип питания: Неизвестно\n";
        }

        if (sps.BatteryLifePercent != 255) {
            cout << "Уровень заряда: " << (int)sps.BatteryLifePercent << "%\n";
        }
        else {
            cout << "Уровень заряда: Неизвестно\n";
        }

        if (sps.BatteryLifeTime != (DWORD)-1) {
            cout << "Осталось времени: " << sps.BatteryLifeTime / 60 << " мин\n";
        }
        else {
            cout << "Осталось времени: Неизвестно\n";
        }
    }
    else {
        cerr << "Ошибка получения статуса энергопитания.\n";
	}
}

std::wstring GuidToString(const GUID& guid) {
    wchar_t guidString[40] = { 0 };
    StringFromGUID2(guid, guidString, 40);
    return std::wstring(guidString);
}

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
    return strTo;
}

std::vector<PowerScheme> GetPowerSchemes() {
    std::vector<PowerScheme> schemes;

    // Получить GUID текущей активной схемы
    GUID activeSchemeGuid = { 0 };
    GUID* pActiveGuid = nullptr;
    if (PowerGetActiveScheme(NULL, &pActiveGuid) == ERROR_SUCCESS && pActiveGuid != nullptr) {
        activeSchemeGuid = *pActiveGuid;
        LocalFree(pActiveGuid);
    }

    DWORD index = 0;
    GUID schemeGuid = { 0 };

    // Пройти по схемам питания
    while (true) {
        DWORD guidSize = sizeof(GUID);

        DWORD enumResult = PowerEnumerate(
            NULL,                  // RootPowerKey 
            NULL,                  // GUID схемы питания
            NULL,                  // GUID подгруппы параметров
            ACCESS_SCHEME,         // Перечислить схемы управления питанием
            index,                 // Индекс текущей схемы
            (UCHAR*)&schemeGuid,   // Буфер для сохранения GUID схемы
            &guidSize
        );

        if (enumResult != ERROR_SUCCESS) {
            break; // Выходим из цикла, когда схемы закончились
        }

        PowerScheme scheme;
        scheme.guid = schemeGuid;
        scheme.isActive = (IsEqualGUID(schemeGuid, activeSchemeGuid) == TRUE);

        // Считать дружественное имя текущей схемы
        DWORD bufferSize = 0;
        DWORD result = PowerReadFriendlyName(
            NULL,
            &schemeGuid,
            NULL,
            NULL,
            NULL,
            &bufferSize
        );

        if (result == ERROR_SUCCESS && bufferSize > 0) {
            std::vector<BYTE> nameBuffer(bufferSize);
            result = PowerReadFriendlyName(
                NULL,
                &schemeGuid,
                NULL,
                NULL,
                nameBuffer.data(),
                &bufferSize
            );

            if (result == ERROR_SUCCESS) {
                const wchar_t* wstr = reinterpret_cast<const wchar_t*>(nameBuffer.data());
                size_t charCount = bufferSize / sizeof(wchar_t);
                if (charCount > 0 && wstr[charCount - 1] == L'\0') {
                    charCount--;
                }
                scheme.name.assign(wstr, charCount);
            }
        }

        if (scheme.name.empty()) {
            scheme.name = L"Неизвестная схема";
        }

        // Считать ключевые параметры текущей схемы
        scheme.settings = GetSchemeSettings(schemeGuid);

        schemes.push_back(scheme);
        index++;
    }

    return schemes;
}

void ReadPowerSetting(const GUID& schemeGuid, const GUID& subGroup, const GUID& settingGuid, DWORD& outAC, DWORD& outDC) {
    PowerReadACValueIndex(NULL, &schemeGuid, &subGroup, &settingGuid, &outAC);
    PowerReadDCValueIndex(NULL, &schemeGuid, &subGroup, &settingGuid, &outDC);
}

SchemeSettings GetSchemeSettings(const GUID& schemeGuid) {
    SchemeSettings settings;

    ReadPowerSetting(schemeGuid, GUID_VIDEO_SUBGROUP, GUID_VIDEO_POWERDOWN_TIMEOUT,
        settings.displayTimeoutAC, settings.displayTimeoutDC);

    ReadPowerSetting(schemeGuid, GUID_SLEEP_SUBGROUP, GUID_STANDBY_TIMEOUT,
        settings.sleepTimeoutAC, settings.sleepTimeoutDC);

    ReadPowerSetting(schemeGuid, GUID_BATTERY_SUBGROUP, GUID_BATTERY_DISCHARGE_LEVEL_CRITICAL,
        settings.criticalBatteryAC, settings.criticalBatteryDC);

    return settings;
}

bool SetActiveScheme(const GUID& schemeGuid) {
    return PowerSetActiveScheme(NULL, &schemeGuid) == ERROR_SUCCESS;
}

bool SetActiveSchemeByString(const std::string& guidStr) {
    GUID schemeGuid;
    std::wstring wGuidStr(guidStr.begin(), guidStr.end());

    if (CLSIDFromString(wGuidStr.c_str(), &schemeGuid) == S_OK) {
        return SetActiveScheme(schemeGuid);
    }
    return false;
}