# Function Explanation Guide

## Goal
Help readers understand what core functions do without requiring deep C language background.

## Function Card Format
For each key function, document these fields:
- Function name and module location
- Purpose: what problem this function solves
- Inputs: each parameter's type and meaning
- Output/Return: what success/failure or value means
- Side effects: what state/hardware/log output this function changes
- Runtime logic: step-by-step behavior in plain language

## Selection Rules
- Cover all core-feature functions on the main execution path.
- Must include at least:
  - entry/scheduler functions
  - sensor read and sampling update functions
  - filter/convert functions
  - calibration/control functions
  - render/display function
  - fault handling/recovery functions

## Writing Rules
- Avoid C syntax-heavy explanations unless necessary.
- Explain parameter meaning before discussing algorithm details.
- Use consistent vocabulary for state variables and flags.
- Mention failure branches explicitly (what triggers failure and what happens next).

## Quality Check
A first-time reader should be able to answer:
1. What data goes into this function?
2. What comes out of this function?
3. What does this function change in the system?
4. What are the success and failure paths?

If any answer is unclear, simplify the explanation and add one short example.
