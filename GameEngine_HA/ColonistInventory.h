#pragma once
#include <string>
#include <vector>
#include <Windows.h>

enum itemId {
    wood, stone, food, ores
};

struct Item {
    itemId id;
    std::string name;
    std::string icon;
    int weight;
};

class ColonistInventory {
public:
    ColonistInventory();
    ~ColonistInventory();

    bool addItem(Item& item);
    bool removeItem(std::string& itemName);
    bool isFull();
    int getCurrentWeight();
    std::vector<Item> getAllItems();

private:
    std::vector<Item> mItems;
    int mWeight;
    int mMaxWeight;
    CRITICAL_SECTION mInventoryCriticalSection;

    void LockInventory();
    void UnlockInventory();
};
