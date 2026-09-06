#include "BatteryMonitor.h"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <windows.h>

std::string FormatTimeout(DWORD seconds) {
    if (seconds == 0) return "Никогда";
    return std::to_string(seconds / 60) + " мин";
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (argc > 1) {
        std::string guidStr = argv[1];
        if (SetActiveSchemeByString(guidStr)) {
            std::cout << "SUCCESS: Scheme changed to " << guidStr << "\n";
            return 0;
        }
        else {
            std::cerr << "ERROR: Failed to change scheme to " << guidStr << "\n";
            return 1;
        }
    }

    getPowerStatus();
    std::cout << "\n";

    std::cout << "--- SCHEMES START ---\n";
    std::vector<PowerScheme> schemes = GetPowerSchemes();

    for (const auto& scheme : schemes) {
        std::cout << "NAME:" << WStringToString(scheme.name) << "\n";
        std::cout << "GUID:" << WStringToString(GuidToString(scheme.guid)) << "\n";
        std::cout << "ACTIVE:" << (scheme.isActive ? "1" : "0") << "\n";
        std::cout << "DISPLAY_AC:" << scheme.settings.displayTimeoutAC << "\n";
        std::cout << "DISPLAY_DC:" << scheme.settings.displayTimeoutDC << "\n";
        std::cout << "SLEEP_AC:" << scheme.settings.sleepTimeoutAC << "\n";
        std::cout << "SLEEP_DC:" << scheme.settings.sleepTimeoutDC << "\n";
        std::cout << "CRIT_AC:" << scheme.settings.criticalBatteryAC << "\n";
        std::cout << "CRIT_DC:" << scheme.settings.criticalBatteryDC << "\n";
        std::cout << "--- SCHEME END ---\n";
    }

    return 0;
}