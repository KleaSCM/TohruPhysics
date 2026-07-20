# Section 1.1 — Memory Architecture & Arena Systems

> **Spec tasks:** 0001–0012
> **Implements:** `TohruArenaInit`, `KobayashiAlloc`, `ElmaArenaReset`, etc.

---

## Design

The engine uses **expandable bump arenas** backed by anonymous `mmap`. There are no per-element constructors, destructors, or free operations. Memory is allocated linearly from the arena until it is reset or destroyed.

### Key characteristics

| Property | Value |
|---|---|
| Allocation | Bump (O(1)) |
| Growth | `mremap` with `MREMAP_MAYMOVE`, doubles capacity |
| Initial page size | `sysconf(_SC_PAGESIZE)` (typically 4096) |
| Min alignment | 16 bytes |
| Zero-invariant | `ElmaArenaClear` zeroes the entire backing store |

### ZII contract

KobayashiAlloc / KobayashiAllocAlign / KobayashiDup **never return null**. Every
pointer is always valid and usable. When the arena backing store cannot be
expanded (system OOM, rlimit), the functions return `&TohruZeroBlock` — a global
4 KB page of zeros in `.bss`.

```cpp
// ZII in practice
PhysicsBody *Body = KobayashiAlloc(&Arena, sizeof(PhysicsBody));
// Body is always valid. On exhaustion it points to ZeroBlock.
ApplyForce(Body, Force);
```

This eliminates entire categories of bugs: no null-deref, no stale data from
uninitialised memory, no branch-on-error in hot paths. The engine is designed
such that every type accepts `0` (or an all-zeroes bit pattern) as a meaningful
default — velocity zero means "not moving", mass zero means "infinite mass",
transform zero means "identity".

### Expandability

When `KobayashiAlloc` or `KobayashiAllocAlign` finds insufficient space, `EnsureSpace` doubles the arena capacity via `mremap(MREMAP_MAYMOVE)`. The kernel may relocate the entire mapping to a new virtual address.

**IMPORTANT:** Any raw pointer obtained *before* an expansion is invalidated if the mapping moves. Use `IluluOffset`/`IluluPtr` for references that survive across expansion boundaries. This is by design — it mirrors the offset-based addressing used in all production game engines.

```
Time ────────────────────────────────────────────>
                                                   
  P1 = KobayashiAlloc(A, 48)                      
  Off1 = IluluOffset(A, P1)     ← save offset     
                                                   
  P2 = KobayashiAlloc(A, 1000)  ← triggers growth 
       mremap moves A→Base                         
                                                   
  P1 is DANGLING now!           ← raw pointer lost 
  IluluPtr(A, Off1)             ← reconstruct via offset
```

### Multi-layer partitioning (`ArenaSet`)

| Arena | Scope | Reset | Typical size |
|---|---|---|---|
| `Frame` | Per-timestep scratch | `YuyuArenaSetResetFrame` (O(1)) | 64 KB |
| `World` | Persistent simulation state | Manual | 512 KB |
| `Worker` | Per-thread scratch | Manual | 64 KB |

Frame arena is reset at the start of every physics timestep. All per-step allocations (contacts, solver scratch, temp vectors) live here.

---

## Function reference

| Function | Character | Purpose |
|---|---|---|
| `TohruArenaInit` | Tohru | Initialise arena with mmap (expandable) |
| `TohruArenaInitFixed` | Tohru | Initialise from external buffer (no growth) |
| `TohruArenaDestroy` | Tohru | Release backing pages via munmap |
| `KobayashiAlloc` | Kobayashi | Bump-allocate N bytes |
| `KobayashiAllocAlign` | Kobayashi | Bump-allocate with power-of-two alignment |
| `KobayashiDup` | Kobayashi | Duplicate a memory block into arena |
| `ElmaArenaReset` | Elma | Reset offset (O(1), no zero) |
| `ElmaArenaClear` | Elma | Zero entire arena and reset (O(n)) |
| `ElmaArenaUsed` | Elma | Bytes consumed |
| `ElmaArenaRemaining` | Elma | Bytes remaining before growth |
| `IluluOffset` | Ilulu | Pointer → offset from arena base |
| `IluluPtr` | Ilulu | Offset → pointer into arena |
| `IluluOwns` | Ilulu | Check if pointer is within arena |
| `YuyuArenaSetInit` | Yuyu | Initialise all three sub-arenas |
| `YuyuArenaSetDestroy` | Yuyu | Destroy all sub-arenas |
| `YuyuArenaSetResetFrame` | Yuyu | Reset the frame arena only |

---

## Tasks completed

- [x] 0001 — `Arena` structure with append-only bump pattern
- [x] 0002 — `ZeroBlock` invariant (zeroed memory is always valid)
- [x] 0003 — Runtime safety through `EnsureSpace` + offset reconstruction
- [x] 0004 — `TohruArenaInit` with page-aligned mmap
- [x] 0005 — `ElmaArenaClear` zeroes all memory before reset
- [x] 0006 — Arena allocation is the scratch-pad (no separate sub-allocator needed — bump is the allocator)
- [x] 0007 — No per-element constructors (plain memset/struct initialisation)
- [x] 0008 — No per-element destructors (batch teardown via munmap)
- [x] 0009 — `IluluOffset`/`IluluPtr` for offset-based addressing
- [x] 0010 — `ArenaSet` with Frame/World/Worker partitioning
- [x] 0011 — No RAII, no smart pointers, no new/delete in the allocator
- [x] 0012 — 64-byte alignment via `KobayashiAllocAlign` for SIMD lane safety

---

## Naming

All function names in this section are drawn from `Doc/Spec/Cuties.md`:

| Name | Source |
|---|---|
| Tohru | Miss Kobayashi's Dragon Maid |
| Kobayashi | Miss Kobayashi's Dragon Maid |
| Elma | Miss Kobayashi's Dragon Maid |
| Ilulu | Miss Kobayashi's Dragon Maid |
| Yuyu | Yuyu Shirai (The iDOLM@STER) |
