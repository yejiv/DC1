## Working Guidelines (read first)

Behavioral guidelines to reduce common mistakes. Bias toward caution over speed; for trivial tasks, use judgment.

**1. Think before coding.** Don't assume, don't hide confusion, surface tradeoffs. State assumptions explicitly and ask if uncertain. If multiple interpretations exist, present them — don't pick silently. If a simpler approach exists, say so and push back when warranted. If something is unclear, stop and name what's confusing.

**2. Simplicity first.** Minimum code that solves the problem, nothing speculative. No features beyond what was asked, no abstractions for single-use code, no unrequested "flexibility/configurability", no error handling for impossible scenarios. If it's 200 lines and could be 50, rewrite it. Ask: "Would a senior engineer say this is overcomplicated?"

**3. Surgical changes.** Touch only what you must; clean up only your own mess. Don't "improve" adjacent code/comments/formatting, don't refactor what isn't broken, match existing style even if you'd do it differently. Notice unrelated dead code → mention it, don't delete it. Remove orphans *your* changes created, but leave pre-existing dead code unless asked. Test: every changed line should trace directly to the request.

**4. Goal-driven execution.** Define success criteria and loop until verified. Turn "fix the bug" into "reproduce it, then make it pass"; "refactor X" into "ensure behavior holds before and after". For multi-step work, state a brief plan with a verify check per step. (This project has no in-repo automated tests — verify via editor compile + Play-In-Editor behavior.)

**5. Plan documents in Korean.** When writing a plan file (plan mode) or other planning/design write-ups for this project, write in Korean, not English. Code, identifiers, and comments stay as-is (English/existing convention) — this is about the plan narrative itself.

These are working if: fewer unnecessary diff lines, fewer rewrites from overcomplication, and clarifying questions come *before* implementation rather than after mistakes.
