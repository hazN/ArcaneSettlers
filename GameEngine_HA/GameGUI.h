#pragma once

class GameGUI
{
protected:

public:
	GameGUI();
	void render();
	bool colonistInfoWindowOpen;
	void renderColonistInfoWindow();
	void renderColonistsWindow();
};
