#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "rc\resource.h"
#include "SettingsDialog.h"
#include "OffHookSettings.h"


int SettingsDialog::RunModal(HINSTANCE hInstance, HWND hParent)
{
	return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SETTINGS), hParent, &StaticDialogProc, (LPARAM)this);
}


INT_PTR CALLBACK SettingsDialog::StaticDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_INITDIALOG)
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
	SettingsDialog *self = (SettingsDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	if(self)
		return self->DialogProc(hwndDlg, uMsg, wParam, lParam);

	return FALSE;
}


// Message handler for our dialog
INT_PTR CALLBACK SettingsDialog::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		m_config.Init(SETTINGS_VENDORNAME, SETTINGS_APPNAME);
		ReadSettings(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ONHOOK_EXIT:
			break;

		case IDOK:
			SaveSettings(hDlg);
			// fallthrough
		case IDCLOSE:
		case IDCANCEL:
			EndDialog(hDlg, wParam);
			m_config.Exit();
			return TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

bool SettingsDialog::ReadSettings(HWND hDlg)
{
	union {
		bool b;
	} val;

	val.b = m_config.ReadBool(SETTINGS_ONHOOK_ON_EXIT, true);
	SendDlgItemMessage(hDlg, IDC_ONHOOK_EXIT, BM_SETCHECK, val.b == true ? BST_CHECKED : BST_UNCHECKED, 0);

	val.b = m_config.ReadBool(SETTINGS_MINIMIZE_TO_TRAY, true);
	SendDlgItemMessage(hDlg, IDC_MINIMIZE_TO_TRAY, BM_SETCHECK, val.b == true ? BST_CHECKED : BST_UNCHECKED, 0);

	return true;
}

bool SettingsDialog::SaveSettings(HWND hDlg)
{
	bool ret = true;
	LRESULT res;

	res = SendDlgItemMessage(hDlg, IDC_ONHOOK_EXIT, BM_GETCHECK, 0, 0);
	ret &= m_config.SaveBool(SETTINGS_ONHOOK_ON_EXIT, res == BST_CHECKED ? true : false);

	res = SendDlgItemMessage(hDlg, IDC_MINIMIZE_TO_TRAY, BM_GETCHECK, 0, 0);
	ret &= m_config.SaveBool(SETTINGS_MINIMIZE_TO_TRAY, res == BST_CHECKED ? true : false);

	return ret;
}
