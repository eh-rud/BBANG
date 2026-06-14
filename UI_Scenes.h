#pragma once

#include "Types.h"
#include "UI_Core.h"

namespace UI {
    void RenderSmartFarmScene(const FactorySnapshot& factory, FactoryCommand& command);
    void RenderStorageScene(const FactorySnapshot& factory);
    void RenderBakeryScene(const FactorySnapshot& factory, UIState& uiState, FactoryCommand& command);
    void RenderFulfillmentScene(const FactorySnapshot& factory, UIState& uiState, FactoryCommand& command);
    void RenderMaintenanceScene(const FactorySnapshot& factory, FactoryCommand& command);
    
    void RenderAutomationControlPanel(const FactorySnapshot& factory, FactoryCommand& command);
    void RenderBakeryAutomationPanel(const FactorySnapshot& factory, FactoryCommand& command);
    void RenderFulfillmentAutomationPanel(const FactorySnapshot& factory, FactoryCommand& command);
}