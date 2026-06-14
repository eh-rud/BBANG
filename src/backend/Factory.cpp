#include "Factory.h"
#include "Product.h"
#include <algorithm>
#include <utility>

Factory::Factory()
    : running_(false), currentTick_(0), tickAccumulator_(0.0f), speed_(2), scenario_(Scenario::Normal),
      finishedBreadCount_(0), dispatchedParcelCount_(0), soldBreadCount_(0), truckLoad_(0), truckCapacity_(50), truckOrderCount_(0), lostProducts_(0), totalBreakdowns_(0),
      nextOrderId_(1), autoPlantEnabled_(false), autoHarvestEnabled_(false), autoMillEnabled_(false), autoMixEnabled_(false), autoBakeEnabled_(false), autoPackEnabled_(false), autoDispatchEnabled_(false), manualHarvestRequested_(false), manualMillRequested_(false), manualMixRequested_(false), manualBakeRequested_(false), manualPackRequested_(false), manualDispatchRequested_(false), nextPlantSearchIndex_(0), nextHarvestSearchIndex_(0), probability_(0.0f, 1.0f) {
    std::random_device rd;
    rng_ = std::mt19937(rd());
    reset();
}

void Factory::reset() {
    running_ = false; currentTick_ = 0; tickAccumulator_ = 0.0f; speed_ = 2; scenario_ = Scenario::Normal;
    finishedBreadCount_ = 0; dispatchedParcelCount_ = 0; soldBreadCount_ = 0; truckLoad_ = 0; truckCapacity_ = 50; truckOrderCount_ = 0; lostProducts_ = 0; totalBreakdowns_ = 0; 
    nextOrderId_ = 1;
    
    autoPlantEnabled_ = false; autoHarvestEnabled_ = false; autoMillEnabled_ = false; autoMixEnabled_ = false; autoBakeEnabled_ = false; autoPackEnabled_ = false; autoDispatchEnabled_ = false;
    manualHarvestRequested_ = false; manualMillRequested_ = false; manualMixRequested_ = false; manualBakeRequested_ = false; manualPackRequested_ = false; manualDispatchRequested_ = false;
    nextPlantSearchIndex_ = 0; nextHarvestSearchIndex_ = 0;

    wheat_ = StorageBin("Wheat Storage", ItemType::Wheat, 20); flour_ = StorageBin("Flour Silo", ItemType::Flour, 20);
    dough_ = StorageBin("Dough Queue", ItemType::Dough, 20); bread_ = StorageBin("Bread Shelf", ItemType::Bread, 20); parcels_ = StorageBin("Packed Orders", ItemType::Parcel, 20);

    for (auto& plot : plots_) plot = FarmPlot();

    objects_.clear(); machineViews_.clear();
    addMachine<FarmGrower>(1, "SmartFarm Grower", "Farm"); addMachine<MillMachine>(2, "Mill Machine", "Bakery");
    addMachine<DoughMixer>(3, "Dough Mixer", "Bakery"); addMachine<Oven>(4, "Oven", "Bakery");
    addMachine<Packager>(5, "Packaging Machine", "Fulfillment"); addMachine<DeliveryTruck>(6, "Delivery Truck", "Fulfillment");
    objects_.push_back(std::make_unique<Technician>(7, "Auto Technician"));

    orders_.clear(); events_.clear();
    addEvent("Factory initialized. Plant wheat, press Start, then submit an order.", EventType::Info);
}

void Factory::update(float deltaTime) {
    if (!running_) return;
    tickAccumulator_ += deltaTime * static_cast<float>(speed_) * 1.4f;
    while (tickAccumulator_ >= 1.0f) { tickAccumulator_ -= 1.0f; stepTick(); }
}

void Factory::applyCommand(const FactoryCommand& command) {
    if (command.reset) { reset(); return; }
    if (command.toggleRunning) { running_ = !running_; addEvent(running_ ? "Simulation started." : "Simulation paused.", running_ ? EventType::Success : EventType::Warning); }
    if (command.changeSpeed) speed_ = std::max(1, std::min(5, command.speed));
    if (command.changeScenario) { scenario_ = command.scenario; addEvent("Scenario changed.", EventType::Info); }
    if (command.clearLog) clearEvents();
    if (command.clickedPlot >= 0) {
        const int index = command.clickedPlot;
        if (index >= 0 && index < static_cast<int>(plots_.size())) {
            if (plots_[index].getState() == PlotState::Empty) plantSeed(index);
            else if (plots_[index].getState() == PlotState::Ready) harvestPlot(index);
        }
    }
    if (command.plantAll) plantAllEmpty();
    if (command.harvestReady) harvestAllReady();
    if (command.toggleAutoPlant) { autoPlantEnabled_ = !autoPlantEnabled_; logAutomationToggle("Auto Plant", autoPlantEnabled_); }
    if (command.toggleAutoHarvest) { autoHarvestEnabled_ = !autoHarvestEnabled_; logAutomationToggle("Auto Harvest", autoHarvestEnabled_); }
    if (command.toggleAutoMill) { autoMillEnabled_ = !autoMillEnabled_; logAutomationToggle("Auto Mill", autoMillEnabled_); }
    if (command.toggleAutoMix) { autoMixEnabled_ = !autoMixEnabled_; logAutomationToggle("Auto Mix", autoMixEnabled_); }
    if (command.toggleAutoBake) { autoBakeEnabled_ = !autoBakeEnabled_; logAutomationToggle("Auto Bake", autoBakeEnabled_); }
    if (command.toggleAutoPack) { autoPackEnabled_ = !autoPackEnabled_; logAutomationToggle("Auto Pack", autoPackEnabled_); }
    if (command.toggleAutoDispatch) { autoDispatchEnabled_ = !autoDispatchEnabled_; logAutomationToggle("Auto Dispatch", autoDispatchEnabled_); }
    if (command.manualHarvestOne) runHarvestOnce(); if (command.manualMillOne) runMillOnce();
    if (command.manualMixOne) runMixOnce(); if (command.manualBakeOne) runBakeOnce();
    if (command.manualPackOne) runPackOnce(); if (command.manualDispatchOne) runDispatchOnce();
    if (command.dispatchTruckNow) dispatchTruck();
    if (command.addOrder) addOrder(command.orderCity, command.orderQuantity);
    if (command.targetMachineId >= 0) {
        if (command.pauseMachine) pauseMachine(command.targetMachineId);
        if (command.resumeMachine) resumeMachine(command.targetMachineId);
        if (command.forceBreak) forceBreakMachine(command.targetMachineId);
        if (command.instantRepair) instantRepairMachine(command.targetMachineId);
        if (command.dispatchTechnician) dispatchTechnician(command.targetMachineId);
    }
}

FactorySnapshot Factory::createSnapshot() const {
    FactorySnapshot snap;
    snap.running = running_; snap.currentTick = currentTick_; snap.speed = speed_; snap.scenario = scenario_;
    snap.finishedBreadCount = finishedBreadCount_; snap.dispatchedParcelCount = dispatchedParcelCount_; snap.soldBreadCount = soldBreadCount_;
    snap.truckLoad = truckLoad_; snap.truckCapacity = truckCapacity_; snap.truckOrderCount = truckOrderCount_; snap.lostProducts = lostProducts_; snap.totalBreakdowns = totalBreakdowns_;
    snap.plantedCount = getPlantedCount(); snap.readyCount = getReadyCount(); snap.workInProgressCount = getWorkInProgressCount(); snap.brokenMachineCount = getBrokenMachineCount();
    snap.autoPlantEnabled = autoPlantEnabled_; snap.autoHarvestEnabled = autoHarvestEnabled_; snap.autoMillEnabled = autoMillEnabled_;
    snap.autoMixEnabled = autoMixEnabled_; snap.autoBakeEnabled = autoBakeEnabled_; snap.autoPackEnabled = autoPackEnabled_; snap.autoDispatchEnabled = autoDispatchEnabled_;

    for (size_t i = 0; i < plots_.size(); ++i) { snap.plots[i].state = plots_[i].getState(); snap.plots[i].growth = plots_[i].getGrowth(); }

    auto storageToSnapshot = [](const StorageBin& bin) {
        StorageSnapshot storage; storage.name = bin.getName(); storage.itemType = bin.getItemType(); storage.count = bin.getCount(); storage.capacity = bin.getCapacity(); return storage;
    };
    snap.wheat = storageToSnapshot(wheat_); snap.flour = storageToSnapshot(flour_); snap.dough = storageToSnapshot(dough_); snap.bread = storageToSnapshot(bread_); snap.parcels = storageToSnapshot(parcels_);

    for (const Machine* machine : machineViews_) {
        if (!machine) continue;
        MachineSnapshot machineSnap;
        machineSnap.id = machine->getId(); machineSnap.name = machine->getName(); machineSnap.zone = machine->getZone(); machineSnap.state = machine->getState();
        machineSnap.progress = machine->getProgress(); machineSnap.health = machine->getHealth(); machineSnap.outputCount = machine->getOutputCount();
        machineSnap.processTicks = machine->getProcessTicks(scenario_); machineSnap.breakdownProbability = machine->getBreakdownProbability();
        machineSnap.enabled = machine->isEnabled(); machineSnap.breakable = machine->isBreakable(); machineSnap.queue = machine->getQueueInfo(*this);
        snap.machines.push_back(machineSnap);
    }
    for (const OrderItem& order : orders_) {
        OrderSnapshot orderSnap;
        orderSnap.id = order.getId(); orderSnap.city = order.getCity(); orderSnap.count = order.getCount();
        orderSnap.priority = order.getPriority(); orderSnap.progress = order.getProgress(); orderSnap.packedCount = order.getPackedCount();
        snap.orders.push_back(orderSnap);
    }
    for (const Product* product : parcels_.peekAll()) {
        const auto* packed = dynamic_cast<const ParcelProduct*>(product);
        if (!packed) continue;
        TruckLoadSnapshot packedSnap;
        packedSnap.orderId = packed->getOrderId(); packedSnap.destination = packed->getDestination(); packedSnap.breadCount = packed->getBreadCount();
        snap.packedOrders.push_back(packedSnap);
    }
    snap.truckLoads = truckLoads_;
    for (const EventItem& event : events_) {
        EventSnapshot eventSnap;
        eventSnap.tick = event.getTick(); eventSnap.text = event.getText(); eventSnap.type = event.getType();
        snap.events.push_back(eventSnap);
    }
    return snap;
}

void Factory::stepTick() {
    ++currentTick_;
    for (auto& plot : plots_) plot.grow(0.045f);
    if (autoPlantEnabled_) autoPlantEmptyPlots();
    maybeTriggerRandomBreakdown();
    for (auto& object : objects_) object->update(*this);
    if (autoDispatchEnabled_ && truckLoad_ > 0) {
        if (truckLoad_ >= truckCapacity_ || (getParcelCount() > 0 && !canLoadNextPackedOrder())) dispatchTruck();
    }
    if (scenario_ == Scenario::Overflow && currentTick_ % 30 == 0 && wheat_.getCount() >= wheat_.getCapacity()) {
        registerOverflow("Wheat Storage overflow: unharvested wheat was lost.");
    }
}

void Factory::maybeTriggerRandomBreakdown() {
    if (!isBreakdownScenario() || currentTick_ % 10 != 0) return;
    for (Machine* machine : machineViews_) {
        if (!machine || !machine->isBreakable() || machine->getState() != MachineState::Working) continue;
        if (probability_(rng_) < 0.06f) {
            if (machine->forceBreak()) { ++totalBreakdowns_; addEvent("Random breakdown occurred in " + machine->getName() + " (6% check).", EventType::Breakdown); }
            break;
        }
    }
}

void Factory::registerOverflow(const std::string& message) { ++lostProducts_; addEvent(message, EventType::Warning); }
bool Factory::isRunning() const { return running_; } int Factory::getCurrentTick() const { return currentTick_; }
int Factory::getSpeed() const { return speed_; } Scenario Factory::getScenario() const { return scenario_; }
int Factory::getFinishedBreadCount() const { return finishedBreadCount_; } int Factory::getDispatchedParcelCount() const { return dispatchedParcelCount_; }
int Factory::getSoldBreadCount() const { return soldBreadCount_; } int Factory::getTruckLoad() const { return truckLoad_; }
int Factory::getTruckCapacity() const { return truckCapacity_; } int Factory::getTruckOrderCount() const { return truckOrderCount_; }
int Factory::getLostProducts() const { return lostProducts_; } int Factory::getTotalBreakdowns() const { return totalBreakdowns_; }
bool Factory::isAutoPlantEnabled() const { return autoPlantEnabled_; } bool Factory::isAutoHarvestEnabled() const { return autoHarvestEnabled_; }
bool Factory::isAutoMillEnabled() const { return autoMillEnabled_; } bool Factory::isAutoMixEnabled() const { return autoMixEnabled_; }
bool Factory::isAutoBakeEnabled() const { return autoBakeEnabled_; } bool Factory::isAutoPackEnabled() const { return autoPackEnabled_; }
bool Factory::isAutoDispatchEnabled() const { return autoDispatchEnabled_; }
bool Factory::shouldRunHarvest() const { return autoHarvestEnabled_ || manualHarvestRequested_; }
bool Factory::shouldRunMill() const { return autoMillEnabled_ || manualMillRequested_; }
bool Factory::shouldRunMix() const { return autoMixEnabled_ || manualMixRequested_; }
bool Factory::shouldRunBake() const { return autoBakeEnabled_ || manualBakeRequested_; }
bool Factory::shouldRunPack() const { return autoPackEnabled_ || manualPackRequested_; }
bool Factory::shouldRunDispatch() const { return autoDispatchEnabled_ || manualDispatchRequested_; }
const std::array<FarmPlot, 20>& Factory::getPlots() const { return plots_; }
const StorageBin& Factory::getWheat() const { return wheat_; } const StorageBin& Factory::getFlour() const { return flour_; }
const StorageBin& Factory::getDough() const { return dough_; } const StorageBin& Factory::getBread() const { return bread_; }
const StorageBin& Factory::getParcels() const { return parcels_; }
const std::vector<Machine*>& Factory::getMachines() const { return machineViews_; }
const std::vector<OrderItem>& Factory::getOrders() const { return orders_; }
const std::vector<EventItem>& Factory::getEvents() const { return events_; }
void Factory::addEvent(const std::string& text, EventType type) { events_.insert(events_.begin(), EventItem(currentTick_, text, type)); if (events_.size() > 100) events_.pop_back(); }
void Factory::clearEvents() { events_.clear(); }
void Factory::logAutomationToggle(const std::string& name, bool enabled) { addEvent(name + std::string(enabled ? " enabled." : " disabled."), enabled ? EventType::Success : EventType::Warning); }

void Factory::autoPlantEmptyPlots() {
    for (size_t i = 0; i < plots_.size(); ++i) {
        size_t idx = (nextPlantSearchIndex_ + i) % plots_.size();
        if (plots_[idx].getState() == PlotState::Empty) {
            plots_[idx].plant(); addEvent("Auto Plant seeded plot #" + std::to_string(idx + 1) + ".", EventType::Production);
            nextPlantSearchIndex_ = (idx + 1) % plots_.size(); return;
        }
    }
}
void Factory::plantSeed(int index) {
    if (index >= 0 && index < static_cast<int>(plots_.size()) && plots_[index].getState() == PlotState::Empty) {
        plots_[index].plant(); addEvent("Seed planted in SmartFarm.", EventType::Success);
    }
}
void Factory::harvestPlot(int index) {
    if (index >= 0 && index < static_cast<int>(plots_.size()) && plots_[index].getState() == PlotState::Ready) {
        if (wheat_.getCount() >= wheat_.getCapacity()) {
            if (isOverflowScenario()) registerOverflow("Wheat Storage full: harvested wheat was lost.");
            else addEvent("Wheat Storage is full.", EventType::Warning);
            return;
        }
        plots_[index].harvest(); wheat_.produce(std::make_unique<WheatProduct>()); addEvent("Harvested wheat moved into storage.", EventType::Production);
    }
}
void Factory::plantAllEmpty() {
    int planted = 0;
    for (auto& plot : plots_) { if (plot.getState() == PlotState::Empty) { plot.plant(); ++planted; } }
    if (planted > 0) addEvent("All empty plots received seeds.", EventType::Success);
}
void Factory::harvestAllReady() {
    int harvested = 0;
    for (int i = 0; i < static_cast<int>(plots_.size()); ++i) {
        if (plots_[i].getState() == PlotState::Ready && wheat_.getCount() < wheat_.getCapacity()) {
            plots_[i].harvest(); wheat_.produce(std::make_unique<WheatProduct>()); ++harvested;
        }
    }
    if (harvested > 0) addEvent("Ready wheat plots were harvested.", EventType::Production);
    else if (wheat_.getCount() >= wheat_.getCapacity()) addEvent("Wheat Storage is full.", EventType::Warning);
}

bool Factory::autoHarvestOneReadyPlot() {
    for (size_t i = 0; i < plots_.size(); ++i) {
        size_t idx = (nextHarvestSearchIndex_ + i) % plots_.size();
        if (plots_[idx].getState() == PlotState::Ready && wheat_.getCount() < wheat_.getCapacity()) {
            plots_[idx].harvest(); wheat_.produce(std::make_unique<WheatProduct>());
            addEvent("SmartFarm Grower harvested wheat.", EventType::Production);
            nextHarvestSearchIndex_ = (idx + 1) % plots_.size(); return true;
        }
    }
    if (hasReadyPlot() && wheat_.getCount() >= wheat_.getCapacity()) addEvent("Wheat Storage is full.", EventType::Warning);
    return false;
}
bool Factory::runHarvestOnce() {
    if (manualHarvestRequested_) { addEvent("Manual Harvest is already queued.", EventType::Info); return false; }
    if (!hasReadyPlot()) { addEvent("Manual Harvest: no ready wheat plot.", EventType::Warning); return false; }
    if (getWheatCount() >= getWheatCapacity()) { addEvent("Manual Harvest: Wheat Storage is full.", EventType::Warning); return false; }
    manualHarvestRequested_ = true; addEvent("Manual Harvest queued: SmartFarm Grower will harvest 1 plot.", EventType::Info); return true;
}
bool Factory::runMillOnce() {
    if (manualMillRequested_) { addEvent("Manual Mill is already queued.", EventType::Info); return false; }
    if (getWheatCount() <= 0) { addEvent("Manual Mill: no wheat available.", EventType::Warning); return false; }
    if (getFlourCount() >= getFlourCapacity() && !isOverflowScenario()) { addEvent("Manual Mill: Flour Silo is full.", EventType::Warning); return false; }
    manualMillRequested_ = true; addEvent("Manual Mill queued: Mill Machine will process 1 wheat.", EventType::Info); return true;
}
bool Factory::runMixOnce() {
    if (manualMixRequested_) { addEvent("Manual Mix is already queued.", EventType::Info); return false; }
    if (getFlourCount() <= 0) { addEvent("Manual Mix: no flour available.", EventType::Warning); return false; }
    if (getDoughCount() >= getDoughCapacity() && !isOverflowScenario()) { addEvent("Manual Mix: Dough Queue is full.", EventType::Warning); return false; }
    manualMixRequested_ = true; addEvent("Manual Mix queued: Dough Mixer will process 1 flour.", EventType::Info); return true;
}
bool Factory::runBakeOnce() {
    if (manualBakeRequested_) { addEvent("Manual Bake is already queued.", EventType::Info); return false; }
    if (getDoughCount() <= 0) { addEvent("Manual Bake: no dough available.", EventType::Warning); return false; }
    if (getBreadCount() >= getBreadCapacity() && !isOverflowScenario()) { addEvent("Manual Bake: Bread Shelf is full.", EventType::Warning); return false; }
    manualBakeRequested_ = true; addEvent("Manual Bake queued: Oven will bake 1 dough.", EventType::Info); return true;
}
bool Factory::runPackOnce() {
    if (manualPackRequested_) { addEvent("Manual Pack is already queued.", EventType::Info); return false; }
    if (!canPackNextBread()) { addEvent("Manual Pack: needs an open order, bread, and packed-order space.", EventType::Warning); return false; }
    manualPackRequested_ = true; addEvent("Manual Pack queued: Packaging Machine will pack 1 bread.", EventType::Info); return true;
}
bool Factory::runDispatchOnce() {
    if (manualDispatchRequested_) { addEvent("Manual Truck Load is already queued.", EventType::Info); return false; }
    if (getParcelCount() <= 0) { addEvent("Manual Truck Load: no packed order ready.", EventType::Warning); return false; }
    if (!canLoadNextPackedOrder()) { addEvent("Manual Truck Load: next order does not fit. Dispatch the truck first.", EventType::Warning); return false; }
    manualDispatchRequested_ = true; addEvent("Manual Truck Load queued: Delivery Truck will load 1 packed order.", EventType::Info); return true;
}

void Factory::finishManualHarvestRequest() { manualHarvestRequested_ = false; }
void Factory::finishManualMillRequest() { manualMillRequested_ = false; }
void Factory::finishManualMixRequest() { manualMixRequested_ = false; }
void Factory::finishManualBakeRequest() { manualBakeRequested_ = false; }
void Factory::finishManualPackRequest() { manualPackRequested_ = false; }
void Factory::finishManualDispatchRequest() { manualDispatchRequested_ = false; }

void Factory::addOrder(const std::string& city, int count) {
    if (city.empty()) return;
    count = std::max(1, std::min(50, count));
    const int priority = (count >= 24) ? 3 : (count >= 12 ? 2 : 1);
    orders_.push_back(OrderItem(nextOrderId_++, city, count, priority));
    addEvent("Manual order placed for " + city + ".", EventType::Info);
}

void Factory::pauseMachine(int id) { Machine* machine = findMachine(id); if (machine && machine->pause()) addEvent(machine->getName() + " paused by operator.", EventType::Warning); }
void Factory::resumeMachine(int id) { Machine* machine = findMachine(id); if (machine && machine->resume()) addEvent(machine->getName() + " resumed by operator.", EventType::Maintenance); }
void Factory::forceBreakMachine(int id) { Machine* machine = findMachine(id); if (machine && machine->forceBreak()) { ++totalBreakdowns_; addEvent("Forced breakdown: " + machine->getName() + ".", EventType::Breakdown); } }
void Factory::instantRepairMachine(int id) { Machine* machine = findMachine(id); if (machine) { machine->instantRepair(); addEvent("Instant repair completed for " + machine->getName() + ".", EventType::Maintenance); } }
void Factory::dispatchTechnician(int id) { Machine* machine = findMachine(id); if (machine && machine->startRepair()) addEvent("Technician dispatched to " + machine->getName() + ".", EventType::Maintenance); }

int Factory::getPlantedCount() const { int count = 0; for (const auto& plot : plots_) { if (plot.getState() != PlotState::Empty) ++count; } return count; }
int Factory::getReadyCount() const { int count = 0; for (const auto& plot : plots_) { if (plot.getState() == PlotState::Ready) ++count; } return count; }
int Factory::getWorkInProgressCount() const { return getPlantedCount() + wheat_.getCount() + flour_.getCount() + dough_.getCount() + bread_.getCount() + parcels_.getCount(); }
int Factory::getBrokenMachineCount() const { int count = 0; for (const Machine* machine : machineViews_) { if (machine && machine->getState() == MachineState::Broken) ++count; } return count; }
bool Factory::hasReadyPlot() const { for (const auto& plot : plots_) { if (plot.getState() == PlotState::Ready) return true; } return false; }
bool Factory::isOverflowScenario() const { return scenario_ == Scenario::Overflow; }
bool Factory::isBreakdownScenario() const { return scenario_ == Scenario::BreakdownManual || scenario_ == Scenario::BreakdownAuto; }
bool Factory::isAutoRepairScenario() const { return scenario_ == Scenario::BreakdownAuto; }

bool Factory::consumeWheat() { return wheat_.consume() != nullptr; }
bool Factory::produceFlour() { if (flour_.produce(std::make_unique<FlourProduct>())) return true; if (isOverflowScenario()) registerOverflow("Flour Silo overflow: flour was lost."); return false; }
bool Factory::consumeFlour() { return flour_.consume() != nullptr; }
bool Factory::produceDough() { if (dough_.produce(std::make_unique<DoughProduct>())) return true; if (isOverflowScenario()) registerOverflow("Dough Queue overflow: dough was lost."); return false; }
bool Factory::consumeDough() { return dough_.consume() != nullptr; }
bool Factory::produceBread() { if (bread_.produce(std::make_unique<BreadProduct>())) { ++finishedBreadCount_; return true; } if (isOverflowScenario()) registerOverflow("Bread Shelf overflow: fresh bread was lost."); return false; }
bool Factory::consumeBread() { return bread_.consume() != nullptr; }
bool Factory::produceParcel(int breadCount, const std::string& destination, int orderId) {
    if (parcels_.produce(std::make_unique<ParcelProduct>(breadCount, destination, orderId))) return true;
    if (isOverflowScenario()) registerOverflow("Packed Orders overflow: packed order was lost."); else addEvent("Packed Orders buffer is full.", EventType::Warning); return false;
}
bool Factory::consumeParcel() { return loadNextPackedOrderToTruck(); }

bool Factory::canLoadNextPackedOrder() const {
    const auto* packed = dynamic_cast<const ParcelProduct*>(parcels_.peek());
    if (!packed) return false;
    return truckLoad_ + packed->getBreadCount() <= truckCapacity_;
}
bool Factory::loadNextPackedOrderToTruck() {
    const auto* preview = dynamic_cast<const ParcelProduct*>(parcels_.peek());
    if (!preview) { addEvent("Delivery Truck: no packed order waiting.", EventType::Warning); return false; }
    if (truckLoad_ + preview->getBreadCount() > truckCapacity_) { addEvent("Delivery Truck is full enough. Dispatch it before loading the next order.", EventType::Warning); return false; }

    auto parcel = parcels_.consume();
    auto* packed = dynamic_cast<ParcelProduct*>(parcel.get());
    const int breadCount = packed ? packed->getBreadCount() : 1;
    const std::string destination = packed ? packed->getDestination() : std::string();
    const int orderId = packed ? packed->getOrderId() : 0;

    truckLoad_ += breadCount; ++truckOrderCount_;
    TruckLoadSnapshot load; load.orderId = orderId; load.destination = destination; load.breadCount = breadCount;
    truckLoads_.push_back(load);

    std::string label = "Loaded " + std::string(orderId > 0 ? "order #" + std::to_string(orderId) : "order") +
                        (destination.empty() ? "" : " for " + destination) + " (" + std::to_string(breadCount) + " bread) onto Delivery Truck (" +
                        std::to_string(truckLoad_) + "/" + std::to_string(truckCapacity_) + ").";
    addEvent(label, EventType::Production);
    if (autoDispatchEnabled_ && truckLoad_ >= truckCapacity_) dispatchTruck();
    return true;
}
bool Factory::dispatchTruck() {
    if (truckLoad_ <= 0) { addEvent("Delivery Truck is empty.", EventType::Warning); return false; }
    ++dispatchedParcelCount_; soldBreadCount_ += truckLoad_;
    std::string detail;
    for (size_t i = 0; i < truckLoads_.size(); ++i) {
        if (i > 0) detail += ", ";
        if (truckLoads_[i].orderId > 0) detail += "#" + std::to_string(truckLoads_[i].orderId) + " ";
        detail += truckLoads_[i].destination.empty() ? "Unknown" : truckLoads_[i].destination;
        detail += " " + std::to_string(truckLoads_[i].breadCount);
    }
    addEvent("Delivery Truck dispatched with " + std::to_string(truckLoad_) + " bread across " + std::to_string(truckOrderCount_) + " order(s)" + (detail.empty() ? "." : ": " + detail + "."), EventType::Success);
    truckLoad_ = 0; truckOrderCount_ = 0; truckLoads_.clear(); return true;
}

bool Factory::canPackNextBread() const { return !orders_.empty() && bread_.getCount() > 0 && (parcels_.getCount() < parcels_.getCapacity() || isOverflowScenario()); }
void Factory::packNextBread() {
    if (!canPackNextBread()) return;
    bread_.consume();
    OrderItem& order = orders_.front();
    order.packOne();
    if (order.isComplete()) {
        if (produceParcel(order.getCount(), order.getCity(), order.getId())) addEvent("Order #" + std::to_string(order.getId()) + " for " + order.getCity() + " finished packaging and is waiting in Packed Orders.", EventType::Production);
        orders_.erase(orders_.begin());
    } else if (order.getPackedCount() % 10 == 0) {
        addEvent("Packaging Machine continues packing order #" + std::to_string(order.getId()) + "...", EventType::Info);
    }
}

int Factory::getWheatCount() const { return wheat_.getCount(); } int Factory::getWheatCapacity() const { return wheat_.getCapacity(); }
int Factory::getFlourCount() const { return flour_.getCount(); } int Factory::getFlourCapacity() const { return flour_.getCapacity(); }
int Factory::getDoughCount() const { return dough_.getCount(); } int Factory::getDoughCapacity() const { return dough_.getCapacity(); }
int Factory::getBreadCount() const { return bread_.getCount(); } int Factory::getBreadCapacity() const { return bread_.getCapacity(); }
int Factory::getParcelCount() const { return parcels_.getCount(); } int Factory::getParcelCapacity() const { return parcels_.getCapacity(); }
Machine* Factory::findMachine(int id) { for (Machine* m : machineViews_) { if (m && m->getId() == id) return m; } return nullptr; }
const Machine* Factory::findMachine(int id) const { for (const Machine* m : machineViews_) { if (m && m->getId() == id) return m; } return nullptr; }
Machine* Factory::findFirstBrokenMachine() { for (Machine* m : machineViews_) { if (m && m->getState() == MachineState::Broken) return m; } return nullptr; }