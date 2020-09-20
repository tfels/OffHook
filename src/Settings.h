#pragma once
#include <windows.h>
#include <string>

class Settings
{
public:
	Settings();
	~Settings();

public:
	bool Init(const char* vendorName, const char* appName);	// vendorName is optional
	void Exit();

	bool DeleteKey();

	bool SaveInt   (const char* valueName, DWORD       value) const;
	bool SaveBool  (const char* valueName, bool        value) const { return SaveInt(valueName, value ? 1 : 0); };
	bool SaveString(const char* valueName, const char* value) const;

	DWORD       ReadInt   (const char* valueName, DWORD       defaultValue=0) const;
	bool        ReadBool  (const char* valueName, bool        defaultValue = false) const;
	std::string ReadString(const char* valueName, const char* defaultValue ="") const;

private:
	HKEY m_hKey = nullptr;
	std::string m_regKeyName;
};
