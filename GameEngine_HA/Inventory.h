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
