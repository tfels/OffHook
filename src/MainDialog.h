#pragma once

#include "Singleton.h"

class MainDialog :public Singleton<MainDialog>
{
	friend class Singleton<MainDialog>;

public:
	INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


	void SetOffHookIcon(bool state);
	void SetBusyLightIcon(bool state);
	void SetMuteIcon(bool state);

	void Log(const char* aFormat, ...);

private:
	void _setButtonImage(int controlId, int imageId);
	void addTrayIcon(HWND hWnd);
	void removeTrayIcon();

private: // constructors
	MainDialog() {};
	~MainDialog() {};
};

