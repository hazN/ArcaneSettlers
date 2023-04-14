#include "GameGUI.h"

#include <iostream>

#include "imgui/imgui.h"
#include "globalThings.h"
#include "JSONPersitence.h"
#include "cBasicTextureManager.h"
extern cBasicTextureManager* g_pTextureManager;

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
    for (const Item item : items) {
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
    if (currentColonist < 0 || currentColonist >= goMap.size())
        return;

    Colonist* colonist = vecColonists[currentColonist];
    // Main window
    ImGui::SetNextWindowSize(ImVec2(180, 0));
    ImGui::Begin("Colonist Info", &colonistInfoWindowOpen, ImGuiWindowFlags_None);

    ImGui::SetWindowFontScale(1.5f);
    ImGui::Text("%s", colonist->name.c_str());
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Image((void*)(intptr_t)g_pTextureManager->getTextureIDFromName(colonist->icon), ImVec2(64, 64));
    std::string weight = "";
    weight = std::to_string(colonist->mInventory->getCurrentWeight()) + "/" + std::to_string(colonist->mInventory->getMaxWeight());
    ImGui::Text("Health: %d\nHunger: %f\nMining: %d\nChopping: %d\nCombat: %d\nWeight: %s", colonist->mStats->hp, colonist->mStats->hunger,
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
    ImGui::SetWindowFontScale(1.8f);
    for (size_t i = 0; i < vecColonists.size(); ++i) 
    {
        if (ImGui::ImageButton((void*)(intptr_t)g_pTextureManager->getTextureIDFromName(vecColonists[i]->icon), ImVec2(48, 48))) {
            currentColonist = i;
            colonistInfoWindowOpen = true;
        }
        ImGui::SameLine();
        ImGui::Text("%s\nHP: %d", vecColonists[i]->currentAction.c_str(), vecColonists[i]->mStats->hp);
    }

    ImGui::End();
}
bool GameGUI::colonistInfoWindowOpen = false;
bool GameGUI::depotInfoWindowOpen = false;
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
}

