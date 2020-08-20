#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <objbase.h>
#include <wbemidl.h>
#include <comdef.h>
#include "ProcessWatcher.h"
#include "MainDialog.h"

#pragma comment(lib, "wbemuuid.lib")

ProcessWatcher* Singleton<ProcessWatcher>::g_instance = nullptr;

#define Log MainDialog::instance()->Log

bool ProcessWatcher::Init()
{
	HRESULT hres;

	// Initialize COM. ------------------------------------------
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if(FAILED(hres))
	{
		Log("Failed to initialize COM library. Error code = 0x%x\r\n", hres);
		return false;
	}

	// Set general COM security levels --------------------------
	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM negotiates service
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
	);

	if(FAILED(hres))
	{
		Log("Failed to initialize security. Error code = 0x%x\r\n", hres);
		CoUninitialize();
		return false;
	}

	// Obtain the initial locator to WMI -------------------------
	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);

	if(FAILED(hres))
	{
		Log("Failed to create IWbemLocator object. Err code = 0x%x\r\n", hres);
		CoUninitialize();
		return false;
	}

	// Connect to WMI through the IWbemLocator::ConnectServer method

	// Connect to the local root\cimv2 namespace
	// and obtain pointer pSvc to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),
		NULL,
		NULL,
		0,
		NULL,
		0,
		0,
		&pSvc
	);

	if(FAILED(hres))
	{
		Log("Could not connect. Error code = 0x%x\r\n", hres);
		Exit();
		return false;
	}

	Log("Connected to ROOT\\CIMV2 WMI namespace\r\n");

	// Set security levels on the proxy -------------------------
	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx 
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx 
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);

	if(FAILED(hres))
	{
		Log("Could not set proxy blanket. Error code = 0x%x\r\n", hres);
		Exit();
		return false;
	}

	// Receive event notifications -----------------------------

	// Use an unsecured apartment for security
	hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&pUnsecApp);

	pSink = new WmiEventSink();
	pSink->AddRef();

	pUnsecApp->CreateObjectStub(pSink, &pStubUnk);

	pStubUnk->QueryInterface(IID_IWbemObjectSink, (void**)&pStubSink);

	// The ExecNotificationQueryAsync method will call
	// the WmiEventQuery::Indicate method when an event occurs
	hres = pSvc->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t("SELECT * "
			"FROM __InstanceCreationEvent WITHIN 1 "
			"WHERE TargetInstance ISA 'Win32_Process'"
			//" AND TargetInstance.Name = 'notepad.exe'"
		),
		//_bstr_t("SELECT * FROM Win32_ProcessTrace WITHIN 1 "),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		pStubSink);

	if (FAILED(hres))
	{
		Log("ExecNotificationQueryAsync failed with = 0x%X\n", hres);
		Exit();
		return false;
	}

	m_running = true;

	return true;
}

void ProcessWatcher::Exit()
{

	if(m_running)
		HRESULT hres = pSvc->CancelAsyncCall(pStubSink);
	m_running = false;

	if(pSvc) {
		pSvc->Release();
		pSvc = nullptr;
	}
	if(pLoc) {
		pLoc->Release();
		pLoc = nullptr;
	}
	if(pUnsecApp) {
		pUnsecApp->Release();
		pUnsecApp = nullptr;
	}
	if(pStubUnk) {
		pStubUnk->Release();
		pStubUnk = nullptr;
	}
	if(pSink) {
		pSink->Release();
		pSink = nullptr;
	}
	if(pStubSink) {
		pStubSink->Release();
		pStubSink = nullptr;
	}

	CoUninitialize();
}

