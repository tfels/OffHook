#pragma once

#include <Common.h>
#include "Singleton.h"

class Jabra :public Singleton<Jabra>
{
	friend class Singleton<Jabra>;
public:
	bool InitSdk();
	bool InitDevice();
	void Exit(bool onHookOnExit = true);
	bool OffHook();
	bool Busy();
	bool Mute();

public: // internal callbacks
	void cbDeviceAttached(Jabra_DeviceInfo deviceInfo);
	void cbFirstScanForDevicesDone();
	void cbBusylightFunc(unsigned short deviceID, bool busylightValue);
	void cbLogDeviceEvent(unsigned short deviceID, char* eventStr);

private: // constructors
	Jabra();
	~Jabra();

private:
	unsigned short deviceId() const { return m_deviceInfo.deviceID; }

private:
	Jabra_DeviceInfo m_deviceInfo;
	bool m_OffHookState = false;
	bool m_BusyLightState = false;
	bool m_MuteState = false;
};
