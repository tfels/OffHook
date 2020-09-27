#pragma once

#include <optional>

#include "Settings.h"

class SettingsDialog
{
public:
	SettingsDialog() {};
	~SettingsDialog() {};

public:
	int RunModal(HINSTANCE hInstance, HWND hParent);

private:
	static INT_PTR CALLBACK StaticDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool ReadSettings(HWND hDlg);
	bool SaveSettings(HWND hDlg);
	bool GetSetAutoRunRegistry(std::optional<bool> setValue); // pass std::nullopt for getting the current value
private:
	Settings m_config;
};
