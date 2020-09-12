#pragma once

#include "Singleton.h"
#include "WmiEventSink.h"

class ProcessWatcher_NotifyInterface
{
public:
	virtual void ProcessStarted(std::string name) = 0;
	virtual void ProcessStopped(std::string name) = 0;
};

class ProcessWatcher :public Singleton<ProcessWatcher>
{
	friend class Singleton<ProcessWatcher>;

public:
	bool Init();
	bool NotifyStartOfProcess(const char* processName, ProcessWatcher_NotifyInterface* notifyCallback);
	void StopNotify();
	void Exit();

private: // constructors
	ProcessWatcher() {};
	~ProcessWatcher() {};

private:
	bool m_running = false;

	ProcessWatcher_NotifyInterface* m_notifyCallback = nullptr;

	IWbemLocator* pLoc = nullptr;
	IWbemServices* pSvc = nullptr;

	IUnsecuredApartment* pUnsecApp = nullptr;
	WmiEventSink* pSink = nullptr;
	IUnknown* pStubUnk = nullptr;
	IWbemObjectSink* pStubSink = nullptr;
};

