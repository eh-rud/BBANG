#include "frontend.h"
#include "UI_Scenes.h"
#include "imgui.h"
#include <algorithm>
#include <string>

void ApplySmartFarmTheme() {
    Theme::ApplySmartFarmTheme();
}

static void RenderTopBar(const FactorySnapshot& factory, FactoryCommand& command) {
    ImGui::BeginChild("TopBar", ImVec2(0, 92), true, Theme::NoScrollFlags());
    ImDrawList* draw = ImGui::GetWindowDrawList(); ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 badgeMin = ImVec2(pos.x, pos.y + 4.0f); ImVec2 badgeMax = ImVec2(pos.x + 44.0f, pos.y + 48.0f);
    draw->AddRectFilled(badgeMin, badgeMax, Theme::ToU32(Theme::COL_GREEN), 10.0f); draw->AddText(ImVec2(badgeMin.x + 14.0f, badgeMin.y + 10.0f), Theme::ToU32(ImVec4(1, 1, 1, 1)), "B");
    ImGui::Dummy(ImVec2(50, 52)); ImGui::SameLine();
    
    ImGui::BeginGroup(); UI::SectionTitle("Simulation Control"); ImGui::SetWindowFontScale(1.12f);
    ImGui::TextUnformatted("Wheat -> Flour -> Dough -> Bread -> Orders -> Truck");
    ImGui::SetWindowFontScale(1.0f); UI::TextMuted(Theme::ScenarioDescription(factory.getScenario())); ImGui::EndGroup();

    char tickText[64]; std::snprintf(tickText, sizeof(tickText), "Tick %d", factory.getCurrentTick());
    ImGuiStyle& style = ImGui::GetStyle(); float spacing = style.ItemSpacing.x;
    float fixedButtonsWidth = 78.0f + 68.0f + 170.0f + 110.0f + (spacing * 4.0f); float tickTextWidth = ImGui::CalcTextSize(tickText).x;
    float totalControlsWidth = tickTextWidth + spacing + fixedButtonsWidth;
    float rightAlignX = ImGui::GetWindowWidth() - totalControlsWidth - style.WindowPadding.x;
    float minX = ImGui::GetCursorPosX() + 20.0f; ImGui::SameLine(std::max(rightAlignX, minX)); ImGui::SetCursorPosY(18.0f);

    ImGui::AlignTextToFramePadding(); UI::TextMuted(tickText); ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, factory.isRunning() ? Theme::COL_WARN : Theme::COL_GREEN);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, factory.isRunning() ? ImVec4(0.90f, 0.58f, 0.19f, 1.0f) : ImVec4(0.12f, 0.44f, 0.24f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, factory.isRunning() ? ImVec4(0.76f, 0.42f, 0.09f, 1.0f) : ImVec4(0.08f, 0.36f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    if (ImGui::Button(factory.isRunning() ? "Pause" : "Start", ImVec2(78, 32))) command.toggleRunning = true;
    ImGui::PopStyleColor(4); ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(68, 32))) command.reset = true; ImGui::SameLine();

    const char* scenarios[] = { "Normal", "Bottleneck", "Breakdown - Manual", "Breakdown - Auto", "Overflow" };
    ImGui::SetNextItemWidth(170.0f); int scenario = static_cast<int>(factory.getScenario());
    if (ImGui::Combo("##Scenario", &scenario, scenarios, IM_ARRAYSIZE(scenarios))) { command.changeScenario = true; command.scenario = static_cast<Scenario>(scenario); }
    ImGui::SameLine(); ImGui::SetNextItemWidth(110.0f); int speed = factory.getSpeed();
    if (ImGui::SliderInt("##Speed", &speed, 1, 5, "x%d")) { command.changeSpeed = true; command.speed = speed; }
    ImGui::EndChild();
}

static bool ZoneButton(UIState& uiState, ZoneTab zone, const char* label, const char* value, ImVec4 color) {
    bool selected = uiState.activeZone == zone;
    ImGui::PushID(label);
    ImGui::PushStyleColor(ImGuiCol_Button, selected ? color : ImVec4(1, 1, 1, 1));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected ? color : ImVec4(0.94f, 0.96f, 0.91f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, selected ? color : ImVec4(0.88f, 0.92f, 0.84f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, selected ? ImVec4(1, 1, 1, 1) : Theme::COL_TEXT);
    bool clicked = ImGui::Button(label, ImVec2(-1, 38)); ImGui::PopStyleColor(4);
    if (clicked) uiState.activeZone = zone;
    ImGui::PushStyleColor(ImGuiCol_Text, selected ? color : Theme::COL_MUTED); ImGui::TextWrapped("%s", value); ImGui::PopStyleColor();
    ImGui::Spacing(); ImGui::PopID();
    return clicked;
}

static void RenderZoneNav(const FactorySnapshot& factory, UIState& uiState) {
    ImGui::BeginChild("Zones", ImVec2(196, 0), true);
    UI::SectionTitle("Factory Floor Overview"); ImGui::Spacing();

    char farmValue[64]; std::snprintf(farmValue, sizeof(farmValue), "%d planted, %d ripe", factory.getPlantedCount(), factory.getReadyCount());
    char storageValue[64]; std::snprintf(storageValue, sizeof(storageValue), "%d in factory", factory.getWorkInProgressCount());
    char bakeryValue[64]; std::snprintf(bakeryValue, sizeof(bakeryValue), "%d bread baked", factory.getFinishedBreadCount());
    char fulfillmentValue[64]; std::snprintf(fulfillmentValue, sizeof(fulfillmentValue), "%d trucks sent", factory.getDispatchedParcelCount());
    char maintenanceValue[64]; std::snprintf(maintenanceValue, sizeof(maintenanceValue), "%d breakdowns", factory.getTotalBreakdowns());

    ZoneButton(uiState, ZoneTab::SmartFarm, "SmartFarm", farmValue, Theme::COL_GREEN);
    ZoneButton(uiState, ZoneTab::Storage, "Storage", storageValue, Theme::COL_WHEAT);
    ZoneButton(uiState, ZoneTab::Bakery, "Bakery", bakeryValue, Theme::COL_BREAD);
    ZoneButton(uiState, ZoneTab::Fulfillment, "Fulfillment", fulfillmentValue, Theme::COL_BLUE);
    ZoneButton(uiState, ZoneTab::Maintenance, "Maintenance", maintenanceValue, Theme::COL_WARN);
    ImGui::Separator(); ImGui::Spacing();

    UI::SectionTitle("Today");
    UI::TextMutedF("Open orders: %d", static_cast<int>(factory.getOrders().size()));
    UI::TextMutedF("Broken machines: %d", factory.getBrokenMachineCount());
    UI::TextMutedF("Lost products: %d", factory.getLostProducts());
    ImGui::EndChild();
}

static void RenderStatistics(const FactorySnapshot& factory) {
    UI::SectionTitle("Statistics"); ImGui::Spacing();
    UI::TextMutedF("Bread baked: %d", factory.getFinishedBreadCount()); UI::TextMutedF("Bread sold: %d", factory.getSoldBreadCount());
    UI::TextMutedF("In-factory items: %d", factory.getWorkInProgressCount()); UI::TextMutedF("Breakdowns: %d", factory.getTotalBreakdowns());
    UI::TextMutedF("Lost products: %d", factory.getLostProducts()); UI::TextMutedF("Delivered trucks: %d", factory.getDispatchedParcelCount());
}

static void DrawBufferLoadRow(const char* label, const StorageSnapshot& storage) {
    char value[64]; std::snprintf(value, sizeof(value), "%d / %d", storage.getCount(), storage.getCapacity());
    ImGui::TextUnformatted(label); ImGui::SameLine(ImGui::GetContentRegionAvail().x - 54.0f); UI::TextMuted(value);
    UI::Progress(storage.getCapacity() > 0 ? static_cast<float>(storage.getCount()) / static_cast<float>(storage.getCapacity()) : 0.0f, Theme::ItemColor(storage.getItemType()), ImVec2(-1, 8));
}

static void RenderPipelineBufferLoad(const FactorySnapshot& factory) {
    UI::SectionTitle("Pipeline Buffer Load"); ImGui::Spacing();
    DrawBufferLoadRow("Wheat Storage", factory.getWheat()); DrawBufferLoadRow("Flour Silo", factory.getFlour());
    DrawBufferLoadRow("Dough Queue", factory.getDough()); DrawBufferLoadRow("Bread Shelf", factory.getBread()); DrawBufferLoadRow("Packed Orders", factory.getParcels());
}

static void RenderMachineBoard(const FactorySnapshot& factory, UIState& uiState, FactoryCommand& command) {
    UI::SectionTitle("Inspector"); ImGui::Spacing(); UI::SectionTitle("Selectable Machine Entries");
    for (const MachineSnapshot& item : factory.getMachines()) {
        bool selected = uiState.selectedMachineId == item.getId();
        if (ImGui::Selectable(item.getName().c_str(), selected)) uiState.selectedMachineId = item.getId();
    }
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    const MachineSnapshot* machine = UI::FindMachine(factory, uiState.selectedMachineId);
    if (!machine) { UI::TextMuted("Select a machine to view details."); return; }

    ImGui::TextWrapped("%s", machine->getName().c_str()); ImGui::PushStyleColor(ImGuiCol_Text, Theme::COL_MUTED); ImGui::Text("Zone: %s", machine->getZone().c_str()); ImGui::PopStyleColor();
    ImGui::Spacing(); ImGui::TextColored(Theme::StatusColor(machine->getState()), "Status: %s", Theme::StatusText(machine->getState()));
    ImGui::Spacing(); UI::TextMuted("Health"); UI::Progress(machine->getHealth(), machine->getHealth() > 0.4f ? Theme::COL_GREEN : Theme::COL_BAD, ImVec2(-1, 10));
    UI::TextMuted("Progress"); UI::Progress(machine->getProgress(), Theme::StatusColor(machine->getState()), ImVec2(-1, 10));

    QueueInfo queue = machine->getQueueInfo();
    ImGui::Text("Process time: %d ticks", machine->getProcessTicks(factory.getScenario()));
    ImGui::Text("Queue depth: %s %d / %d", queue.label.c_str(), queue.count, queue.capacity); ImGui::Spacing();
    UI::DrawStatChip("Total Output", std::to_string(machine->getOutputCount()).c_str(), Theme::COL_BLUE); ImGui::Spacing();

    command.targetMachineId = uiState.selectedMachineId;
    bool canPause = machine->getState() == MachineState::Idle || machine->getState() == MachineState::Working;
    if (!canPause) ImGui::BeginDisabled(); if (ImGui::Button("Pause Machine", ImVec2(-1, 32))) command.pauseMachine = true; if (!canPause) ImGui::EndDisabled();
    bool canResume = machine->getState() == MachineState::Paused;
    if (!canResume) ImGui::BeginDisabled(); if (ImGui::Button("Resume Machine", ImVec2(-1, 32))) command.resumeMachine = true; if (!canResume) ImGui::EndDisabled();
    bool canForceBreak = machine->isBreakable() && machine->getState() != MachineState::Paused && machine->getState() != MachineState::Broken && machine->getState() != MachineState::Repairing;
    if (!canForceBreak) ImGui::BeginDisabled(); if (ImGui::Button("Force Break", ImVec2(-1, 32))) command.forceBreak = true; if (!canForceBreak) ImGui::EndDisabled();
    bool canRepair = machine->getState() == MachineState::Broken;
    if (!canRepair) ImGui::BeginDisabled(); if (ImGui::Button("Dispatch Technician", ImVec2(-1, 32))) command.dispatchTechnician = true; if (!canRepair) ImGui::EndDisabled();
    bool canInstantRepair = machine->getState() == MachineState::Broken || machine->getState() == MachineState::Repairing;
    if (!canInstantRepair) ImGui::BeginDisabled(); if (ImGui::Button("Instant Repair", ImVec2(-1, 32))) command.instantRepair = true; if (!canInstantRepair) ImGui::EndDisabled();
}

static void RenderEventFeed(const FactorySnapshot& factory, FactoryCommand& command) {
    UI::SectionTitle("Event Log"); ImGui::SameLine(); if (ImGui::Button("Clear Log")) command.clearLog = true; ImGui::Spacing();
    ImGui::BeginChild("EventList", ImVec2(0, ImGui::GetContentRegionAvail().y), false);
    for (const EventSnapshot& ev : factory.getEvents()) {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::COL_MUTED); ImGui::Text("[%04d]", ev.getTick()); ImGui::PopStyleColor(); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::EventColor(ev.getType())); ImGui::TextWrapped("%s", ev.getText().c_str()); ImGui::PopStyleColor();
    }
    ImGui::EndChild();
}

static void RenderScene(const FactorySnapshot& factory, UIState& uiState, FactoryCommand& command) {
    ImGui::BeginChild("MainScene", ImVec2(0, 0), true);
    UI::SectionTitle("Factory Floor"); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    switch (uiState.activeZone) {
    case ZoneTab::SmartFarm: UI::RenderSmartFarmScene(factory, command); break;
    case ZoneTab::Storage: UI::RenderStorageScene(factory); break;
    case ZoneTab::Bakery: UI::RenderBakeryScene(factory, uiState, command); break;
    case ZoneTab::Fulfillment: UI::RenderFulfillmentScene(factory, uiState, command); break;
    case ZoneTab::Maintenance: UI::RenderMaintenanceScene(factory, command); break;
    }
    ImGui::EndChild();
}

void RenderSmartFarmBakeryUI(const FactorySnapshot& factory, UIState& uiState, FactoryCommand& command) {
    ImGui::SetNextWindowPos(ImVec2(0, 0)); ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("SmartFarm Bakery Factory", nullptr, flags);

    RenderTopBar(factory, command); ImGui::Spacing();
    float fullW = ImGui::GetContentRegionAvail().x; float fullH = ImGui::GetContentRegionAvail().y;
    float leftW = 184.0f; float rightW = 392.0f; float gap = 10.0f;
    float centerW = fullW - leftW - rightW - gap * 2.0f; if (centerW < 500.0f) centerW = 500.0f;

    RenderZoneNav(factory, uiState);
    ImGui::SameLine(0, gap); ImGui::BeginChild("CenterColumn", ImVec2(centerW, fullH), false); RenderScene(factory, uiState, command); ImGui::EndChild();
    ImGui::SameLine(0, gap); ImGui::BeginChild("RightColumn", ImVec2(rightW, fullH), true);

    float eventLogH = std::max(280.0f, fullH * 0.38f); if (eventLogH > fullH - 260.0f) eventLogH = std::max(220.0f, fullH - 260.0f);
    float upperH = std::max(220.0f, fullH - eventLogH - 12.0f);

    ImGui::BeginChild("RightColumnUpper", ImVec2(0, upperH), false);
    RenderMachineBoard(factory, uiState, command); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    RenderStatistics(factory); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    RenderPipelineBufferLoad(factory); ImGui::EndChild();

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    ImGui::BeginChild("EventLogPanel", ImVec2(0, eventLogH), false); RenderEventFeed(factory, command); ImGui::EndChild();
    ImGui::EndChild(); ImGui::End();
}