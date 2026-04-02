---
name: ci-fix
description: Scan all CI builds, find failures, fetch error logs, and fix the code. Prioritizes uno, attiny85, esp32s3, esp32c6, teensy41. Use when CI is red and you need to diagnose and repair build failures.
argument-hint: [--board <name>] [--all]
context: fork
---

Find and fix CI build failures across all platforms.

Arguments: $ARGUMENTS

## Step 1: Scan builds and get error logs

Run the CI build scanner to discover all failing builds with error context:

```bash
uv run ci/tools/gh/ci_builds.py --errors $ARGUMENTS
```

This auto-discovers builds from `.github/workflows/build_*.yml` files. Adding a new workflow file there automatically adds it to this report.

## Step 2: Analyze and prioritize

Review the output. Builds are already sorted by priority:

1. **uno** — AVR 8-bit, most constrained, most users
2. **attiny85** — AVR tiny, extreme flash/RAM constraints
3. **esp32s3** — ESP32-S3, primary ESP target
4. **esp32c6** — ESP32-C6, RISC-V ESP target
5. **teensy41** — Teensy 4.1, ARM Cortex-M7
6. **Everything else** — alphabetical within non-priority builds

Fix priority builds first. If `--board <name>` was passed, focus only on those.

## Step 3: For each failing build

1. Read the error logs from the scan output
2. Identify the root cause (missing header, type error, platform ifdef, linker error, etc.)
3. Find the relevant source file(s) and read them
4. Make the fix — prefer the simplest change that fixes the error:
   - Missing platform guard → add `#ifdef` / `#if defined()`
   - Missing include → add the include
   - Type mismatch → fix the type
   - API difference → use conditional compilation
5. After fixing, verify by compiling: `bash compile <board> --examples Blink`
   (use the board arg from the scan output, e.g. `bash compile uno --examples Blink`)

## Step 4: Verify fixes don't break other platforms

After fixing a failing build, consider whether the change could break other platforms. If you changed shared code (not platform-guarded), spot-check by compiling another platform:

```bash
bash compile wasm --examples Blink
```

## Notes

- The scanner discovers builds dynamically from workflow files — no hardcoded list to maintain
- To add a new build to the scanner, just add a `build_<name>.yml` workflow file
- Use `--board uno,esp32s3` to focus on specific boards
- Use `--all` flag if you want to see passing builds too (default: only failing with --errors)
- Error logs show up to 5 error blocks per build with surrounding context
- The priority list is defined in `ci/tools/gh/ci_builds.py` in the `PRIORITY_BOARDS` constant
