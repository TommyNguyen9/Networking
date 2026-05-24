#include "../MMO_Server/MMO_Common.h"

#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

#include <unordered_map>


class MMOGame : public olc::PixelGameEngine, olc::net::client_interface<GameMsg>
{
public:
	MMOGame()
	{
		sAppName = "MMO Client";
	}

private:
	olc::TileTransformedView tv;

	std::string sWorldMap = 
		"################################"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..........####...####.........#"
		"#..........#.........#.........#"
		"#..........#.........#.........#"
		"#..........#.........#.........#"
		"#..........##############......#"
		"#..............................#"
		"#..................#.#.#.#.....#"
		"#..............................#"
		"#..................#.#.#.#.....#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"#..............................#"
		"################################";

	olc::vi2d vWorldSize = { 32, 32 };

private:
	std::unordered_map<uint32_t, sPlayerDescription> mapObjects;
	uint32_t nPlayerID = 0;


public:
	bool OnUserCreate() override
	{
		tv = olc::TileTransformedView({ ScreenWidth(), ScreenHeight() }, { 8, 8 });
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		// Controlling player object:
		mapObjects[nPlayerID].vVel = { 0.0f, 0.0f };
		if (GetKey(olc::Key::W).bHeld) mapObjects[nPlayerID].vVel += olc::vf2d{ 0.0f, -1.0f };
		if (GetKey(olc::Key::S).bHeld) mapObjects[nPlayerID].vVel += olc::vf2d{ 0.0f, +1.0f };
		if (GetKey(olc::Key::A).bHeld) mapObjects[nPlayerID].vVel += olc::vf2d{ -1.0f, 0.0f };
		if (GetKey(olc::Key::D).bHeld) mapObjects[nPlayerID].vVel += olc::vf2d{ +1.0f, 0.0f };

		if (mapObjects[nPlayerID].vVel.mag2() > 0)
			mapObjects[nPlayerID].vVel = mapObjects[nPlayerID].vVel.norm() * 4.0f;
		
		// Update object locally:
		for (auto& object : mapObjects)
		{
			// Circle, rectangle collisions:
			olc::vf2d vPotentialPosition = object.second.vPos + object.second.vVel * fElapsedTime;

			// Extract region of world cells that have collisions
			olc::vi2d vCurrentCell = object.second.vPos.floor();
			olc::vi2d vTargetCell = vPotentialPosition;
			olc::vi2d vAreaTL = (vCurrentCell.min(vTargetCell) - olc::vi2d(1, 1)).max({ 0, 0 });
			olc::vi2d vAreaBR = (vCurrentCell.max(vTargetCell) + olc::vi2d(1, 1)).min(vWorldSize);

			// Iterate through each cell:
			olc::vi2d vCell;
			for (vCell.y = vAreaTL.y; vCell.y <= vAreaBR.y; vCell.y++)
			{
				for (vCell.x = vAreaTL.x; vCell.x <= vAreaBR.x; vCell.x++)
				{
					// Check if cell is solid
					if (sWorldMap[vCell.y * vWorldSize.x + vCell.x] == '#')
					{
						// Calculate nearest point to future player position
						olc::vf2d vNearestPoint;
						vNearestPoint.x = std::max(float(vCell.x), std::min(vPotentialPosition.x, float(vCell.x + 1)));
						vNearestPoint.y = std::max(float(vCell.y), std::min(vPotentialPosition.y, float(vCell.y + 1)));


						// Modifications to ensure it works:
						olc::vf2d vRayToNearest = vNearestPoint - vPotentialPosition;
						float fOverlap = object.second.fRadius - vRayToNearest.mag();
						if (std::isnan(fOverlap)) fOverlap = 0;

						if (fOverlap > 0)
						{
							// Resolve collision statistically
							vPotentialPosition = vPotentialPosition - vRayToNearest.norm() * fOverlap;
						}
					}
				}
			}
		}


		// Pan & Zoom:
		if (GetMouse(2).bPressed) tv.StartPan(GetMousePos());
		if (GetMouse(2).bHeld) tv.UpdatePan(GetMousePos());
		if (GetMouse(2).bReleased) tv.EndPan(GetMousePos());
		if (GetMouseWheel() > 0) tv.ZoomAtScreenPos(1.5f, GetMousePos());
		if (GetMouseWheel() < 0) tv.ZoomAtScreenPos(0.75f, GetMousePos());

		// Clear world
		Clear(olc::BLACK);

		// Draw world:
		olc::vi2d vTL = tv.GetTopLeftTile().max({ 0, 0 });
		olc::vi2d vBR = tv.GetBottomRightTile().min(vWorldSize);
		olc::vi2d vTile;
		for (vTile.y = vTL.y; vTile.y < vBR.y; vTile.y++)
			for (vTile.x = vTL.x; vTile.x < vBR.x; vTile.x++)
			{
				if (sWorldMap[vTile.y * vWorldSize.x + vTile.x] == '#')
				{
					tv.DrawRect(vTile, { 1.0f, 1.0f });
					tv.DrawRect(olc::vf2d(vTile) + olc::vf2d(0.1f, 0.1f), { 0.8f, 0.8f });
				}
			}

		// Draw World Objects:
		for (auto& object : mapObjects)
		{
			// Drawing the boundary
			tv.DrawCircle(object.second.vPos, object.second.fRadius);

			// Draw velocity
			if (object.second.vVel.mag2() > 0)
				tv.DrawLine(object.second.vPos, object.second.vPos + object.second.vVel.norm() * object.second.fRadius, olc::MAGENTA);

			// Drawing name:
			olc::vi2d vNameSize = GetTextSizeProp("ID: " + std::to_string(object.first));
			tv.DrawStringPropDecal(object.second.vPos - olc::vf2d{ vNameSize.x * 0.5f * 0.25f * 0.125f, -object.second.fRadius * 1.25f }, "ID: " + std::to_string(object.first), olc::BLUE, { 0.25f, 0.25f });
		}

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