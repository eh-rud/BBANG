#pragma once

#include <string>
#include <vector>
#include <array>

enum class ZoneTab { SmartFarm, Storage, Bakery, Fulfillment, Maintenance };
enum class PlotState { Empty, Seeded, Growing, Ready };
enum class MachineState { Idle, Working, Paused, Broken, Repairing };
enum class Scenario { Normal, Bottleneck, BreakdownManual, BreakdownAuto, Overflow };
enum class ItemType { Wheat, Flour, Dough, Bread, Parcel };
enum class EventType { Info, Success, Warning, Error, Production, Breakdown, Maintenance };

struct QueueInfo {
    std::string label;
    int count = 0;
    int capacity = 0;
};

struct FactoryCommand {
    bool toggleRunning = false; bool reset = false; bool clearLog = false;
    bool plantAll = false; bool harvestReady = false;
    bool toggleAutoPlant = false; bool toggleAutoHarvest = false;
    bool toggleAutoMill = false; bool toggleAutoMix = false;
    bool toggleAutoBake = false; bool toggleAutoPack = false;
    bool toggleAutoDispatch = false;
    bool manualHarvestOne = false; bool manualMillOne = false;
    bool manualMixOne = false; bool manualBakeOne = false;
    bool manualPackOne = false; bool manualDispatchOne = false;
    bool dispatchTruckNow = false; int clickedPlot = -1;
    bool changeScenario = false; Scenario scenario = Scenario::Normal;
    bool changeSpeed = false; int speed = 1;
    bool addOrder = false; std::string orderCity; int orderQuantity = 0;
    int targetMachineId = -1; bool forceBreak = false; bool pauseMachine = false;
    bool resumeMachine = false; bool instantRepair = false;
    bool dispatchTechnician = false;
};

struct PlotSnapshot {
    PlotState state = PlotState::Empty; float growth = 0.0f;
    PlotState getState() const { return state; }
    float getGrowth() const { return growth; }
};

struct StorageSnapshot {
    std::string name; ItemType itemType = ItemType::Wheat; int count = 0; int capacity = 0;
    const std::string& getName() const { return name; }
    ItemType getItemType() const { return itemType; }
    int getCount() const { return count; }
    int getCapacity() const { return capacity; }
};

struct OrderSnapshot {
    int id = 0; std::string city; int count = 0; int priority = 0; float progress = 0.0f; int packedCount = 0;
    int getId() const { return id; } const std::string& getCity() const { return city; }
    int getCount() const { return count; } int getPriority() const { return priority; }
    float getProgress() const { return progress; } int getPackedCount() const { return packedCount; }
};

struct EventSnapshot {
    int tick = 0; std::string text; EventType type = EventType::Info;
    int getTick() const { return tick; } const std::string& getText() const { return text; }
    EventType getType() const { return type; }
};

struct MachineSnapshot {
    int id = 0; std::string name; std::string zone; MachineState state = MachineState::Idle;
    float progress = 0.0f; float health = 1.0f; int outputCount = 0; int processTicks = 0;
    float breakdownProbability = 0.0f; bool enabled = true; bool breakable = true; QueueInfo queue;
    int getId() const { return id; } const std::string& getName() const { return name; }
    const std::string& getZone() const { return zone; } MachineState getState() const { return state; }
    float getProgress() const { return progress; } float getHealth() const { return health; }
    int getOutputCount() const { return outputCount; } int getProcessTicks(Scenario) const { return processTicks; }
    float getBreakdownProbability() const { return breakdownProbability; } bool isEnabled() const { return enabled; }
    bool isBreakable() const { return breakable; } QueueInfo getQueueInfo() const { return queue; }
};

struct TruckLoadSnapshot {
    int orderId = 0; std::string destination; int breadCount = 0;
    int getOrderId() const { return orderId; } const std::string& getDestination() const { return destination; }
    int getBreadCount() const { return breadCount; }
};

struct FactorySnapshot {
    bool running = false; int currentTick = 0; int speed = 1; Scenario scenario = Scenario::Normal;
    int finishedBreadCount = 0; int dispatchedParcelCount = 0; int soldBreadCount = 0;
    int truckLoad = 0; int truckCapacity = 50; int truckOrderCount = 0; int lostProducts = 0; int totalBreakdowns = 0;
    int plantedCount = 0; int readyCount = 0; int workInProgressCount = 0; int brokenMachineCount = 0;
    bool autoPlantEnabled = false; bool autoHarvestEnabled = false; bool autoMillEnabled = false;
    bool autoMixEnabled = false; bool autoBakeEnabled = false; bool autoPackEnabled = false; bool autoDispatchEnabled = false;

    std::array<PlotSnapshot, 20> plots;
    StorageSnapshot wheat; StorageSnapshot flour; StorageSnapshot dough; StorageSnapshot bread; StorageSnapshot parcels;
    std::vector<MachineSnapshot> machines; std::vector<OrderSnapshot> orders;
    std::vector<TruckLoadSnapshot> packedOrders; std::vector<TruckLoadSnapshot> truckLoads; std::vector<EventSnapshot> events;

    bool isRunning() const { return running; } int getCurrentTick() const { return currentTick; }
    int getSpeed() const { return speed; } Scenario getScenario() const { return scenario; }
    int getFinishedBreadCount() const { return finishedBreadCount; } int getDispatchedParcelCount() const { return dispatchedParcelCount; }
    int getSoldBreadCount() const { return soldBreadCount; } int getTruckLoad() const { return truckLoad; }
    int getTruckCapacity() const { return truckCapacity; } int getTruckOrderCount() const { return truckOrderCount; }
    int getLostProducts() const { return lostProducts; } int getTotalBreakdowns() const { return totalBreakdowns; }
    int getPlantedCount() const { return plantedCount; } int getReadyCount() const { return readyCount; }
    int getWorkInProgressCount() const { return workInProgressCount; } int getBrokenMachineCount() const { return brokenMachineCount; }
    bool isAutoPlantEnabled() const { return autoPlantEnabled; } bool isAutoHarvestEnabled() const { return autoHarvestEnabled; }
    bool isAutoMillEnabled() const { return autoMillEnabled; } bool isAutoMixEnabled() const { return autoMixEnabled; }
    bool isAutoBakeEnabled() const { return autoBakeEnabled; } bool isAutoPackEnabled() const { return autoPackEnabled; }
    bool isAutoDispatchEnabled() const { return autoDispatchEnabled; }

    const std::array<PlotSnapshot, 20>& getPlots() const { return plots; }
    const StorageSnapshot& getWheat() const { return wheat; } const StorageSnapshot& getFlour() const { return flour; }
    const StorageSnapshot& getDough() const { return dough; } const StorageSnapshot& getBread() const { return bread; }
    const StorageSnapshot& getParcels() const { return parcels; }
    const std::vector<MachineSnapshot>& getMachines() const { return machines; }
    const std::vector<OrderSnapshot>& getOrders() const { return orders; }
    const std::vector<TruckLoadSnapshot>& getPackedOrders() const { return packedOrders; }
    const std::vector<TruckLoadSnapshot>& getTruckLoads() const { return truckLoads; }
    const std::vector<EventSnapshot>& getEvents() const { return events; }

    int getWheatCount() const { return wheat.count; } int getWheatCapacity() const { return wheat.capacity; }
    int getFlourCount() const { return flour.count; } int getFlourCapacity() const { return flour.capacity; }
    int getDoughCount() const { return dough.count; } int getDoughCapacity() const { return dough.capacity; }
    int getBreadCount() const { return bread.count; } int getBreadCapacity() const { return bread.capacity; }
    int getParcelCount() const { return parcels.count; } int getParcelCapacity() const { return parcels.capacity; }

    const MachineSnapshot* findMachine(int id) const {
        for (const auto& machine : machines) {
            if (machine.id == id) return &machine;
        }
        return nullptr;
    }
};