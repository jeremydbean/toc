# Agent Notes

## Farslayer / `spell_vengence`
- `spell_vengence` (see `src/magic2.c`) summons the legendary sword Farslayer to strike a chosen victim anywhere in the world.
- The caster targets a specific `victim` (via `TAR_IGNORE` routing in `obj_cast_spell`) and triggers a dramatic series of emotes before the attack resolves.
- Success is determined by `number_percent()`:
  - **Chance > 50:** the victim is struck; they are reduced to 1 HP/mana/move, killed via `raw_kill`, and a WANTED flag is set on the caster with immortal notifications logged.
  - **Chance ≤ 50:** the spell backfires; the caster suffers the same killing sequence and WANTED flag.
- Casting always exacts a steep cost on non-immortal casters _before_ the kill outcome: current HP/mana/move are set to 1, max pools and permanent pools are reduced by at least 25 (or 2×level), and one random permanent stat is reduced by 3.
- Immortal victims trigger a thematic echo but otherwise behave normally; self-targeting is disallowed.

## Maintenance Tips
- Keep messaging synchronized between the success/backfire paths so players see consistent emotes.
- The spell intentionally punishes casters even on success; avoid “safety” tweaks without design approval.
- Build warnings should stay clean—remove unused locals and align intent with behavior when touching this spell.
