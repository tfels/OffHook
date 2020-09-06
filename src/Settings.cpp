#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <memory>
#include "Settings.h"

// ----------------------------------------
Settings::Settings()
{
}

Settings::~Settings()
{
	Exit();
}

// ----------------------------------------
// public funcs
// ----------------------------------------
bool Settings::Init(const char *vendorName, const char* appName)
{
	std::string regPath = "Software\\";
	if(vendorName) {
		regPath += vendorName;
		regPath += '\\';
	}
	if(!appName)
		return false;
	regPath += appName;

	LSTATUS err = RegCreateKeyEx(HKEY_CURRENT_USER, regPath.c_str(),
		0, NULL,
		0, KEY_READ | KEY_SET_VALUE | KEY_WOW64_64KEY, NULL,
		&m_hKey, NULL);
	return (err == ERROR_SUCCESS);
}

void Settings::Exit()
{
	if(m_hKey == nullptr)
		return;
	RegCloseKey(m_hKey);
	m_hKey = nullptr;
}


bool Settings::SaveInt(const char* valueName, DWORD value) const
{
	if(m_hKey == nullptr)
		return false;
	LSTATUS err = RegSetValueEx(m_hKey, valueName, 0, REG_DWORD, reinterpret_cast<const BYTE *>(&value), sizeof(value));
	return (err == ERROR_SUCCESS);
}

bool Settings::SaveString(const char* valueName, const char* value) const
{
	if(m_hKey == nullptr)
		return false;
	LSTATUS err = RegSetValueEx(m_hKey, valueName, 0, REG_SZ, reinterpret_cast<const BYTE *>(value), strlen(value)+1);
	return (err == ERROR_SUCCESS);
}


DWORD Settings::ReadInt(const char* valueName, DWORD defaultValue) const
{
	if(m_hKey == nullptr)
		return defaultValue;

	DWORD ret = 0;
	DWORD retSize = sizeof(ret);
	LSTATUS err = RegGetValue(m_hKey, NULL, valueName,
		RRF_RT_REG_DWORD, NULL,
		&ret, &retSize);

	return (err == ERROR_SUCCESS) ? ret : defaultValue;
}

bool Settings::ReadBool(const char* valueName, bool defaultValue) const
{
	DWORD ret = ReadInt(valueName, 42);
	if(ret == 1)
		return true;
	else if(ret == 0)
		return false;
	else
		return defaultValue;
};

std::string Settings::ReadString(const char* valueName, const char *defaultValue) const
{
	if(m_hKey == nullptr)
		return defaultValue;

	// determine size
	DWORD valueSize = 0;
	LSTATUS err = RegGetValue(m_hKey, NULL, valueName,
		RRF_RT_REG_SZ, NULL,
		nullptr, &valueSize);
	if(err != ERROR_SUCCESS)
		return defaultValue;

	auto buf = std::make_unique<char[]>(valueSize);

	err = RegGetValue(m_hKey, NULL, valueName,
		RRF_RT_REG_SZ, NULL,
		buf.get(), &valueSize);

	return (err == ERROR_SUCCESS) ? std::string(buf.get()) : defaultValue;
}