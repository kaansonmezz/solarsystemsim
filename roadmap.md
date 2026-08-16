# SOLAR — Development Roadmap

> **Project type:** Scientific visualization + astrodynamics + rocket engineering + KSP/RSS mission planning application  
> **Primary language:** C++  
> **Target:** Linux first, Windows compatible  
> **Architecture:** Multi-module, multi-file, desktop GUI application  
> **Rendering:** Real-time 3D  
> **Long-term goal:** A unified tool for real Solar System visualization, orbital mechanics, rocket/stage analysis, interplanetary mission planning, trajectory simulation, and KSP Real Solar System mission support.

---

# 1. Project Vision

SOLAR should not be designed as a simple 3D planet viewer.

The final application should combine:

- Real Solar System visualization
- Real astronomical ephemerides
- Planet orientation and coordinate frames
- Orbital mechanics
- Rocket and stage engineering
- Delta-v / TWR / burn-time calculations
- Hohmann and Lambert transfers
- Launch-window analysis
- Porkchop plots
- Mission building
- Patched-conics trajectories
- Finite-burn simulation
- Atmospheric ascent simulation
- Gravity-turn analysis
- Landing / reentry tools
- KSP Real Solar System support
- Live KSP telemetry
- Planned-vs-actual trajectory comparison
- Advanced astrodynamics research tools

The core design principle is:

```text
Correct physics and coordinate systems first.
Visualization second.
Optimization and automation after validation.
```

---

# 2. Recommended Technology Stack

## Language

```text
C++20 / C++23
```

Prefer modern C++ practices:

- RAII
- smart pointers only where ownership requires them
- value semantics where possible
- `std::chrono`
- `std::filesystem`
- `std::span`
- `std::optional`
- `std::variant`
- ranges where useful
- strong types for physical quantities where practical

---

## Build System

```text
CMake
```

Recommended minimum:

```cmake
cmake_minimum_required(VERSION 3.24)
```

Use separate targets where appropriate:

```text
solar_core
solar_astronomy
solar_astrodynamics
solar_rocket
solar_simulation
solar_renderer
solar_ui
solar_ksp
solar_tests
solar
```

---

## Window / Input

Recommended:

```text
GLFW
```

Responsibilities:

- window creation
- keyboard input
- mouse input
- OpenGL context
- resize callbacks
- fullscreen/windowed mode

---

## Rendering

Start with:

```text
OpenGL 4.6
```

Do **not** start with Vulkan.

The renderer should be abstracted enough that Vulkan could theoretically be added later, but OpenGL should be the only implementation during the first major versions.

Recommended helpers:

```text
GLAD
GLM
stb_image
```

---

## GUI

Recommended:

```text
Dear ImGui
```

For scientific plots:

```text
ImPlot
```

The GUI should be dockable.

Expected panels:

- Solar System
- Body Inspector
- Simulation Time
- Rocket Editor
- Stage Inspector
- Orbit Calculator
- Maneuver Planner
- Mission Builder
- Transfer Planner
- Porkchop Plot
- Ascent Simulator
- Telemetry
- Post-flight Analysis
- Settings
- Debug Console

---

## Astronomy / Ephemeris

Recommended:

```text
NASA CSPICE
```

Use SPICE for:

- ephemeris
- planetary positions
- velocities
- reference frames
- time conversion
- body constants
- body orientation where available

Keep a wrapper around CSPICE so the rest of the application does not directly depend on raw SPICE API calls.

---

## Math

Recommended:

```text
GLM
```

Use double precision in scientific calculations:

```cpp
glm::dvec3
glm::dmat3
glm::dmat4
```

Rendering can use float after origin rebasing / coordinate scaling.

---

## Serialization

Recommended:

```text
nlohmann/json
```

Use JSON for:

- rocket definitions
- engine database
- mission files
- application settings
- RSS profiles
- saved scenarios

---

## Logging

Recommended:

```text
spdlog
```

Logging levels:

```text
trace
debug
info
warn
error
critical
```

---

## Testing

Recommended:

```text
Catch2
```

or:

```text
GoogleTest
```

Catch2 is sufficient for the initial project.

---

# 3. High-Level Architecture

```text
                         ┌─────────────────────┐
                         │      SOLAR APP      │
                         └──────────┬──────────┘
                                    │
               ┌────────────────────┼────────────────────┐
               │                    │                    │
          APPLICATION             GUI              RENDERER
               │                    │                    │
               └──────────────┬─────┴─────┬──────────────┘
                              │           │
                         SIMULATION      SCENE
                              │
        ┌─────────────────────┼─────────────────────────┐
        │                     │                         │
   ASTRONOMY             ASTRODYNAMICS                ROCKET
        │                     │                         │
   CSPICE wrapper         Orbits / Lambert        Engine / Stage
   Time / Frames          Transfers / SOI         Δv / TWR
        │                     │                         │
        └─────────────────────┼─────────────────────────┘
                              │
                           MISSION
                              │
                    ┌─────────┴──────────┐
                    │                    │
               OPTIMIZATION            KSP
                                      / RSS
```

---

# 4. Proposed Repository Structure

```text
solar/
│
├── CMakeLists.txt
├── README.md
├── roadmap.md
├── LICENSE
│
├── cmake/
│   ├── Dependencies.cmake
│   ├── CompilerWarnings.cmake
│   └── Sanitizers.cmake
│
├── assets/
│   ├── textures/
│   ├── fonts/
│   ├── icons/
│   └── kernels/
│
├── config/
│   ├── app.json
│   ├── bodies.json
│   ├── rss.json
│   └── engines.json
│
├── shaders/
│   ├── planet.vert
│   ├── planet.frag
│   ├── orbit.vert
│   ├── orbit.frag
│   ├── line.vert
│   ├── line.frag
│   ├── star.vert
│   └── star.frag
│
├── src/
│   │
│   ├── main.cpp
│   │
│   ├── app/
│   │   ├── Application.hpp
│   │   ├── Application.cpp
│   │   ├── AppState.hpp
│   │   └── AppState.cpp
│   │
│   ├── core/
│   │   ├── Types.hpp
│   │   ├── Constants.hpp
│   │   ├── Units.hpp
│   │   ├── MathUtils.hpp
│   │   ├── Time.hpp
│   │   ├── Time.cpp
│   │   ├── Logger.hpp
│   │   ├── Logger.cpp
│   │   ├── FileSystem.hpp
│   │   └── FileSystem.cpp
│   │
│   ├── astronomy/
│   │   ├── SpiceContext.hpp
│   │   ├── SpiceContext.cpp
│   │   ├── Ephemeris.hpp
│   │   ├── Ephemeris.cpp
│   │   ├── CelestialBody.hpp
│   │   ├── CelestialBody.cpp
│   │   ├── BodyDatabase.hpp
│   │   ├── BodyDatabase.cpp
│   │   ├── ReferenceFrame.hpp
│   │   ├── ReferenceFrame.cpp
│   │   ├── Orientation.hpp
│   │   └── Orientation.cpp
│   │
│   ├── astrodynamics/
│   │   ├── StateVector.hpp
│   │   ├── OrbitalElements.hpp
│   │   ├── Orbit.hpp
│   │   ├── Orbit.cpp
│   │   ├── Kepler.hpp
│   │   ├── Kepler.cpp
│   │   ├── VisViva.hpp
│   │   ├── Maneuver.hpp
│   │   ├── Maneuver.cpp
│   │   ├── Hohmann.hpp
│   │   ├── Hohmann.cpp
│   │   ├── Lambert.hpp
│   │   ├── Lambert.cpp
│   │   ├── PatchedConics.hpp
│   │   ├── PatchedConics.cpp
│   │   ├── Propagator.hpp
│   │   ├── RK4Propagator.cpp
│   │   ├── RK45Propagator.cpp
│   │   ├── GravityAssist.hpp
│   │   └── GravityAssist.cpp
│   │
│   ├── rocket/
│   │   ├── Engine.hpp
│   │   ├── Engine.cpp
│   │   ├── Stage.hpp
│   │   ├── Stage.cpp
│   │   ├── Vehicle.hpp
│   │   ├── Vehicle.cpp
│   │   ├── DeltaV.hpp
│   │   ├── DeltaV.cpp
│   │   ├── TWR.hpp
│   │   ├── TWR.cpp
│   │   ├── Burn.hpp
│   │   └── Burn.cpp
│   │
│   ├── mission/
│   │   ├── Mission.hpp
│   │   ├── Mission.cpp
│   │   ├── MissionNode.hpp
│   │   ├── MissionNode.cpp
│   │   ├── MissionBudget.hpp
│   │   ├── MissionBudget.cpp
│   │   ├── TransferPlanner.hpp
│   │   ├── TransferPlanner.cpp
│   │   ├── LaunchWindow.hpp
│   │   ├── LaunchWindow.cpp
│   │   ├── Porkchop.hpp
│   │   ├── Porkchop.cpp
│   │   ├── MissionFeasibility.hpp
│   │   └── MissionFeasibility.cpp
│   │
│   ├── simulation/
│   │   ├── SimulationClock.hpp
│   │   ├── SimulationClock.cpp
│   │   ├── SpacecraftState.hpp
│   │   ├── FiniteBurnSimulator.hpp
│   │   ├── FiniteBurnSimulator.cpp
│   │   ├── Atmosphere.hpp
│   │   ├── Atmosphere.cpp
│   │   ├── AscentSimulator.hpp
│   │   ├── AscentSimulator.cpp
│   │   ├── GravityTurn.hpp
│   │   ├── GravityTurn.cpp
│   │   ├── ReentrySimulator.hpp
│   │   ├── ReentrySimulator.cpp
│   │   ├── LandingSimulator.hpp
│   │   └── LandingSimulator.cpp
│   │
│   ├── renderer/
│   │   ├── Renderer.hpp
│   │   ├── Renderer.cpp
│   │   ├── Camera.hpp
│   │   ├── Camera.cpp
│   │   ├── Shader.hpp
│   │   ├── Shader.cpp
│   │   ├── Mesh.hpp
│   │   ├── Mesh.cpp
│   │   ├── Texture.hpp
│   │   ├── Texture.cpp
│   │   ├── PlanetRenderer.hpp
│   │   ├── PlanetRenderer.cpp
│   │   ├── OrbitRenderer.hpp
│   │   ├── OrbitRenderer.cpp
│   │   ├── TrajectoryRenderer.hpp
│   │   ├── TrajectoryRenderer.cpp
│   │   ├── GridRenderer.hpp
│   │   └── GridRenderer.cpp
│   │
│   ├── scene/
│   │   ├── Scene.hpp
│   │   ├── Scene.cpp
│   │   ├── SceneObject.hpp
│   │   ├── SolarSystemScene.hpp
│   │   └── SolarSystemScene.cpp
│   │
│   ├── ui/
│   │   ├── Gui.hpp
│   │   ├── Gui.cpp
│   │   ├── Dockspace.hpp
│   │   ├── Dockspace.cpp
│   │   ├── SolarSystemPanel.hpp
│   │   ├── BodyInspectorPanel.hpp
│   │   ├── TimePanel.hpp
│   │   ├── RocketPanel.hpp
│   │   ├── StagePanel.hpp
│   │   ├── OrbitPanel.hpp
│   │   ├── MissionPanel.hpp
│   │   ├── TransferPanel.hpp
│   │   ├── PorkchopPanel.hpp
│   │   ├── AscentPanel.hpp
│   │   ├── TelemetryPanel.hpp
│   │   └── SettingsPanel.hpp
│   │
│   └── ksp/
│       ├── KspClient.hpp
│       ├── KspClient.cpp
│       ├── RssProfile.hpp
│       ├── RssProfile.cpp
│       ├── Telemetry.hpp
│       ├── Telemetry.cpp
│       ├── ManeuverExporter.hpp
│       └── ManeuverExporter.cpp
│
├── tests/
│   ├── test_time.cpp
│   ├── test_frames.cpp
│   ├── test_orbit.cpp
│   ├── test_kepler.cpp
│   ├── test_hohmann.cpp
│   ├── test_lambert.cpp
│   ├── test_delta_v.cpp
│   ├── test_twr.cpp
│   ├── test_propagator.cpp
│   └── test_mission.cpp
│
└── tools/
    ├── kernel_downloader/
    ├── data_converter/
    └── validation/
```

---

# 5. Development Principles

## 5.1 Scientific data and rendering data must be separate

Never modify physical values just to make them visible.

Example:

```cpp
glm::dvec3 physicalPositionKm;
glm::vec3 renderPosition;
```

Use:

```text
physical world
      ↓
reference frame transform
      ↓
origin rebasing
      ↓
visual scaling
      ↓
GPU
```

---

## 5.2 Double precision for physics

Use `double` for:

- ephemeris
- orbital states
- trajectory propagation
- Lambert solutions
- time-of-flight calculations
- body positions
- velocities

Use `float` mainly for GPU vertex data after transformation.

---

## 5.3 One authoritative simulation clock

Every subsystem must use:

```text
SimulationClock
```

Do not independently call the system clock from astronomy, mission or rendering modules.

Required clock modes:

- real time
- paused
- accelerated time
- reverse time
- direct date/time selection

---

## 5.4 Reference-frame awareness

Every position/velocity must have a known frame.

Avoid APIs such as:

```cpp
getPosition();
```

Prefer APIs conceptually like:

```cpp
getState(body, observer, frame, time);
```

Frames may include:

- J2000 / ICRF
- ecliptic
- heliocentric
- barycentric
- geocentric
- body-centered
- render-local frame

---

## 5.5 Validation before optimization

Every major physics feature requires reference validation before adding advanced optimizations.

Priority:

```text
correct
→ tested
→ profiled
→ optimized
```

---

# 6. Phase 0 — Repository Bootstrap

## Goals

- Repository structure
- CMake
- executable target
- dependency strategy
- logging
- test target
- basic CI-ready build

## Tasks

- [ ] Create root `CMakeLists.txt`
- [ ] Create `src/main.cpp`
- [ ] Configure GLFW
- [ ] Configure GLAD
- [ ] Configure GLM
- [ ] Configure Dear ImGui
- [ ] Configure ImPlot
- [ ] Configure nlohmann/json
- [ ] Configure spdlog
- [ ] Configure test framework
- [ ] Add compiler warnings
- [ ] Add Debug / Release presets
- [ ] Add Linux build instructions
- [ ] Add Windows build instructions

## Exit Criteria

```text
solar executable opens an empty window.
ImGui dockspace is visible.
Tests compile and run.
```

---

# 7. Phase 1 — Application Shell and GUI

## Goals

Build the desktop application framework before adding physics.

## Required Layout

```text
┌──────────────────────────────────────────────────────┐
│ File  View  Simulation  Mission  Tools  Help        │
├───────────────┬──────────────────────────────────────┤
│               │                                      │
│ Inspector     │                                      │
│               │             3D VIEWPORT              │
│ Mission       │                                      │
│ Rocket        │                                      │
│               │                                      │
├───────────────┴──────────────────────────────────────┤
│ Timeline / Logs / Telemetry                         │
└──────────────────────────────────────────────────────┘
```

## Tasks

- [ ] ImGui docking
- [ ] viewport panel
- [ ] menu bar
- [ ] status bar
- [ ] logger console
- [ ] application state
- [ ] settings persistence
- [ ] window resize handling
- [ ] keyboard shortcut system
- [ ] theme configuration

## Exit Criteria

The GUI is usable even before the Solar System is rendered.

---

# 8. Phase 2 — Renderer Foundation

## Goals

Build a reusable OpenGL renderer.

## Tasks

- [ ] OpenGL debug context
- [ ] shader class
- [ ] mesh abstraction
- [ ] vertex/index buffers
- [ ] camera
- [ ] perspective projection
- [ ] free camera
- [ ] orbit camera
- [ ] pan
- [ ] zoom
- [ ] depth testing
- [ ] line rendering
- [ ] sphere mesh generation
- [ ] grid rendering
- [ ] framebuffer rendering into ImGui viewport

## Important

Render the 3D world into an offscreen framebuffer and display the resulting texture in an ImGui viewport panel.

This gives much better UI integration than treating the OpenGL window itself as the entire application.

## Exit Criteria

The application renders:

- a sphere
- grid
- XYZ axes

inside the dockable viewport.

---

# 9. Phase 3 — Simulation Time System

## Features

- [ ] current simulation UTC
- [ ] real-time mode
- [ ] pause
- [ ] 1×
- [ ] 10×
- [ ] 100×
- [ ] 1,000×
- [ ] 10,000×
- [ ] 100,000×
- [ ] reverse time
- [ ] date/time input
- [ ] reset to current real time

## Design

```cpp
class SimulationClock
{
public:
    void update(double realDeltaSeconds);

    void setTime(...);
    void setScale(double scale);
    void pause();
    void resume();

    ...
};
```

## Exit Criteria

All displayed simulation time comes from one clock.

---

# 10. Phase 4 — CSPICE Integration

## Goals

Introduce real astronomical data.

## Tasks

- [ ] CSPICE wrapper
- [ ] kernel loading
- [ ] error handling
- [ ] UTC ↔ ET conversion
- [ ] body lookup
- [ ] state-vector query
- [ ] planetary constants
- [ ] kernel status panel
- [ ] offline kernel support

## Initial Bodies

- [ ] Sun
- [ ] Mercury
- [ ] Venus
- [ ] Earth
- [ ] Moon
- [ ] Mars
- [ ] Jupiter
- [ ] Saturn
- [ ] Uranus
- [ ] Neptune

## Exit Criteria

For any supported date:

```text
body position
body velocity
observer-relative state
```

can be queried reliably.

---

# 11. Phase 5 — Coordinate Frame System

## Required Concepts

- [ ] barycentric
- [ ] heliocentric
- [ ] geocentric
- [ ] J2000 / ICRF
- [ ] ecliptic
- [ ] body-centered
- [ ] render-local

## Critical API Rule

The frame should never be implicit where ambiguity is possible.

## Rendering Precision

Implement:

```text
origin rebasing
```

Example:

```text
Camera focused on Jupiter
        ↓
Jupiter physical position becomes render origin
        ↓
other bodies rendered relative to Jupiter
```

## Exit Criteria

Switching origin/reference frame does not alter physical calculations.

---

# 12. Phase 6 — Real Solar System Scene

## Features

- [ ] render real body positions
- [ ] body radius exaggeration
- [ ] distance scale
- [ ] Sun
- [ ] planets
- [ ] Moon
- [ ] body selection
- [ ] focus selected
- [ ] follow selected
- [ ] body labels
- [ ] orbit lines
- [ ] reference grid
- [ ] trajectory trail

## UI

Body inspector should show:

```text
Name
Position
Velocity
Distance from Sun
Distance from Earth
Radius
Mass
μ
Rotation Period
Axial Tilt
Surface Gravity
Escape Velocity
```

## Exit Criteria

The application is already useful as an astronomical Solar System viewer.

---

# 13. Phase 7 — Planet Orientation

## Features

- [ ] axial tilt
- [ ] rotation period
- [ ] retrograde rotation
- [ ] pole orientation
- [ ] prime meridian
- [ ] equatorial plane
- [ ] rotation axis visualization

## Special Cases

Validate:

- Venus retrograde rotation
- Uranus extreme axial tilt
- Earth prime meridian
- Saturn rings aligned with equatorial frame

## Exit Criteria

Planet orientation changes correctly with time.

---

# 14. Phase 8 — Lighting and Visual Quality v1

## Features

- [ ] Sun as light source
- [ ] diffuse planetary lighting
- [ ] ambient component
- [ ] day/night separation
- [ ] Saturn ring geometry
- [ ] basic planet textures
- [ ] star background

Do not spend excessive time on visual effects yet.

---

# 15. Phase 9 — Orbital Mechanics Core

Implement and test:

## Circular Orbit Velocity

\[
v_c = \sqrt{\frac{\mu}{r}}
\]

## Escape Velocity

\[
v_e = \sqrt{\frac{2\mu}{r}}
\]

## Vis-Viva

\[
v = \sqrt{\mu\left(\frac{2}{r}-\frac{1}{a}\right)}
\]

## Orbital Period

\[
T = 2\pi\sqrt{\frac{a^3}{\mu}}
\]

## Specific Orbital Energy

\[
\epsilon = \frac{v^2}{2}-\frac{\mu}{r}
\]

## Specific Angular Momentum

\[
\mathbf{h} = \mathbf{r}\times\mathbf{v}
\]

## Required Features

- [ ] state vectors
- [ ] classical orbital elements
- [ ] state → elements
- [ ] elements → state
- [ ] periapsis
- [ ] apoapsis
- [ ] eccentricity
- [ ] inclination
- [ ] longitude of ascending node
- [ ] argument of periapsis
- [ ] true anomaly

## Exit Criteria

Known reference orbit problems pass automated tests.

---

# 16. Phase 10 — Orbit and Maneuver Sandbox

## Features

- [ ] create spacecraft
- [ ] assign central body
- [ ] define initial orbit
- [ ] render spacecraft orbit
- [ ] maneuver point
- [ ] prograde
- [ ] retrograde
- [ ] normal
- [ ] anti-normal
- [ ] radial-in
- [ ] radial-out
- [ ] live resulting-orbit preview

## UI

```text
Maneuver

Prograde     +100.0 m/s
Normal          0.0 m/s
Radial          0.0 m/s

Result
Ap: ...
Pe: ...
Inclination: ...
```

---

# 17. Phase 11 — Rocket Engineering Core

## Engine Model

Fields:

- [ ] name
- [ ] mass
- [ ] vacuum thrust
- [ ] sea-level thrust
- [ ] vacuum Isp
- [ ] sea-level Isp
- [ ] minimum throttle
- [ ] maximum throttle
- [ ] restart count
- [ ] optional propellant definition

## Stage Model

Fields:

- [ ] dry mass
- [ ] propellant mass
- [ ] engine list
- [ ] payload above
- [ ] staging type

## Vehicle Model

- [ ] ordered stages
- [ ] payload
- [ ] total mass
- [ ] save/load JSON

---

# 18. Phase 12 — Delta-v / TWR / Burn Calculator

## Delta-v

\[
\Delta v =
I_{sp}g_0
\ln\left(\frac{m_0}{m_f}\right)
\]

## TWR

\[
TWR=\frac{F}{mg}
\]

## Mass Flow

\[
\dot m=\frac{F}{I_{sp}g_0}
\]

## Required Outputs

For every stage:

- [ ] wet mass
- [ ] dry mass
- [ ] propellant mass
- [ ] stage Δv
- [ ] cumulative Δv
- [ ] ignition TWR
- [ ] burnout TWR
- [ ] burn time
- [ ] mass flow
- [ ] payload fraction

## Exit Criteria

Rocket values agree with independently verified analytical calculations.

---

# 19. Phase 13 — Hohmann Transfer Tool

## Features

- [ ] circular-to-circular transfer
- [ ] departure burn
- [ ] arrival burn
- [ ] transfer time
- [ ] phase angle
- [ ] 3D visualization

Use this as an intermediate validation step before Lambert solving.

---

# 20. Phase 14 — Patched-Conics Infrastructure

Model:

```text
Planet-centered parking orbit
        ↓
planet escape hyperbola
        ↓
Sun-centered transfer
        ↓
target SOI encounter
        ↓
target-centered hyperbola
        ↓
capture orbit
```

## Features

- [ ] sphere-of-influence model
- [ ] hyperbolic excess velocity
- [ ] parking-orbit escape
- [ ] capture burn
- [ ] arrival periapsis

---

# 21. Phase 15 — Lambert Solver

This is a major milestone.

## Inputs

```text
r1
r2
time of flight
μ
```

## Outputs

```text
v1
v2
```

## Initial Solver Scope

- [ ] prograde
- [ ] short-way
- [ ] zero-revolution

Later:

- [ ] long-way
- [ ] retrograde
- [ ] multi-revolution

## Validation

Lambert solver must be tested against published reference cases.

Never trust visual appearance alone.

---

# 22. Phase 16 — Interplanetary Transfer Planner

## Inputs

```text
Departure body
Target body
Departure date
Arrival date
Departure parking orbit
Target parking orbit
```

## Outputs

- [ ] time of flight
- [ ] departure V∞
- [ ] arrival V∞
- [ ] C3
- [ ] departure injection Δv
- [ ] arrival capture Δv
- [ ] total mission transfer Δv

## Visualization

Display in 3D:

- departure planet
- target planet
- transfer orbit
- velocity vectors
- SOI encounter

---

# 23. Phase 17 — Launch Window Search

## Features

- [ ] departure date range
- [ ] arrival date range
- [ ] minimum flight time
- [ ] maximum flight time
- [ ] maximum departure V∞
- [ ] maximum arrival V∞
- [ ] solution filtering

Implement calculation in worker threads so the UI remains responsive.

---

# 24. Phase 18 — Porkchop Plot

Use ImPlot.

## Axis

```text
X = departure date
Y = arrival date
```

## Contours

- [ ] departure C3
- [ ] departure V∞
- [ ] arrival V∞
- [ ] total Δv
- [ ] transfer duration

## Interaction

Click a point:

```text
→ create transfer solution
→ display trajectory in 3D
→ populate Mission Planner
```

---

# 25. Phase 19 — Mission Builder

Mission should be represented as ordered nodes/events.

Example:

```text
Earth Surface
      ↓
200 km LEO
      ↓
TMI
      ↓
Mid-course Correction
      ↓
Mars SOI
      ↓
Mars Capture
      ↓
250 km Mars Orbit
      ↓
Landing
```

## Mission Node Data

- [ ] event type
- [ ] central body
- [ ] epoch
- [ ] initial state
- [ ] final state
- [ ] required Δv
- [ ] assigned vehicle stage
- [ ] notes

---

# 26. Phase 20 — Mission Delta-v Budget

Example output:

```text
Earth ascent        9,400 m/s
TMI                 3,550 m/s
Correction            100 m/s
Mars capture        1,400 m/s
Landing               700 m/s
------------------------------
Required           15,150 m/s
Vehicle            16,200 m/s
Margin              1,050 m/s
```

Support configurable reserves:

```text
Transfer reserve
Landing reserve
Contingency reserve
```

---

# 27. Phase 21 — Vehicle ↔ Mission Matching

The application should automatically determine:

- [ ] which stage performs which burn
- [ ] where stage depletion occurs
- [ ] remaining propellant
- [ ] remaining Δv
- [ ] TWR at each major event
- [ ] burn duration
- [ ] mission margin

The result should not be based only on total Δv.

---

# 28. Phase 22 — Mission Feasibility Engine

Evaluate:

```text
Δv
TWR
burn duration
stage availability
propellant reserve
arrival constraints
landing TWR
```

Statuses:

```text
PASS
WARNING
FAIL
```

Example:

```text
Earth ascent TWR       PASS
TMI Δv                 PASS
TMI burn time          WARNING
Mars capture           PASS
Landing TWR            FAIL

MISSION NOT FEASIBLE
```

---

# 29. Phase 23 — Numerical Propagation Core

Implement generic numerical propagation.

Start with:

```text
RK4
```

Then add:

```text
RK45 / adaptive integrator
```

State:

\[
\dot{\mathbf r}=\mathbf v
\]

\[
\dot{\mathbf v}=\mathbf a
\]

Design propagator to support pluggable force models.

---

# 30. Phase 24 — Finite Burn Simulation

Replace instantaneous burn assumptions when requested.

Equations:

\[
\dot{\mathbf r}=\mathbf v
\]

\[
\dot{\mathbf v}
=
-\frac{\mu}{r^3}\mathbf r
+
\frac{\mathbf T}{m}
\]

\[
\dot m=-\dot m_p
\]

## Outputs

- [ ] burn start
- [ ] burn midpoint
- [ ] burn end
- [ ] finite-burn trajectory
- [ ] required correction
- [ ] impulsive-vs-finite penalty

---

# 31. Phase 25 — KSP/RSS Universe Profile

Do not assume real-world physical data and KSP/RSS values are always identical.

Create explicit universe profiles.

```text
REAL_SOLAR_SYSTEM
KSP_RSS
STOCK_KSP
```

Each profile can define:

- radius
- μ
- mass
- atmosphere
- rotation period
- SOI
- epoch parameters

---

# 32. Phase 26 — KSP Maneuver Output

Convert mission solution into KSP-friendly components:

```text
Burn UT
Prograde
Normal
Radial
```

Also show:

- [ ] estimated burn time
- [ ] recommended burn start
- [ ] expected post-burn orbit

---

# 33. Phase 27 — Atmospheric Model

Required for ascent, entry, aerobraking and drag.

Model:

- [ ] density vs altitude
- [ ] pressure
- [ ] temperature
- [ ] speed of sound
- [ ] Mach
- [ ] drag

Drag:

\[
F_D=\frac12\rho v^2 C_D A
\]

Dynamic pressure:

\[
q=\frac12\rho v^2
\]

---

# 34. Phase 28 — Ascent Simulator

## Inputs

- launch body
- launch latitude
- launch altitude
- vehicle
- aerodynamic parameters
- pitch program
- throttle program
- staging events
- target orbit

## State

- position
- velocity
- mass
- orientation
- current stage

## Outputs

- altitude
- velocity
- downrange
- apoapsis
- periapsis
- TWR
- acceleration
- dynamic pressure
- Mach
- propellant
- stage events

---

# 35. Phase 29 — Ascent Telemetry Graphs

Using ImPlot:

- [ ] altitude vs time
- [ ] velocity vs time
- [ ] acceleration vs time
- [ ] TWR vs time
- [ ] Mach vs time
- [ ] dynamic pressure vs time
- [ ] apoapsis vs time
- [ ] propellant vs time
- [ ] pitch vs time

---

# 36. Phase 30 — Gravity Turn

## Inputs

- vertical ascent duration
- pitch-over velocity
- pitch kick
- target pitch curve
- throttle profile

## Outputs

- orbit achieved
- gravity loss
- drag loss
- steering loss
- remaining fuel

---

# 37. Phase 31 — Ascent Loss Decomposition

Estimate:

```text
Useful orbital-energy contribution
Gravity loss
Drag loss
Steering loss
```

Use this to compare ascent strategies.

---

# 38. Phase 32 — Ascent Optimization

Possible objective functions:

- [ ] maximize payload
- [ ] minimize propellant
- [ ] minimize total Δv expenditure
- [ ] minimize gravity loss

Constraints:

- [ ] Max-Q
- [ ] maximum acceleration
- [ ] target orbit
- [ ] minimum TWR
- [ ] engine throttle limits

---

# 39. Phase 33 — Landing / Suicide Burn Planner

For airless bodies first.

## Inputs

- altitude
- vertical speed
- horizontal speed
- mass
- thrust
- Isp
- local gravity

## Outputs

- ignition altitude
- burn duration
- throttle profile
- required Δv
- propellant remaining

Later extend to guided 2D/3D descent.

---

# 40. Phase 34 — Reentry / Aerobraking

## Features

- [ ] atmospheric trajectory
- [ ] Mach
- [ ] dynamic pressure
- [ ] deceleration
- [ ] approximate heating
- [ ] apoapsis change after pass
- [ ] aerobraking pass planner

---

# 41. Phase 35 — Rendezvous

## Features

- [ ] target-relative frame
- [ ] phase angle
- [ ] phasing orbit
- [ ] intercept
- [ ] closing velocity
- [ ] rendezvous Δv
- [ ] docking approach visualization

---

# 42. Phase 36 — KSP Live Telemetry

Add only after the internal simulation and mission systems are stable.

Possible telemetry:

- [ ] universal time
- [ ] vessel mass
- [ ] stage
- [ ] propellant
- [ ] thrust
- [ ] TWR
- [ ] position
- [ ] velocity
- [ ] orbit
- [ ] apoapsis
- [ ] periapsis
- [ ] maneuver nodes

KSP connector must remain isolated behind:

```text
KspClient
```

so core physics never depends on KSP.

---

# 43. Phase 37 — Planned vs Actual

Overlay:

```text
planned trajectory
actual KSP trajectory
```

Calculate:

- [ ] position error
- [ ] velocity error
- [ ] predicted encounter difference
- [ ] periapsis error
- [ ] Δv deviation

---

# 44. Phase 38 — Flight Recorder

Record:

```text
UT
position
velocity
altitude
mass
fuel
TWR
acceleration
Mach
Q
apoapsis
periapsis
stage
```

Export:

```text
CSV
JSON
```

---

# 45. Phase 39 — Post-Flight Analysis

Provide:

- planned Δv
- actual Δv
- gravity losses
- drag losses
- steering losses
- stage performance
- unused fuel
- trajectory error
- ascent efficiency

---

# 46. Phase 40 — Gravity Assist Planner

After Lambert and patched conics are thoroughly validated.

## Features

- [ ] incoming V∞
- [ ] outgoing V∞
- [ ] periapsis
- [ ] turning angle
- [ ] B-plane
- [ ] flyby geometry

Later:

```text
Earth → Venus → Earth → Jupiter
```

sequence search.

---

# 47. Phase 41 — Low-Thrust Propagation

Support:

- ion propulsion
- electric propulsion
- continuous low thrust

Use numerical integration with mass depletion.

---

# 48. Phase 42 — Lagrange Points / CR3BP

## Features

- [ ] L1
- [ ] L2
- [ ] L3
- [ ] L4
- [ ] L5
- [ ] rotating frame visualization
- [ ] zero-velocity curves
- [ ] Lyapunov orbits
- [ ] halo-orbit experiments

---

# 49. Phase 43 — N-Body Propagation

Equation:

\[
\ddot{\mathbf r_i}
=
\sum_{j\ne i}
Gm_j
\frac{\mathbf r_j-\mathbf r_i}
{|\mathbf r_j-\mathbf r_i|^3}
\]

Use JPL/SPICE as reference truth for validation.

---

# 50. Phase 44 — Monte Carlo Analysis

Perturb:

- thrust
- Isp
- timing
- burn direction
- mass
- navigation state

Run large batches and display:

- encounter distributions
- periapsis distributions
- mission success probability

---

# 51. Phase 45 — Optimization Engine

Create a reusable optimization layer.

## Objective Functions

- minimum Δv
- minimum travel time
- maximum payload
- maximum fuel reserve
- minimum launch mass
- earliest arrival

## Constraints

- TWR
- burn duration
- Max-Q
- acceleration
- arrival date
- periapsis
- propellant
- engine restart limits

---

# 52. Phase 46 — Rendering Quality v2

Only after the scientific core is mature.

Add:

- [ ] high-resolution textures
- [ ] normal maps
- [ ] Earth cloud layer
- [ ] Earth night lights
- [ ] atmosphere scattering
- [ ] HDR
- [ ] bloom
- [ ] shadow mapping
- [ ] improved Sun shader
- [ ] transparent Saturn rings
- [ ] Milky Way background
- [ ] asteroid instancing

---

# 53. Phase 47 — Performance and Concurrency

Profile first.

Potential worker tasks:

- Lambert grids
- porkchop calculations
- Monte Carlo
- trajectory optimization
- numerical propagation
- SPICE preprocessing
- data loading

Renderer must remain responsive.

Recommended approach:

```text
Main/UI thread
Render thread or render-on-main depending on OpenGL design
Worker thread pool
```

Avoid premature multithreading.

---

# 54. Phase 48 — Data and Project Persistence

Save:

```text
.solarmission
.solarrocket
.solarscenario
```

Internally JSON is acceptable.

Store:

- selected universe
- simulation time
- camera
- rocket
- stages
- mission
- transfer
- maneuver nodes
- UI layout

---

# 55. Phase 49 — Validation Strategy

Every major calculation requires an independent reference.

## Rocket Equation

Compare with hand calculations.

## Orbital Mechanics

Compare with analytical textbook problems.

## Lambert

Compare against published Lambert test cases.

## Ephemeris

Compare with direct SPICE queries.

## Numerical Propagator

Check energy/angular momentum conservation in two-body propagation.

## Ascent

Use known simplified benchmark trajectories before attempting realistic tuning.

## KSP

Compare application outputs with in-game values where the same assumptions apply.

---

# 56. Unit Testing Priorities

Mandatory automated tests:

```text
Units
Time conversion
Coordinate transforms
Circular velocity
Escape velocity
Vis-viva
State ↔ orbital elements
Hohmann
Rocket equation
Stage mass propagation
TWR
Burn duration
Lambert
Hyperbolic escape
Capture
RK4
Mission budget
```

Physics code without tests should not be considered complete.

---

# 57. Version Milestones

## v0.1 — Solar Viewer

Required:

- GUI shell
- OpenGL viewport
- camera
- simulation clock
- CSPICE
- Sun and planets
- real positions
- body selection
- basic orbit rendering
- body inspector

Result:

```text
A usable real Solar System viewer.
```

---

## v0.2 — Orbital Mechanics

Required:

- orbital elements
- orbit calculator
- maneuver sandbox
- circular/escape/vis-viva
- Hohmann transfers

Result:

```text
A useful orbital-mechanics learning and analysis tool.
```

---

## v0.3 — Rocket Engineering

Required:

- engine editor
- stage editor
- vehicle editor
- Δv
- TWR
- burn time
- save/load rocket

Result:

```text
A rocket and staging calculator usable with RSS designs.
```

---

## v0.4 — Interplanetary Mission Planner

Required:

- patched conics
- Lambert solver
- launch window
- porkchop
- parking-orbit injection
- target capture
- 3D transfer rendering

Result:

```text
Earth → Mars style mission planning becomes practical.
```

---

## v0.5 — Mission Engineering

Required:

- mission builder
- mission Δv budget
- stage assignment
- mission feasibility
- reserves
- maneuver output

Result:

```text
The application can determine whether a vehicle can execute a planned mission.
```

---

## v0.6 — Numerical Flight Dynamics

Required:

- RK4/RK45
- finite burns
- finite-burn correction
- trajectory propagation

---

## v0.7 — Launch / Ascent

Required:

- atmosphere
- ascent simulator
- gravity turn
- Max-Q
- ascent plots
- loss decomposition

---

## v0.8 — KSP/RSS Integration

Required:

- RSS profile
- KSP telemetry
- maneuver data
- flight recorder

---

## v0.9 — Mission Control

Required:

- planned vs actual
- post-flight analysis
- trajectory deviation
- live mission dashboard

---

## v1.0 — Stable Integrated Release

Minimum scope:

```text
REAL SOLAR SYSTEM VIEWER
+
ORBITAL MECHANICS
+
ROCKET ENGINEERING
+
INTERPLANETARY MISSION PLANNER
+
MISSION FEASIBILITY
+
FINITE BURN SIMULATION
+
ASCENT SIMULATION
+
KSP RSS INTEGRATION
```

All major physics modules must have validation tests.

---

# 58. Post-1.0 Research Features

After v1.0:

- gravity assist
- B-plane targeting
- low-thrust trajectories
- Lagrange points
- CR3BP
- halo orbits
- N-body
- aerocapture
- Monte Carlo
- multi-objective optimization
- multi-flyby search
- advanced atmospheric flight
- mission reliability analysis

---

# 59. Development Priority

The recommended strict order is:

```text
PROJECT SHELL
      ↓
GUI
      ↓
RENDERER
      ↓
SIMULATION TIME
      ↓
CSPICE
      ↓
REFERENCE FRAMES
      ↓
SOLAR SYSTEM VIEWER
      ↓
BODY ORIENTATION
      ↓
ORBITAL MECHANICS
      ↓
MANEUVER SANDBOX
      ↓
ROCKET / STAGING
      ↓
Δv / TWR / BURN TIME
      ↓
HOHMANN
      ↓
PATCHED CONICS
      ↓
LAMBERT
      ↓
LAUNCH WINDOWS
      ↓
PORKCHOP
      ↓
MISSION BUILDER
      ↓
MISSION FEASIBILITY
      ↓
NUMERICAL PROPAGATION
      ↓
FINITE BURNS
      ↓
KSP / RSS PROFILE
      ↓
ASCENT SIMULATION
      ↓
GRAVITY TURN
      ↓
KSP TELEMETRY
      ↓
PLANNED VS ACTUAL
      ↓
ADVANCED ASTRODYNAMICS
```

Do not reverse major dependencies.

For example:

```text
Do not build porkchop plots before Lambert is validated.
Do not build ascent optimization before the ascent simulator is validated.
Do not build KSP live automation before the mission core works offline.
```

---

# 60. GUI Target Layout

Long-term interface concept:

```text
┌──────────────────────────────────────────────────────────────────────────────┐
│ SOLAR | File | View | Simulation | Mission | Tools | Help                 │
├──────────────────┬───────────────────────────────────────────────────────────┤
│                  │                                                           │
│ SOLAR SYSTEM     │                                                           │
│ ─────────────    │                                                           │
│ Sun              │                                                           │
│ Mercury          │                       3D VIEWPORT                         │
│ Venus            │                                                           │
│ Earth            │              Earth ─────────────→ Mars                   │
│ Mars             │                                                           │
│ ...              │                                                           │
│                  │                                                           │
├──────────────────┼───────────────────────────────────────────────────────────┤
│ INSPECTOR        │ Mission Timeline / Graphs / Telemetry                    │
│                  │                                                           │
│ Position         │                                                           │
│ Velocity         │                                                           │
│ Orbit            │                                                           │
│ Δv               │                                                           │
├──────────────────┴───────────────────────────────────────────────────────────┤
│ UTC 2031-04-21 13:42:18 | 1000× | Earth→Mars | Δv Margin 612 m/s | 60 FPS │
└──────────────────────────────────────────────────────────────────────────────┘
```

---

# 61. Main GUI Panels

## Solar System Panel

- body list
- visibility
- body selection
- focus/follow
- reference frame

## Body Inspector

- physical data
- current state vector
- orbital data
- orientation

## Time Panel

- UTC
- simulation time
- speed
- pause
- direct date jump

## Rocket Panel

- vehicle tree
- stages
- payload
- engine configuration

## Orbit Panel

- elements
- state vectors
- orbit calculations

## Maneuver Panel

- prograde
- normal
- radial
- burn time
- preview orbit

## Mission Panel

- mission nodes
- total Δv
- stage assignment
- reserves
- feasibility

## Transfer Panel

- departure
- arrival
- time of flight
- Lambert solution

## Porkchop Panel

- interactive plot
- filters
- solution selection

## Ascent Panel

- launch parameters
- gravity turn
- telemetry plots

## Telemetry Panel

- KSP live state
- recording
- planned-vs-actual

---

# 62. Performance Targets

Initial targets:

```text
UI / 3D viewport:
60 FPS on a normal desktop GPU

Solar System ephemeris:
negligible frame impact

Lambert single solution:
interactive

Porkchop:
background calculation

Numerical propagation:
real-time where practical

Monte Carlo:
offline/background worker
```

Never execute large transfer-grid or optimization jobs directly in the GUI frame loop.

---

# 63. Error Handling

The application must report understandable failures for:

- missing SPICE kernels
- invalid SPICE date
- OpenGL initialization
- shader compilation
- corrupt JSON
- invalid rocket configuration
- impossible transfer
- Lambert non-convergence
- numerical integrator failure
- KSP connection failure

A failed subsystem should not necessarily crash the entire application.

---

# 64. Debugging Tools

Include developer panels:

- frame time
- render stats
- draw calls
- camera position
- reference frame
- current render origin
- SPICE state
- selected body state
- physics-step count
- worker queue
- memory statistics where possible

Add visualization toggles:

- axes
- velocity vectors
- acceleration vectors
- orbital plane
- equatorial plane
- SOI
- maneuver vectors
- body frames

---

# 65. Definition of Project Success

The project should eventually answer a question such as:

```text
I am playing KSP with Real Solar System.

My rocket has:
- these stages
- these engines
- this payload
- this TWR
- this available Δv

I want to:
- launch from Earth
- reach a 200 km LEO
- depart on a selected date
- arrive at Mars within a selected date range
- enter a 250 km Mars orbit

Tell me:
- whether the vehicle can do it
- the launch window
- departure date
- arrival date
- flight duration
- required Earth departure burn
- Mars arrival velocity
- capture burn
- total mission Δv
- stage usage
- burn durations
- remaining margin
- maneuver vectors

Then show the complete mission in 3D.
```

If SOLAR can answer this reliably and visually, the core project has succeeded.

---

# 66. Immediate Next Steps

The first implementation sprint should contain only:

## Sprint 1

- [ ] repository structure
- [ ] CMake
- [ ] GLFW
- [ ] GLAD
- [ ] OpenGL context
- [ ] Dear ImGui
- [ ] ImPlot
- [ ] dockspace
- [ ] framebuffer viewport
- [ ] camera
- [ ] grid
- [ ] sphere rendering
- [ ] logging
- [ ] Catch2

## Sprint 2

- [ ] SimulationClock
- [ ] CSPICE wrapper
- [ ] kernel loader
- [ ] Sun/Earth/Mars ephemeris
- [ ] reference-frame foundation
- [ ] render real body positions

## Sprint 3

- [ ] all major planets
- [ ] body inspector
- [ ] orbit visualization
- [ ] body focus/follow
- [ ] distance/radius visualization scaling
- [ ] origin rebasing

Only after these three sprints should work begin on the orbital-mechanics and rocket subsystems.

---

# Final Architecture Principle

SOLAR should be designed as:

```text
SCIENTIFIC CORE
      ↓
SIMULATION
      ↓
MISSION / ROCKET LOGIC
      ↓
RENDERING + GUI
      ↓
KSP INTEGRATION
```

and **not** as:

```text
GUI code containing physics formulas everywhere.
```

The core physics modules must be able to run without the renderer or GUI.

That separation is the most important architectural decision in the entire project.
