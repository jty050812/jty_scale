---
name: embedded-readme-report
description: 'Analyze embedded projects and generate detailed report-grade README files. Use when: embedded report writing, README generation, lab report, course project report, project analysis, architecture analysis, scheduling design, filtering/calibration logic, fault handling, troubleshooting, build verification, S5PV210, bare-metal ARM, 嵌入式报告, 项目分析, README文档, 实验报告.'
argument-hint: 'project=<folder> audience=<instructor|teammate> depth=<quick|standard|detailed> evidence=<required|optional> include=<benchmark,troubleshooting,code-evidence-table>'
user-invocable: true
---

# Embedded Project README Report

Create a report-grade README from source code, build scripts, and runtime behavior.

## Reader Mode
- Default mode: clear and accessible engineering report.
- Assume readers may not know embedded basics, toolchain setup, or board flashing workflow.
- Prefer "what it is -> why it matters -> how to verify" explanation order.

## When To Use
- You need a complete README for a lab, assignment, or demo report.
- The project has low-level code (startup, drivers, linker script, Makefile).
- You must explain both implementation logic and reproducible build/run steps.

## Inputs
- Target project path (single folder preferred in multi-root workspaces)
- Reader type: instructor, teammate, or external reviewer
- Depth level: quick (1-2 pages), standard (3-5 pages), detailed (6+ pages)
- Whether runtime logs/screenshots are available

## Output Contract
- Output file: `README.md` in target project root (unless explicitly overridden).
- Output style: report-grade, evidence-backed, sectioned markdown.
- Mandatory sections: architecture, scheduling, filtering, calibration, fault handling, troubleshooting.

- Mandatory guidance sections:
	- Quick Start
	- Term Notes (technical terms explained at first appearance)
	- Function I/O and Logic Walkthrough
	- Reproduction checkpoints (expected output per step)
- Mandatory evidence blocks:
	- Build verification block (command + result summary + produced artifacts)
	- Runtime evidence block (key serial logs)
	- Code evidence table mapping claim -> file/module

## Default Profile For This Workspace
- Primary project: `template-serial-shell` (electronic scale)
- Output depth: detailed (6+ pages)
- Evidence policy: mandatory build verification and key serial runtime logs

## Procedure
1. Select scope and output target.
- If workspace has multiple projects, pick one primary project first.
- If no target file is provided, default to `README.md` in the selected project root.
- In this workspace, default to `template-serial-shell/README.md` unless explicitly overridden.

1.5 Resolve ambiguities before drafting when needed.
- Ask exactly 2-3 focused questions only if one of these is unknown:
	- target project in multi-root workspace
	- output depth (quick/standard/detailed)
	- whether evidence is mandatory

2. Collect factual evidence before writing.
- Read project metadata: `README.md`, `LICENSE`, `Makefile`, `link.ld`.
- Read execution entry and flow: `source/main.c`, `source/tester-*.c`.
- Read core domain modules: `include/*.h`, main algorithm files in `source/`.
- Read hardware and platform files: `source/hardware/*.c`, `include/hardware/*.h`.
- Capture build outputs and names (e.g., `*.elf`, `*.bin`, map files).
- Use [evidence checklist](./references/evidence-checklist.md).

2.5 Build a term list before drafting.
- Collect technical terms that may be unfamiliar on first read (example: bare-metal, linker script, jiffies, polling, debounce, tare, counts/gram, symbol extension).
- Follow [term explanation guide](./references/term-explanation-guide.md).

2.6 Select core functions for walkthrough.
- Cover all core-feature functions from the main execution path (entry, scheduling, sampling, conversion, calibration, render, fault handling, diagnostics/logging).
- Follow [function explanation guide](./references/function-explanation-guide.md).

3. Derive architecture and control flow.
- Build a layer map: startup, HAL/drivers, middleware/utilities, app logic, UI/IO.
- Identify timing model: polling, interrupt, tick scheduling, periods and deadlines.
- Identify state model: key structs, flags, transitions, fault states.
- Extract key formulas/constants and explain why each matters.
- Build a claim list first, then attach evidence source for each claim.
- Build a function call chain summary from `main` to core processing functions.

4. Validate reproducibility.
- Run the canonical build command or task if available.
- Confirm generated artifacts and paths.
- Note host assumptions (toolchain, OS differences, serial settings, board address).
- Do not skip this step for report mode; include command output summary.

5. Draft README using a strict structure.
- Start from [report template](./assets/readme-report-template.md).
- Keep all claims evidence-backed (file references, command outputs, constants).
- Separate facts from assumptions with explicit labels.
- Add a dedicated "Code Evidence Index" section using claim-to-source rows.
- For first appearance of a technical term, append a plain-language explanation in one sentence.
- For each operation step, include expected output so readers can self-check progress.
- For each key function, explain:
	- purpose
	- inputs (type + meaning)
	- outputs/return value
	- side effects (state changes, hardware IO, logs)
	- step-by-step runtime logic in plain language

5.5 Add a reproducible quick-start path.
- Add a "Quick Start" section with:
	- prerequisites checklist
	- step-by-step commands
	- expected artifacts/logs per step
	- common failure and immediate fix
- Prefer deterministic commands that match the current workspace task where possible.

6. Apply quality gates.
- Completeness: architecture, build, run, module logic, faults, limitations, next steps.
- Verifiability: each critical claim links to code/file evidence.
- Report readability: narrative from system goal -> design -> implementation -> validation.
- Teaching value: include troubleshooting and "why this design" rationale.
- Score the output with [report rubric](./references/report-rubric.md) before finalizing.
- Ensure function explanations are understandable without reading C syntax first.

7. Final polish and consistency checks.
- Ensure terminology is consistent (same names for modules/signals/states).
- Ensure command blocks are executable as written.
- Ensure section order supports logical reading without backtracking.
- Ensure no speculative statement appears without either evidence or explicit assumption label.

## Branching Logic
- If existing README is strong: preserve sections, upgrade depth and evidence links.
- If existing README is minimal/missing: generate full report from template.
- If hardware details are incomplete: add "Unknowns and Assumptions" and list what to verify.
- If build cannot be executed: provide "Build Not Verified" with exact blocker and expected command.
- If runtime logs are unavailable: provide "Expected Logs" and mark as unverified.
- If codebase is large: summarize secondary modules and prioritize core execution path evidence.
- If output length is constrained: keep concise wording but retain term notes and function I/O cards.

## Completion Criteria
- README has reproducible build/run steps with concrete commands.
- README explains core control loop/state transitions, not just feature bullets.
- README includes risk/fault handling and troubleshooting paths.
- README is suitable for direct submission as project report with minor edits only.
- README includes build evidence and key serial log excerpts as mandatory proof.
- README includes a claim-to-source evidence index.
- Every first-mentioned technical term is explained in plain language.
- Readers can follow the quick-start steps and verify success at each checkpoint.
- README contains function-level I/O and runtime-logic explanations for all core-feature functions.
