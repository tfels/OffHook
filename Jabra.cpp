#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <JabraNativeHid.h>
#include "Jabra.h"
#include "OffHook.h"

#pragma comment(lib, "libjabra.lib")


Jabra* Jabra::g_instance = nullptr;


static void gFirstScanForDevicesDoneFunc(void)
{
	Jabra::instance()->cbFirstScanForDevicesDone();
}
static void gDeviceAttachedFunc(Jabra_DeviceInfo deviceInfo)
{
	Jabra::instance()->cbDeviceAttached(deviceInfo);
}
static void gDeviceRemovedFunc(unsigned short deviceID)
{
	Log("DeviceRemovedFunc called\r\n");
}
static void gButtonInDataRawHidFunc(unsigned short deviceID, unsigned short usagePage, unsigned short usage, bool buttonInData)
{
	Log("ButtonInDataRawHidFunc called\r\n");
}
static void gButtonInDataTranslatedFunc(unsigned short deviceID, Jabra_HidInput translatedInData, bool buttonInData)
{
	Log("ButtonInDataTranslatedFunc called\r\n");
}
static void gBusylightFunc(unsigned short deviceID, bool busylightValue)
{
	Jabra::instance()->cbBusylightFunc(deviceID, busylightValue);
}
static void gLogDeviceEvent(unsigned short deviceID, char* eventStr)
{
	Jabra::instance()->cbLogDeviceEvent(deviceID, eventStr);
}

// ----------------------------------------
Jabra::Jabra()
{
}

Jabra::~Jabra()
{
	Exit();
}

// ----------------------------------------
// SDK callbacks
// ----------------------------------------
void Jabra::cbDeviceAttached(Jabra_DeviceInfo deviceInfo)
{
	m_deviceInfo = deviceInfo;
	Log("DeviceAttachedFunc: using device \"%s\"\r\n", deviceInfo.deviceName);
}
void Jabra::cbFirstScanForDevicesDone()
{
	InitDevice();
}
void Jabra::cbBusylightFunc(unsigned short deviceID, bool busylightValue)
{
	if(deviceID != deviceId()) {
		Log("Got a busy light event for unknown device id (%d)\r\n", deviceID);
		return;
	}

	m_BusyLightState = busylightValue;
	SetBusyLightIcon(m_BusyLightState);
}
void Jabra::cbLogDeviceEvent(unsigned short deviceID, char* eventStr)
{
	if(deviceID != deviceId()) {
		Log("Got a log event for unknown device id (%d)\r\n", deviceID);
		return;
	}

	Log("cbLogDeviceEvent called (%s)\r\n", eventStr);
	//Json* j = new Json(eventStr)
}



// ----------------------------------------
// public funcs
// ----------------------------------------
bool Jabra::InitSdk()
{
	Exit();

	char ver[256];
	Jabra_ReturnCode ret = Jabra_GetVersion(ver, sizeof(ver));
	if (ret != Return_Ok) {
		Log("failed to determine Jabra SDK version!\r\n");
		return false;
	}
	Log("found Jabra SDK version: %s\r\n", ver);
	
	Jabra_SetAppID("qx9c9/w8FCa+KdvDwsGrCMOPqQXqeAgEQ1atKLY7BXE=");

	unsigned int instance = 1;
	Config_params configParams = { 0 };
		
	bool ok = Jabra_InitializeV2(
		(*gFirstScanForDevicesDoneFunc),
		(*gDeviceAttachedFunc),
		(*gDeviceRemovedFunc),
		(*gButtonInDataRawHidFunc),
		(*gButtonInDataTranslatedFunc),
		instance,
		&configParams);
	if (!ok) {
		Log("failed to initialize Jabra SDK!\r\n");
		return false;
	}

	return true;
}


bool Jabra::InitDevice()
{
	bool ok;
	
	// OffHook
	ok = Jabra_IsOffHookSupported(deviceId());
	Log("OffHook Supported=%d\r\n", ok);
	SetOffHookIcon(m_OffHookState);

	// Busylight
	ok = Jabra_IsBusylightSupported(deviceId());
	Log("Busylight Supported=%d\r\n", ok);

	ok = Jabra_GetBusylightStatus(deviceId());
	Log("Busylight Status=%d\r\n", ok);
	m_BusyLightState = ok;
	SetBusyLightIcon(m_BusyLightState);

	Jabra_RegisterBusylightEvent(gBusylightFunc);

	// Mute
	ok = Jabra_IsMuteSupported(deviceId());
	Log("Mute Supported=%d\r\n", ok);
	SetMuteIcon(m_MuteState);

	Jabra_RegisterDevLogCallback(gLogDeviceEvent);

	return true;
}

void Jabra::Exit()
{
	if(m_OffHookState)
		OffHook();
	Jabra_Uninitialize();
}


bool Jabra::OffHook()
{
	Jabra_ReturnCode ret = Jabra_SetOffHook(deviceId(), !m_OffHookState);
	if(ret == Return_Ok) {
		m_OffHookState = !m_OffHookState;
		cbBusylightFunc(m_deviceInfo.deviceID, m_OffHookState);
	}  else
		Log("ERROR: SetOffHook failed with ret=%d\r\n", ret);

	SetOffHookIcon(m_OffHookState);
	return m_OffHookState;
}

bool Jabra::Busy()
{
	Jabra_ReturnCode ret = Jabra_SetBusylightStatus(deviceId(), !m_BusyLightState);
	if(ret == Return_Ok)
		m_BusyLightState = !m_BusyLightState;

	SetBusyLightIcon(m_BusyLightState);
	return m_BusyLightState;
}

bool Jabra::Mute()
{
	Jabra_ReturnCode ret = Jabra_SetMute(deviceId(), !m_MuteState);
	if(ret == Return_Ok)
		m_MuteState = !m_MuteState;

	SetMuteIcon(m_MuteState);
	return m_MuteState;
}
