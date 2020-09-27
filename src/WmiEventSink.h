#pragma once
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <objbase.h>
#include <wbemidl.h>
#include <string>

class ProcessWatcher_NotifyInterface; 

class WmiEventSink : public IWbemObjectSink
{
	LONG m_lRef = 0;
	bool bDone  = false;

public:
	WmiEventSink(ProcessWatcher_NotifyInterface* notifyCallback) :
		m_notifyCallback(notifyCallback)
	{ m_lRef = 0; }
	~WmiEventSink() { bDone = true; }

	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	virtual HRESULT
		STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

	virtual HRESULT STDMETHODCALLTYPE Indicate(
		LONG lObjectCount,
		IWbemClassObject __RPC_FAR* __RPC_FAR* apObjArray
	);

	virtual HRESULT STDMETHODCALLTYPE SetStatus(
		/* [in] */ LONG lFlags,
		/* [in] */ HRESULT hResult,
		/* [in] */ BSTR strParam,
		/* [in] */ IWbemClassObject __RPC_FAR* pObjParam
	);

private:
	void logClassName(IWbemClassObject* wmiObject);
	std::string getStringProperty(IWbemClassObject* wmiObject, const char* propertyName);

private:
	ProcessWatcher_NotifyInterface* m_notifyCallback = nullptr;
};
