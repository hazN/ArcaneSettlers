#include "ColonistInventory.h"

ColonistInventory::ColonistInventory() : mWeight(0), mMaxWeight(20) {
    InitializeCriticalSection(&mInventoryCriticalSection);
}

ColonistInventory::~ColonistInventory() {
    DeleteCriticalSection(&mInventoryCriticalSection);
}

bool ColonistInventory::addItem(Item& item) {
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

bool ColonistInventory::removeItem(std::string& itemName) {
    LockInventory();
    for (std::vector<Item>::iterator itemIt = mItems.begin(); itemIt != mItems.end(); ++itemIt) {
        if (itemIt->name == itemName) {
            mWeight -= itemIt->weight;
            mItems.erase(itemIt);
            UnlockInventory();
            return true;
        }
    }
    UnlockInventory();
    return false;
}

bool ColonistInventory::isFull() {
    LockInventory();
    bool result = mWeight >= mMaxWeight;
    UnlockInventory();
    return result;
}

int ColonistInventory::getCurrentWeight() {
    LockInventory();
    int result = mWeight;
    UnlockInventory();
    return result;
}

std::vector<Item> ColonistInventory::getAllItems() {
    return mItems;
}

void ColonistInventory::LockInventory() {
    EnterCriticalSection(&mInventoryCriticalSection);
}

void ColonistInventory::UnlockInventory() {
    LeaveCriticalSection(&mInventoryCriticalSection);
}
