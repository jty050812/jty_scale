# Project Title

One-paragraph summary: problem, platform, and key result.

## 0. Quick Start
- Who this is for (assume no prior project knowledge)
- Prerequisites checklist (toolchain, board, cable, serial tool)
- Step-by-step runbook:
	- Step 1: command/action
	- Expected result/checkpoint
	- Step 2: command/action
	- Expected result/checkpoint
- If failed at this step -> quick fix pointer

## 0.5 Term Notes
- Explain each technical term in plain language at first use.
- Keep each explanation to one sentence, with why it matters here.

## 1. Project Objective and Scope
- Objective
- Constraints (hardware, timing, memory, toolchain)
- Out-of-scope items

## 2. Platform and Environment
- SoC/MCU, peripherals, sensors, display/input
- Toolchain and host OS assumptions
- Memory map / load address (if applicable)

## 3. Repository Structure
- Folder-level map and role of each major directory
- Which files are primary entry points

## 4. Build and Deployment
- Build command(s)
- Output artifacts (`.elf`, `.bin`, `.map`)
- Flash/load steps and runtime launch flow
- Add checkpoint table: command -> expected output -> pass condition

## 5. System Architecture
- Layered architecture (startup -> drivers -> app)
- Data flow and control flow
- Scheduling model (tick/interrupt/polling)

## 6. Core Modules and Logic
For each important module:
- Responsibility
- Key APIs / structs / constants
- Internal algorithm and decision logic
- Failure handling behavior

## 6.5 Function I/O and Logic Walkthrough
For each core function, provide a clear function card:

| Function | Inputs | Output/Return | Side Effects | Runtime Logic (Plain Language) |
|---|---|---|---|---|
| Example: scale_update_raw | state: scale state object | bool: success/fail | updates raw/sample buffer/fault flags | Read sensor value, update filter window, compute stable raw, set fault status when timeout repeats |

Minimum coverage requirement:
- all core-feature functions in the main path
- must include entry, scheduling, sampling, conversion/filter, calibration, render, and fault-handling functions

## 7. Runtime Behavior and Validation
- Boot log and expected serial output
- Functional validation scenarios
- Boundary and failure scenarios
- Observed results and interpretation

## 7.5 Code Evidence Index
Use a table to map claims to source evidence.

| Claim | Evidence Source (file/module) | Verification Type |
|---|---|---|
| Example: Sampling period is 50ms | source/tester-scale.c, include/scale.h | Code inspection |

## 8. Troubleshooting Guide
- Symptom -> likely cause -> verification -> fix
- Fast checks for wiring/config/timing
- Include a fast path for the top 3 failure modes

## 9. Limitations and Improvements
- Current limitations
- Prioritized improvement plan

## 10. Conclusion
- What was achieved
- Why the implementation is reasonable
- What can be reused in future projects

## Appendix (Optional)
- Command transcripts
- Parameter tables
- State diagrams
