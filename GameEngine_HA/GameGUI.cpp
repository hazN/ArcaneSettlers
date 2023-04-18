#include "GameGUI.h"

#include <iostream>

#include "imgui/imgui.h"
#include "globalThings.h"
#include "JSONPersitence.h"
#include "cBasicTextureManager.h"
extern cBasicTextureManager* g_pTextureManager;

void GameGUI::renderConsole()
{
	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 400, ImGui::GetIO().DisplaySize.y - 250), ImGuiCond_FirstUseEver);

	ImGui::Begin("Console", &consoleWindowOpen);
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	for (std::string& message : GameGUI::mMessages)
	{
		ImGui::TextUnformatted(message.c_str());
	}

	ImGui::SetScrollHereY(1.0f);
	ImGui::EndChild();
	ImGui::End();
}

void GameGUI::renderBottomBar()
{
	// Bar gui
	ImVec2 barSize = ImVec2(ImGui::GetIO().DisplaySize.x, 50);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
	ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - barSize.y));
	ImGui::SetNextWindowSize(barSize);
	ImGui::Begin("BottomBar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

	if (ImGui::Button("Build"))
	{
		buildingMenuOpen = !buildingMenuOpen;
	}

	ImGui::SameLine();

	// Display resource count
	std::vector<std::pair<std::string, int>> resources = { {"Wood.bmp", gDepot->inventory->getItemCount(wood)}, {"Stone.bmp", gDepot->inventory->getItemCount(stone)}, {"Minerals.bmp", gDepot->inventory->getItemCount(ores)}, {"Crystal.bmp", gDepot->inventory->getItemCount(gems)} };

	for (std::pair<std::string, int> resource : resources)
	{
		ImGui::Image((void*)(intptr_t)g_pTextureManager->getTextureIDFromName(resource.first), ImVec2(32, 32));
		ImGui::SameLine();
		ImGui::Text("%d", resource.second);
		ImGui::SameLine();
	}

	ImGui::SameLine();
	if (ImGui::Button("Console"))
	{
		consoleWindowOpen = !consoleWindowOpen;
	}

	// Display debug message in the bottom bar
	if (!mMessages.empty())
	{
		ImGui::SameLine();
		ImGui::Text(mMessages.back().c_str());
	}

	ImGui::End();
	ImGui::PopStyleVar();

	if (buildingMenuOpen)
	{
		renderBuildingMenu();
	}
	if (consoleWindowOpen)
	{
		renderConsole();
	}
}

void GameGUI::renderBuildingMenu()
{
	// Building menu gui
	ImGui::SetNextWindowSize(ImVec2(200, 400));
	ImGui::Begin("Building Menu", &buildingMenuOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

	// Loop through recipes and list them out 
	for (std::pair<BuildingType, std::map<itemId, int>> recipe : buildingRecipes)
	{
		bool canCraft = buildingManager->canCraft(recipe.first);
		// Grey out the button if its not craftable 
		if (!canCraft)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		// Show button to build if they are craftable
		if (ImGui::ImageButton((void*)(intptr_t)g_pTextureManager->getTextureIDFromName(getBuildingIcon(recipe.first)), ImVec2(48, 48)))
		{
			if (canCraft)
			{
				// Allow to unselect by clicking on it again
				if (selectedBuilding == recipe.first)
				{
					selectedBuilding = NONE;
				} else selectedBuilding = recipe.first;
			}
		}

		if (!canCraft)
		{
			ImGui::PopStyleVar();
		}

		ImGui::SameLine();
		if (!canCraft)
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		else ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		ImGui::Text("%s", getBuildingName(recipe.first).c_str());
		ImGui::PopStyleColor();
	}

	ImGui::End();
}


void GameGUI::renderDepotInfoWindow()
{
	if (!depotInfoWindowOpen) {
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(400, 400));
	ImGui::Begin("Depot Info", &depotInfoWindowOpen, ImGuiWindowFlags_None);

	ImGui::Text("Depot Inventory");

	ImGui::BeginChild("DepotInventory", ImVec2(ImGui::GetWindowContentRegionWidth(), 300), true);
	std::vector<Item> items = gDepot->inventory->getAllItems();
	int itemCount = 0;
	for (Item item : items) {
		ImGui::Image((void*)(intptr_t)g_pTextureManager->getTextureIDFromName(item.icon), ImVec2(32, 32));
		itemCount++;
		if (itemCount % 8 != 0) {
			ImGui::SameLine();
		}
	}
	ImGui::EndChild();

	ImGui::End();
}

void GameGUI::renderColonistInfoWindow()
{
	// Make sure a colonist is selected
	if (currentColonist < 0 || currentColonist >= vecColonists.size())
		return;

	Colonist* colonist = vecColonists[currentColonist];
	// Main window
	ImGui::SetNextWindowSize(ImVec2(180, 0));
	ImGui::Begin("Colonist Info", &colonistInfoWindowOpen, ImGuiWindowFlags_None);

	ImGui::SetWindowFontScale(1.25f);
	ImGui::Text("%s", colonist->name.c_str());
	ImGui::SetWindowFontScale(1.0f);

	ImGui::Image((void*)(intptr_t)g_pTextureManager->getTextureIDFromName(colonist->icon), ImVec2(64, 64));
	std::string weight = "";
	weight = std::to_string(colonist->mInventory->getCurrentWeight()) + "/" + std::to_string(colonist->mInventory->getMaxWeight());
	ImGui::Text("Health: %.2f\nHunger: %.2f\nMining: %d\nChopping: %d\nCombat: %d\nWeight: %s", colonist->mStats->hp, colonist->mStats->hunger,
		colonist->mStats->mining, colonist->mStats->chopping, colonist->mStats->combat, weight.c_str());

	// Sub-window for inventory
	ImGui::BeginChild("Inventory", ImVec2(ImGui::GetWindowContentRegionWidth(), 200), true);
	std::vector<Item> items = colonist->mInventory->getAllItems();
	int itemCount = 0;
	for (const Item item : items) {
		ImGui::Image((void*)(intptr_t)g_pTextureManager->getTextureIDFromName(item.icon), ImVec2(32, 32));
		itemCount++;
		// Make it 4 items per row in inventory
		if (itemCount % 4 != 0) {
			ImGui::SameLine();
		}
	}
	ImGui::EndChild();

	ImGui::End();
}

void GameGUI::renderColonistsWindow()
{
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 250, 0));
	ImGui::SetNextWindowSize(ImVec2(250, ImGui::GetIO().DisplaySize.y));
	ImGui::Begin("Colonists", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::SetWindowFontScale(1.5f);
	for (size_t i = 0; i < vecColonists.size(); ++i)
	{
		std::string buttonId = "ColonistButton##" + std::to_string(i);

		if (ImGui::ImageButton(buttonId.c_str(), (void*)(intptr_t)g_pTextureManager->getTextureIDFromName(vecColonists[i]->icon), ImVec2(48, 48))) {
			currentColonist = i;
			colonistInfoWindowOpen = true;
		}
		ImGui::SameLine();
		ImGui::Text("%s\n%s\nHP: %.2f", vecColonists[i]->name.substr(0, vecColonists[i]->name.find(' ')).c_str(), vecColonists[i]->currentAction.c_str(), vecColonists[i]->mStats->hp);
	}

	ImGui::End();
}
bool GameGUI::colonistInfoWindowOpen = false;
bool GameGUI::depotInfoWindowOpen = false;
bool GameGUI::buildingMenuOpen = false;
bool GameGUI::consoleWindowOpen = false;
std::vector<std::string> GameGUI::mMessages;
GameGUI::GameGUI()
{
	currentColonist = -1;
}

void GameGUI::render()
{
	renderColonistsWindow();
	if (colonistInfoWindowOpen) {
		renderColonistInfoWindow();
	}
	if (depotInfoWindowOpen)
	{
		renderDepotInfoWindow();
	}
	renderBottomBar();
}

void GameGUI::addMessage(const char* message)
{
	GameGUI::mMessages.push_back(message);
}