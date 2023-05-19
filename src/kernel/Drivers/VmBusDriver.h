#pragma once

#include "Driver.h"
#include <linux/hyperv.h>
#include <Objects/KEvent.h>
#include <Objects/KSemaphore.h>
#include <shared/MetalOS.Types.h>
#include <KThread.h>

#include <list>

class VmBusDriver : public Driver
{
public:
	VmBusDriver(Device& device);

	Result Initialize() override;
	Result Read(char* buffer, size_t length, size_t* bytesRead = nullptr) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

	static void OnInterrupt(void* arg) { ((VmBusDriver*)arg)->OnInterrupt(); };
	static size_t ThreadLoop(void* arg) { return ((VmBusDriver*)arg)->ThreadLoop(); };

	HV_HYPERCALL_RESULT_VALUE PostMessage(const uint32_t size, const void* message, VmBusResponse& response);

	void SetMonitor(uint32_t monitorId);

	void SetCallback(uint32_t id, const CallContext& context);

private:
	struct BusRequest
	{
		uint32_t Gpadl;
		uint32_t child_relid;
		uint32_t openid;
		KEvent* Event;
		VmBusResponse& Response;
	};

	static const uint32_t VMBUS_MESSAGE_SINT = 2;

	enum VMBUS_SINT
	{
		VMBUS_MESSAGE_CONNECTION_ID = 1,
		VMBUS_MESSAGE_CONNECTION_ID_4 = 4,
		VMBUS_MESSAGE_PORT_ID = 1,
		VMBUS_EVENT_CONNECTION_ID = 2,
		VMBUS_EVENT_PORT_ID = 2,
		VMBUS_MONITOR_CONNECTION_ID = 3,
		VMBUS_MONITOR_PORT_ID = 3,
	};

	BusRequest* FindRequest(uint32_t gpadl);
	BusRequest* FindRequest(uint32_t child_relid, uint32_t openid);


	void OnInterrupt();
	uint32_t ThreadLoop();

	static KERNEL_PAGE_ALIGN volatile uint8_t MonitorPage1[PageSize];
	static KERNEL_PAGE_ALIGN volatile uint8_t MonitorPage2[PageSize];


	KSemaphore m_threadSync;
	KEvent m_connectEvent;
	KThread* m_thread;
	std::list<HV_MESSAGE> m_queue;

	//Channel callbacks
	std::map<uint32_t, CallContext> m_channelCallbacks;

	//Connection properties
	uint32_t m_msg_conn_id;
	uint32_t m_nextGpadlHandle;

	//Bus Requests
	std::list<BusRequest> m_requests;
};

/*
Connection order:
Guest: CHANNELMSG_INITIATE_CONTACT. Host: CHANNELMSG_VERSION_RESPONSE
Guest: CHANNELMSG_REQUESTOFFERS. Host: multiple CHANNELMSG_OFFERCHANNEL, ended with CHANNELMSG_ALLOFFERS_DELIVERED
*/
