#pragma once

#include "Types.h"
#include "imgui.h"
#include <string>

struct UIState {
    ZoneTab activeZone = ZoneTab::SmartFarm;
    int selectedMachineId = 4;
};

namespace Theme {
    inline const ImVec4 COL_BG = ImVec4(0.91f, 0.94f, 0.92f, 1.0f);
    inline const ImVec4 COL_PANEL = ImVec4(0.99f, 0.99f, 0.96f, 1.0f);
    inline const ImVec4 COL_TEXT = ImVec4(0.13f, 0.15f, 0.13f, 1.0f);
    inline const ImVec4 COL_MUTED = ImVec4(0.43f, 0.47f, 0.42f, 1.0f);
    inline const ImVec4 COL_LINE = ImVec4(0.75f, 0.78f, 0.71f, 1.0f);
    inline const ImVec4 COL_GREEN = ImVec4(0.16f, 0.52f, 0.29f, 1.0f);
    inline const ImVec4 COL_GREEN_LIGHT = ImVec4(0.73f, 0.88f, 0.57f, 1.0f);
    inline const ImVec4 COL_BLUE = ImVec4(0.18f, 0.39f, 0.78f, 1.0f);
    inline const ImVec4 COL_WARN = ImVec4(0.82f, 0.49f, 0.12f, 1.0f);
    inline const ImVec4 COL_BAD = ImVec4(0.78f, 0.16f, 0.12f, 1.0f);
    inline const ImVec4 COL_WHEAT = ImVec4(0.91f, 0.67f, 0.22f, 1.0f);
    inline const ImVec4 COL_SOIL = ImVec4(0.48f, 0.31f, 0.18f, 1.0f);
    inline const ImVec4 COL_FLOUR = ImVec4(0.92f, 0.90f, 0.83f, 1.0f);
    inline const ImVec4 COL_DOUGH = ImVec4(0.80f, 0.58f, 0.37f, 1.0f);
    inline const ImVec4 COL_BREAD = ImVec4(0.74f, 0.38f, 0.15f, 1.0f);
    inline const ImVec4 COL_PARCEL = ImVec4(0.52f, 0.39f, 0.27f, 1.0f);

    void ApplySmartFarmTheme();
    ImGuiWindowFlags NoScrollFlags();
    ImU32 ToU32(ImVec4 color);
    ImVec4 StatusColor(MachineState state);
    ImVec4 ItemColor(ItemType item);
    ImVec4 EventColor(EventType type);
    const char* StatusText(MachineState state);
    const char* ScenarioName(Scenario scenario);
    const char* ScenarioDescription(Scenario scenario);
}

// 재사용 가능한 UI 컴포넌트 네임스페이스
namespace UI {
    void TextMuted(const char* text);
    void TextMuted(const std::string& text);
    void TextMutedWrapped(const char* text);
    void TextMutedF(const char* fmt, int a);
    void TextMutedF(const char* fmt, int a, int b);
    void SectionTitle(const char* title);
    void Progress(float value, ImVec4 color, const ImVec2& size = ImVec2(-1, 8));

    void DrawPlotTile(const FactorySnapshot& factory, FactoryCommand& command, int index, float size);
    void DrawStorageGrid(const StorageSnapshot& bin, float slotSize = 42.0f);
    void DrawMachineCard(const FactorySnapshot& factory, UIState& uiState, const MachineSnapshot* machine, float width, float height);
    void DrawOrderCard(const OrderSnapshot& order, float width);
    void DrawStatChip(const char* label, const char* value, ImVec4 color);
    
    void AutomationToggleButton(const char* visibleLabel, const char* uniqueId, bool enabled, bool& commandFlag, float width = 130.0f);
    void DrawAutomationRow(const char* routeLabel, const char* id, bool autoEnabled, bool& toggleCommand, bool& manualCommand, const char* manualLabel);

    const MachineSnapshot* FindMachine(const FactorySnapshot& factory, int id);
}