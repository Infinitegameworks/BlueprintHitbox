# Blueprint Hitbox Plugin for Unreal Engine 5

A complete hitbox data pipeline for 2D games. Import hitbox/socket data from JSON into Data Assets, visualize in the editor, link to Paper2D Flipbooks, and use efficient C++ collision detection from Blueprints.

## Features

- **Data Assets** - Import once, reference everywhere (no runtime JSON loading)
- **Visual Editor** - Preview hitboxes and sockets directly in Unreal
- **One-Click Reimport** - Update data when you change the JSON
- **Flipbook Linking** - Associate animations with Paper2D Flipbooks
- **C++ Collision Detection** - Efficient batch collision checks exposed to Blueprints
- **World-Space Conversion** - Automatic flip and scale handling

## Installation

1. Copy `BlueprintHitbox` folder to `YourProject/Plugins/`
2. Right-click `.uproject` → Generate Visual Studio project files
3. Build in Visual Studio
4. Launch Unreal Editor

## Quick Start

### 1. Import Your JSON

Right-click in Content Browser → **Import Hitbox Data** → Select your JSON file

This creates a `HitboxDataAsset` in your Content folder.

### 2. Link Flipbooks (Optional)

Double-click the asset to open the editor. For each animation, you can assign a Paper2D Flipbook reference.

### 3. Use in Blueprints

```
Character Blueprint:

Variables:
  - HitboxData (type: Hitbox Data Asset) → drag your asset here

On Animation Frame Changed:
  - Get Frame (HitboxData, "Attack", FrameIndex) → FrameData
  - Check Hitbox Collision (my frame, my pos, enemy frame, enemy pos) → Results
  - For Each Result: Apply Damage
```

## JSON Format

The plugin reads your Hitbox Editor's export format:

```json
{
  "Idle": {
    "player_idle_01": {
      "hitboxes": [
        { "type": "hurtbox", "rect": [16, 8, 24, 48], "damage": 0, "knockback": 0 }
      ],
      "sockets": [
        { "name": "Hand", "pos": [32, 24] }
      ]
    }
  },
  "Attack": {
    "player_attack_01": {
      "hitboxes": [
        { "type": "attack", "rect": [45, 12, 32, 24], "damage": 25, "knockback": 150 },
        { "type": "hurtbox", "rect": [10, 5, 30, 50], "damage": 0, "knockback": 0 }
      ],
      "sockets": [
        { "name": "Muzzle", "pos": [64, 20] }
      ]
    }
  }
}
```

## Data Asset Functions

### Getting Data

| Function | Description |
|----------|-------------|
| `Get Animation Names` | List all animation names |
| `Get Animation` | Get full animation data by name |
| `Get Frame` | Get frame data by animation + index |
| `Get Frame By Name` | Get frame data by animation + filename |
| `Get Frame Count` | Number of frames in animation |
| `Find Animation By Flipbook` | Lookup animation by its linked Flipbook |

### Direct Access

| Function | Description |
|----------|-------------|
| `Get Hitboxes` | All hitboxes for a frame |
| `Get Hitboxes By Type` | Filter by Attack/Hurtbox/Collision |
| `Get Sockets` | All sockets for a frame |
| `Find Socket` | Get socket by name |

## Collision Detection

The plugin provides efficient C++ collision functions:

### Check Hitbox Collision

```
Check Hitbox Collision
├── Attacker Frame Data
├── Attacker Position (2D)
├── Attacker Flip X
├── Attacker Scale
├── Defender Frame Data
├── Defender Position (2D)
├── Defender Flip X
├── Defender Scale
├── Out Results (array)
└── Return: bool (any hit?)
```

Each result contains:
- `bHit` - collision occurred
- `AttackHitbox` - the attack box that hit
- `HurtHitbox` - the hurtbox that was hit
- `HitLocation` - world-space collision center
- `Damage` - damage value from attack
- `Knockback` - knockback value from attack

### Quick Hit Check

For simple yes/no collision without details:

```
Quick Hit Check → bool
```

### 3D Position Variants

If your 2D game uses 3D coordinates (X/Z plane):

```
Check Hitbox Collision 3D
Hitbox To World Space 3D
Socket To World Space 3D
```

## World-Space Conversion

### Hitbox To World Space

```
Hitbox To World Space
├── Hitbox Data
├── World Position (character location)
├── Flip X (facing left?)
├── Scale (sprite scale)
└── Return: Box2D in world coordinates
```

### Socket To World Space

```
Socket To World Space
├── Socket Data
├── World Position
├── Flip X
├── Scale
└── Return: Vector2D in world coordinates
```

## Editor Features

### Hitbox Data Asset Editor

Double-click any HitboxDataAsset to open the visual editor:

- **Animation List** - All imported animations
- **Preview Window** - Visual hitbox overlay
- **Details Panel** - Hitbox coordinates and properties
- **Reimport Button** - Update from source JSON

### Context Menu Actions

Right-click a HitboxDataAsset:
- **Reimport from JSON** - Refresh data from source file
- **Show Source File** - Open folder containing JSON

### Flipbook Linking

In the asset details, expand any animation to set its Flipbook reference:

```
Animations
└── Idle
    ├── Animation Name: "Idle"
    ├── Flipbook: [Select your PaperFlipbook]
    └── Frames: [...]
```

Then in Blueprints:
```
Find Animation By Flipbook (MyFlipbook) → AnimationData
```

## Example: Combat System

```
// In your Character Blueprint

Variables:
  HitboxAsset: Hitbox Data Asset
  CurrentAnim: String
  CurrentFrame: Integer

// On attack input
Event Attack:
  Set CurrentAnim = "Attack"
  Play Flipbook

// Every frame during attack
Event Tick (or Flipbook Frame Changed):
  Get Frame (HitboxAsset, CurrentAnim, CurrentFrame) → MyFrame
  
  If Has Attack Hitboxes (MyFrame):
    For Each Enemy:
      Get Frame (Enemy.HitboxAsset, Enemy.CurrentAnim, Enemy.CurrentFrame) → EnemyFrame
      
      Check Hitbox Collision (
        MyFrame, GetActorLocation2D, bFacingLeft, 1.0,
        EnemyFrame, Enemy.GetActorLocation2D, Enemy.bFacingLeft, 1.0
      ) → Results
      
      For Each Result where bHit:
        Enemy.TakeDamage(Result.Damage)
        Enemy.ApplyKnockback(Result.Knockback)
```

## Tips

1. **Load assets at BeginPlay** - Store reference in a variable
2. **Use Quick Hit Check first** - For culling before detailed collision
3. **Reimport preserves Flipbook links** - Your assignments survive updates

## License

Free to use, modify, and distribute.
