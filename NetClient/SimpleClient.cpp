
#include <iostream>
#include <olc_net.h>
#include <net_client.h>
#include <net_connection.h>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};

class CustomClient : public olc::net::client_interface<CustomMsgTypes>
{

};

int main()
{
	std::cout << "Client starting...\n";

	CustomClient c;

	std::cout << "Trying to connect...\n";
	c.Connect("community.onelonecoder.com", 60000);

	std::cout << "Client running. Press enter to exit...\n";
	std::cin.get();

	return 0;
}

