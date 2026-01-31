# ML_AC_Project

## Overview
**ML_AC_Project** is a prototype exploring how **Machine Learning can be applied to a traditional Window Air Conditioner** to improve comfort, energy efficiency, and reliability — without turning the product into an over-engineered “smart appliance”.

The goal is **invisible intelligence**: better cooling behavior using sensor data and lightweight ML, not flashy dashboards.

---

## Motivation
Conventional window ACs:
- Cool aggressively but inefficiently
- Cycle poorly, causing temperature swings
- Waste energy during steady-state operation
- Provide no early warning for faults or degradation

This project investigates how **data-driven control** can address these issues while keeping hardware cost low.

---

## Core Objectives
- Faster time-to-comfort
- Lower energy consumption
- Reduced noise during steady operation
- Stable indoor temperature (less cycling)
- Early detection of abnormal behavior

---

## ML Use Cases (Prototype Scope)
- Cooling rate prediction
- Adaptive compressor and fan control
- Energy optimization under varying load
- Anomaly detection for faults (sensor-level)
- Filter / coil health inference

---

## Data Sources
- Indoor temperature & humidity
- Outdoor temperature
- Compressor current & voltage
- Fan speed / duty cycle
- Runtime and cycling patterns

*(Sensor list may evolve during prototyping)*

---

## Design Philosophy
- Edge-first (no cloud dependency)
- Simple models > complex models
- Explainable behavior over black-box control
- Customer comfort > benchmark metrics

---

## Project Status
🚧 **Early prototype / research stage**

This repository currently focuses on:
- Feature identification
- Control logic experiments
- ML feasibility validation

---

## Future Directions
- Hardware-in-the-loop testing
- Long-term energy benchmarking
- Robust fault prediction models
- Minimal UI / diagnostics interface

---

## Disclaimer
This is a **research and prototyping project**, not a commercial product.  
Safety-critical controls must always include hard-coded fail-safes.

---

## License
MIT License
