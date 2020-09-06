#pragma once

#define SETTINGS_VENDORNAME "Tim Felser"
#define SETTINGS_APPNAME    "OffHook"

#define SETTINGS_ONHOOK_ON_EXIT    "OnHookOnExit"
#define SETTINGS_MINIMIZE_TO_TRAY  "TrayIcon"

struct OffHookSettings {
	bool OnHookOnExit;
	bool MinimizeToTray;
};
