#include "UI_Core.h"
#include <algorithm>
#include <cstdio>

namespace Theme {
    void ApplySmartFarmTheme() {
        ImGui::StyleColorsLight();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f; style.ChildRounding = 8.0f; style.FrameRounding = 6.0f;
        style.GrabRounding = 6.0f; style.ScrollbarRounding = 6.0f; style.WindowBorderSize = 0.0f;
        style.FrameBorderSize = 1.0f; style.ItemSpacing = ImVec2(7, 7); style.WindowPadding = ImVec2(12, 12);
        style.Colors[ImGuiCol_WindowBg] = COL_BG; style.Colors[ImGuiCol_ChildBg] = COL_PANEL;
        style.Colors[ImGuiCol_Border] = COL_LINE; style.Colors[ImGuiCol_Text] = COL_TEXT;
        style.Colors[ImGuiCol_Button] = ImVec4(1, 1, 1, 1); style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.94f, 0.96f, 0.91f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.88f, 0.92f, 0.84f, 1.0f); style.Colors[ImGuiCol_FrameBg] = ImVec4(1, 1, 1, 1);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.97f, 0.92f, 1.0f); style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.91f, 0.94f, 0.88f, 1.0f);
        style.Colors[ImGuiCol_SliderGrab] = COL_GREEN; style.Colors[ImGuiCol_SliderGrabActive] = COL_GREEN; style.Colors[ImGuiCol_CheckMark] = COL_GREEN;
    }
    ImGuiWindowFlags NoScrollFlags() { return ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse; }
    ImU32 ToU32(ImVec4 color) { return ImGui::ColorConvertFloat4ToU32(color); }
    ImVec4 StatusColor(MachineState state) {
        switch (state) {
        case MachineState::Idle: return COL_MUTED; case MachineState::Working: return COL_GREEN;
        case MachineState::Paused: return COL_BLUE; case MachineState::Broken: return COL_BAD; case MachineState::Repairing: return COL_WARN;
        } return COL_MUTED;
    }
    ImVec4 ItemColor(ItemType item) {
        switch (item) {
        case ItemType::Wheat: return COL_WHEAT; case ItemType::Flour: return COL_FLOUR; case ItemType::Dough: return COL_DOUGH;
        case ItemType::Bread: return COL_BREAD; case ItemType::Parcel: return COL_PARCEL;
        } return COL_MUTED;
    }
    ImVec4 EventColor(EventType type) {
        switch (type) {
        case EventType::Info: return COL_BLUE; case EventType::Success: return COL_GREEN; case EventType::Warning: return COL_WARN;
        case EventType::Error: return COL_BAD; case EventType::Production: return COL_BREAD; case EventType::Breakdown: return COL_BAD; case EventType::Maintenance: return COL_WARN;
        } return COL_MUTED;
    }
    const char* StatusText(MachineState state) {
        switch (state) {
        case MachineState::Idle: return "IDLE"; case MachineState::Working: return "WORKING"; case MachineState::Paused: return "PAUSED";
        case MachineState::Broken: return "BROKEN"; case MachineState::Repairing: return "REPAIRING";
        } return "";
    }
    const char* ScenarioName(Scenario scenario) {
        switch (scenario) {
        case Scenario::Normal: return "Normal"; case Scenario::Bottleneck: return "Bottleneck";
        case Scenario::BreakdownManual: return "Breakdown - Manual Repair"; case Scenario::BreakdownAuto: return "Breakdown - Auto Technician"; case Scenario::Overflow: return "Overflow";
        } return "";
    }
    const char* ScenarioDescription(Scenario scenario) {
        switch (scenario) {
        case Scenario::Normal: return "Balanced line: machines run at default process time.";
        case Scenario::Bottleneck: return "Oven process time increases to 12 ticks, causing dough buildup.";
        case Scenario::BreakdownManual: return "Machines may break during work. Use Dispatch Technician manually.";
        case Scenario::BreakdownAuto: return "Machines may break, and the Technician repairs them automatically.";
        case Scenario::Overflow: return "Full queues can waste products and increase the lost-product counter.";
        } return "";
    }
}

namespace UI {
    static float Clamp01(float value) { return std::max(0.0f, std::min(1.0f, value)); }
    void TextMuted(const char* text) { ImGui::PushStyleColor(ImGuiCol_Text, Theme::COL_MUTED); ImGui::TextUnformatted(text); ImGui::PopStyleColor(); }
    void TextMuted(const std::string& text) { TextMuted(text.c_str()); }
    void TextMutedWrapped(const char* text) { ImGui::PushStyleColor(ImGuiCol_Text, Theme::COL_MUTED); ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x); ImGui::TextWrapped("%s", text); ImGui::PopTextWrapPos(); ImGui::PopStyleColor(); }
    void TextMutedF(const char* fmt, int a) { ImGui::PushStyleColor(ImGuiCol_Text, Theme::COL_MUTED); ImGui::Text(fmt, a); ImGui::PopStyleColor(); }
    void TextMutedF(const char* fmt, int a, int b) { ImGui::PushStyleColor(ImGuiCol_Text, Theme::COL_MUTED); ImGui::Text(fmt, a, b); ImGui::PopStyleColor(); }
    void SectionTitle(const char* title) { ImGui::PushStyleColor(ImGuiCol_Text, Theme::COL_TEXT); ImGui::TextUnformatted(title); ImGui::PopStyleColor(); }
    void Progress(float value, ImVec4 color, const ImVec2& size) { ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color); ImGui::ProgressBar(Clamp01(value), size, ""); ImGui::PopStyleColor(); }

    const MachineSnapshot* FindMachine(const FactorySnapshot& factory, int id) { return factory.findMachine(id); }

    // --- 내부 그래픽 그리기 유틸리티 (헤더에 노출할 필요 없는 함수들) ---
    static void DrawSeed(ImDrawList* draw, ImVec2 center, float scale) {
        draw->AddCircleFilled(center, 7.0f * scale, Theme::ToU32(ImVec4(0.58f, 0.36f, 0.19f, 1.0f)), 18);
        draw->AddCircle(center, 7.0f * scale, Theme::ToU32(ImVec4(0.31f, 0.20f, 0.12f, 1.0f)), 18, 1.5f);
    }
    static void DrawSprout(ImDrawList* draw, ImVec2 center, float scale, float growth) {
        float h = (18.0f + 24.0f * growth) * scale; ImVec2 base = ImVec2(center.x, center.y + 22.0f * scale); ImVec2 tip = ImVec2(center.x, base.y - h);
        ImU32 stem = Theme::ToU32(Theme::COL_GREEN); draw->AddLine(base, tip, stem, 3.0f * scale); draw->AddCircleFilled(tip, 3.0f * scale, stem, 12);
        draw->AddCircleFilled(ImVec2(center.x - 10.0f * scale, tip.y + 8.0f * scale), 7.0f * scale, stem, 12); draw->AddCircleFilled(ImVec2(center.x + 11.0f * scale, tip.y + 14.0f * scale), 7.0f * scale, stem, 12);
    }
    static void DrawWheat(ImDrawList* draw, ImVec2 center, float scale) {
        ImU32 stalk = Theme::ToU32(ImVec4(0.77f, 0.53f, 0.11f, 1.0f)); ImU32 grain = Theme::ToU32(Theme::COL_WHEAT); float baseY = center.y + 26.0f * scale;
        for (int s = -1; s <= 1; ++s) {
            float x = center.x + s * 12.0f * scale; ImVec2 base = ImVec2(x, baseY); ImVec2 top = ImVec2(x + s * 3.0f * scale, center.y - 30.0f * scale);
            draw->AddLine(base, top, stalk, 2.4f * scale);
            for (int i = 0; i < 5; ++i) {
                float y = top.y + i * 8.0f * scale; draw->AddCircleFilled(ImVec2(top.x - 4.0f * scale, y), 4.2f * scale, grain, 12); draw->AddCircleFilled(ImVec2(top.x + 4.0f * scale, y + 3.0f * scale), 4.2f * scale, grain, 12);
            }
        }
    }
    static void DrawBreadIcon(ImDrawList* draw, ImVec2 min, ImVec2 max, ImVec4 color) {
        ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f); float w = (max.x - min.x) * 0.62f; float h = (max.y - min.y) * 0.42f;
        ImVec2 loafMin = ImVec2(center.x - w * 0.5f, center.y - h * 0.35f); ImVec2 loafMax = ImVec2(center.x + w * 0.5f, center.y + h * 0.55f);
        draw->AddRectFilled(loafMin, loafMax, Theme::ToU32(color), h * 0.45f); draw->AddRect(loafMin, loafMax, Theme::ToU32(ImVec4(0.42f, 0.22f, 0.10f, 1.0f)), h * 0.45f, 0, 1.5f);
        for (int i = -1; i <= 1; ++i) { float x = center.x + i * w * 0.18f; draw->AddLine(ImVec2(x - 4.0f, center.y - 6.0f), ImVec2(x + 3.0f, center.y + 7.0f), Theme::ToU32(ImVec4(0.98f, 0.78f, 0.45f, 1.0f)), 2.0f); }
    }
    static void DrawParcelIcon(ImDrawList* draw, ImVec2 min, ImVec2 max) {
        ImVec2 pad = ImVec2((max.x - min.x) * 0.22f, (max.y - min.y) * 0.22f); ImVec2 a = ImVec2(min.x + pad.x, min.y + pad.y); ImVec2 b = ImVec2(max.x - pad.x, max.y - pad.y);
        draw->AddRectFilled(a, b, Theme::ToU32(Theme::COL_PARCEL), 4.0f); draw->AddRect(a, b, Theme::ToU32(ImVec4(0.28f, 0.20f, 0.13f, 1.0f)), 4.0f, 0, 1.5f);
        draw->AddLine(ImVec2((a.x + b.x) * 0.5f, a.y), ImVec2((a.x + b.x) * 0.5f, b.y), Theme::ToU32(ImVec4(0.72f, 0.60f, 0.43f, 1.0f)), 2.0f);
        draw->AddLine(ImVec2(a.x, (a.y + b.y) * 0.5f), ImVec2(b.x, (a.y + b.y) * 0.5f), Theme::ToU32(ImVec4(0.72f, 0.60f, 0.43f, 1.0f)), 2.0f);
    }
    static void DrawStorageIcon(ImDrawList* draw, ImVec2 min, ImVec2 max, const StorageSnapshot& bin) {
        const ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f + 2.0f); const float w = max.x - min.x; const float h = max.y - min.y;
        switch (bin.getItemType()) {
        case ItemType::Wheat: DrawWheat(draw, ImVec2(center.x, center.y + 3.0f), 0.34f); break;
        case ItemType::Flour: { const float r = std::min(w, h) * 0.20f; draw->AddCircleFilled(center, r, Theme::ToU32(Theme::ItemColor(bin.getItemType())), 24); draw->AddCircle(center, r, Theme::ToU32(Theme::COL_LINE), 24, 1.2f); break; }
        case ItemType::Dough: { const float r = std::min(w, h) * 0.22f; draw->AddCircleFilled(center, r, Theme::ToU32(Theme::ItemColor(bin.getItemType())), 24); draw->AddCircle(center, r, Theme::ToU32(ImVec4(0.45f, 0.30f, 0.18f, 1.0f)), 24, 1.0f); break; }
        case ItemType::Bread: DrawBreadIcon(draw, min, max, Theme::ItemColor(bin.getItemType())); break;
        case ItemType::Parcel: DrawParcelIcon(draw, min, max); break;
        }
    }

    void DrawPlotTile(const FactorySnapshot& factory, FactoryCommand& command, int index, float size) {
        const PlotSnapshot& plot = factory.getPlots()[index];
        ImGui::PushID(index);
        ImGui::InvisibleButton("plot", ImVec2(size, size));
        bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
        bool hovered = ImGui::IsItemHovered();
        if (clicked && (plot.getState() == PlotState::Empty || plot.getState() == PlotState::Ready)) command.clickedPlot = index;

        ImVec2 min = ImGui::GetItemRectMin(); ImVec2 max = ImGui::GetItemRectMax(); ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec4 fill = hovered ? ImVec4(0.64f, 0.43f, 0.26f, 1.0f) : Theme::COL_SOIL;
        ImVec4 border = plot.getState() == PlotState::Ready ? Theme::COL_WHEAT : Theme::COL_LINE;

        draw->AddRectFilled(min, max, Theme::ToU32(fill), 8.0f);
        draw->AddRect(min, max, Theme::ToU32(border), 8.0f, 0, plot.getState() == PlotState::Ready ? 2.5f : 1.0f);

        for (int row = 0; row < 3; ++row) {
            float y = min.y + 18.0f + row * 20.0f;
            draw->AddLine(ImVec2(min.x + 10.0f, y), ImVec2(max.x - 10.0f, y + 4.0f), Theme::ToU32(ImVec4(0.39f, 0.25f, 0.14f, 1.0f)), 1.0f);
        }

        if (plot.getState() == PlotState::Empty) {
            draw->AddCircleFilled(center, 18.0f, Theme::ToU32(ImVec4(0.95f, 0.87f, 0.68f, 1.0f)), 24);
            draw->AddLine(ImVec2(center.x - 8.0f, center.y), ImVec2(center.x + 8.0f, center.y), Theme::ToU32(Theme::COL_GREEN), 2.5f);
            draw->AddLine(ImVec2(center.x, center.y - 8.0f), ImVec2(center.x, center.y + 8.0f), Theme::ToU32(Theme::COL_GREEN), 2.5f);
        } else if (plot.getState() == PlotState::Seeded) { DrawSeed(draw, center, 1.0f);
        } else if (plot.getState() == PlotState::Growing) { DrawSprout(draw, center, 1.0f, plot.getGrowth());
        } else { DrawWheat(draw, ImVec2(center.x, center.y + 5.0f), 0.7f); }

        float barPad = 10.0f; ImVec2 barMin = ImVec2(min.x + barPad, max.y - 14.0f); ImVec2 barMax = ImVec2(max.x - barPad, max.y - 8.0f);
        draw->AddRectFilled(barMin, barMax, Theme::ToU32(ImVec4(0.28f, 0.21f, 0.14f, 1.0f)), 3.0f);
        if (plot.getState() != PlotState::Empty) {
            ImVec2 fillMax = ImVec2(barMin.x + (barMax.x - barMin.x) * plot.getGrowth(), barMax.y);
            draw->AddRectFilled(barMin, fillMax, Theme::ToU32(plot.getState() == PlotState::Ready ? Theme::COL_WHEAT : Theme::COL_GREEN_LIGHT), 3.0f);
        }
        ImGui::PopID();
    }

    void DrawStorageGrid(const StorageSnapshot& bin, float slotSize) {
        char countText[64]; std::snprintf(countText, sizeof(countText), "%d / %d", bin.getCount(), bin.getCapacity());
        ImGui::BeginGroup(); SectionTitle(bin.getName().c_str()); ImGui::SameLine(); TextMuted(countText);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        int columns = 5; float gap = 6.0f;
        for (int i = 0; i < bin.getCapacity(); ++i) {
            ImGui::PushID(i); ImGui::Dummy(ImVec2(slotSize, slotSize));
            ImVec2 min = ImGui::GetItemRectMin(); ImVec2 max = ImGui::GetItemRectMax();
            bool filled = i < bin.getCount();
            ImVec4 slotColor = filled ? ImVec4(0.96f, 0.93f, 0.82f, 1.0f) : ImVec4(0.87f, 0.89f, 0.84f, 1.0f);
            draw->AddRectFilled(min, max, Theme::ToU32(slotColor), 6.0f);
            draw->AddRect(min, max, Theme::ToU32(filled ? Theme::ItemColor(bin.getItemType()) : Theme::COL_LINE), 6.0f, 0, filled ? 1.8f : 1.0f);
            if (filled) {
                draw->PushClipRect(min, max, true);
                DrawStorageIcon(draw, ImVec2(min.x + 4.0f, min.y + 4.0f), ImVec2(max.x - 4.0f, max.y - 4.0f), bin);
                draw->PopClipRect();
            }
            ImGui::PopID();
            if ((i + 1) % columns != 0) ImGui::SameLine(0, gap);
        }
        ImGui::EndGroup();
    }

    void DrawStatChip(const char* label, const char* value, ImVec4 color) {
        ImGui::PushID(label); ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1, 1, 1, 1));
        ImGui::BeginChild("##StatChipBody", ImVec2(0, 86), true, Theme::NoScrollFlags());
        TextMuted(label); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, color); ImGui::SetWindowFontScale(1.18f); ImGui::TextWrapped("%s", value); ImGui::SetWindowFontScale(1.0f); ImGui::PopStyleColor();
        ImGui::EndChild(); ImGui::PopStyleColor(); ImGui::PopID();
    }

    void DrawMachineCard(const FactorySnapshot& factory, UIState& uiState, const MachineSnapshot* machine, float width, float height) {
        if (!machine) return;
        ImGui::PushID(machine->getId());
        bool selected = uiState.selectedMachineId == machine->getId();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, selected ? ImVec4(0.98f, 0.95f, 0.83f, 1.0f) : ImVec4(1, 1, 1, 1));
        ImGui::BeginChild(machine->getName().c_str(), ImVec2(width, height), true, Theme::NoScrollFlags());

        ImDrawList* draw = ImGui::GetWindowDrawList(); ImVec2 min = ImGui::GetWindowPos(); ImVec2 max = ImVec2(min.x + ImGui::GetWindowSize().x, min.y + ImGui::GetWindowSize().y);
        draw->AddRectFilled(min, ImVec2(min.x + 5.0f, max.y), Theme::ToU32(Theme::StatusColor(machine->getState())), 4.0f);
        if (ImGui::Selectable("##selectMachine", false, 0, ImVec2(width - 16.0f, height - 18.0f))) uiState.selectedMachineId = machine->getId();

        ImVec2 cardMin = ImGui::GetItemRectMin(); ImVec2 iconCenter = ImVec2(cardMin.x + width * 0.5f, cardMin.y + 58.0f);
        ImU32 body = Theme::ToU32(ImVec4(0.74f, 0.76f, 0.70f, 1.0f)); ImU32 dark = Theme::ToU32(ImVec4(0.34f, 0.36f, 0.33f, 1.0f));
        draw->AddRectFilled(ImVec2(iconCenter.x - 40.0f, iconCenter.y - 22.0f), ImVec2(iconCenter.x + 40.0f, iconCenter.y + 22.0f), body, 8.0f);
        draw->AddRect(ImVec2(iconCenter.x - 40.0f, iconCenter.y - 22.0f), ImVec2(iconCenter.x + 40.0f, iconCenter.y + 22.0f), dark, 8.0f, 0, 1.5f);
        draw->AddCircleFilled(ImVec2(iconCenter.x - 20.0f, iconCenter.y), 7.0f, Theme::ToU32(Theme::StatusColor(machine->getState())), 18);
        draw->AddCircleFilled(ImVec2(iconCenter.x + 20.0f, iconCenter.y), 7.0f, dark, 18);

        if (machine->getState() == MachineState::Broken) {
            draw->AddLine(ImVec2(iconCenter.x - 26.0f, iconCenter.y - 28.0f), ImVec2(iconCenter.x + 26.0f, iconCenter.y + 28.0f), Theme::ToU32(Theme::COL_BAD), 4.0f);
            draw->AddLine(ImVec2(iconCenter.x + 26.0f, iconCenter.y - 28.0f), ImVec2(iconCenter.x - 26.0f, iconCenter.y + 28.0f), Theme::ToU32(Theme::COL_BAD), 4.0f);
        } else if (machine->getState() == MachineState::Paused) {
            draw->AddRectFilled(ImVec2(iconCenter.x - 13.0f, iconCenter.y - 17.0f), ImVec2(iconCenter.x - 5.0f, iconCenter.y + 17.0f), Theme::ToU32(Theme::COL_BLUE), 2.0f);
            draw->AddRectFilled(ImVec2(iconCenter.x + 5.0f, iconCenter.y - 17.0f), ImVec2(iconCenter.x + 13.0f, iconCenter.y + 17.0f), Theme::ToU32(Theme::COL_BLUE), 2.0f);
        }

        ImGui::SetCursorPos(ImVec2(12.0f, 12.0f)); ImGui::TextColored(Theme::StatusColor(machine->getState()), "%s", Theme::StatusText(machine->getState()));
        ImGui::SetCursorPos(ImVec2(12.0f, height - 70.0f)); ImGui::PushTextWrapPos(width - 12.0f);
        if (selected) ImGui::TextColored(Theme::COL_BLUE, "%s", machine->getName().c_str()); else ImGui::TextWrapped("%s", machine->getName().c_str());
        ImGui::PopTextWrapPos();
        ImGui::SetCursorPos(ImVec2(12.0f, height - 34.0f)); Progress(machine->getProgress(), Theme::StatusColor(machine->getState()), ImVec2(width - 24.0f, 8.0f));
        ImGui::EndChild(); ImGui::PopStyleColor(); ImGui::PopID();
    }

    void DrawOrderCard(const OrderSnapshot& order, float width) {
        ImGui::PushID(order.getId());
        ImGui::BeginChild("OrderCard", ImVec2(width, 110), true, Theme::NoScrollFlags());
        char title[96]; std::snprintf(title, sizeof(title), "#%d  %s", order.getId(), order.getCity().c_str());
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::COL_TEXT); ImGui::TextUnformatted(title); ImGui::PopStyleColor();
        TextMutedF("Packed: %d / %d", order.getPackedCount(), order.getCount()); TextMutedF("Priority: %d", order.getPriority());
        Progress(order.getProgress(), order.getPriority() >= 3 ? Theme::COL_BAD : Theme::COL_BLUE, ImVec2(-1, 8));
        ImGui::EndChild(); ImGui::PopID();
    }

    void AutomationToggleButton(const char* visibleLabel, const char* uniqueId, bool enabled, bool& commandFlag, float width) {
        std::string buttonLabel = std::string(visibleLabel) + "##" + uniqueId;
        ImGui::PushStyleColor(ImGuiCol_Button, enabled ? Theme::COL_GREEN : ImVec4(1, 1, 1, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, enabled ? ImVec4(0.12f, 0.44f, 0.24f, 1.0f) : ImVec4(0.94f, 0.96f, 0.91f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, enabled ? ImVec4(0.08f, 0.36f, 0.18f, 1.0f) : ImVec4(0.88f, 0.92f, 0.84f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, enabled ? ImVec4(1, 1, 1, 1) : Theme::COL_TEXT);
        if (ImGui::Button(buttonLabel.c_str(), ImVec2(width, 28))) commandFlag = true;
        ImGui::PopStyleColor(4);
    }

    void DrawAutomationRow(const char* routeLabel, const char* id, bool autoEnabled, bool& toggleCommand, bool& manualCommand, const char* manualLabel) {
        ImGui::PushID(id); ImGui::TextUnformatted(routeLabel); ImGui::SameLine(260.0f);
        AutomationToggleButton(autoEnabled ? "AUTO" : "MANUAL", "toggle", autoEnabled, toggleCommand, 92.0f);
        ImGui::SameLine();
        std::string manualButton = std::string(manualLabel) + "##manual";
        if (ImGui::Button(manualButton.c_str(), ImVec2(112.0f, 28))) manualCommand = true;
        ImGui::PopID();
    }
}