#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>

#include <sstream>
#include <thread>

#include <Common.h>
#include <JabraNativeHid.h>

static const unsigned short g_myDeviceId = 1;
static void triggerLogDeviceEvent();

//*************************************************************
//* stuff fromm Common.h
//*************************************************************
// general setup 
Jabra_ReturnCode Jabra_GetVersion(char* const version, int count)
{
	snprintf(version, count, "JabraSDKEmulation");
	return Return_Ok;
}

void Jabra_SetAppID(const char*) {}

bool Jabra_InitializeV2(void(*FirstScanForDevicesDoneFunc)(void),
	void(*DeviceAttachedFunc)(Jabra_DeviceInfo deviceInfo),
	void(*DeviceRemovedFunc)(unsigned short deviceID),
	void(*ButtonInDataRawHidFunc)(unsigned short deviceID, unsigned short usagePage, unsigned short usage, bool buttonInData),
	void(*ButtonInDataTranslatedFunc)(unsigned short deviceID, Jabra_HidInput translatedInData, bool buttonInData),
	bool nonJabraDeviceDectection,
	Config_params* configParams
)
{
	if(DeviceAttachedFunc) {
		Jabra_DeviceInfo info = { 0 };
		info.deviceID = g_myDeviceId;
		info.deviceName = (char*)"No SDK device";
		DeviceAttachedFunc(info);
	}
	if(FirstScanForDevicesDoneFunc)
		FirstScanForDevicesDoneFunc();
	return true;
}


bool Jabra_Uninitialize(void) { return true; }


// busy light
static bool g_busyLight = false;
static void(*g_BusylightEventCallback)(unsigned short deviceID, bool busylightValue) = nullptr;
bool Jabra_IsBusylightSupported(unsigned short) { return true; }
bool Jabra_GetBusylightStatus(unsigned short) { return g_busyLight; };
Jabra_ReturnCode Jabra_SetBusylightStatus(unsigned short, bool value)
{
	g_busyLight = value;
	return Return_Ok;
};
void Jabra_RegisterBusylightEvent(void(*BusylightFunc)(unsigned short deviceID, bool busylightValue))
{ g_BusylightEventCallback = BusylightFunc; }


// device log
static void(*g_LogDeviceEventCallback)(unsigned short deviceID, char* eventStr) = nullptr;
Jabra_ReturnCode Jabra_EnableDevLog(unsigned short, bool) { return Return_Ok; };
bool Jabra_IsDevLogEnabled(unsigned short) { return true; }
void Jabra_RegisterDevLogCallback(void(*LogDeviceEvent)(unsigned short deviceID, char* eventStr))
{ g_LogDeviceEventCallback = LogDeviceEvent; }


//*************************************************************
//* stuff fromm JabraNativeHid.h
//*************************************************************

// offhook
static bool g_offHook = false;
bool Jabra_IsOffHookSupported(unsigned short) { return true; }
Jabra_ReturnCode Jabra_SetOffHook(unsigned short, bool offHook)
{
	g_offHook = offHook;
	if(!g_offHook)
		Jabra_SetMute(g_myDeviceId, false);
	return Return_Ok;
}

// mute
static bool g_mute = false;
bool Jabra_IsMuteSupported(unsigned short) { return true; }
Jabra_ReturnCode Jabra_SetMute(unsigned short, bool mute)
{ 
	g_mute = mute;
	triggerLogDeviceEvent();

	return Return_Ok;
}


//*************************************************************
//* internal functions
//*************************************************************
// device log event (threaded)
static void delayedDeviceEvent();
static void logDeviceEvent();

static void triggerLogDeviceEvent()
{
	std::thread eventTread(delayedDeviceEvent);
	eventTread.detach();
}

static void delayedDeviceEvent()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	logDeviceEvent();
}

static void logDeviceEvent()
{
	if(!g_LogDeviceEventCallback)
		return;

	std::ostringstream eventStr;
	eventStr << "{" << std::endl;
	eventStr << "   \"Mute State\" : \"" << (g_mute ? "TRUE" : "FALSE") << "\"" << std::endl;
	eventStr << "}" << std::endl;
	g_LogDeviceEventCallback(g_myDeviceId, (char*)eventStr.str().c_str());
}
