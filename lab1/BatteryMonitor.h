#pragma once
#define _WIN32_WINNT 0x0600

#include <iostream>
#include <initguid.h>
#include <windows.h>
#include <vector>
#include <string>
#include <powrprof.h>
#include <powersetting.h>

#pragma comment(lib, "PowrProf.lib")

void getPowerStatus();

struct SchemeSettings {
    DWORD displayTimeoutAC = 0;   
    DWORD displayTimeoutDC = 0;  
    DWORD sleepTimeoutAC = 0;     
    DWORD sleepTimeoutDC = 0;    
    DWORD criticalBatteryAC = 0;  
    DWORD criticalBatteryDC = 0; 
};

struct PowerScheme {
	GUID guid;
	std::wstring name;
	bool isActive;
    SchemeSettings settings;
};

std::wstring GuidToString(const GUID& guid);

std::vector<PowerScheme> GetPowerSchemes();

std::string WStringToString(const std::wstring& wstr);

void ReadPowerSetting(const GUID& schemeGuid, const GUID& subGroup, const GUID& settingGuid, DWORD& outAC, DWORD& outDC);

SchemeSettings GetSchemeSettings(const GUID& schemeGuid);

bool SetActiveScheme(const GUID& schemeGuid);
bool SetActiveSchemeByString(const std::string& guidStr);

