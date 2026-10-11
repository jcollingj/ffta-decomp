---
name: luna-driver
description: Delegate FFTA gameplay execution to Luna. The parent sets strategic intent; Luna presses buttons, watches the screen, saves checkpoints, and logs. Use when playing or advancing the game.
---

# Luna driver: strategist/executor split for playing FFTA

Claude is the **strategist**: decides what should happen next in the game
(e.g. "get through the snowball fight tutorial, prioritize attacking").
**Luna** (harness model `gpt-6-luna`, or configurable Codex CLI model) is the **executor**: presses
buttons via `tools/emu/fftactl`, screenshots, retries, and logs. One Luna
thread persists across calls so it accumulates game knowledge. This exists
to save Claude tokens (and the user's cash): Claude should NOT drive the
game frame-by-frame or read every screenshot.

## Commands (run from repo root)

```sh
tools/emu/luna new [extra instructions]  # start fresh thread; sends tools/emu/LUNA_BRIEFING.md
tools/emu/luna "<strategic intent>"      # send next intent to the pinned thread
tools/emu/luna status                    # show pinned thread id (logs/.luna_thread)
```

- Luna's final report prints to stdout; token usage goes to stderr.
- Thread id is pinned in `logs/.luna_thread` — do not rely on `--last`
  (other codex sessions in this dir would steal it).
- Emulator must be up (`tools/emu/fftactl start`); Luna can restart it if told.

## Protocol for Claude

1. Ensure emulator + viewer are running (viewer: `cd viewer && bun dev`,
   streams screen + `logs/playthrough.log` to http://localhost:3000).
2. Send ONE clear intent per call: goal, constraints, and when to stop.
   Good: "Advance dialogue until the snowball fight battle starts, then stop
   and report the battle UI." Bad: "play the game".
3. After each report, spot-check by Reading the screenshot path Luna reports
   (or `tools/emu/fftactl screen check.png`) — trust but verify.
4. Course-correct with the next intent. Only take manual fftactl control for
   debugging/memory work, not routine play.
5. If the thread degrades (confused, lost context), `tools/emu/luna new`
   with a state summary in the extra instructions.

## Gotchas

- The CLI defaults to `gpt-6-luna`; override with `LUNA_MODEL` if the account exposes a different Luna model. `CODEX_BIN` overrides the executable. In the integrated harness, select an explicitly available Luna model. `codex exec --json` events: `thread.started` carries
  `thread_id`; `item.completed`/`agent_message` carries text.
- The codex sandbox blocks network by default — the wrapper passes
  `-c sandbox_workspace_write.network_access=true` so Luna can reach the
  SkyEmu HTTP server on localhost:8080.
- Luna logs to `logs/playthrough.log` (the viewer tails it); screenshots to
  `logs/shots/`. If logs stop appearing, remind Luna of the logging duty.
- Screenshots leave the game paused. Advance animations explicitly; do not rely on thinking time.
- Capture named checkpoints after milestones with `fftactl checkpoint save PATH`. Screenshots are not savestates. Create an in-game battery save before claiming a reusable cross-build baseline.
