#include "Inventory.h"
#include <iostream>

Inventory::Inventory() 
{
    mWeight = 0;
    mMaxWeight = 20;
    InitializeCriticalSection(&mInventoryCriticalSection);
}

Inventory::Inventory(int maxWeight)
{
    InitializeCriticalSection(&mInventoryCriticalSection);
    mWeight = 0;
    mMaxWeight = maxWeight;
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

int Inventory::addItem(Item item, int amount)
{
    LockInventory();

    int itemsAdded = 0;
    float totalWeight = item.weight * amount;

    // Check if all can be added
    if (mWeight + totalWeight <= mMaxWeight)
    {
        for (int i = 0; i < amount; i++)
        {
            mItems.push_back(item);
            itemsAdded++;
            std::cout << "Added " << item.name << " to inventory" << std::endl;
        }
        mWeight += totalWeight;
    }
    // Otherwise add as much as possible
    else
    {
        int maxItemsToAdd = (int)((mMaxWeight - mWeight) / item.weight);

        for (int i = 0; i < maxItemsToAdd; i++)
        {
            mItems.push_back(item);
            itemsAdded++;
            std::cout << "Added " << item.name << " to inventory" << std::endl;
        }
        mWeight += maxItemsToAdd * item.weight;
    }

    UnlockInventory();
    return itemsAdded;
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

int Inventory::removeItem(itemId id, int amount)
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
    return removedCount;
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

int Inventory::getMaxWeight()
{
    return mMaxWeight;
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
