#pragma once
#include "Types.h"
#include "Storage.h"
#include "Machine.h"
#include <array>
#include <vector>
#include <memory>
#include <random>

class Factory {
private:
    bool running_; int currentTick_; float tickAccumulator_; int speed_; Scenario scenario_;
    int finishedBreadCount_; int dispatchedParcelCount_; int soldBreadCount_; int truckLoad_;
    int truckCapacity_; int truckOrderCount_; std::vector<TruckLoadSnapshot> truckLoads_;
    int lostProducts_; int totalBreakdowns_; int nextOrderId_;
    bool autoPlantEnabled_; bool autoHarvestEnabled_; bool autoMillEnabled_; bool autoMixEnabled_;
    bool autoBakeEnabled_; bool autoPackEnabled_; bool autoDispatchEnabled_;
    bool manualHarvestRequested_; bool manualMillRequested_; bool manualMixRequested_;
    bool manualBakeRequested_; bool manualPackRequested_; bool manualDispatchRequested_;
    size_t nextPlantSearchIndex_; size_t nextHarvestSearchIndex_;
    std::array<FarmPlot, 20> plots_; StorageBin wheat_; StorageBin flour_; StorageBin dough_; StorageBin bread_; StorageBin parcels_;
    std::vector<std::unique_ptr<SimulationObject>> objects_;
    std::vector<Machine*> machineViews_;
    std::vector<OrderItem> orders_;
    std::vector<EventItem> events_;
    std::mt19937 rng_; std::uniform_real_distribution<float> probability_;

    void stepTick(); void maybeTriggerRandomBreakdown(); void registerOverflow(const std::string& message);
    void autoPlantEmptyPlots(); void logAutomationToggle(const std::string& name, bool enabled);

    template <typename MachineType, typename... Args>
    void addMachine(Args&&... args) {
        auto machine = std::make_unique<MachineType>(std::forward<Args>(args)...);
        machineViews_.push_back(machine.get());
        objects_.push_back(std::move(machine));
    }

public:
    Factory();
    void reset(); void update(float deltaTime); void applyCommand(const FactoryCommand& command);
    FactorySnapshot createSnapshot() const;
    bool isRunning() const; int getCurrentTick() const; int getSpeed() const; Scenario getScenario() const;
    int getFinishedBreadCount() const; int getDispatchedParcelCount() const; int getSoldBreadCount() const;
    int getTruckLoad() const; int getTruckCapacity() const; int getTruckOrderCount() const;
    int getLostProducts() const; int getTotalBreakdowns() const;
    bool isAutoPlantEnabled() const; bool isAutoHarvestEnabled() const; bool isAutoMillEnabled() const;
    bool isAutoMixEnabled() const; bool isAutoBakeEnabled() const; bool isAutoPackEnabled() const; bool isAutoDispatchEnabled() const;
    bool shouldRunHarvest() const; bool shouldRunMill() const; bool shouldRunMix() const;
    bool shouldRunBake() const; bool shouldRunPack() const; bool shouldRunDispatch() const;
    const std::array<FarmPlot, 20>& getPlots() const;
    const StorageBin& getWheat() const; const StorageBin& getFlour() const; const StorageBin& getDough() const;
    const StorageBin& getBread() const; const StorageBin& getParcels() const;
    const std::vector<Machine*>& getMachines() const; const std::vector<OrderItem>& getOrders() const; const std::vector<EventItem>& getEvents() const;
    void addEvent(const std::string& text, EventType type); void clearEvents();
    void plantSeed(int index); void harvestPlot(int index); void plantAllEmpty(); void harvestAllReady();
    bool autoHarvestOneReadyPlot(); bool runHarvestOnce(); bool runMillOnce(); bool runMixOnce();
    bool runBakeOnce(); bool runPackOnce(); bool runDispatchOnce(); bool dispatchTruck();
    void finishManualHarvestRequest(); void finishManualMillRequest(); void finishManualMixRequest();
    void finishManualBakeRequest(); void finishManualPackRequest(); void finishManualDispatchRequest();
    void addOrder(const std::string& city, int count);
    void pauseMachine(int id); void resumeMachine(int id); void forceBreakMachine(int id);
    void instantRepairMachine(int id); void dispatchTechnician(int id);
    int getPlantedCount() const; int getReadyCount() const; int getWorkInProgressCount() const; int getBrokenMachineCount() const;
    bool hasReadyPlot() const; bool isOverflowScenario() const; bool isBreakdownScenario() const; bool isAutoRepairScenario() const;
    bool consumeWheat(); bool produceFlour(); bool consumeFlour(); bool produceDough(); bool consumeDough();
    bool produceBread(); bool consumeBread(); bool produceParcel(int breadCount = 1, const std::string& destination = "", int orderId = 0);
    bool consumeParcel(); bool canLoadNextPackedOrder() const; bool loadNextPackedOrderToTruck();
    bool canPackNextBread() const; void packNextBread();
    int getWheatCount() const; int getWheatCapacity() const; int getFlourCount() const; int getFlourCapacity() const;
    int getDoughCount() const; int getDoughCapacity() const; int getBreadCount() const; int getBreadCapacity() const;
    int getParcelCount() const; int getParcelCapacity() const;
    Machine* findMachine(int id); const Machine* findMachine(int id) const; Machine* findFirstBrokenMachine();
};