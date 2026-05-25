#include <iostream>
#include <unordered_map>

#include "MMO_Common.h"

class GameServer : public olc::net::server_interface<GameMsg>
{
public:
	GameServer(uint16_t nPort) : olc::net::server_interface<GameMsg>(nPort)
	{

	}

protected:
	bool OnClientConnect(std::shared_ptr<olc::net::connection<GameMsg>> client) override
	{
		// Allow all
		return true;
	}
};