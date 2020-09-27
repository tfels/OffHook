#pragma once

#define SETTINGS_VENDORNAME "Tim Felser"
#define SETTINGS_APPNAME    "OffHook"

#define SETTINGS_ONHOOK_ON_EXIT        "OnHookOnExit"     // default: true
#define SETTINGS_TRAY_ICON             "TrayIcon"         // default: true
#define SETTINGS_MINIMIZE_TO_TRAY      "MinimizeToTray"   // default: true
#define SETTINGS_AUTO_OFFHOOK          "AutoOffHook"      // default: true
#define SETTINGS_AUTO_OFFHOOK_PROCESS  "AutoOffHookProcess"
#define SETTINGS_DEFAULT_AUTO_OFFHOOK_PROCESS  "g2mvideoconference.exe"
#define SETTINGS_AUTO_OFFHOOK_BALLOON  "AutoOffHookBalloonTip"   // default: true

struct OffHookSettings {
	bool OnHookOnExit       = true;
	bool TrayIcon           = true;
	bool MinimizeToTray     = true;
	bool AutoOffHook        = true;
	std::string AutoOffHookProcess = "";
	bool AutoOffHookBalloon = true;
};
