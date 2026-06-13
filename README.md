# Q*bert

**Source repository:** https://github.com/PechaPatrik/prog4_pecha_patrik

## Design notes

**Sound runs on a worker thread.** The sound system pushes requests onto a mutex-protected queue and signals a condition variable. A dedicated thread wakes up, dequeues, and plays. This keeps audio off the main thread entirely so a burst of sound triggers in one frame never causes a hitch.

**Coily input uses a shared_ptr alive flag.** `CoilyMoveCommand` holds a raw pointer to `CoilyComponent`, which becomes dangling when the round ends and the scene destroys the object before input bindings are cleared. Rather than restructuring ownership, `CoilyComponent` exposes a `shared_ptr<bool>` that it sets to false in its destructor. The command holds a copy and checks it before touching the pointer.

**Cube tile behaviour is entirely data-driven through state + enum.** Each cube holds a `unique_ptr<CubeTileState>`. The three level rules (single step, double step, revert on retouch) are expressed as a `LevelRule` enum passed into `OnStep`, so all three level behaviours fall out of the same three state classes with no branching in the cube component itself.

**All gameplay parameters are in JSON.** Nothing gameplay-related is hardcoded. `game_config.json` controls timing, point values, lives, arc height, and more. Level files control the cube rule, color scheme per round, enemy spawn intervals and locations, and disc counts.

**Resource manager caches by filename and size.** Textures and fonts are stored in maps keyed by filename (and size for fonts) using `shared_ptr`. The same asset loaded twice returns the same pointer. `UnloadUnusedResources` evicts entries whose use count has dropped to one, meaning nothing in the game still holds a reference.
