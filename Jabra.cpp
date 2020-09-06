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
	Log("DeviceRemovedFunc called\n");
}
static void gButtonInDataRawHidFunc(unsigned short deviceID, unsigned short usagePage, unsigned short usage, bool buttonInData)
{
	Log("ButtonInDataRawHidFunc called\n");
}
static void gButtonInDataTranslatedFunc(unsigned short deviceID, Jabra_HidInput translatedInData, bool buttonInData)
{
	Log("ButtonInDataTranslatedFunc called\n");
}
static void gBusylightFunc(unsigned short deviceID, bool busylightValue)
{
	Jabra::instance()->cbBusylightFunc(deviceID, busylightValue);
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
	Log("DeviceAttachedFunc: using device \"%s\"\n", deviceInfo.deviceName);
}
void Jabra::cbFirstScanForDevicesDone()
{
	InitDevice();
}
void Jabra::cbBusylightFunc(unsigned short deviceID, bool busylightValue)
{
	if(deviceID != deviceId()) {
		Log("Got a busy light event for unknown device id (%d)\n", deviceID);
		return;
	}

	m_BusyLightState = busylightValue;
	SetBusyLightIcon(m_BusyLightState);
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
		Log("failed to determine Jabra SDK version!\n");
		return false;
	}
	Log("found Jabra SDK version: %s\n", ver);
	
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
		Log("failed to initialize Jabra SDK!\n");
		return false;
	}

	return true;
}


bool Jabra::InitDevice()
{
	bool ok;
	
	ok = Jabra_IsOffHookSupported(deviceId());
	Log("OffHook Supported=%d\n", ok);
	SetOffHookIcon(m_OffHookState);

	ok = Jabra_IsBusylightSupported(deviceId());
	Log("Busylight Supported=%d\n", ok);

	ok = Jabra_GetBusylightStatus(deviceId());
	Log("Busylight Status=%d\n", ok);
	m_BusyLightState = ok;
	SetBusyLightIcon(m_BusyLightState);

	Jabra_RegisterBusylightEvent(gBusylightFunc);

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
		Log("ERROR: SetOffHook failed with ret=%d\n", ret);

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
