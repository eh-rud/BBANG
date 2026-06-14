#pragma once
#include "Types.h"
#include <string>

class Factory; // Forward declaration

class SimulationObject {
public:
    virtual ~SimulationObject() = default;
    virtual void update(Factory& factory) = 0;
    virtual std::string getInfo() const = 0;
};

class Machine : public SimulationObject {
private:
    int id_; std::string name_; std::string zone_; MachineState state_;
    float progress_; float health_; int outputCount_; int baseProcessTicks_;
    int workTicks_; int repairTicks_; int repairWorkTicks_; float breakdownProbability_; bool enabled_;
protected:
    void markWorking(); void markIdle(); void resetProgress();
    bool advanceWork(const Factory& factory); void addOutput(); virtual bool isBottleneckTarget() const;
public:
    Machine(int id, std::string name, std::string zone, int processTicks, float breakdownProbability);
    ~Machine() override = default;
    int getId() const; const std::string& getName() const; const std::string& getZone() const;
    MachineState getState() const; float getProgress() const; float getHealth() const;
    int getOutputCount() const; int getProcessTicks(Scenario scenario) const;
    float getBreakdownProbability() const; bool isEnabled() const; bool isBreakable() const;
    bool pause(); bool resume(); bool forceBreak(); bool startRepair(); void instantRepair();
    void update(Factory& factory) override; std::string getInfo() const override;
    virtual void performWork(Factory& factory) = 0;
    virtual QueueInfo getQueueInfo(const Factory& factory) const = 0;
};

class ProducerMachine : public Machine {
public:
    using Machine::Machine;
    virtual bool canProduce(const Factory& factory) const = 0;
    virtual void produce(Factory& factory) = 0;
    void performWork(Factory& factory) override;
};

class ProcessorMachine : public Machine {
public:
    using Machine::Machine;
    virtual bool checkIngredients(const Factory& factory) const = 0;
    virtual void process(Factory& factory) = 0;
    void performWork(Factory& factory) override;
};

class ConsumerMachine : public Machine {
public:
    using Machine::Machine;
    virtual bool canConsume(const Factory& factory) const = 0;
    virtual void consume(Factory& factory) = 0;
    void performWork(Factory& factory) override;
};

class FarmGrower : public ProducerMachine {
public:
    FarmGrower(int id, std::string name, std::string zone);
    bool canProduce(const Factory& factory) const override; void produce(Factory& factory) override;
    QueueInfo getQueueInfo(const Factory& factory) const override;
};

class MillMachine : public ProcessorMachine {
public:
    MillMachine(int id, std::string name, std::string zone);
    bool checkIngredients(const Factory& factory) const override; void process(Factory& factory) override;
    QueueInfo getQueueInfo(const Factory& factory) const override;
};

class DoughMixer : public ProcessorMachine {
public:
    DoughMixer(int id, std::string name, std::string zone);
    bool checkIngredients(const Factory& factory) const override; void process(Factory& factory) override;
    QueueInfo getQueueInfo(const Factory& factory) const override;
};

class Oven : public ProcessorMachine {
protected:
    bool isBottleneckTarget() const override;
public:
    Oven(int id, std::string name, std::string zone);
    bool checkIngredients(const Factory& factory) const override; void process(Factory& factory) override;
    QueueInfo getQueueInfo(const Factory& factory) const override;
};

class Packager : public ProcessorMachine {
public:
    Packager(int id, std::string name, std::string zone);
    bool checkIngredients(const Factory& factory) const override; void process(Factory& factory) override;
    QueueInfo getQueueInfo(const Factory& factory) const override;
};

class DeliveryTruck : public ConsumerMachine {
public:
    DeliveryTruck(int id, std::string name, std::string zone);
    bool canConsume(const Factory& factory) const override; void consume(Factory& factory) override;
    QueueInfo getQueueInfo(const Factory& factory) const override;
};

class Worker : public SimulationObject {
private:
    int id_; std::string name_; std::string role_;
public:
    Worker(int id, std::string name, std::string role);
    ~Worker() override = default;
    int getId() const; const std::string& getName() const; const std::string& getRole() const;
    std::string getInfo() const override;
};

class Technician : public Worker {
public:
    Technician(int id, std::string name);
    void update(Factory& factory) override;
};