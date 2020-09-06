#include <comdef.h>
#include "WmiEventSink.h"
#include "MainDialog.h"

#define Log MainDialog::instance()->Log


ULONG WmiEventSink::AddRef()
{
	return InterlockedIncrement(&m_lRef);
}

ULONG WmiEventSink::Release()
{
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT WmiEventSink::QueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
	{
		*ppv = (IWbemObjectSink*)this;
		AddRef();
		return WBEM_S_NO_ERROR;
	} else
		return E_NOINTERFACE;
}


HRESULT WmiEventSink::Indicate(long lObjectCount, IWbemClassObject** apObjArray)
{
	HRESULT hres = S_OK;

	for (int i = 0; i < lObjectCount; i++)
	{
		Log("Event occurred ... ");

		if(apObjArray[i]->InheritsFrom(L"__InstanceCreationEvent") == WBEM_S_NO_ERROR)
			Log("create\r\n");
		else if(apObjArray[i]->InheritsFrom(L"__InstanceDeletionEvent") == WBEM_S_NO_ERROR)
			Log("delete\r\n");
		else {
			Log("unknown\r\n");
			logClassName(apObjArray[i]);
		}

		// get Win32_Process object
		VARIANT proc;
		hres = apObjArray[i]->Get(_bstr_t(L"TargetInstance"), 0, &proc, 0, 0);
		if(FAILED(hres)){
			Log("Failed to get the TargetInstance (Win32_Process object) from the query. Error code = 0x%x\r\n", hres);
			return WBEM_E_FAILED;
		}
		IUnknown* unkown = V_UNKNOWN(&proc);
		IWbemClassObject* processObject;
		hres = unkown->QueryInterface(IID_IWbemClassObject, (void**)&processObject);
		if(FAILED(hres)) {
			Log("Failed to get the Win32_Process interface. Error code = 0x%x\r\n", hres);
			return WBEM_E_FAILED;
		}

		// get name
		std::string name = getStringProperty(processObject, "Name");
		Log("Name: %s\r\n", name.c_str());
		if(name.empty())
			return WBEM_E_FAILED;

		name = getStringProperty(processObject, "ExecutablePath");
		Log("ExecutablePath: %s\r\n", name.c_str());
		if(name.empty())
			return WBEM_E_FAILED;
	}

	return WBEM_S_NO_ERROR;
}


HRESULT WmiEventSink::SetStatus(
	/* [in] */ LONG lFlags,
	/* [in] */ HRESULT hResult,
	/* [in] */ BSTR strParam,
	/* [in] */ IWbemClassObject __RPC_FAR* pObjParam
)
{
	// we have to process the event notification within 100 MSEC!
	if (lFlags == WBEM_STATUS_COMPLETE)
	{
		// no SendMessage here!!
		//Log("Call complete. hResult = 0x%X\r\n", hResult);
		if (hResult == WBEM_E_CALL_CANCELLED)
			OutputDebugString("WmiEventSink::SetStatus: query canceled.\n");
		else
			OutputDebugString("WmiEventSink::SetStatus: Call complete.\n");
	} else if (lFlags == WBEM_STATUS_PROGRESS)
	{
		//Log("Call in progress.\r\n");
		OutputDebugString("WmiEventSink::SetStatus: Call in progess.\n");
	}
	return WBEM_S_NO_ERROR;
}


void WmiEventSink::logClassName(IWbemClassObject* wmiObject)
{
	// get class name
	VARIANT v;
	HRESULT hr;
	hr = wmiObject->Get(L"__CLASS", 0, &v, 0, 0);
	if(SUCCEEDED(hr) && (V_VT(&v) == VT_BSTR))
		Log("The class name is '%ls'\r\n", V_BSTR(&v));
	else
		Log("Error in getting specified object\r\n");
	VariantClear(&v);
}


std::string WmiEventSink::getStringProperty(IWbemClassObject* wmiObject, const char* propertyName)
{
	// get class name
	std::string ret;
	VARIANT v;
	HRESULT hr;
	hr = wmiObject->Get(_bstr_t(propertyName), 0, &v, 0, 0);
	if(SUCCEEDED(hr) && (V_VT(&v) == VT_BSTR)) {
		_bstr_t name;
		name.Attach(V_BSTR(&v));
		ret = (const char*)name; // calls internal GetString()
		name.Detach();
	} else
		Log("Error while getting the property of an WMI object. Error code = 0x%x\r\n", hr);
	VariantClear(&v);
	return ret;
}
