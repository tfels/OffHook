#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <strsafe.h>
#include <shellapi.h>
#include "rc\resource.h"
#include "OffHook.h"
#include "MainDialog.h"
#include "SettingsDialog.h"
#include "Jabra.h"
#include "ProcessWatcher.h"

MainDialog* Singleton<MainDialog>::g_instance = nullptr;

// some defines
#define ID_TRAY_ICON 1
#define WM_TRAY_ICON (WM_APP+1)


// Message handler for our dialog
INT_PTR CALLBACK MainDialog::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	DWORD ret;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		m_hWnd = hDlg;
		{
		HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP));
		SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		DestroyIcon(hIcon);
		}
		readSettings();
		if(m_settings.TrayIcon)
			addTrayIcon();

		ret = SetTimer(hDlg, IDC_INIT, 100, nullptr);
		return (INT_PTR)TRUE;

	case WM_CLOSE:
		removeTrayIcon();
		ProcessWatcher::instance()->Exit();
		Jabra::instance()->Exit(m_settings.OnHookOnExit);
		DestroyWindow(hDlg);
		return (INT_PTR)TRUE;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;

	case WM_TIMER:
		switch (wParam)
		{
		case IDC_INIT:
			KillTimer(hDlg, IDC_INIT);
			PostMessage(hDlg, WM_COMMAND, IDC_INIT, 0);
			break;
		}
		break;

	case WM_SIZE:
		if(m_settings.MinimizeToTray) {
			if(wParam == SIZE_MINIMIZED) {
				if(!m_settings.TrayIcon) // tray icon not present --> add
					addTrayIcon();
				m_prevGwlExStyle = SetWindowLong(hDlg, GWL_EXSTYLE, WS_EX_NOACTIVATE);
			} else if(wParam == SIZE_RESTORED) {
				SetWindowLong(hDlg, GWL_EXSTYLE, m_prevGwlExStyle);
				if(!m_settings.TrayIcon)
					removeTrayIcon();
			}
		}
		break;

	case WM_TRAY_ICON:
		if(wParam == ID_TRAY_ICON) {
			switch(lParam)
			{
			case WM_LBUTTONDBLCLK:
				ShowWindow(hDlg, SW_RESTORE);
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_INIT:
			Jabra::instance()->InitSdk();
			ProcessWatcher::instance()->Init();
			configureProcessWatcher();
			break;
		case IDC_BUSYLIGHT:
			Jabra::instance()->Busy();
			break;
		case IDC_OFFHOOK:
			Jabra::instance()->OffHook();
			break;
		case IDC_MUTE:
			Jabra::instance()->Mute();
			break;
		case IDC_SETTINGS:
		{
			SettingsDialog dlg;
			if(dlg.RunModal(g_hInstance, hDlg) == IDOK) {
				readSettings();
				// apply settings
				if(m_settings.TrayIcon)
					addTrayIcon();
				else
					removeTrayIcon();

				configureProcessWatcher();
			}
		}
		break;

		case IDCLOSE:
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


//----------------------------------------------------------------------
// status icon handling
//----------------------------------------------------------------------
void MainDialog::_setButtonImage(int controlId, int imageId)
{
	HANDLE handle = LoadImage(g_hInstance, MAKEINTRESOURCE(imageId), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
	HANDLE hOld = (HANDLE)SendDlgItemMessage(g_hMainDlg, controlId, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)handle);
	if(hOld && hOld != handle)
		DeleteObject(hOld);

	BITMAP bitmapInfo;
	GetObject(handle, sizeof(BITMAP), &bitmapInfo);
	SetWindowPos(GetDlgItem(g_hMainDlg, controlId), 0, 0, 0, bitmapInfo.bmWidth, bitmapInfo.bmHeight, SWP_NOMOVE | SWP_NOZORDER);
}

void MainDialog::SetOffHookIcon(bool state)
{
	_setButtonImage(IDC_OFFHOOK, state ? IDB_OFFHOOK : IDB_ONHOOK);
}

void MainDialog::SetBusyLightIcon(bool state)
{
	_setButtonImage(IDC_BUSYLIGHT, state ? IDB_BUSY_ON : IDB_BUSY_OFF);
}

void MainDialog::SetMuteIcon(bool state)
{
	_setButtonImage(IDC_MUTE, state ? IDB_MIC_OFF : IDB_MIC_ON);
}

//----------------------------------------------------------------------
// tray icon handling
//----------------------------------------------------------------------
void MainDialog::addTrayIcon()
{
	ZeroMemory(&m_niData, sizeof(m_niData));
	m_niData.cbSize = sizeof(NOTIFYICONDATA);
	m_niData.hWnd = m_hWnd;
	m_niData.uID = ID_TRAY_ICON;
	m_niData.uFlags = NIF_ICON | NIF_MESSAGE;
	m_niData.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP));
	m_niData.uCallbackMessage = WM_TRAY_ICON;
	BOOL ret = Shell_NotifyIcon(NIM_ADD, &m_niData);
}
void MainDialog::removeTrayIcon()
{
	BOOL ret = Shell_NotifyIcon(NIM_DELETE, &m_niData);
}

void MainDialog::showBallontip(const char *text)
{
	strcpy_s(m_niData.szInfoTitle, "OffHook");
	strcpy_s(m_niData.szInfo, text);
	m_niData.uTimeout = 5 * 1000;
	m_niData.dwInfoFlags = NIIF_INFO;
	m_niData.uFlags |= NIF_INFO;
	BOOL ret = Shell_NotifyIcon(NIM_MODIFY, &m_niData);
}

//----------------------------------------------------------------------
// Process Watcher
//----------------------------------------------------------------------
void MainDialog::configureProcessWatcher()
{
	if(!m_settings.AutoOffHook) {
		ProcessWatcher::instance()->StopNotify();
		showBallontip("");
		return;
	}

	if(m_settings.AutoOffHookProcess.empty() || m_settings.AutoOffHookProcess == "*")
		ProcessWatcher::instance()->NotifyStartOfProcess(nullptr, this);
	else
		ProcessWatcher::instance()->NotifyStartOfProcess(m_settings.AutoOffHookProcess.c_str(), this);

	if(!m_settings.AutoOffHookBalloon)
		showBallontip("");
}

void MainDialog::ProcessStarted(std::string name)
{
	if(m_settings.AutoOffHookBalloon)
		showBallontip("Setting off hook mode due to process start");
	Jabra::instance()->OffHook(true);
}

void MainDialog::ProcessStopped(std::string name)
{
	if(m_settings.AutoOffHookBalloon)
		showBallontip("Resetting to on hook due to process stop");
	Jabra::instance()->OffHook(false);
}


//----------------------------------------------------------------------
// generic functions
//----------------------------------------------------------------------
void MainDialog::readSettings()
{
	Settings m_config;
	m_config.Init(SETTINGS_VENDORNAME, SETTINGS_APPNAME);

	m_settings.OnHookOnExit         = m_config.ReadBool(SETTINGS_ONHOOK_ON_EXIT, true);
	m_settings.TrayIcon             = m_config.ReadBool(SETTINGS_TRAY_ICON, true);
	m_settings.MinimizeToTray       = m_config.ReadBool(SETTINGS_MINIMIZE_TO_TRAY, true);
	m_settings.AutoOffHook          = m_config.ReadBool(SETTINGS_AUTO_OFFHOOK, true);
	m_settings.AutoOffHookProcess   = m_config.ReadString(SETTINGS_AUTO_OFFHOOK_PROCESS, SETTINGS_DEFAULT_AUTO_OFFHOOK_PROCESS);
	m_settings.AutoOffHookBalloon   = m_config.ReadBool(SETTINGS_AUTO_OFFHOOK_BALLOON, true);
	m_settings.AutoOffHookRestoreUi = m_config.ReadBool(SETTINGS_AUTO_OFFHOOK_RESTORE_UI, true);

	m_config.Exit();
}

//----------------------------------------------------------------------
void MainDialog::Log(const TCHAR* aFormat, ...)
{
	TCHAR buf[256];
	va_list	stArg;
	va_start(stArg, aFormat);

	vsnprintf(buf, sizeof(buf), aFormat, stArg);
	//StringCchVPrintf(buf, sizeof(buf), aFormat, stArg);

	va_end(stArg);
	buf[sizeof(buf) - 1] = '\0';

	HWND hLogCtrl = GetDlgItem(g_hMainDlg, IDC_LOG);
	SendMessage(hLogCtrl, EM_SETSEL, -1, -1);
	SendMessage(hLogCtrl, EM_REPLACESEL, FALSE, (LPARAM)buf);
	SendMessage(hLogCtrl, WM_VSCROLL, SB_BOTTOM, (LPARAM)NULL);
}
