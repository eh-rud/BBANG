#include "Machine.h"
#include "Factory.h"
#include <algorithm>
#include <utility>

Machine::Machine(int id, std::string name, std::string zone, int processTicks, float breakdownProbability)
    : id_(id), name_(std::move(name)), zone_(std::move(zone)), state_(MachineState::Idle),
      progress_(0.0f), health_(1.0f), outputCount_(0), baseProcessTicks_(processTicks),
      workTicks_(0), repairTicks_(5), repairWorkTicks_(0), breakdownProbability_(breakdownProbability), enabled_(true) {}

void Machine::markWorking() { state_ = MachineState::Working; }
void Machine::markIdle() { state_ = MachineState::Idle; }
void Machine::resetProgress() { progress_ = 0.0f; workTicks_ = 0; }
void Machine::addOutput() { ++outputCount_; }
bool Machine::isBottleneckTarget() const { return false; }

bool Machine::advanceWork(const Factory& factory) {
    const int requiredTicks = std::max(1, getProcessTicks(factory.getScenario()));
    ++workTicks_; progress_ = static_cast<float>(workTicks_) / static_cast<float>(requiredTicks);
    if (workTicks_ >= requiredTicks) { resetProgress(); return true; }
    return false;
}

int Machine::getId() const { return id_; }
const std::string& Machine::getName() const { return name_; }
const std::string& Machine::getZone() const { return zone_; }
MachineState Machine::getState() const { return state_; }
float Machine::getProgress() const { return progress_; }
float Machine::getHealth() const { return health_; }
int Machine::getOutputCount() const { return outputCount_; }
float Machine::getBreakdownProbability() const { return breakdownProbability_; }
bool Machine::isEnabled() const { return enabled_; }
int Machine::getProcessTicks(Scenario scenario) const { return (scenario == Scenario::Bottleneck && isBottleneckTarget()) ? 12 : baseProcessTicks_; }
bool Machine::isBreakable() const { return id_ != 1; }

bool Machine::pause() {
    if (state_ == MachineState::Broken || state_ == MachineState::Repairing || state_ == MachineState::Paused) return false;
    enabled_ = false; state_ = MachineState::Paused; return true;
}
bool Machine::resume() {
    if (state_ != MachineState::Paused) return false;
    enabled_ = true; state_ = MachineState::Idle; return true;
}
bool Machine::forceBreak() {
    if (!isBreakable() || state_ == MachineState::Broken || state_ == MachineState::Repairing) return false;
    enabled_ = true; state_ = MachineState::Broken; health_ = std::max(0.1f, health_ - 0.3f); resetProgress(); return true;
}
bool Machine::startRepair() {
    if (state_ != MachineState::Broken) return false;
    state_ = MachineState::Repairing; progress_ = 0.0f; repairWorkTicks_ = 0; return true;
}
void Machine::instantRepair() {
    enabled_ = true; state_ = MachineState::Idle; health_ = 1.0f; progress_ = 0.0f; workTicks_ = 0; repairWorkTicks_ = 0;
}
void Machine::update(Factory& factory) {
    if (!enabled_ || state_ == MachineState::Paused) { state_ = MachineState::Paused; return; }
    if (state_ == MachineState::Broken) return;
    if (state_ == MachineState::Repairing) {
        ++repairWorkTicks_; progress_ = static_cast<float>(repairWorkTicks_) / static_cast<float>(repairTicks_);
        if (repairWorkTicks_ >= repairTicks_) { instantRepair(); factory.addEvent(getName() + " repair completed.", EventType::Success); }
        return;
    }
    performWork(factory);
}
std::string Machine::getInfo() const { return name_ + " in " + zone_; }

void ProducerMachine::performWork(Factory& factory) {
    if (canProduce(factory)) { markWorking(); if (advanceWork(factory)) { produce(factory); addOutput(); } }
    else { markIdle(); resetProgress(); }
}
void ProcessorMachine::performWork(Factory& factory) {
    if (checkIngredients(factory)) { markWorking(); if (advanceWork(factory)) { process(factory); addOutput(); } }
    else { markIdle(); resetProgress(); }
}
void ConsumerMachine::performWork(Factory& factory) {
    if (canConsume(factory)) { markWorking(); if (advanceWork(factory)) { consume(factory); addOutput(); } }
    else { markIdle(); resetProgress(); }
}

FarmGrower::FarmGrower(int id, std::string name, std::string zone) : ProducerMachine(id, std::move(name), std::move(zone), 5, 0.0f) {}
bool FarmGrower::canProduce(const Factory& factory) const { return factory.shouldRunHarvest() && factory.hasReadyPlot() && factory.getWheatCount() < factory.getWheatCapacity(); }
void FarmGrower::produce(Factory& factory) { factory.autoHarvestOneReadyPlot(); factory.finishManualHarvestRequest(); }
QueueInfo FarmGrower::getQueueInfo(const Factory& factory) const { return {"Ready plots", factory.getReadyCount(), 20}; }

MillMachine::MillMachine(int id, std::string name, std::string zone) : ProcessorMachine(id, std::move(name), std::move(zone), 3, 0.02f) {}
bool MillMachine::checkIngredients(const Factory& factory) const { return factory.shouldRunMill() && factory.getWheatCount() > 0 && (factory.getFlourCount() < factory.getFlourCapacity() || factory.isOverflowScenario()); }
void MillMachine::process(Factory& factory) { factory.consumeWheat(); factory.produceFlour(); factory.addEvent("Mill Machine produced flour.", EventType::Production); factory.finishManualMillRequest(); }
QueueInfo MillMachine::getQueueInfo(const Factory& factory) const { return {"Input: Wheat", factory.getWheatCount(), factory.getWheatCapacity()}; }

DoughMixer::DoughMixer(int id, std::string name, std::string zone) : ProcessorMachine(id, std::move(name), std::move(zone), 5, 0.025f) {}
bool DoughMixer::checkIngredients(const Factory& factory) const { return factory.shouldRunMix() && factory.getFlourCount() > 0 && (factory.getDoughCount() < factory.getDoughCapacity() || factory.isOverflowScenario()); }
void DoughMixer::process(Factory& factory) { factory.consumeFlour(); factory.produceDough(); factory.addEvent("Dough Mixer produced dough.", EventType::Production); factory.finishManualMixRequest(); }
QueueInfo DoughMixer::getQueueInfo(const Factory& factory) const { return {"Input: Flour", factory.getFlourCount(), factory.getFlourCapacity()}; }

Oven::Oven(int id, std::string name, std::string zone) : ProcessorMachine(id, std::move(name), std::move(zone), 4, 0.03f) {}
bool Oven::isBottleneckTarget() const { return true; }
bool Oven::checkIngredients(const Factory& factory) const { return factory.shouldRunBake() && factory.getDoughCount() > 0 && (factory.getBreadCount() < factory.getBreadCapacity() || factory.isOverflowScenario()); }
void Oven::process(Factory& factory) { factory.consumeDough(); factory.produceBread(); factory.addEvent("Oven baked fresh bread.", EventType::Production); factory.finishManualBakeRequest(); }
QueueInfo Oven::getQueueInfo(const Factory& factory) const { return {"Input: Dough", factory.getDoughCount(), factory.getDoughCapacity()}; }

Packager::Packager(int id, std::string name, std::string zone) : ProcessorMachine(id, std::move(name), std::move(zone), 3, 0.02f) {}
bool Packager::checkIngredients(const Factory& factory) const { return factory.shouldRunPack() && factory.canPackNextBread(); }
void Packager::process(Factory& factory) { factory.packNextBread(); factory.finishManualPackRequest(); }
QueueInfo Packager::getQueueInfo(const Factory& factory) const { return {"Input: Bread", factory.getBreadCount(), factory.getBreadCapacity()}; }

DeliveryTruck::DeliveryTruck(int id, std::string name, std::string zone) : ConsumerMachine(id, std::move(name), std::move(zone), 4, 0.01f) {}
bool DeliveryTruck::canConsume(const Factory& factory) const { return factory.shouldRunDispatch() && factory.canLoadNextPackedOrder(); }
void DeliveryTruck::consume(Factory& factory) { factory.loadNextPackedOrderToTruck(); factory.finishManualDispatchRequest(); }
QueueInfo DeliveryTruck::getQueueInfo(const Factory& factory) const { return {"Truck Load", factory.getTruckLoad(), factory.getTruckCapacity()}; }

Worker::Worker(int id, std::string name, std::string role) : id_(id), name_(std::move(name)), role_(std::move(role)) {}
int Worker::getId() const { return id_; } const std::string& Worker::getName() const { return name_; }
const std::string& Worker::getRole() const { return role_; } std::string Worker::getInfo() const { return name_ + " (" + role_ + ")"; }

Technician::Technician(int id, std::string name) : Worker(id, std::move(name), "Technician") {}
void Technician::update(Factory& factory) {
    if (!factory.isAutoRepairScenario()) return;
    Machine* broken = factory.findFirstBrokenMachine();
    if (broken && broken->startRepair()) { factory.addEvent("Auto Technician dispatched to " + broken->getName() + ".", EventType::Maintenance); }
}