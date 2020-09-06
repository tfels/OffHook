#pragma once

#include "Singleton.h"
#include "Settings.h"
#include "OffHookSettings.h"

class MainDialog :public Singleton<MainDialog>
{
	friend class Singleton<MainDialog>;

public:
	INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


	void SetOffHookIcon(bool state);
	void SetBusyLightIcon(bool state);
	void SetMuteIcon(bool state);

	void Log(const char* aFormat, ...);

private: // constructors
	MainDialog() {};
	~MainDialog() {};

private:
	void _setButtonImage(int controlId, int imageId);
	void addTrayIcon(HWND hWnd);
	void removeTrayIcon();
	void readSettings();

private:
	struct OffHookSettings m_settings;
};
