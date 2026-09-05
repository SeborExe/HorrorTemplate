# HorrorTemplate — Unreal Engine 5.8 project

## Layout

Repository root is one level above the Unreal project.

```
<repo root>/                      <- git root, run claude from here
├── .gitignore
├── README.md
├── CLAUDE.md                     <- this file
└── HorrorTemplate/               <- Unreal project
    ├── HorrorTemplate.uproject
    ├── Config/
    ├── Content/                  <- binary assets, do not edit
    └── Source/
        ├── HorrorTemplate/       <- primary game module
        ├── HorrorTemplate.Target.cs
        └── HorrorTemplateEditor.Target.cs
```

Absolute paths on this machine:

- Repo root: `C:\Users\harad\Documents\GitHub\HorrorTemplate`
- Project file: `C:\Users\harad\Documents\GitHub\HorrorTemplate\HorrorTemplate\HorrorTemplate.uproject`
- Engine: `C:\Program Files\Epic Games\UE_5.8\Engine`

Environment: Unreal Engine 5.8, Windows Win64, JetBrains Rider 2026.

Plugins in use (from `HorrorTemplate/HorrorTemplate.uproject`):

- **StateTree** — runtime StateTree framework.
- **GameplayStateTree** — `UStateTreeAIComponent` and gameplay StateTree integration;
  drives the Shooter NPC AI.
- **ModelingToolsEditorMode** — editor-only mesh modelling tools, allow-listed to the
  `Editor` target. No runtime impact.

The primary module additionally links Enhanced Input, `AIModule` (AI perception, EQS,
navigation) and `UMG`/`Slate` — see `Source/HorrorTemplate/HorrorTemplate.Build.cs`
(`EnhancedInput`, `AIModule`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`,
`Slate`). The `.uproject` also names `Engine`, `AIModule` and `UMG` as additional
module dependencies. These are engine features rather than separately toggled plugins.

## Hard rules

1. **Never guess engine API.** Before using any UE type, function or macro, grep
   `C:\Program Files\Epic Games\UE_5.8\Engine\Source` and confirm the exact signature.
   My training data predates UE 5.8 — anything I "remember" about 5.8 APIs may be wrong.
2. **Never edit `.uasset` or `.umap`.** They are binary. If a change belongs in a
   Blueprint, describe the node changes so I can apply them in the editor, or propose
   moving the logic to C++.
3. **Never launch or close the Unreal Editor without asking.**
4. **Ask before touching `Config/*.ini`.** Renderer and scalability settings there
   affect every performance measurement.
5. Ignore entirely: `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`,
   `Content/`, `.idea/`.

## Build and test

Build the editor target (run from anywhere):

```
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" HorrorTemplateEditor Win64 Development -Project="C:\Users\harad\Documents\GitHub\HorrorTemplate\HorrorTemplate\HorrorTemplate.uproject" -WaitMutex
```

Regenerate project files after adding or removing source files:

```
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="C:\Users\harad\Documents\GitHub\HorrorTemplate\HorrorTemplate\HorrorTemplate.uproject" -game -rocket -progress
```

Run automation tests headless:

```
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Users\harad\Documents\GitHub\HorrorTemplate\HorrorTemplate\HorrorTemplate.uproject" -ExecCmds="Automation RunTests HorrorTemplate" -unattended -nopause -nosplash -testexit="Automation Test Queue Empty" -log
```

Always build after C++ changes. Do not report a task as done if the build fails.
The editor must be closed for a build to succeed — if the build fails on locked DLLs,
tell me to close the editor rather than killing the process yourself.

## Code conventions

- UE prefixes: `U` UObject, `A` AActor, `F` struct, `E` enum, `I` interface, `T` template.
- `TObjectPtr<>` for UPROPERTY object references; raw pointers only for locals.
- Forward-declare in headers, include in `.cpp`. Keep headers lean.
- `check()` for programmer errors that must abort, `ensure()` for recoverable ones.
- `UE_LOG` with a project log category — no `AddOnScreenDebugMessage` in committed code.
- Designer-facing values: `UPROPERTY(EditAnywhere, BlueprintReadWrite)` with a `Category`.
- Replicated state needs `GetLifetimeReplicatedProps`.

## C++ / Blueprint split

The project is the Unreal "First Person" sample with two variants (Horror, Shooter).
**Every gameplay system currently lives in C++**, each as an `abstract` class; Blueprints
only subclass them to supply assets — meshes, `UInputAction` / `UInputMappingContext`,
montages, anim instances — plus data tables, StateTree assets, AI perception config,
widget layouts (via `BlueprintImplementableEvent`) and level content. No gameplay logic
has been moved into Blueprint graphs yet.

Default direction: logic that runs every frame or across many actors belongs in C++.
Blueprints are for composition, designer tuning and one-off scripted behaviour.

### Systems implemented in C++

**Shared first-person framework (`Source/HorrorTemplate/`)**

- `AHorrorTemplateCharacter` — base FP pawn: capsule, owner-only first-person skeletal
  mesh, camera on the `head` socket, Enhanced Input bindings for move / look / jump /
  mouse-look. Input is routed through virtual `DoMove` / `DoAim` / `DoJumpStart` /
  `DoJumpEnd` so either controls or touch UI can drive the pawn.
- `AHorrorTemplatePlayerController` — sets the custom camera manager, adds Input Mapping
  Contexts through the Enhanced Input subsystem, spawns UMG touch controls on mobile or
  when forced.
- `AHorrorTemplateCameraManager` — `APlayerCameraManager` that clamps view pitch.
- `AHorrorTemplateGameMode` — minimal `AGameModeBase`.
- `HorrorTemplate.h/.cpp` — module bootstrap and the `LogHorrorTemplate` log category.

**Horror variant (`Variant_Horror/`)**

- `AHorrorCharacter` — head-mounted `USpotLightComponent` flashlight plus a stamina-based
  sprint system driven by a fixed-interval **timer** (not `Tick`): burns / regenerates a
  sprint meter and swaps `MaxWalkSpeed` between walk / sprint / recovery speeds. Exposes
  `OnSprintMeterUpdated` and `OnSprintStateChanged` multicast delegates.
- `AHorrorPlayerController` — IMC + touch setup; on possess spawns `UHorrorUI` and binds
  it to the character.
- `AHorrorGameMode` — split-screen local player creation; tag-based (`Player0`,
  `Player1`, …) `PlayerStart` selection.
- `UHorrorUI` — listens to the sprint delegates and forwards to BP events.

**Shooter variant (`Variant_Shooter/`)**

Weapons (`Weapons/`):

- `IShooterWeaponHolder` — interface between a weapon and its holder (player or NPC):
  attach meshes, play montage, apply recoil, update HUD, provide aim location, grant a
  weapon, activate / deactivate, semi-auto refire notification.
- `AShooterWeapon` — base weapon actor with first- and third-person skeletal meshes,
  magazine / ammo tracking, semi- and full-auto fire via timers, projectile spawn
  transform with an aim-variance cone, recoil, and AI-perception noise on every shot.
  Sets the holder's anim instance classes on activation. (`bCanEverTick = true` with no
  `Tick` override — a perf-review candidate.)
- `AShooterProjectile` — sphere collision + `UProjectileMovementComponent`; single-hit or
  radial explosion damage (`OverlapMultiByObjectType`), physics impulse on hit, AI noise
  on impact, deferred self-destruction timer, `BP_OnProjectileHit` hook.
- `AShooterPickup` — weapon pickup configured from a `FWeaponTableRow` DataTable row
  (mesh + weapon class); sphere overlap grants the weapon through the holder interface,
  then hides and respawns on a timer with a BP respawn hook.

Characters:

- `AShooterCharacter` — player FP pawn implementing `IShooterWeaponHolder`: weapon
  inventory and switching, pickup ingestion, health / `TakeDamage`, death and timed
  respawn, camera line trace for the aim point, recoil, `UPawnNoiseEmitterComponent`,
  team byte, and `OnBulletCountUpdated` / `OnDamaged` HUD delegates.
- `AShooterNPC` — AI FP pawn also implementing `IShooterWeaponHolder`: spawns and holds a
  weapon, aims with a vertical offset + cone randomness + line-of-sight trace, HP /
  damage, ragdoll death with deferred destruction, `StartShooting` / `StopShooting`
  (called from StateTree tasks), `OnPawnDeath` delegate.

AI (`AI/`):

- `AShooterAIController` — owns a `UStateTreeAIComponent` (manual start) and a
  `UAIPerceptionComponent`; relays perception updated / forgotten events to StateTree via
  delegates, tracks the current target, tears down on pawn death.
- `AShooterNPCSpawner` — spawns NPCs one at a time, waiting for the current one to die
  before spawning the next; honours `AShooterGameMode::ShouldSpawnEnemyNPCs()`
  (single-player only). Tick disabled.
- `UEnvQueryContext_Target` — EQS context returning the controller's current target actor
  (falls back to the controller itself).
- `ShooterStateTreeUtility` — custom StateTree nodes: condition **Has Line of Sight to
  Target** (multi-height LOS traces inside a cone) and tasks **Face Towards Actor**,
  **Face Towards Location**, **Set Random Float**, **Shoot at Target**, and **Sense
  Enemies** (binds the controller's perception delegates; distinguishes a direct sighting
  from a partial stimulus, broadcasts investigate-location vs. attack, decays stimulus
  strength over time).

Framework + UI:

- `AShooterPlayerController` — IMC / touch setup, spawns the bullet-counter widget, tags
  and teams the pawn, respawns it at a team-tagged `PlayerStart` when destroyed, forwards
  ammo / damage delegates to the HUD.
- `AShooterGameMode` — spawns the scoreboard UI, creates local players with alternating
  team assignment, tag-based `PlayerStart` selection, keeps a per-team score map
  (`IncrementTeamScore`), and gates NPC spawning (`ShouldSpawnEnemyNPCs`).
- `UShooterBulletCounterUI` — BP events for ammo count and damage / life feedback.
- `UShooterUI` — BP event for scoreboard updates.

When converting Blueprint to C++, expose the same pins as `UPROPERTY`/`UFUNCTION` so
existing Blueprint subclasses keep working, and list which BP assets need reparenting.

## Performance

Frame time is the current focus. Treat every gameplay change as a performance change.

Flag on sight, in C++ and in Blueprint graphs alike:

- `Tick` where a timer or longer tick interval would do; default to
  `PrimaryActorTick.bCanEverTick = false`
- `GetAllActorsOfClass` / `GetAllActorsWithTag` outside one-time init
- Casts and `FindComponentByClass` in hot paths — cache them
- Per-frame allocations, `TArray` growth in loops, string building
- Dynamic material instance creation per frame
- Overlap events where a single trace would be enough

Profiling workflow:

1. `stat unit` first — Frame / Game / Draw / GPU decides where to look at all.
2. `csvprofile start` / `csvprofile stop` writes to `Saved/Profiling/CSV`.
   Analyse the CSV rather than reasoning about the code in the abstract.
3. `memreport -full` for memory questions.
4. Measure a baseline, change one thing, measure again with the same scenario.
   Never claim an optimisation worked without a second measurement.

## Workflow preferences

- Plan before editing when a task touches more than two files.
- Small, reviewable diffs. One concern per commit.
- Explain trade-offs in one or two sentences, not in essays.
- If a task needs the editor or a Blueprint change, stop and tell me what to click.
