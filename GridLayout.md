# Grid layout notes

- The app will have a grid layout.
-  Widgets can be placed on the grid, moved around, and removed


# Components

- WidgetGrid
    - widgets: WidgetFrame[]

- WidgetFrame
    - name : string
    - x : int
    - y : int
    - width : int
    - height : int


# TODO

- ABM runs in the background
    - pause/play/restart
- Add Agent manager widgets with controls
    - Creating the widget adds the manager to the ABM (Calling the constructor)
    - Destroying removes the manager (call the destructor)
- Add Spread and Depth Widgets to view the simulation
    - These can be set to view certain assets being traded (FOOD, OIL, FUEL, ect...)