#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <strsafe.h>
#include <commctrl.h>
#include "resource.h"
#include "Jabra.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HINSTANCE g_hInstance;
HWND g_hMainDlg;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR    lpCmdLine,
					  _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	g_hInstance = hInstance;

	g_hMainDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DialogProc, 0);
	ShowWindow(g_hMainDlg, nCmdShow);


	// Main message loop:
	BOOL ret;
	MSG msg;
	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1) /* error found */
			return -1;

		if (!IsDialogMessage(g_hMainDlg, &msg)) {
			TranslateMessage(&msg); /* translate virtual-key messages */
			DispatchMessage(&msg); /* send it to dialog procedure */
		}
	}

	return 0;
}


// Message handler for our dialog
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	DWORD ret;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		ret = SetTimer(hDlg, IDC_INIT, 100, nullptr);
		return (INT_PTR)TRUE;

	case WM_CLOSE:
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


void _setButtonImage(int controlId, int imageId)
{
	HANDLE handle = LoadImage(g_hInstance, MAKEINTRESOURCE(imageId), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT);
	HANDLE hOld = (HANDLE)SendDlgItemMessage(g_hMainDlg, controlId, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)handle);
	if(hOld && hOld != handle)
		DeleteObject(hOld);

	BITMAP bitmapInfo;
	GetObject(handle, sizeof(BITMAP), &bitmapInfo);
	SetWindowPos(GetDlgItem(g_hMainDlg, controlId), 0, 0, 0, bitmapInfo.bmWidth, bitmapInfo.bmHeight, SWP_NOMOVE | SWP_NOZORDER);
}

void SetOffHookIcon(bool state)
{
	_setButtonImage(IDC_OFFHOOK, state ? IDB_OFFHOOK : IDB_ONHOOK);
}

void SetBusyLightIcon(bool state)
{
	_setButtonImage(IDC_BUSYLIGHT, state ? IDB_BUSY_ON : IDB_BUSY_OFF);
}

void SetMuteIcon(bool state)
{
	_setButtonImage(IDC_MUTE, state ? IDB_MIC_OFF : IDB_MIC_ON);
}


void Log(const TCHAR* aFormat, ...)
{
	TCHAR buf[256];
	va_list	stArg;
	va_start(stArg, aFormat);

	vsnprintf(buf, sizeof(buf), aFormat, stArg);
	//StringCchVPrintf(buf, sizeof(buf), aFormat, stArg);

	va_end(stArg);
	buf[sizeof(buf) - 1] = '\0';
	
	SendDlgItemMessage(g_hMainDlg, IDC_LOG, EM_SETSEL, -1, -1);
	SendDlgItemMessage(g_hMainDlg, IDC_LOG, EM_REPLACESEL, FALSE, (LPARAM)buf);
	SendDlgItemMessage(g_hMainDlg, IDC_LOG, WM_VSCROLL, SB_BOTTOM, (LPARAM)NULL);
}
