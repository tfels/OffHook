#pragma once

#define SETTINGS_VENDORNAME "Tim Felser"
#define SETTINGS_APPNAME    "OffHook"

#define SETTINGS_ONHOOK_ON_EXIT        "OnHookOnExit"  // default: true
#define SETTINGS_MINIMIZE_TO_TRAY      "TrayIcon"      // default: true
#define SETTINGS_AUTO_OFFHOOK          "AutoOffHook"   // default: true
#define SETTINGS_AUTO_OFFHOOK_PROCESS  "AutoOffHookProcess"
#define SETTINGS_DEFAULT_AUTO_OFFHOOK_PROCESS  "g2mvideoconference.exe"

struct OffHookSettings {
	bool OnHookOnExit;
	bool MinimizeToTray;
	bool AutoOffHook;
	std::string AutoOffHookProcess;
};
