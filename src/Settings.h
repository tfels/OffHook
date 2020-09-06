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

	bool SaveInt(const char* valueName, DWORD value);
	bool SaveString(const char* valueName, const char* value);

	DWORD ReadInt(const char* valueName);			// returns 0 on error
	std::string ReadString(const char* valueName);	// returns "" on error

private:
	HKEY m_hKey = nullptr;
};
