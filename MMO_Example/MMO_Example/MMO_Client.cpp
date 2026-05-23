#include "../MMO_Server/MMO_Common.h"


class MMOGame : public olc::PixelGameEngine, olc::net::client_interface<GameMsg>
{
public:
	MMOGame()
	{
		sAppName = "MMO Client";
	}

public:
	bool OnUserCreate() override
	{
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		return true;
	}
};

int main()
{
	MMOGame demo;
	if (demo.Construct(640, 480, 2, 2))
		demo.Start();
	return 0;
}