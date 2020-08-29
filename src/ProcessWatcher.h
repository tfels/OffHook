#pragma once

#include "Singleton.h"
#include "WmiEventSink.h"

class ProcessWatcher :public Singleton<ProcessWatcher>
{
	friend class Singleton<ProcessWatcher>;

public:
	bool Init();
	bool NotifyStartOfProcess(const char* processName);
	void StopNotify();
	void Exit();

private: // constructors
	ProcessWatcher() {};
	~ProcessWatcher() {};

private:
	bool m_running = false;

	IWbemLocator* pLoc = nullptr;
	IWbemServices* pSvc = nullptr;

	IUnsecuredApartment* pUnsecApp = nullptr;
	WmiEventSink* pSink = nullptr;
	IUnknown* pStubUnk = nullptr;
	IWbemObjectSink* pStubSink = nullptr;
};

