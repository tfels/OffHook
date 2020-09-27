#pragma once

#include <shellapi.h>

#include "Singleton.h"
#include "Settings.h"
#include "OffHookSettings.h"
#include "ProcessWatcher.h"

class MainDialog :public Singleton<MainDialog>, public ProcessWatcher_NotifyInterface
{
	friend class Singleton<MainDialog>;

public:
	INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void SetOffHookIcon(bool state);
	void SetBusyLightIcon(bool state);
	void SetMuteIcon(bool state);

	void Log(const char* aFormat, ...);

public: // implement ProcessWatcher_NotifyInterface
	virtual void ProcessStarted(std::string name);
	virtual void ProcessStopped(std::string name);

private: // constructors
	MainDialog() : m_niData({ 0 }) {};
	~MainDialog() {};

private:
	void _setButtonImage(int controlId, int imageId);
	void addTrayIcon();
	void removeTrayIcon();
	void showBallontip(const char* text);
	void configureProcessWatcher();
	void readSettings();

private:
	NOTIFYICONDATA m_niData;
	struct OffHookSettings m_settings;
	HWND m_hWnd;
};
