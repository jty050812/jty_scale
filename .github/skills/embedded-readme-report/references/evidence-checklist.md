# Evidence Checklist for Report-Grade README

## A. Basic Facts
- [ ] Project target name from Makefile
- [ ] Build toolchain prefix and CPU/FPU flags
- [ ] Linker script and load address facts
- [ ] Actual output artifact names and locations

## B. Architecture Evidence
- [ ] Entry path from main to runtime loop
- [ ] Scheduler/timing periods and conversion method
- [ ] Core state struct fields and meaning
- [ ] Fault flags and recovery conditions

## C. Module Evidence
- [ ] Sensor/IO driver responsibilities
- [ ] Filtering/calibration/math logic (if applicable)
- [ ] UI/render or serial interaction path
- [ ] Key constants with rationale (thresholds/timeouts)

## C.5 Function-Level Evidence
- [ ] Core functions list exists (covering all core-feature functions in main execution path)
- [ ] Each listed function documents inputs, outputs, and side effects
- [ ] Runtime logic for each listed function is explained in plain language
- [ ] Function explanations are mapped to source files/modules

## D. Verification Evidence
- [ ] Build command executed (or blocker documented)
- [ ] Representative runtime log lines
- [ ] At least 3 troubleshooting cases
- [ ] Explicit assumptions section when data is missing

## E. Readability and Report Quality
- [ ] Section order follows objective -> design -> implementation -> validation
- [ ] No unverified claims
- [ ] Terms and naming are consistent across sections
- [ ] Commands are copy-runnable in target environment

## F. Reproduction Readiness
- [ ] Quick Start section exists and can be followed without prior project context
- [ ] First-mentioned technical terms include plain-language explanations
- [ ] Each critical step has expected output/checkpoint
- [ ] Top 3 common failures include immediate fix path
- [ ] Readers can understand key function behavior without reading raw C code first
