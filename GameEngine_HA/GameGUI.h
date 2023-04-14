#pragma once

class GameGUI
{
protected:

public:
	GameGUI();
	void render();
	static bool colonistInfoWindowOpen;
	static bool depotInfoWindowOpen;
	void renderColonistInfoWindow();
	void renderColonistsWindow();
	void renderDepotInfoWindow();
};
