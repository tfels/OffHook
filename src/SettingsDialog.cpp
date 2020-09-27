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
		case IDC_AUTO_OFFHOOK:
			if(HIWORD(wParam) == BN_CLICKED) {
				LRESULT res = SendDlgItemMessage(hDlg, IDC_AUTO_OFFHOOK, BM_GETCHECK, 0, 0);
				EnableWindow(GetDlgItem(hDlg, IDC_AUTO_OFFHOOK_PROCESS), res == BST_CHECKED ? true : false);
				EnableWindow(GetDlgItem(hDlg, IDC_AUTO_OFFHOOK_BALLOON), res == BST_CHECKED ? true : false);
			}
			break;

		case IDC_ONHOOK_EXIT:
			break;

		case ID_CLEANUP_REGISTRY:
			if(IDYES != MessageBox(hDlg, "All settings will be deleted in the registry!\r\nContinue?", "OffHook - Clear Settings", MB_YESNO | MB_ICONWARNING))
				break;
			m_config.DeleteKey();
			GetSetAutoRunRegistry(false);
			SendMessage(hDlg, WM_CLOSE, 0, 0);
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
	bool boolVal;
	std::string strVal;

	boolVal = m_config.ReadBool(SETTINGS_ONHOOK_ON_EXIT, true);
	SendDlgItemMessage(hDlg, IDC_ONHOOK_EXIT, BM_SETCHECK, boolVal == true ? BST_CHECKED : BST_UNCHECKED, 0);

	boolVal = m_config.ReadBool(SETTINGS_TRAY_ICON, true);
	SendDlgItemMessage(hDlg, IDC_TRAY_ICON, BM_SETCHECK, boolVal == true ? BST_CHECKED : BST_UNCHECKED, 0);

	boolVal = m_config.ReadBool(SETTINGS_MINIMIZE_TO_TRAY, true);
	SendDlgItemMessage(hDlg, IDC_MINIMIZE_TO_TRAY, BM_SETCHECK, boolVal == true ? BST_CHECKED : BST_UNCHECKED, 0);

	// Auto OffHook Settings
	boolVal = m_config.ReadBool(SETTINGS_AUTO_OFFHOOK, true);
	SendDlgItemMessage(hDlg, IDC_AUTO_OFFHOOK, BM_SETCHECK, boolVal == true ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_AUTO_OFFHOOK, BN_CLICKED), 0);

	strVal = m_config.ReadString(SETTINGS_AUTO_OFFHOOK_PROCESS, SETTINGS_DEFAULT_AUTO_OFFHOOK_PROCESS);
	SetWindowText(GetDlgItem(hDlg, IDC_AUTO_OFFHOOK_PROCESS), strVal.c_str());

	boolVal = m_config.ReadBool(SETTINGS_AUTO_OFFHOOK_BALLOON, true);
	SendDlgItemMessage(hDlg, IDC_AUTO_OFFHOOK_BALLOON, BM_SETCHECK, boolVal == true ? BST_CHECKED : BST_UNCHECKED, 0);

	boolVal = GetSetAutoRunRegistry(std::nullopt);
	SendDlgItemMessage(hDlg, IDC_AUTORUN, BM_SETCHECK, boolVal == true ? BST_CHECKED : BST_UNCHECKED, 0);

	return true;
}

bool SettingsDialog::SaveSettings(HWND hDlg)
{
	bool ret = true;
	int textLen;
	std::string text;
	LRESULT res;

	res = SendDlgItemMessage(hDlg, IDC_ONHOOK_EXIT, BM_GETCHECK, 0, 0);
	ret &= m_config.SaveBool(SETTINGS_ONHOOK_ON_EXIT, res == BST_CHECKED ? true : false);

	res = SendDlgItemMessage(hDlg, IDC_TRAY_ICON, BM_GETCHECK, 0, 0);
	ret &= m_config.SaveBool(SETTINGS_TRAY_ICON, res == BST_CHECKED ? true : false);

	res = SendDlgItemMessage(hDlg, IDC_MINIMIZE_TO_TRAY, BM_GETCHECK, 0, 0);
	ret &= m_config.SaveBool(SETTINGS_MINIMIZE_TO_TRAY, res == BST_CHECKED ? true : false);

	// Auto OffHook Settings
	res = SendDlgItemMessage(hDlg, IDC_AUTO_OFFHOOK, BM_GETCHECK, 0, 0);
	ret &= m_config.SaveBool(SETTINGS_AUTO_OFFHOOK, res == BST_CHECKED ? true : false);

	textLen = GetWindowTextLength(GetDlgItem(hDlg, IDC_AUTO_OFFHOOK_PROCESS));
	textLen++; // add space for a closing \0
	text.resize(textLen, '\0');
	GetWindowText(GetDlgItem(hDlg, IDC_AUTO_OFFHOOK_PROCESS), &text[0], textLen);
	ret &= m_config.SaveString(SETTINGS_AUTO_OFFHOOK_PROCESS, text.c_str());

	res = SendDlgItemMessage(hDlg, IDC_AUTO_OFFHOOK_BALLOON, BM_GETCHECK, 0, 0);
	ret &= m_config.SaveBool(SETTINGS_AUTO_OFFHOOK_BALLOON, res == BST_CHECKED ? true : false);

	res = SendDlgItemMessage(hDlg, IDC_AUTORUN, BM_GETCHECK, 0, 0);
	ret &= GetSetAutoRunRegistry(res == BST_CHECKED ? true : false);

	return ret;
}


bool SettingsDialog::GetSetAutoRunRegistry(std::optional<bool> setValue)
// pass std::nullopt for getting the current value,
// else the value to set
{
	HKEY hKey;

	LSTATUS err = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		0, NULL,
		0, KEY_READ | KEY_SET_VALUE, NULL,
		&hKey, NULL);
	if(err != ERROR_SUCCESS)
		return false;

	if(setValue.has_value()) {
		if(setValue.value()) { // set value
			char path[MAX_PATH];
			GetModuleFileName(NULL, path, sizeof(path));
			std::string cmdLine = "\"";
			cmdLine.append(path);
			cmdLine.append("\" -minimized");
			err = RegSetValueEx(hKey, SETTINGS_APPNAME, 0, REG_SZ, reinterpret_cast<const BYTE*>(cmdLine.c_str()), cmdLine.length() + 1);
		} else { // delete value
			err = RegDeleteValue(hKey, SETTINGS_APPNAME);
		}
	} else { // read current value
		err = RegGetValue(hKey, NULL, SETTINGS_APPNAME, RRF_RT_REG_SZ, NULL, NULL, NULL);
	}

	RegCloseKey(hKey);

	return (err == ERROR_SUCCESS);
}