#pragma once

#include "kernel/Drivers/Driver.h"
#include "kernel/Objects/KEvent.h"
#include "kernel/HyperV/Interrupts.h"
#include "kernel/HyperV/VmBus.h"
#include "kernel/HyperV/HyperV.h"
#include "Lib/StaticVector.h"
#include "Lib/StaticMap.h"

//These aren't used


class VmBusDriver : public Driver
{
public:
	static void OnInterrupt(void* arg) { ((VmBusDriver*)arg)->OnInterrupt(); };

	VmBusDriver(KDevice& device);

	Result Initialize(Arena& arena) override;
	Result Enumerate(Arena& arena) override;

	HyperV::HV_HYPERCALL_RESULT_VALUE PostMessage(const uint32_t size, const void* message, HyperV::VmBusResponse& response);
	void SetCallback(uint32_t id, const ActionContext& context);

private:
	static constexpr HyperV::HV_MESSAGE_TYPE VmbusMessageType = (HyperV::HV_MESSAGE_TYPE)1;

	static HYPERV_PAGE_ALIGN volatile uint8_t MonitorPage1[PageSize];
	static HYPERV_PAGE_ALIGN volatile uint8_t MonitorPage2[PageSize];

	struct BusRequest
	{
		uint32_t Gpadl;
		uint32_t child_relid;
		uint32_t openid;
		KEvent* Event;
		HyperV::VmBusResponse* Response;
	};

	BusRequest* FindRequest(uint32_t gpadl);
	BusRequest* FindRequest(uint32_t child_relid, uint32_t openid);

	void OnInterrupt();

	Arena* m_arena;

	KEvent m_connectEvent;
	KThread* m_thread;

	//Channel callbacks
	StaticMap<uint32_t, ActionContext, 16> m_channelCallbacks;

	//Connection properties
	uint32_t m_msg_conn_id;
	uint32_t m_nextGpadlHandle;

	//Bus Requests
	StaticVector<BusRequest, 16> m_requests;
};

/*
Connection order:
Guest: CHANNELMSG_INITIATE_CONTACT. Host: CHANNELMSG_VERSION_RESPONSE
Guest: CHANNELMSG_REQUESTOFFERS. Host: multiple CHANNELMSG_OFFERCHANNEL, ended with CHANNELMSG_ALLOFFERS_DELIVERED
*/
