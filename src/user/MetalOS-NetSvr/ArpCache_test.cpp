//Lives beside Main.cpp so the stack's "Net/X.h" includes resolve as they do in the unity build
#include "Net/ArpCache.cpp"

namespace
{
	Net::ipv4_addr_t TestIp(const uint8_t host)
	{
		return Net::build_ip(10, 0, 0, host);
	}

	Net::eth_mac_t TestMac(const uint8_t id)
	{
		return { .bytes = {0x00, 0x15, 0x5D, 0x00, 0x00, id} };
	}

	bool IsCached(const uint8_t host, const uint8_t id)
	{
		Net::eth_mac_t mac = {};
		return Net::ArpCache::Lookup(TestIp(host), mac) && mac == TestMac(id);
	}
}

void ArpCache_test()
{
	//0.0.0.0 is never a peer, so it doesn't resolve
	Net::eth_mac_t mac = {};
	Assert(!Net::ArpCache::Lookup(Net::empty, mac));

	//Broadcast resolves without a cache slot
	Assert(Net::ArpCache::Lookup(Net::broadcast, mac));
	Assert(mac == Net::eth_broadcast);

	//All eight slots hold peers
	static_assert(CacheSize == 8);
	for (uint8_t i = 1; i <= 8; i++)
		Net::ArpCache::Update(TestIp(i), TestMac(i));
	for (uint8_t i = 1; i <= 8; i++)
		Assert(IsCached(i, i));

	//A ninth peer replaces the oldest instead of failing
	Net::ArpCache::Update(TestIp(9), TestMac(9));
	Assert(!Net::ArpCache::Lookup(TestIp(1), mac));
	for (uint8_t i = 2; i <= 9; i++)
		Assert(IsCached(i, i));

	//A known peer is refreshed in place, and the next replacement takes the next oldest
	Net::ArpCache::Update(TestIp(3), TestMac(99));
	Net::ArpCache::Update(TestIp(10), TestMac(10));
	Assert(!Net::ArpCache::Lookup(TestIp(2), mac));
	Assert(IsCached(3, 99));
	for (uint8_t i = 4; i <= 10; i++)
		Assert(IsCached(i, i));

	//An ARP probe's 0.0.0.0 sender takes no slot
	Net::ArpCache::Update(Net::empty, TestMac(100));
	Assert(IsCached(3, 99));
	for (uint8_t i = 4; i <= 10; i++)
		Assert(IsCached(i, i));
	Assert(!Net::ArpCache::Lookup(Net::empty, mac));

	//The subnet's directed broadcast is a broadcast; hosts, other subnets and /31 or /32 interfaces have none
	const Net::ipv4_addr_t if_addr = Net::build_ip(192, 168, 0, 10);
	const Net::ipv4_addr_t subnet_24 = Net::build_ip(255, 255, 255, 0);
	Assert(Net::is_subnet_broadcast(Net::build_ip(192, 168, 0, 255), if_addr, subnet_24));
	Assert(!Net::is_subnet_broadcast(Net::build_ip(192, 168, 0, 254), if_addr, subnet_24));
	Assert(!Net::is_subnet_broadcast(if_addr, if_addr, subnet_24));
	Assert(!Net::is_subnet_broadcast(Net::build_ip(192, 168, 1, 255), if_addr, subnet_24));
	Assert(Net::is_subnet_broadcast(Net::build_ip(10, 255, 255, 255), Net::build_ip(10, 0, 0, 2), Net::build_ip(255, 0, 0, 0)));
	Assert(!Net::is_subnet_broadcast(Net::build_ip(192, 168, 0, 11), if_addr, Net::build_ip(255, 255, 255, 254)));
	Assert(!Net::is_subnet_broadcast(if_addr, if_addr, Net::subnet_32));
	Assert(!Net::is_subnet_broadcast(Net::broadcast, Net::empty, Net::subnet_32));

	//A pending request blocks another until it's answered or arp_retry_time passes
	Net::arp_request_t request = {};
	Assert(request.try_begin(5000));
	Assert(!request.try_begin(5000));
	Assert(!request.try_begin(5000 + Net::arp_retry_time - 1));
	Assert(request.try_begin(5000 + Net::arp_retry_time));
	Assert(!request.try_begin(5000 + Net::arp_retry_time + 1));
	request.end();
	Assert(request.try_begin(5000 + Net::arp_retry_time + 1));
}
