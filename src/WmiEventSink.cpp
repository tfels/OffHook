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

		// get class name
		{
			VARIANT v;
			BSTR strClassProp = SysAllocString(L"__CLASS");
			HRESULT hr;
			hr = apObjArray[i]->Get(strClassProp, 0, &v, 0, 0);
			SysFreeString(strClassProp);
			if(SUCCEEDED(hr) && (V_VT(&v) == VT_BSTR))
				Log("The class name is %ls\r\n", V_BSTR(&v));
			else
				Log("Error in getting specified object\r\n");
			VariantClear(&v);
		}
		// get Win32_Process object
		VARIANT proc;
		{
			HRESULT hres = apObjArray[i]->Get(_bstr_t(L"TargetInstance"), 0, &proc, 0, 0);

			if(FAILED(hres))
			{
				Log("Failed to get the data from the query. Error code = 0x%x\r\n", hres);
				return WBEM_E_FAILED;       // Program has failed.
			}
		}
		IUnknown* unkown = V_UNKNOWN(&proc);
		IWbemClassObject* processObject;
		HRESULT hr = unkown->QueryInterface(IID_IWbemClassObject, (void**)&processObject);

		// get name
		{
			VARIANT varName;
			HRESULT hres = processObject->Get(_bstr_t(L"Name"), 0, &varName, 0, 0);

			if(FAILED(hres))
			{
				Log("Failed to get the data from the query. Error code = 0x%x\r\n", hres);
				return WBEM_E_FAILED;       // Program has failed.
			}

			Log("Name: %ls\r\n", V_BSTR(&varName));
		}
		{
			VARIANT varName;
			HRESULT hres = processObject->Get(_bstr_t(L"ExecutablePath"), 0, &varName, 0, 0);

			if(FAILED(hres))
			{
				Log("Failed to get the data from the query. Error code = 0x%x\r\n", hres);
				return WBEM_E_FAILED;       // Program has failed.
			}

			Log("ExecutablePath: %ls\r\n", V_BSTR(&varName));
		}
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
