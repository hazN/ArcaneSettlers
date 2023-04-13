#include "Inventory.h"
#include <iostream>

Inventory::Inventory() : mWeight(0), mMaxWeight(20) {
    InitializeCriticalSection(&mInventoryCriticalSection);
}

Inventory::Inventory(int maxWeight)
{
    InitializeCriticalSection(&mInventoryCriticalSection);

}

Inventory::~Inventory() {
    DeleteCriticalSection(&mInventoryCriticalSection);
}

bool Inventory::addItem(Item item) 
{
    LockInventory();
    if (mWeight + item.weight > mMaxWeight) {
        UnlockInventory();
        return false;
    }
    mItems.push_back(item);
    mWeight += item.weight;
    UnlockInventory();
    return true;
}

bool Inventory::addItem(Item item, int amount) 
{
    LockInventory();

    float totalWeight = item.weight * amount;
    if (mWeight + totalWeight > mMaxWeight) 
    {
        UnlockInventory();
        return false;
    }

    for (int i = 0; i < amount; i++) 
    {
        mItems.push_back(item);
        std::cout << "Added " << item.name << " to inventory" << std::endl;
    }

    mWeight += totalWeight;
    UnlockInventory();
    return true;
}

bool Inventory::removeItem(itemId id)
{
    LockInventory();
    for (std::vector<Item>::iterator itemIt = mItems.begin(); itemIt != mItems.end(); ++itemIt) 
    {
        if (itemIt->id == id)
        {
            mWeight -= itemIt->weight;
            mItems.erase(itemIt);
            UnlockInventory();
            return true;
        }
    }
    UnlockInventory();
    return false;
}

bool Inventory::removeItem(itemId id, int amount)
{
    LockInventory();

    std::vector<Item>::iterator itemIt = mItems.begin();
    int removedCount = 0;
    while (itemIt != mItems.end() && removedCount < amount) 
    {
        if (itemIt->id == id) {
            mWeight -= itemIt->weight;
            itemIt = mItems.erase(itemIt);
            removedCount++;
        }
        else 
        {
            ++itemIt;
        }
    }

    UnlockInventory();
    std::cout << "Removed " << removedCount << " of " << id << " from inventory" << std::endl;
    return removedCount == amount;
}

bool Inventory::isFull() 
{
    LockInventory();
    bool result = mWeight >= mMaxWeight;
    UnlockInventory();
    return result;
}

int Inventory::getCurrentWeight() 
{
    LockInventory();
    int result = mWeight;
    UnlockInventory();
    return result;
}

std::vector<Item> Inventory::getAllItems()
{
    return mItems;
}

int Inventory::getItemCount(itemId id)
{
    int count = 0;
    for (Item item : mItems)
    {
        if (item.id == id)
        {
            count++;
        }
    }
    return count;
}

void Inventory::LockInventory() 
{
    EnterCriticalSection(&mInventoryCriticalSection);
}

void Inventory::UnlockInventory() 
{
    LeaveCriticalSection(&mInventoryCriticalSection);
}
