#include "HyperVScsiDriver.h"
#include "Main.h"
#include "HyperVDevice.h"

HyperVScsiDriver::HyperVScsiDriver(Device& device) :
	Driver(device),
	m_sizeDelta(sizeof(struct vmscsi_win8_extension)),
	m_semaphore(),
	m_channel(SCSI_VSC_SEND_RING_BUFFER_SIZE, SCSI_VSC_RECV_RING_BUFFER_SIZE, { &HyperVScsiDriver::Callback, this })
{
	m_semaphore = kernel.CreateSemaphore(0, 0, "HyperVScsiDriver");

}

Result HyperVScsiDriver::Initialize()
{
	HyperVDevice* device = (HyperVDevice*)&m_device;
	vmbus_channel_offer_channel* offerChannel = (vmbus_channel_offer_channel*)device->GetResource(HyperVDevice::ResourceType::OfferChannel);

	//Connect to channel
	vmstorage_channel_properties properties = { 0 };
	Buffer buffer = { &properties, sizeof(vmstorage_channel_properties) };
	m_channel.Initialize(offerChannel, &buffer);

	Transaction transaction;
	
	//Init storage driver
	{
		memset(&transaction, 0, sizeof(Transaction));
		transaction.Request.operation = VSTOR_OPERATION_BEGIN_INITIALIZATION;
		Execute(&transaction, true);
	}

	//Query protocol
	{
		memset(&transaction, 0, sizeof(Transaction));
		transaction.Request.operation = VSTOR_OPERATION_QUERY_PROTOCOL_VERSION;
		transaction.Request.version.major_minor = VMSTOR_PROTO_VERSION_WIN10;
		transaction.Request.version.revision = 0;
		Execute(&transaction, true);
		m_sizeDelta = 0;
	}

	//Query properties
	{
		memset(&transaction, 0, sizeof(Transaction));
		transaction.Request.operation = VSTOR_OPERATION_QUERY_PROPERTIES;
		Execute(&transaction, true);
		Print("Max Channels 0x%x\n", transaction.Response.storage_channel_properties.max_channel_cnt);
	}


	return Result::ResultSuccess;
}

Result HyperVScsiDriver::Read(const char* buffer, size_t length)
{
	return Result::ResultNotImplemented;
}

Result HyperVScsiDriver::Write(const char* buffer, size_t length)
{
	return Result::ResultNotImplemented;
}

Result HyperVScsiDriver::EnumerateChildren()
{
	return Result::ResultNotImplemented;
}

void HyperVScsiDriver::OnCallback()
{
	Print("HyperVScsiDriver::OnCallback\n");

	vmpacket_descriptor* packet;
	uint32_t length = sizeof(vmpacket_descriptor);
	//The nuance here is that this structure vmpacket_descriptor is what is sent, which has an extra field (transactionid)
	//compared to what is received. the code uses the offset field from desc pointer to compensate for this, but it should really be refactored
	while ((packet = (vmpacket_descriptor*)m_channel.ReadPacket(length)) != nullptr)
	{
		Transaction* transaction = (Transaction*)packet->trans_id;
		void* data = (void*)((uintptr_t)packet + (packet->offset8 << 3));

		Print("Rec: 0x%x Small: 0x%x Large: 0x%x\n", packet->len8 << 3, (sizeof(struct vstor_packet) - vmscsi_size_delta), sizeof(struct vstor_packet));
		kernel.PrintBytes((char*)data, packet->len8 << 3);

		//Switch on request
		if (transaction->Request.operation == VSTOR_OPERATION_BEGIN_INITIALIZATION ||
			transaction->Request.operation == VSTOR_OPERATION_QUERY_PROTOCOL_VERSION)
		{
			memcpy(&transaction->Response, data, (sizeof(struct vstor_packet) - vmscsi_size_delta));
			kernel.ReleaseSemaphore(transaction->Semaphore, 1);
			kernel.CloseSemaphore(transaction->Semaphore);
		}
		else
		{
			memcpy(&transaction->Response, data, sizeof(struct vstor_packet));
			Print("response: 0x%x\n", transaction->Response.operation);
			Print("scsi_status 0x%x srb_status: 0x%x\n", transaction->Response.vm_srb.scsi_status, transaction->Response.vm_srb.srb_status);
			Print("Sense length: 0x%x\n", transaction->Response.vm_srb.sense_info_length);
			Print("Data length: 0x%x\n", transaction->Response.vm_srb.data_transfer_length);
			switch (transaction->Response.operation)
			{
			case VSTOR_OPERATION_COMPLETE_IO:
				Assert(transaction->Response.vm_srb.scsi_status == 0 && transaction->Response.vm_srb.srb_status == SRB_STATUS_SUCCESS);
				kernel.ReleaseSemaphore(transaction->Semaphore, 1);
				kernel.CloseSemaphore(transaction->Semaphore);
				break;

			case VSTOR_OPERATION_REMOVE_DEVICE:
			case VSTOR_OPERATION_ENUMERATE_BUS:

				break;

			case VSTOR_OPERATION_FCHBA_DATA:

				break;

			default:
				Assert(false);
				break;
			}
		}

		m_channel.NextPacket(packet->len8 << 3);
	}
	m_channel.StopRead();
}

void HyperVScsiDriver::Execute(Transaction* transaction, bool status_check)
{
	Print("Execute. Delta: 0x%x\n", m_sizeDelta);

	//Create semaphore
	transaction->Semaphore = kernel.CreateSemaphore(0, 0, "HyperVScsiDriver");

	transaction->Request.flags = REQUEST_COMPLETION_FLAG;

	this->m_channel.SendPacket(&transaction->Request, sizeof(vstor_packet) - m_sizeDelta, (uint64_t)transaction, VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	kernel.WaitForSemaphore(transaction->Semaphore, INT64_MAX);

	if (!status_check)
		return;

	Assert(transaction->Response.operation == VSTOR_OPERATION_COMPLETE_IO && transaction->Response.status == 0);
}
