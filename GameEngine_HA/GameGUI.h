#pragma once
#include <vector>
#include <string>
class GameGUI
{
protected:
	static std::vector<std::string> mMessages;
	bool mConsoleOpen;
public:
	GameGUI();
	void render();
	static bool colonistInfoWindowOpen;
	static bool depotInfoWindowOpen;
	static bool buildingMenuOpen;
	static bool consoleWindowOpen;
	static void addMessage(const char* message);
	void renderColonistInfoWindow();
	void renderColonistsWindow();
	void renderDepotInfoWindow();
	void renderBottomBar();
	void renderBuildingMenu();
	void renderConsole();
};
