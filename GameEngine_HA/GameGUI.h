#pragma once

class GameGUI
{
protected:

public:
	GameGUI();
	void render();
	static bool colonistInfoWindowOpen;
	static bool depotInfoWindowOpen;
	static bool buildingMenuOpen;
	void renderColonistInfoWindow();
	void renderColonistsWindow();
	void renderDepotInfoWindow();
	void renderBottomBar();
	void renderBuildingMenu();
};
