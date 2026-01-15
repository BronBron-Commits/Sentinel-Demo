# Sentinel Demo — Deterministic 3D Multiverse Visualization

This repository is a **visual demonstration of deterministic simulation** using the Sentinel stack.  
It renders a **3D, camera-following timeline** with historical “time ghosts” while printing a
**per-tick cryptographic hash** of simulation state for verification.

The demo proves that:
- Simulation state is deterministic
- Visual output is derived strictly from authoritative state
- History, causality, and motion can be inspected in real time
- Long-running simulations remain stable and observable

---

## What This Shows

- **Deterministic sim-core**
  - Fixed-point math
  - No wall-clock time
  - No floating-point drift
- **Per-tick state hashing**
  - Identical hash streams across runs
  - Numeric proof of determinism
- **3D time visualization**
  - Current state centered
  - Past states rendered as translucent “ghosts”
  - Time encoded as depth (Z axis)
- **Contextual rendering**
  - Ground grid for spatial reference
  - Forward axis indicator
  - Camera locked to causality

This is not a physics engine or a game.
It is a **simulation inspection and verification tool**.

---

## Architecture

```

sentinel-core        → deterministic simulation + hashing
sentinel-orchestra   → coordination / rollback infrastructure
sentinel-demo        → visualization layer (this repo)

````

- The simulation **does not know about rendering**
- Rendering **never mutates simulation state**
- All visuals are derived from recorded snapshots

---

## Controls

- Close window to exit
- No user input is required (pure deterministic playback)

---

## Build & Run

### Dependencies (Debian/Ubuntu)

```bash
sudo apt install libsdl2-dev libglu1-mesa-dev
````

### Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Run

```bash
./sentinel-demo
```

You should see:

* A white cube (current state)
* A trail of translucent blue cubes (past states)
* Console output like:

```
tick=123 hash=0xdeadbeef...
```

Restarting the program produces the **exact same hash sequence**.

---

## Why This Exists

This demo exists to make determinism **visible**.

It is intended for:

* Simulation research
* Rollback / lockstep systems
* Defense / audit-grade verification
* Debugging causality and divergence
* Demonstrating correctness under replay

---

## Future Extensions

* Rewind and branch timelines
* Divergence visualization
* Multi-agent multiverse
* Shader-based effects
* Input recording and replay

---

## License

MIT
