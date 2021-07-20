# Core Components

## ECS World

- component type creation
- system creation
- component type querying
- namespaced component type lookup

## Module Loading

- version information
- ECS access control
- namespace creation

## Space

- spatial components (position, rotation, bounding box, etc.)
- entity spatial hierarchy
- spatial partitioning (probably grid + hashmap)
- queries (ray cast, bounding box, sphere, etc.)
- LOD?

## Rendering Interface

- cameras and viewports
- viewport frustum culling w/ spatial queries
- frame synchronization

## Asset Management

- binary blob hashing
- metadata with JSON?
- ID lookup
- from disk
- caching
- from network

## Console and Configuration

- interactable text console
- informative logging
- cvar registry
- TOML loading?

## Scripting Interface

- exposure to console
- user resources
  * asset refcounting
- sandboxing
  * by entity
	* by component type
- scheduling
  * component type r/w access
  * interrupting rogue scripts

## Networking Interface

- entity range mapping
- component mutability
- interpolation?

## Debugging Utilities

- log files
- Tracy profiling
- component/system statistics

# Resource Namespacing

- everything starts with `mondradiko.` (leaves the door open for interop!)
- prefix for each type? (like `$` for component types and `#` for asset types or something)
- `snake_case` or `CamelCase`?

Modules:
`mondradiko.physics`
`mondradiko.audio`
`mondradiko.core`

Components:
`mondradiko.core.position`
`mondradiko.renderer.mesh_renderer`
`mondradiko.renderer.point_light`
`mondradiko.audio.audio_source`

Assets:
`mondradiko.physics.shape`
`mondradiko.renderer.mesh`

# External Modules

Not necessarily in a different repository.

## Vulkan Renderer

## Client-Server Networking Implementation

## Bullet Physics

## OpenXR Display

## Windowed Display

## WebAssembly Scripting Environment

## GraphQL World Query Server
