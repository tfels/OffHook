#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <strsafe.h>
#include <shellapi.h>
#include "rc\resource.h"
#include "OffHook.h"
#include "MainDialog.h"
#include "Jabra.h"

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
		{
		HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP));
		SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		DestroyIcon(hIcon);
		}
		m_config.Init("Tim Felser", "OffHook");

		ret = SetTimer(hDlg, IDC_INIT, 100, nullptr);
		return (INT_PTR)TRUE;

	case WM_CLOSE:
		removeTrayIcon();
		Jabra::instance()->Exit();
		m_config.Exit();
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
	{
		static LONG s_prevExState;
		if(wParam == SIZE_MINIMIZED) {
			addTrayIcon(hDlg);
			s_prevExState = SetWindowLong(hDlg, GWL_EXSTYLE, WS_EX_NOACTIVATE);
		} else if(wParam == SIZE_RESTORED) {
			SetWindowLong(hDlg, GWL_EXSTYLE, s_prevExState);
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
		switch (LOWORD(wParam))
		{
		case IDC_INIT:
			Jabra::instance()->InitSdk();
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
static NOTIFYICONDATA g_niData = {0};

void MainDialog::addTrayIcon(HWND hWnd)
{
	ZeroMemory(&g_niData, sizeof(g_niData));
	g_niData.cbSize = sizeof(NOTIFYICONDATA);
	g_niData.hWnd = hWnd;
	g_niData.uID = ID_TRAY_ICON;
	g_niData.uFlags = NIF_ICON | NIF_MESSAGE;
	g_niData.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP));
	g_niData.uCallbackMessage = WM_TRAY_ICON;
	BOOL ret = Shell_NotifyIcon(NIM_ADD, &g_niData);
}
void MainDialog::removeTrayIcon()
{
	BOOL ret = Shell_NotifyIcon(NIM_DELETE, &g_niData);
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
