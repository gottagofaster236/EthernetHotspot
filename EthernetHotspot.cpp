// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2023 Lev Leontev

#include <iostream>
#include <chrono>
#include <thread>
#include <string_view>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Networking.Connectivity.h>
#include <winrt/Windows.Networking.NetworkOperators.h>
#include <windows.h>

using namespace winrt::Windows::Networking::Connectivity;
using namespace winrt::Windows::Networking::NetworkOperators;

static bool isAlreadyRunning();
static void tryTurnOnHotspot();

// Comment this line for the console to be shown.
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
int main() {
    if (isAlreadyRunning()) {
        std::cout << "EthernetHotspot is running already";
        return -1;
    }
    std::cout << "EthernetHotspot: application loaded" << std::endl;

    winrt::init_apartment();
    while (true) {
        tryTurnOnHotspot();
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

static TetheringOperationalState getDesiredTetheringOperationalState(const ConnectionProfile& profile);
static std::string_view tetheringOperationStatusToString(TetheringOperationStatus status);

bool isAlreadyRunning() {
    CreateMutexA(0, FALSE, "Local\\EthernetHotspot");
    return GetLastError() == ERROR_ALREADY_EXISTS;
}

void tryTurnOnHotspot() {
    ConnectionProfile profile = NetworkInformation::GetInternetConnectionProfile();
    if (!profile) {
        std::cout << "Network connection profile is null" << std::endl;
        return;
    }

    TetheringOperationalState desiredState = getDesiredTetheringOperationalState(profile);

    NetworkOperatorTetheringManager tetheringManager = NetworkOperatorTetheringManager::CreateFromConnectionProfile(profile);
    if (tetheringManager.TetheringOperationalState() == desiredState) {
        if (desiredState == TetheringOperationalState::On) {
            std::cout << "Hotspot is on" << std::endl;
        } else {
            std::cout << "Hotspot is off" << std::endl;
        }
        return;
    }

    NetworkOperatorTetheringOperationResult result{nullptr};
    if (desiredState == TetheringOperationalState::On) {
        std::cout << "Turning on the hotspot..." << std::endl;
        result = tetheringManager.StartTetheringAsync().get();
    } else {
        std::cout << "Turning off the hotspot..." << std::endl;
        result = tetheringManager.StopTetheringAsync().get();
    }
    std::cout << "Status: " << tetheringOperationStatusToString(result.Status()) << std::endl;
}

TetheringOperationalState getDesiredTetheringOperationalState(const ConnectionProfile& profile) {
    if (profile.IsWlanConnectionProfile()) {
        std::cout << "This is a Wi-Fi connection!" << std::endl;
        return TetheringOperationalState::Off;
    }
    if (profile.IsWwanConnectionProfile()) {
        std::cout << "This is a mobile connection!" << std::endl;
        return TetheringOperationalState::Off;
    }
    return TetheringOperationalState::On;
}

std::string_view tetheringOperationStatusToString(TetheringOperationStatus status) {
    switch (status) {
    case TetheringOperationStatus::Success:
        return "Success";
    case TetheringOperationStatus::Unknown:
        return "Unknown";
    case TetheringOperationStatus::MobileBroadbandDeviceOff:
        return "MobileBroadbandDeviceOff";
    case TetheringOperationStatus::WiFiDeviceOff:
        return "WiFiDeviceOff";
    case TetheringOperationStatus::EntitlementCheckTimeout:
        return "EntitlementCheckTimeout";
    case TetheringOperationStatus::EntitlementCheckFailure:
        return "EntitlementCheckFailure";
    case TetheringOperationStatus::OperationInProgress:
        return "OperationInProgress";
    case TetheringOperationStatus::BluetoothDeviceOff:
        return "BluetoothDeviceOff";
    case TetheringOperationStatus::NetworkLimitedConnectivity:
        return "NetworkLimitedConnectivity";
    default:
        return "Unknown";
    }
}
