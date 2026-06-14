#pragma once
#include "Types.h"
#include <string>

class Product {
private:
    std::string name_;
    ItemType type_;
public:
    Product(std::string name, ItemType type);
    virtual ~Product() = default;
    const std::string& getName() const;
    ItemType getType() const;
    virtual std::string getCategory() const = 0;
};

class RawMaterial : public Product {
public:
    RawMaterial(std::string name, ItemType type);
    std::string getCategory() const override;
};

class IntermediateProduct : public Product {
public:
    IntermediateProduct(std::string name, ItemType type);
    std::string getCategory() const override;
};

class FinishedGood : public Product {
public:
    FinishedGood(std::string name, ItemType type);
    std::string getCategory() const override;
};

class WheatProduct : public RawMaterial { public: WheatProduct(); };
class FlourProduct : public IntermediateProduct { public: FlourProduct(); };
class DoughProduct : public IntermediateProduct { public: DoughProduct(); };
class BreadProduct : public FinishedGood { public: BreadProduct(); };

class ParcelProduct : public FinishedGood {
private:
    int breadCount_; std::string destination_; int orderId_;
public:
    explicit ParcelProduct(int breadCount = 1, std::string destination = "", int orderId = 0);
    int getBreadCount() const;
    const std::string& getDestination() const;
    int getOrderId() const;
};