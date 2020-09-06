#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "rc\resource.h"
#include "SettingsDialog.h"


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
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ONHOOK_EXIT:
			break;

		case IDOK:
			// ToDo: save settings
			// fallthrough
		case IDCLOSE:
		case IDCANCEL:
			EndDialog(hDlg, wParam);
			return TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
