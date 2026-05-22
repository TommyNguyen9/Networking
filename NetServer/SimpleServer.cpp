
#include <iostream>
#include "../NetCommon/olc_net.h"



enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

class CustomServer : public olc::net::server_interface<CustomMsgTypes>
{
public:
    CustomServer(uint16_t nPort) : olc::net::server_interface<CustomMsgTypes>(nPort)
    {

    }

protected:
    virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
    {
        return true;
    }

    // Called when client has disconnected:

    virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
    {

    }

    // Called when a message arrives:
    virtual void onMessage(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client, olc::net::message<CustomMsgTypes>& msg)
    {
        switch (msg.header.id)
        {
        case CustomMsgTypes::ServerPing:
        {
            std::cout << "[" << client->GetID() << "]: Server Ping\n";

            // Bounce message back to client
            client->Send(msg);
        }
        break;
        }
    }
};


int main()
{
    CustomServer server(60000);
    server.Start();

    while (1)
    {
        server.Update();
    }
    return 0;
}

