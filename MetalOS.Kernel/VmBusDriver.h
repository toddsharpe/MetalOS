#pragma once

#include "Driver.h"
#include "Device.h"
#include "MicrosoftHyperV.h"
#include "HyperV.h"

class VmBusDriver : public Driver
{
public:
	VmBusDriver(Device& device);

	Result Initialize() override;
	Result Read(const char* buffer, size_t length) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

	static void OnInterrupt(void* arg) { ((VmBusDriver*)arg)->OnInterrupt(); };
	static uint32_t ThreadLoop(void* arg) { return ((VmBusDriver*)arg)->ThreadLoop(); };

private:
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

	void OnInterrupt();
	uint32_t ThreadLoop();

	static KERNEL_PAGE_ALIGN volatile uint8_t MonitorPage1[PAGE_SIZE];
	static KERNEL_PAGE_ALIGN volatile uint8_t MonitorPage2[PAGE_SIZE];


	Handle m_threadSignal;
	Handle m_connectSemaphore;
	std::list<HV_MESSAGE> m_queue;

	//Connection properties
	uint32_t m_msg_conn_id;
};

/*
Connection order:
Guest: CHANNELMSG_INITIATE_CONTACT. Host: CHANNELMSG_VERSION_RESPONSE
Guest: CHANNELMSG_REQUESTOFFERS. Host: multiple CHANNELMSG_OFFERCHANNEL, ended with CHANNELMSG_ALLOFFERS_DELIVERED
*/
