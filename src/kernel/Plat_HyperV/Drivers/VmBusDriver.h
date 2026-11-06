#pragma once

#include "kernel/Drivers/Driver.h"
#include "kernel/Objects/KEvent.h"
#include "kernel/Plat_HyperV/Interrupts.h"
#include "kernel/Plat_HyperV/VmBus.h"
#include "kernel/Plat_HyperV/HyperV.h"
#include "Lib/StaticVector.h"
#include "Lib/StaticMap.h"

class VmBusDriver;

//A channel the host offered; its device's Context points here
struct VmBusChild
{
	static const VmBusChild& Of(const KDevice& device) { return *(const VmBusChild*)device.Context; }

	uint32_t RelId;
	uint32_t ConnectionId;
	HyperV::vmbus_channel_offer_channel Offer;
	VmBusDriver* Bus; //Creates the channel's GPADLs and opens it
};

class VmBusDriver : public Driver
{
public:
	static void OnInterrupt(void* arg) { ((VmBusDriver*)arg)->OnInterrupt(); };

	VmBusDriver(KDevice& device);

	bool Initialize() override;

	uint32_t CreateGpadl(const uint32_t childRelid, const paddr_t address, const size_t pageCount);
	HyperV::HV_HYPERCALL_RESULT_VALUE OpenChannel(const HyperV::vmbus_channel_open_channel& message, HyperV::VmBusResponse& response);
	void SetCallback(uint32_t id, const ActionContext& context);
	void RemoveCallback(const uint32_t id);
	bool CloseChannel(const uint32_t childRelid); //The host doesn't answer a close
	bool TeardownGpadl(const uint32_t childRelid, const uint32_t gpadl, const milli_t timeout); //False if the host didn't release it in time

private:
	static constexpr HyperV::HV_MESSAGE_TYPE VmbusMessageType = (HyperV::HV_MESSAGE_TYPE)1;

	struct BusRequest
	{
		uint32_t Gpadl;
		uint32_t child_relid;
		uint32_t openid;
		KEvent* Event;
		HyperV::VmBusResponse* Response;
	};

	void AddRequest(const BusRequest& request);
	BusRequest TakeRequest(const uint32_t gpadl, const uint32_t child_relid, const uint32_t openid);
	bool TryTakeRequest(const uint32_t gpadl, const uint32_t child_relid, const uint32_t openid, BusRequest& request);
	bool PostChannelMessage(const void* const message, const uint32_t size);

	void OnInterrupt();


	KEvent m_connectEvent;

	//Channel callbacks
	StaticMap<uint32_t, ActionContext, 16> m_channelCallbacks;

	//Connection properties
	uint32_t m_msg_conn_id;
	uint32_t m_nextGpadlHandle;

	//Requests awaiting a host response; each is removed when answered
	StaticVector<BusRequest, 16> m_requests;

	//Offered channels, which live as long as the bus
	StaticVector<VmBusChild, 32> m_children;
};

/*
Connection order:
Guest: CHANNELMSG_INITIATE_CONTACT. Host: CHANNELMSG_VERSION_RESPONSE
Guest: CHANNELMSG_REQUESTOFFERS. Host: multiple CHANNELMSG_OFFERCHANNEL, ended with CHANNELMSG_ALLOFFERS_DELIVERED
*/
