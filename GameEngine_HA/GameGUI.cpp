#include "GameGUI.h"

#include <iostream>

#include "imgui/imgui.h"
#include "globalThings.h"
#include "JSONPersitence.h"
#include "cBasicTextureManager.h"
extern cBasicTextureManager* g_pTextureManager;

void renderColonistsWindow() {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 250, 0));
    ImGui::SetNextWindowSize(ImVec2(250, ImGui::GetIO().DisplaySize.y));
    ImGui::Begin("Colonists", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowFontScale(1.8f);
    for (const Colonist* colonist : vecColonists) {
        ImGui::Image((void*)(intptr_t)g_pTextureManager->getTextureIDFromName(colonist->icon), ImVec2(48, 48));
        ImGui::SameLine();
        ImGui::Text("%s\nHP: %d", colonist->currentAction.c_str(), colonist->mStats->hp);
    }

    ImGui::End();
}

void GameGUI::render()
{
	renderColonistsWindow();
}

