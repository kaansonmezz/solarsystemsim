# SOLAR

SOLAR is a native C++20 desktop application for scientifically grounded Solar
System visualization, astrodynamics, and multistage rocket engineering, with
mission planning and KSP Real Solar System support planned for later phases.

Version 0.4 combines the real CSPICE Solar System viewer with renderer-independent
two-body astrodynamics workstation. It converts between state vectors and classical
elements, analytically propagates elliptic and hyperbolic trajectories in either time
direction, provides an interactive spacecraft and impulsive-maneuver sandbox, and adds
a validated SI rocket-engineering layer for ideal multistage performance. Lambert
solving, mission planning, finite-burn ascent, and KSP integration remain future phases.

## Current features

- OpenGL 4.6 core renderer with a resizable framebuffer-backed ImGui viewport
- dockable Solar System, Body Inspector, Astronomy, Time, Console, and camera panels
- CSPICE N0067 isolated behind `SpiceContext`; raw CSPICE calls do not escape it
- DE442s geometric ephemerides for the Sun, Mercury, Venus, Earth, Moon, Mars,
  Jupiter, Saturn, Uranus, and Neptune
- J2000 and ECLIPJ2000 axes with barycentric, heliocentric, geocentric, and
  selected-body origins
- double-precision scientific state cache in km and km/s
- origin rebasing before double-to-float render conversion
- current CSPICE-derived osculating elements for every planet and an Earth-centered
  Moon solution, including apsides, period, angular momentum, and node geometry
- transparent selected-body orbital/reference planes, orbital normal, inclination
  label, AN/DN markers, SPICE-oriented rotation axis, and equatorial-plane display
- basic Sun lighting, emissive Sun, Saturn rings, labels, and 3D SPICE orbit lines
- orbit/pan/zoom/free camera plus body focus and follow modes
- deterministic pause, acceleration, direct-time, and reverse-time behavior
- validated direct UTC calendar input and configurable startup origin/axis frame
- double-precision state-to-elements and elements-to-state conversion, including
  documented circular/equatorial singularity conventions
- bounded Newton solvers for elliptic and hyperbolic Kepler equations and analytical
  two-body propagation driven by `SimulationClock`
- Earth/body-centered spacecraft creation from apsis altitudes or an explicit state
  vector, trajectory rendering, focus/follow, and Pe/Ap/AN/DN markers
- prograde/normal/radial impulsive maneuver preview and apply, circularization and
  opposite-apsis helpers, Hohmann calculator/preview, and ideal plane-change calculator
- SI engine, propellant, stage, and chronological multistage vehicle models with
  automatic stack mass, engine-cluster mass/thrust, and JSON persistence
- ideal vacuum/sea-level-reference delta-v, body-dependent initial/final TWR,
  mass-flow, burn-time, remaining-delta-v, and required-propellant calculations
- dockable ROCKET / VEHICLE, Rocket Stage, and Rocket Engine editors with live
  stage tables, mass/delta-v charts, engine presets, and contextual warnings
- Catch2 coverage for clock, CSPICE, frames, orbit conversions, propagation invariants,
  time reversal, spacecraft state, maneuvers, Hohmann transfers, plane changes, and
  analytical rocket/staging invariants

## Dependencies and Linux build

The first configure downloads pinned source revisions through CMake
`FetchContent`, including the official CSPICE N0067 toolkit package. It does not
download kernels while SOLAR is running. You need Git, CMake 3.24+, Ninja or Make,
a C++20 compiler, OpenGL development files, and the native GLFW window-system
dependencies.

On Arch-based Linux:

```bash
sudo pacman -S --needed base-devel cmake ninja git mesa libx11 libxrandr \
  libxinerama libxcursor libxi wayland libxkbcommon
```

Configure, build, test, and run:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/solar
```

Build directories and generated `solar_imgui.ini` state are ignored by the root
`.gitignore` and must not be committed. For an independent clean-tree check, configure
an empty directory such as `build-fresh` and run the same build/test commands there.

Optional AddressSanitizer and UndefinedBehaviorSanitizer instrumentation:

```bash
cmake -S . -B build-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DSOLAR_ENABLE_SANITIZERS=ON
cmake --build build-sanitize -j
ctest --test-dir build-sanitize --output-on-failure
```

Runtime `shaders/`, `config/`, and `assets/` trees are copied beside the
executable. The resource locator also checks the current and compile-time source
directories, so SOLAR need not be launched from the repository root.

## CSPICE kernels

The manifest is [`assets/kernels/manifest.json`](assets/kernels/manifest.json).
The required kernel files are:

| File | Role |
|---|---|
| `naif0012.tls` | leap seconds and UTC/ET conversion |
| `de442s.bsp` | compact JPL planetary and lunar ephemeris |
| `pck00011.tpc` | planetary constants and IAU body-fixed orientation models |

DE442s is approximately 31 MB and covers **1849-12-26 through 2150-01-22**.
SOLAR reports a controlled SPICE error outside loaded coverage; it does not
extrapolate or pretend the kernel supports another date. To use a separately
managed kernel directory, change `astronomy.kernelDirectory` in
[`config/app.json`](config/app.json). Relative paths are resolved from the SOLAR
runtime resource root.

If distributing kernels separately, obtain them from the official NASA NAIF
generic-kernel archive and retain the same filenames or edit the manifest.

## Scientific conventions

- Authoritative time: `SimulationClock` UTC, converted by CSPICE to ET/TDB seconds
  past J2000. No astronomy or rendering subsystem reads wall-clock time directly.
- Fundamental state: geometric SPICE state with aberration correction `NONE`.
  Apparent/light-time-corrected sky positions are a future, separate mode.
- State units: position in km; velocity in km/s; angles in radians internally.
- Axes: `J2000` or `ECLIPJ2000`; reference origin is independently selectable and
  both origin and axes are always identified in the Astronomy and Body Inspector UI.
  `ECLIPJ2000`, the default Solar System view, uses the J2000 mean ecliptic and
  equinox. Its scientific X-Y plane is therefore the reference plane for the
  displayed inclination. `J2000` instead uses the Earth mean equator and equinox
  at J2000, and its X-Y plane becomes the reference plane when selected.
- Outer-planet render centers use the planetary barycenters provided by DE442s;
  the physical body NAIF IDs remain available in the body database and inspector.
- Orientation: SPICE IAU body-fixed frame transformed at the current ET. Rotation
  is never accumulated frame-to-frame, so reverse time and date jumps do not drift.
- Orbit lines: 128 geometric SPICE samples spanning roughly one orbital period.
  Per-body cache epochs use shorter refresh intervals for the Moon/inner planets and
  longer intervals for outer planets, so large forward or reverse date jumps invalidate
  stale paths without regenerating them every frame. The Moon is sampled relative to
  Earth; planets are sampled relative to the Sun.
- Planet orbit samples retain all three SPICE position components. They are not
  replaced with ideal Kepler ellipses and are not projected to `Z = 0`; the
  scientific-to-render mapping sends `(X,Y,Z)` to `(X,Z,-Y)` only after origin
  rebasing and distance scaling.
- Current osculating orbital elements are recomputed from the selected axes' real
  central-body-relative CSPICE state at the current ET with the existing
  `stateToOrbitalElements` conversion. Mercury through Neptune use the Sun and its
  gravitational parameter; the Moon uses Earth and Earth's gravitational parameter.
  These are instantaneous two-body elements fitted to the JPL state `r(t),v(t)`, not
  immutable planet parameters, so they can vary slightly with epoch under the real
  multi-body ephemeris. Reverse time and date jumps derive a fresh solution rather
  than integrating elements frame-to-frame.
- The orbital plane normal is the normalized physical angular momentum
  `h = r cross v`. Orbital-plane bases and ascending/descending node positions are
  prepared in double-precision scientific coordinates before conversion to render
  coordinates. The renderer only draws this prepared geometry.
- The ECLIPJ2000/J2000 X-Y plane maps to the renderer X-Z grid plane. Scientific
  vectors remain unchanged by this display-coordinate mapping.
- Orbital inclination and axial tilt are distinct. The orbital plane/normal comes
  from central-body-relative translational state, while the rotation axis and
  equatorial plane come from the current SPICE IAU body-fixed orientation. In
  particular, enabling both planes for Uranus exposes its unusual axial orientation
  without conflating it with orbital inclination.

### Spacecraft astrodynamics

- Internal position is km, velocity and delta-v are km/s, gravitational parameter is
  km^3/s^2, elapsed time is seconds, and angles are radians. GUI maneuver fields use
  m/s and angle fields use degrees only at the presentation boundary.
- An `Orbit` owns its central-body-relative inertial state at an explicit epoch and
  central-body `mu`; elements and propagated states are derived, preventing stale dual
  representations. The selected body radius is used only to convert altitude to radius.
- Classical elements use the scientific frame's +Z reference plane. For equatorial
  orbits RAAN is defined as zero. For circular inclined orbits argument of periapsis is
  zero and true anomaly carries argument of latitude; for circular equatorial orbits it
  carries true longitude. Eccentric equatorial orbits store direction-aware longitude
  of periapsis: inertial longitude is `+omega` prograde and `-omega` retrograde. These
  finite conventions preserve state vectors even though singular angles are nonunique.
- Maneuver components use an orthonormal RTN frame: radial-out `R = unit(r)`, normal
  `N = unit(r cross v)`, and tangential `T = N cross R`. The GUI label
  **Tangential / prograde** means local along-track `T`, not `unit(v)` when eccentric
  motion has radial velocity. Thus component norm always equals applied delta-v norm.
- Spacecraft motion is ideal point-mass, two-body Keplerian motion. Maneuvers are
  instantaneous impulses: there is no drag, J2, third-body perturbation, finite burn,
  fuel, mass, thrust, or burn duration yet. Near-parabolic analytical propagation is
  rejected explicitly rather than returning an unreliable state.
- Hyperbolic paths are clipped before their true-anomaly asymptotes for visualization;
  their scientific states remain unscaled doubles. Trajectory points pass through the
  same origin-rebased render transform as astronomical states.

### Rocket engineering

- The `solar_rocket` library is renderer-independent. It contains the authoritative
  engine, stage, vehicle, delta-v, TWR, mass-flow, burn-time, validation, analysis,
  and JSON code; UI panels only edit definitions and display analysis results.
- Rocket calculations use SI units: kg, N, s, m/s, m/s², and kg/s. Astrodynamics
  remains in km and km/s. Named helpers in `rocket/Units.hpp` make the boundary
  explicit: `1 km/s = 1000 m/s`; applying an impulsive maneuver does not consume
  rocket propellant in this phase.
- Specific impulse is converted with the central standard-gravity constant
  `g0 = 9.80665 m/s²`. This is deliberately distinct from local body gravity.
  Effective exhaust velocity is `ve = Isp g0`, mass flow is `mdot = F/(Isp g0)`,
  and ideal delta-v is `Isp g0 ln(m0/mf)`.
- An engine definition stores independent vacuum and sea-level thrust/Isp values,
  engine mass, and a valid throttle range. A stage stores structural mass, other dry
  mass, interstage mass, aggregate propellant, one engine definition, engine count,
  and throttle. Zero thrust is supported as a passive-stage definition and produces
  an explicit warning; negative/non-finite performance, non-positive Isp, invalid
  throttle ranges, and non-positive engine counts are rejected.
- Stage dry mass is computed as `structural + other dry + interstage + engine mass ×
  engine count`; users must not include engine mass in structural mass. Stage wet mass
  is dry mass plus usable propellant. Vehicle dry mass shown in the summary includes
  all stage dry masses plus payload. Engines are therefore counted exactly once.
- Vehicle stages are stored in chronological burn order: `stages[0]` burns first.
  Vehicle payload is a separate mass that remains attached through all stage burns.
  A lower stage automatically accelerates its own wet mass, every upper stage wet
  mass, and payload. At burnout only that stage's propellant has disappeared; its dry
  mass remains until instantaneous separation. The next stage ignition mass excludes
  every already-separated lower-stage dry mass.
- Per-stage burn mass ratio means stack `m0/mf` before separation, not isolated stage
  wet/dry ratio. Total available ideal delta-v is the sum of stage ideal delta-v.
  The UI labels full-burn vacuum-Isp and sea-level-Isp calculations as reference
  calculations; it does not claim a real ascent remains at either condition.
- Initial and near-burnout TWR use selected constant thrust divided by current stack
  weight. Weight uses `CelestialBody::surfaceGravityMetersPerSec2()`, derived from the
  selected body's radius and gravitational parameter. Thus changing Earth to Moon or
  Mars changes TWR but never changes rocket-equation `g0`. TWR below one is contextual:
  it prevents static vertical liftoff at that gravity but can be valid for an orbital
  or vacuum upper stage.
- Burn time is the constant-performance estimate `usable propellant / cluster mass
  flow`. The partial-burn helper inverts Tsiolkovsky to calculate required propellant,
  and the remaining-delta-v helper uses current wet mass and remaining propellant.
  Neither mutates a mission fuel state.
- Prompt 4 assumptions are: ideal rocket equation, instantaneous staging, constant
  selected Isp and thrust during each stage calculation, 100% default throttle, no
  residual propellant, no aerodynamic drag, no gravity or steering losses, and no
  finite-burn trajectory propagation. Reported delta-v is capability under these
  assumptions, not guaranteed mission or ascent performance.

The convenience engine database is [`config/engines.json`](config/engines.json).
Its entries are explicitly illustrative sample data, not sourced real-engine
specifications. Vehicles do not depend on that database: each saved stage embeds its
complete custom engine definition. The versioned vehicle schema is demonstrated by
[`config/vehicles/two_stage_sample.json`](config/vehicles/two_stage_sample.json):

```json
{
  "schemaVersion": 1,
  "name": "Vehicle Name",
  "payloadMassKg": 5000.0,
  "stages": [
    {
      "name": "Stage 1 — First Burn",
      "structuralMassKg": 20000.0,
      "otherDryMassKg": 0.0,
      "interstageMassKg": 0.0,
      "propellant": { "name": "Generic Propellant", "category": "Generic", "massKg": 100000.0 },
      "engine": {
        "name": "Custom Engine",
        "manufacturer": "",
        "description": "",
        "thrustVacuumN": 1100000.0,
        "thrustSeaLevelN": 1000000.0,
        "ispVacuumSec": 320.0,
        "ispSeaLevelSec": 285.0,
        "massKg": 1200.0,
        "minimumThrottle": 0.4,
        "maximumThrottle": 1.0
      },
      "engineCount": 2,
      "throttle": 1.0
    }
  ]
}
```

`distanceScale` controls render units per AU. `bodyRadiusScale` and the minimum
visible-radius safeguard affect rendering only. **Planet radius exaggeration never
changes physical radii, state vectors, distances, gravity, or other astronomical
calculations.** The Moon is not moved away from Earth to improve visibility.

## Controls

Mouse camera drags must begin over the viewport; active GUI editors suppress camera
and shortcut input. `renderer.orbitSensitivity` is measured in degrees per mouse pixel.
Middle-button drag never changes orbit distance; only wheel rotation performs zoom.

| Input | Action |
|---|---|
| Left click a body in Viewport | Select, focus, and follow the body |
| Middle mouse drag in Viewport | Orbit in yaw/pitch around the current target |
| Right mouse drag in Viewport | Pan target and camera |
| Mouse wheel in Viewport | Zoom |
| W / A / S / D with Viewport focused | Move forward / left / backward / right |
| Q / E with Viewport focused | Move down / up |
| Space | Pause or resume simulation |
| R | Return to real time |
| I | Edge-on inclination view for the selected body's real orbital plane |
| 1–8 | Select Mercury through Neptune |
| 9 | Select Sun |
| Esc | Exit |

The Solar System panel provides body visibility/selection, double-click focus,
follow mode, **Frame Solar System**, and **Earth-Moon** shortcuts. The Astronomy
panel selects the reference origin and axes and shows current ET, UTC, kernel, and
coverage status.
The Solar System and Body Inspector panels provide **Show Orbit**, **Show Orbital
Plane**, **Show Reference Plane**, **Show Orbital Normal**, **Show Ascending /
Descending Nodes**, **Show Rotation Axis**, and **Show Equatorial Plane** controls.
Only orbit paths are enabled by default. The orbital/reference grids share the
selected orbit's central body and physical scale; their viewport label reports the
current inclination and active reference axes. The Body Inspector's **Current
Osculating Orbit** section reports `a`, `e`, `i`, longitude of ascending node,
argument of periapsis, true anomaly, apsides, period, primary, reference plane, and
optional `r`, `v`, and `h` debug vectors. Moon values there are explicitly
Earth-centered.
The Solar System panel's **Inclination View** button (or the `I` shortcut) frames
the selected orbit along its scientifically derived ascending-node direction and
enables both comparison planes. This is the maximum-separation edge-on view for
reading the real angle; no inclination or vertex height is exaggerated. The selected
SPICE path is also color-emphasized while other paths remain visible.
The Simulation Time panel accepts an exact Gregorian UTC year/month/day and
hour/minute/second. Invalid calendar values are rejected without changing the clock;
dates outside DE442s coverage remain selectable but produce a controlled astronomy
unavailable status rather than extrapolated ephemerides.

The **Spacecraft** panel creates or replaces the active orbit. **Focus Spacecraft**
switches to a body-centered visualization scale and the adjacent Follow control keeps
the moving craft targeted. The **Orbit** panel reports elements, apsides, period,
energy, angular momentum, eccentricity vector, and current state. The **Maneuver**
panel's Preview leaves the current orbit untouched, Apply commits the preview at its
burn epoch, and Reset discards it. Current and preview trajectories are solid cyan and
dashed amber respectively; local basis and delta-v vectors are drawn at the burn point.
The spacecraft Hohmann helper derives its initial radius from the current state and is
disabled unless the active orbit satisfies the documented circularity tolerance.

## Windows direction

Use Visual Studio 2022 with its Desktop C++ workload and CMake support. CSPICE's
official 64-bit Visual C package is selected automatically:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
.\build\Debug\solar.exe
```

## Architecture

The dependency direction is enforced by CMake targets. Astronomy state feeds both
the renderer-independent astrodynamics/simulation path and the scene bridge; the
scene now caches the astrodynamics-derived planetary geometry alongside its current
scientific states:

```text
solar_core
    ↓
solar_astronomy  →  CSPICE
    ↓
solar_astrodynamics
    ├──────────────→ solar_rocket
    ├──────────────→ solar_simulation
    └──────────────→ solar_scene
                         ↓
       solar_rocket + solar_simulation + solar_scene
                         ↓
                    solar_renderer
                         ↓
               solar_ui / application
```

`solar_astrodynamics` has no Dear ImGui or OpenGL dependency. The GUI issues domain
commands and the renderer consumes physical trajectories only after render conversion;
neither is a source of scientific state. Raw CSPICE remains isolated in `SpiceContext`.
Because CSPICE's kernel pool is process-global, SOLAR permits one non-copyable,
non-movable application-owned `SpiceContext` at a time.
