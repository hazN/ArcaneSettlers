#pragma once
#include <string>
#include <vector>
#include <Windows.h>

enum itemId {
    wood, stone, food, ores
};

struct Item {
    Item()
    {
        this->id = wood;
        this->name = "Unset item...";
        this->icon = "";
        this->weight = 0;
    }
    Item(itemId id, std::string name, std::string icon, int weight)
    {
        this->id = id;
        this->name = name;
        this->icon = icon;
        this->weight = weight;
    }
    itemId id;
    std::string name;
    std::string icon;
    int weight;
};

class Inventory {
public:
    Inventory();
    Inventory(int maxWeight);
    ~Inventory();

    bool addItem(Item item);
    bool addItem(Item item, int amount);
    bool removeItem(itemId id);
    int removeItem(itemId id, int amount);
    bool isFull();
    int getCurrentWeight();
    int getMaxWeight();
    std::vector<Item> getAllItems();
    int getItemCount(itemId id);

private:
    std::vector<Item> mItems;
    int mWeight;
    int mMaxWeight;
    CRITICAL_SECTION mInventoryCriticalSection;

    void LockInventory();
    void UnlockInventory();
};
