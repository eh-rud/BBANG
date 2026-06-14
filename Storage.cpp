#include "Storage.h"
#include <utility>

FarmPlot::FarmPlot() : state_(PlotState::Empty), growth_(0.0f) {}
PlotState FarmPlot::getState() const { return state_; }
float FarmPlot::getGrowth() const { return growth_; }
void FarmPlot::plant() {
    if (state_ != PlotState::Empty) return;
    state_ = PlotState::Seeded; growth_ = 0.02f;
}
void FarmPlot::grow(float amount) {
    if (state_ == PlotState::Seeded || state_ == PlotState::Growing) {
        state_ = PlotState::Growing; growth_ += amount;
        if (growth_ >= 1.0f) { growth_ = 1.0f; state_ = PlotState::Ready; }
    }
}
void FarmPlot::harvest() { state_ = PlotState::Empty; growth_ = 0.0f; }

StorageBin::StorageBin() : name_(""), itemType_(ItemType::Wheat), capacity_(20) {}
StorageBin::StorageBin(std::string name, ItemType itemType, int capacity)
    : name_(std::move(name)), itemType_(itemType), capacity_(capacity) {}
const std::string& StorageBin::getName() const { return name_; }
ItemType StorageBin::getItemType() const { return itemType_; }
int StorageBin::getCount() const { return static_cast<int>(items_.size()); }
int StorageBin::getCapacity() const { return capacity_; }
bool StorageBin::produce(std::unique_ptr<Product> product) {
    if (!product) return false;
    if (items_.size() >= static_cast<size_t>(capacity_)) return false;
    items_.push_back(std::move(product));
    return true;
}
std::unique_ptr<Product> StorageBin::consume() {
    if (items_.empty()) return nullptr;
    auto item = std::move(items_.back());
    items_.pop_back();
    return item;
}
const Product* StorageBin::peek() const {
    if (items_.empty()) return nullptr;
    return items_.back().get();
}
std::vector<const Product*> StorageBin::peekAll() const {
    std::vector<const Product*> products; products.reserve(items_.size());
    for (const auto& item : items_) products.push_back(item.get());
    return products;
}
void StorageBin::clear() { items_.clear(); }

OrderItem::OrderItem(int id, std::string city, int count, int priority)
    : id_(id), city_(std::move(city)), count_(count), priority_(priority), progress_(0.0f), packedCount_(0) {}
int OrderItem::getId() const { return id_; }
const std::string& OrderItem::getCity() const { return city_; }
int OrderItem::getCount() const { return count_; }
int OrderItem::getPriority() const { return priority_; }
float OrderItem::getProgress() const { return progress_; }
int OrderItem::getPackedCount() const { return packedCount_; }
bool OrderItem::packOne() {
    if (packedCount_ >= count_) return false;
    ++packedCount_; progress_ = static_cast<float>(packedCount_) / static_cast<float>(count_); return true;
}
bool OrderItem::isComplete() const { return packedCount_ >= count_; }
bool OrderItem::isLastBread() const { return packedCount_ + 1 >= count_; }

EventItem::EventItem(int tick, std::string text, EventType type) : tick_(tick), text_(std::move(text)), type_(type) {}
int EventItem::getTick() const { return tick_; }
const std::string& EventItem::getText() const { return text_; }
EventType EventItem::getType() const { return type_; }