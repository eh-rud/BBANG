#include "UI_Scenes.h"
#include "imgui.h"
#include <algorithm>

namespace UI {

    void RenderSmartFarmScene(const FactorySnapshot& factory, FactoryCommand& command) {
        char plantedText[32]; char readyText[32]; char wheatText[32];
        std::snprintf(plantedText, sizeof(plantedText), "%d / 20", factory.getPlantedCount());
        std::snprintf(readyText, sizeof(readyText), "%d", factory.getReadyCount());
        std::snprintf(wheatText, sizeof(wheatText), "%d / %d", factory.getWheatCount(), factory.getWheatCapacity());

        float chipW = (ImGui::GetContentRegionAvail().x - 36.0f) / 4.0f;
        ImGui::BeginChild("FarmStats", ImVec2(0, 80), false, Theme::NoScrollFlags());
        ImGui::BeginChild("Planted", ImVec2(chipW, 66), true, Theme::NoScrollFlags()); TextMuted("Planted plots"); ImGui::TextUnformatted(plantedText); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("Ready", ImVec2(chipW, 66), true, Theme::NoScrollFlags()); TextMuted("Ready wheat"); ImGui::TextUnformatted(readyText); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("WheatStock", ImVec2(chipW, 66), true, Theme::NoScrollFlags()); TextMuted("Wheat storage"); ImGui::TextUnformatted(wheatText); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("AutoPlantMode", ImVec2(chipW, 66), true, Theme::NoScrollFlags()); TextMuted("Plant mode"); ImGui::TextColored(factory.isAutoPlantEnabled() ? Theme::COL_GREEN : Theme::COL_WARN, "%s", factory.isAutoPlantEnabled() ? "AUTO" : "MANUAL"); ImGui::EndChild();
        ImGui::EndChild();

        TextMuted(factory.isAutoPlantEnabled() ? "Plant: AUTO" : "Plant: MANUAL"); ImGui::SameLine();
        TextMuted(factory.isAutoHarvestEnabled() ? "Harvest: AUTO" : "Harvest: MANUAL"); ImGui::Spacing();

        AutomationToggleButton(factory.isAutoPlantEnabled() ? "Auto Plant: ON" : "Auto Plant: OFF", "smartfarm_auto_plant", factory.isAutoPlantEnabled(), command.toggleAutoPlant, 142.0f); ImGui::SameLine();
        AutomationToggleButton(factory.isAutoHarvestEnabled() ? "Auto Harvest: ON" : "Auto Harvest: OFF", "smartfarm_auto_harvest", factory.isAutoHarvestEnabled(), command.toggleAutoHarvest, 154.0f); ImGui::SameLine();
        if (ImGui::Button("Plant All##SmartFarm", ImVec2(104, 32))) command.plantAll = true; ImGui::SameLine();
        if (ImGui::Button("Harvest Ready##SmartFarm", ImVec2(128, 32))) command.harvestReady = true; ImGui::Spacing();

        float availW = ImGui::GetContentRegionAvail().x; float storageW = 270.0f; float fieldW = std::max(360.0f, availW - storageW - 16.0f); float tile = std::max(68.0f, std::min(106.0f, (fieldW - 4.0f * 8.0f) / 5.0f));

        ImGui::BeginGroup(); SectionTitle("Field"); ImGui::Spacing();
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 5; ++col) {
                int index = row * 5 + col; DrawPlotTile(factory, command, index, tile); if (col < 4) ImGui::SameLine(0, 8.0f);
            }
        }
        ImGui::EndGroup(); ImGui::SameLine(0, 16.0f);

        ImGui::BeginGroup(); DrawStorageGrid(factory.getWheat(), 38.0f); ImGui::EndGroup();
    }

    void RenderBakeryAutomationPanel(const FactorySnapshot& factory, FactoryCommand& command) {
        SectionTitle("Bakery Automation"); ImGui::Spacing();
        DrawAutomationRow("Wheat Storage -> Flour Silo", "bakery_auto_mill", factory.isAutoMillEnabled(), command.toggleAutoMill, command.manualMillOne, "Mill 1");
        DrawAutomationRow("Flour Silo -> Dough Queue", "bakery_auto_mix", factory.isAutoMixEnabled(), command.toggleAutoMix, command.manualMixOne, "Mix 1");
        DrawAutomationRow("Dough Queue -> Bread Shelf", "bakery_auto_bake", factory.isAutoBakeEnabled(), command.toggleAutoBake, command.manualBakeOne, "Bake 1");
    }

    void RenderBakeryScene(const FactorySnapshot& factory, UIState& uiState, FactoryCommand& command) {
        char lineA[32]; char lineB[32]; char lineC[32];
        std::snprintf(lineA, sizeof(lineA), "%d wheat", factory.getWheatCount());
        std::snprintf(lineB, sizeof(lineB), "%d dough", factory.getDoughCount());
        std::snprintf(lineC, sizeof(lineC), "%d bread", factory.getBreadCount());

        float chipW = (ImGui::GetContentRegionAvail().x - 24.0f) / 3.0f;
        ImGui::BeginChild("BakeryStats", ImVec2(0, 80), false, Theme::NoScrollFlags());
        ImGui::BeginChild("BakeryStatA", ImVec2(chipW, 66), true, Theme::NoScrollFlags()); TextMuted("Input"); ImGui::TextUnformatted(lineA); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("BakeryStatB", ImVec2(chipW, 66), true, Theme::NoScrollFlags()); TextMuted("In process"); ImGui::TextUnformatted(lineB); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("BakeryStatC", ImVec2(chipW, 66), true, Theme::NoScrollFlags()); TextMuted("Output"); ImGui::TextUnformatted(lineC); ImGui::EndChild(); ImGui::EndChild();

        ImGui::BeginChild("BakeryAutomationControls", ImVec2(0, 195), true, Theme::NoScrollFlags()); RenderBakeryAutomationPanel(factory, command); ImGui::EndChild(); ImGui::Spacing();

        SectionTitle("Bakery Line"); TextMuted("Mill -> Mixer -> Oven"); ImGui::Spacing();
        float cardW = std::max(156.0f, (ImGui::GetContentRegionAvail().x - 24.0f) / 3.0f); float cardH = 170.0f;

        int drawnBakery = 0;
        for (const MachineSnapshot& machine : factory.getMachines()) {
            if (machine.getZone() != "Bakery") continue;
            if (drawnBakery > 0 && drawnBakery % 3 != 0) ImGui::SameLine();
            DrawMachineCard(factory, uiState, &machine, cardW, cardH);
            drawnBakery++;
        }
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        float gridW = (ImGui::GetContentRegionAvail().x - 16.0f) / 3.0f; float gridH = 200.0f;
        ImGui::BeginChild("FlourGrid", ImVec2(gridW, gridH), true, Theme::NoScrollFlags()); DrawStorageGrid(factory.getFlour(), 31.0f); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("DoughGrid", ImVec2(gridW, gridH), true, Theme::NoScrollFlags()); DrawStorageGrid(factory.getDough(), 31.0f); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("BreadGrid", ImVec2(gridW, gridH), true, Theme::NoScrollFlags()); DrawStorageGrid(factory.getBread(), 31.0f); ImGui::EndChild();
    }

    void RenderStorageScene(const FactorySnapshot& factory) {
        SectionTitle("Storehouse"); ImGui::Spacing();
        
        float cardW = (ImGui::GetContentRegionAvail().x - 16.0f) / 3.0f; 
        float cardH = 200.0f; 
        float slotSize = 31.0f;
        
        ImGui::BeginChild("StoreWheat", ImVec2(cardW, cardH), true, Theme::NoScrollFlags()); DrawStorageGrid(factory.getWheat(), slotSize); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("StoreFlour", ImVec2(cardW, cardH), true, Theme::NoScrollFlags()); DrawStorageGrid(factory.getFlour(), slotSize); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("StoreDough", ImVec2(cardW, cardH), true, Theme::NoScrollFlags()); DrawStorageGrid(factory.getDough(), slotSize); ImGui::EndChild(); ImGui::Spacing();
        ImGui::BeginChild("StoreBread", ImVec2(cardW, cardH), true, Theme::NoScrollFlags()); DrawStorageGrid(factory.getBread(), slotSize); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("StoreParcel", ImVec2(cardW, cardH), true, Theme::NoScrollFlags()); DrawStorageGrid(factory.getParcels(), slotSize); ImGui::EndChild();
    }

    void RenderFulfillmentAutomationPanel(const FactorySnapshot& factory, FactoryCommand& command) {
        SectionTitle("Fulfillment Automation"); TextMuted("Truck capacity: 50 bread"); ImGui::Spacing();
        DrawAutomationRow("Bread Shelf -> Packed Orders", "fulfill_auto_pack", factory.isAutoPackEnabled(), command.toggleAutoPack, command.manualPackOne, "Pack 1");
        DrawAutomationRow("Packed Orders -> Delivery Truck", "fulfill_auto_dispatch", factory.isAutoDispatchEnabled(), command.toggleAutoDispatch, command.manualDispatchOne, "Load 1");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        char truckText[64]; std::snprintf(truckText, sizeof(truckText), "Truck load: %d / %d bread", factory.getTruckLoad(), factory.getTruckCapacity());
        TextMuted(truckText); Progress(factory.getTruckCapacity() > 0 ? static_cast<float>(factory.getTruckLoad()) / static_cast<float>(factory.getTruckCapacity()) : 0.0f, Theme::COL_BLUE, ImVec2(-1, 8));
        TextMutedF("Loaded orders: %d", static_cast<int>(factory.getTruckLoads().size())); ImGui::SameLine();
        if (ImGui::Button("Dispatch Truck", ImVec2(140, 28))) command.dispatchTruckNow = true;
        ImGui::Spacing();
        ImGui::BeginChild("LoadedTruckOrders", ImVec2(0, 0), true);
        if (factory.getTruckLoads().empty()) { TextMutedWrapped("Truck is empty. Use Load 1 or Auto Dispatch to load packed orders."); }
        else {
            for (const TruckLoadSnapshot& load : factory.getTruckLoads()) {
                char line[160]; std::snprintf(line, sizeof(line), "#%d  %s  -  %d bread", load.getOrderId(), load.getDestination().empty() ? "Unknown" : load.getDestination().c_str(), load.getBreadCount());
                ImGui::TextUnformatted(line);
            }
        }
        ImGui::EndChild();
    }

    void RenderFulfillmentScene(const FactorySnapshot& factory, UIState& uiState, FactoryCommand& command) {
        char queueText[32]; char parcelText[32]; char dispatchText[32];
        std::snprintf(queueText, sizeof(queueText), "%d orders", static_cast<int>(factory.getOrders().size()));
        std::snprintf(parcelText, sizeof(parcelText), "%d packed", factory.getParcelCount());
        std::snprintf(dispatchText, sizeof(dispatchText), "%d / %d bread", factory.getTruckLoad(), factory.getTruckCapacity());

        float chipW = (ImGui::GetContentRegionAvail().x - 24.0f) / 3.0f;
        ImGui::BeginChild("FulfillmentStats", ImVec2(0, 80), false, Theme::NoScrollFlags());
        ImGui::BeginChild("OrderCount", ImVec2(chipW, 66), true, Theme::NoScrollFlags()); TextMuted("Order queue"); ImGui::TextUnformatted(queueText); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("ParcelCount", ImVec2(chipW, 66), true, Theme::NoScrollFlags()); TextMuted("Packed orders"); ImGui::TextUnformatted(parcelText); ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("DispatchCount", ImVec2(chipW, 66), true, Theme::NoScrollFlags()); TextMuted("Truck load"); ImGui::TextUnformatted(dispatchText); ImGui::EndChild(); ImGui::EndChild();
        TextMutedF("Delivered trucks: %d", factory.getDispatchedParcelCount());

        SectionTitle("Manual Order Entry"); ImGui::Spacing();
        ImGui::BeginChild("ManualOrderForm", ImVec2(0, 56), true, Theme::NoScrollFlags());
        static char regionInput[64] = ""; static int quantityInput = 10;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f); ImGui::PushItemWidth(120.0f); ImGui::InputText("Destination", regionInput, sizeof(regionInput)); ImGui::SameLine(0, 16.0f); ImGui::InputInt("Qty (max 50)", &quantityInput); ImGui::PopItemWidth();
        if (quantityInput < 1) quantityInput = 1; if (quantityInput > 50) quantityInput = 50;
        ImGui::SameLine(0, 16.0f);
        if (ImGui::Button("Submit Order", ImVec2(118, 26))) { if (regionInput[0] != '\0') { command.addOrder = true; command.orderCity = regionInput; command.orderQuantity = quantityInput; regionInput[0] = '\0'; quantityInput = 10; } }
        ImGui::EndChild(); ImGui::Spacing();

        ImGui::BeginChild("FulfillmentAutomationControls", ImVec2(0, 380), true, Theme::NoScrollFlags()); RenderFulfillmentAutomationPanel(factory, command); ImGui::EndChild(); ImGui::Spacing();

        SectionTitle("Orders"); ImGui::Spacing();
        float orderW = std::max(180.0f, (ImGui::GetContentRegionAvail().x - 16.0f) / 3.0f);
        for (size_t i = 0; i < factory.getOrders().size(); ++i) { DrawOrderCard(factory.getOrders()[i], orderW); if ((i + 1) % 3 != 0) ImGui::SameLine(); }
        if (factory.getOrders().empty()) TextMuted("No orders currently being packaged.");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        SectionTitle("Packed Orders Waiting for Truck");
        ImGui::BeginChild("PackedOrdersWaitingList", ImVec2(0, 130), true);
        if (factory.getPackedOrders().empty()) { TextMuted("No packed orders waiting."); }
        else {
            for (const TruckLoadSnapshot& packed : factory.getPackedOrders()) {
                char line[180]; std::snprintf(line, sizeof(line), "#%d  %s  -  %d bread  [waiting]", packed.getOrderId(), packed.getDestination().empty() ? "Unknown" : packed.getDestination().c_str(), packed.getBreadCount());
                ImGui::TextUnformatted(line);
            }
        }
        ImGui::EndChild(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        SectionTitle("Packing Line"); ImGui::Spacing();
        float packCardW = std::max(180.0f, (ImGui::GetContentRegionAvail().x - 8.0f) / 2.0f);
        int drawnFulfillment = 0;
        for (const MachineSnapshot& machine : factory.getMachines()) {
            if (machine.getZone() != "Fulfillment") continue;
            if (drawnFulfillment > 0 && drawnFulfillment % 2 != 0) ImGui::SameLine();
            DrawMachineCard(factory, uiState, &machine, packCardW, 170.0f); drawnFulfillment++;
        }
        ImGui::Spacing(); DrawStorageGrid(factory.getParcels(), 38.0f);
    }

    void RenderMaintenanceScene(const FactorySnapshot& factory, FactoryCommand&) {
        SectionTitle("Maintenance & Metrics");
        ImGui::Spacing();

        ImGui::Text("Current Tick: %d", factory.getCurrentTick());
        ImGui::Text("Scenario: %s", Theme::ScenarioName(factory.getScenario()));
        ImGui::Text("Total Breakdowns: %d", factory.getTotalBreakdowns());
        ImGui::Text("Bread Sold: %d", factory.getSoldBreadCount());
        ImGui::Text("Delivered Trucks: %d", factory.getDispatchedParcelCount());
        ImGui::Text("Active Orders: %d", static_cast<int>(factory.getOrders().size()));
        ImGui::Text("Lost Products: %d", factory.getLostProducts());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        SectionTitle("Storage Counts");
        ImGui::Text("Wheat %d/%d | Flour %d/%d",
                    factory.getWheatCount(), factory.getWheatCapacity(),
                    factory.getFlourCount(), factory.getFlourCapacity());
        ImGui::Text("Dough %d/%d | Bread %d/%d",
                    factory.getDoughCount(), factory.getDoughCapacity(),
                    factory.getBreadCount(), factory.getBreadCapacity());
        ImGui::Text("Packed Orders %d/%d", factory.getParcelCount(), factory.getParcelCapacity());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        SectionTitle("Repair Control");
        TextMuted("Select a machine in Inspector to force break or repair.");
    }
}