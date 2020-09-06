#pragma once

#include <Common.h>

class Jabra
{
public:
	static Jabra* instance()
	{
		static CGuard g;
		if (!g_instance)
			g_instance = new Jabra();
		return g_instance;
	}

	bool InitSdk();
	bool InitDevice();
	bool Busy();
	bool OffHook();

public: // internal callbacks
	void cbDeviceAttached(Jabra_DeviceInfo deviceInfo);
	void cbFirstScanForDevicesDone();

private: // constructors
	Jabra();
	Jabra(const Jabra&);
	~Jabra();
private:
	unsigned short deviceID() const { return m_deviceInfo.deviceID; }

private:
	static Jabra* g_instance;
	Jabra_DeviceInfo m_deviceInfo;
	bool m_OffHookState = false;
	bool m_BusyLightState = false;

private:
	class CGuard
	{
	public:
		~CGuard()
		{
			if (NULL != Jabra::g_instance)
			{
				delete Jabra::g_instance;
				Jabra::g_instance = nullptr;
			}
		}
	};
};
