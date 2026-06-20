# BBANG - Smart Farm Bakery Simulation

## UML Class Diagram

#1. Backend Core Inheritance
![Backend Core Inheritance](UML/backend_Core_Inheritance_UML.png)
A class hierarchy of machines and products that achieves perfect polymorphism by utilizing SimulationObject as the base class, completely eliminating runtime type branching (if/switch statements).


#2. Backend Factory Composition
![Backend Factory Composition](UML/backend_Factory_Composition_UML.png)
An architectural view illustrating how the Factory core centrally manages the simulation state and owned data, while generating read-only Snapshot and write-only Command objects to communicate with the UI.


#3. Frontend/UI Overview
![Frontend/UI Overview](UML/frontend_UI_UML.png)
A modular rendering architecture completely isolated from the business logic, utilizing ImGui and reusable components (UI_Core) to visualize simulation data and capture user commands.


#4. Full Project Overview
![Full Project Overview](UML/full_Project_Overview_UML.png)
A comprehensive blueprint of the system demonstrating loose coupling between the application (main), frontend, backend, and data layers via Data Transfer Objects (DTOs) without direct dependencies.

## Simulation

#1. Manual_Normal
![Manual_Normal Simulation](Simulation_GIF/manual.gif)

#2. Auto_Overflow
![Auto_Overflow Simulation](Simulation_GIF/auto_overflow.gif)
