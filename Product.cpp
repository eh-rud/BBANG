#include "Product.h"
#include <algorithm>
#include <utility>

Product::Product(std::string name, ItemType type) : name_(std::move(name)), type_(type) {}
const std::string& Product::getName() const { return name_; }
ItemType Product::getType() const { return type_; }

RawMaterial::RawMaterial(std::string name, ItemType type) : Product(std::move(name), type) {}
std::string RawMaterial::getCategory() const { return "Raw Material"; }

IntermediateProduct::IntermediateProduct(std::string name, ItemType type) : Product(std::move(name), type) {}
std::string IntermediateProduct::getCategory() const { return "Intermediate Product"; }

FinishedGood::FinishedGood(std::string name, ItemType type) : Product(std::move(name), type) {}
std::string FinishedGood::getCategory() const { return "Finished Good"; }

WheatProduct::WheatProduct() : RawMaterial("Wheat", ItemType::Wheat) {}
FlourProduct::FlourProduct() : IntermediateProduct("Flour", ItemType::Flour) {}
DoughProduct::DoughProduct() : IntermediateProduct("Dough", ItemType::Dough) {}
BreadProduct::BreadProduct() : FinishedGood("Bread", ItemType::Bread) {}

ParcelProduct::ParcelProduct(int breadCount, std::string destination, int orderId)
    : FinishedGood(destination.empty() ? "Packed Order" : "Packed Order to " + destination, ItemType::Parcel),
      breadCount_(std::max(1, breadCount)), destination_(std::move(destination)), orderId_(orderId) {}
int ParcelProduct::getBreadCount() const { return breadCount_; }
const std::string& ParcelProduct::getDestination() const { return destination_; }
int ParcelProduct::getOrderId() const { return orderId_; }