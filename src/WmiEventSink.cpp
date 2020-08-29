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
		Log("Event occurred\r\n");

		//logClassName(apObjArray[i]);

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
		VARIANT varName;
		hres = processObject->Get(_bstr_t(L"Name"), 0, &varName, 0, 0);
		if(FAILED(hres)){
			Log("Failed to get the process name from the Win32_Process object. Error code = 0x%x\r\n", hres);
			return WBEM_E_FAILED;
		}
		Log("Name: %ls\r\n", V_BSTR(&varName));
		VariantClear(&varName);

		hres = processObject->Get(_bstr_t(L"ExecutablePath"), 0, &varName, 0, 0);
		if(FAILED(hres)) {
			Log("Failed to get the executable path from the Win32_Process object. Error code = 0x%x\r\n", hres);
			return WBEM_E_FAILED;
		}
		Log("ExecutablePath: %ls\r\n", V_BSTR(&varName));
		VariantClear(&varName);
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
		Log("The class name is %ls\r\n", V_BSTR(&v));
	else
		Log("Error in getting specified object\r\n");
	VariantClear(&v);
}
