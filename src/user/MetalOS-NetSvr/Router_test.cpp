//The router: manual routes, then the most specific interface that is up, then the first one up
#include "Net/Net.h"
#include "Net/NetDriver.h"
#include "Net/NetIf.h"
#include "Net/Packet.h"

namespace
{
	//Stands in for a NIC whose link a test takes up and down
	class RouterTestDriver : public Net::NetDriver
	{
	public:
		RouterTestDriver() :
			Up(true),
			m_mac()
		{

		}

		void Receive(Net::NetIf&) override
		{

		}

		bool Send(Net::NetIf&, Net::Packet& packet) override
		{
			Net::free_tx(packet.backing);
			return true;
		}

		const Net::eth_mac_t& GetMac() const override
		{
			return m_mac;
		}

		bool IsLinkUp() const override
		{
			return Up;
		}

		milli_t GetTicks() const override
		{
			return 0;
		}

		bool Up;

	private:
		Net::eth_mac_t m_mac;
	};

	RouterTestDriver wideDriver;
	Net::NetIf wideIf(wideDriver, Net::l2_t::Ethernet);
	RouterTestDriver narrowDriver;
	Net::NetIf narrowIf(narrowDriver, Net::l2_t::Ethernet);
}

void Router_test()
{
	Assert(Net::AddNetIf(wideIf));
	Assert(Net::AddNetIf(narrowIf));
	wideIf.ipv4.addr = Net::build_ip(10, 9, 0, 1);
	wideIf.ipv4.subnet_mask = Net::build_ip(255, 255, 0, 0);
	narrowIf.ipv4.addr = Net::build_ip(10, 9, 1, 1);
	narrowIf.ipv4.subnet_mask = Net::build_ip(255, 255, 255, 0);
	const Net::ipv4_addr_t inBoth = Net::build_ip(10, 9, 1, 5);

	//The most specific subnet wins, and an address only the wider one holds goes there
	Assert(Net::Router::Resolve(inBoth) == &narrowIf);
	Assert(Net::Router::Resolve(Net::build_ip(10, 9, 2, 5)) == &wideIf);

	//An up interface wins over a more specific one that is down, which is the answer only when no up one matches
	narrowDriver.Up = false;
	Assert(Net::Router::Resolve(inBoth) == &wideIf);
	wideDriver.Up = false;
	Assert(Net::Router::Resolve(inBoth) == &narrowIf);
	wideDriver.Up = true;
	narrowDriver.Up = true;

	//An address no interface holds goes to the first one up
	Assert(Net::Router::Resolve(Net::build_ip(192, 0, 2, 1)) == Net::GetNetIfs()[0]);

	//A manual route comes before the subnets
	Assert(Net::Router::AddRoute(wideIf, Net::build_ip(10, 9, 1, 0), Net::build_ip(255, 255, 255, 0)));
	Assert(Net::Router::Resolve(inBoth) == &wideIf);
}
