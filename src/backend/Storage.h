#pragma once
#include "Types.h"
#include "Product.h"
#include <string>
#include <vector>
#include <memory>

class FarmPlot {
private:
    PlotState state_;
    float growth_;
public:
    FarmPlot();
    PlotState getState() const;
    float getGrowth() const;
    void plant();
    void grow(float amount);
    void harvest();
};

class StorageBin {
private:
    std::string name_; ItemType itemType_; int capacity_;
    std::vector<std::unique_ptr<Product>> items_;
public:
    StorageBin();
    StorageBin(std::string name, ItemType itemType, int capacity);
    const std::string& getName() const; ItemType getItemType() const;
    int getCount() const; int getCapacity() const;
    bool produce(std::unique_ptr<Product> product);
    std::unique_ptr<Product> consume();
    const Product* peek() const;
    std::vector<const Product*> peekAll() const;
    void clear();
};

class OrderItem {
private:
    int id_; std::string city_; int count_; int priority_; float progress_; int packedCount_;
public:
    OrderItem(int id, std::string city, int count, int priority);
    int getId() const; const std::string& getCity() const; int getCount() const;
    int getPriority() const; float getProgress() const; int getPackedCount() const;
    bool packOne(); bool isComplete() const; bool isLastBread() const;
};

class EventItem {
private:
    int tick_; std::string text_; EventType type_;
public:
    EventItem(int tick, std::string text, EventType type);
    int getTick() const; const std::string& getText() const; EventType getType() const;
};