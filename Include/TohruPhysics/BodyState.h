/**
 * BodyState — production rigid body physical state components.
 * TohruPhysics用のプロダクション剛体物理状態コンポーネントよ。
 *
 * Complete component set for rigid body simulation: position/velocity state,
 * mass properties, inertia tensor, surface material, sleep control, damping,
 * velocity limits, body classification, and inertia computation helpers.
 *
 * DESIGN PHILOSOPHY:
 * A rigid body's full state decomposes into seven components, each with a
 * specific update frequency and access pattern:
 *
 * BodyID (1 u64) — persistent handle, assigned at creation
 *
 * BodyFlags (1 u32) — dirty bits for broadphase, active state, CCD request
 *
 * RigidBodyState (30 scalars) — written every frame by integrator,
 * read by solver and collision detection. Position/Rotation are the
 * "current" state; PreviousPosition/Rotation support interpolation and CCD.
 *
 * MassProperties (4 scalars) — constant after init. Mass = 0 means infinite
 * (static body). InverseMass is precomputed for solver performance.
 *
 * InertiaTensor (18 scalars) — InverseInertiaLocal is constant.
 * InverseInertiaWorld is recomputed when orientation changes (R * I * R^T).
 * The inertia computation helpers (Box, Sphere, Capsule) compute I_local
 * from shape parameters using standard formulas.
 *
 * DampingConfig (4 scalars) — linear/angular damping applied each frame
 * as velocity *= 1/(1 + damping * dt). Standard exponential decay model
 * from Bullet/Box2D.
 *
 * SleepConfig (2 scalars + 1 timer) — bodies below thresholds for a
 * dwell time are put to sleep. Solver skips sleeping bodies entirely.
 *
 * MaterialConfig (3 scalars) — restitution, static friction, dynamic
 * friction. Combined between bodies via configurable mode (Multiply,
 * Average, Min, Max per Bullet's combined restitution/friction).
 *
 * KinematicConfig — body type enum (Static=0, Dynamic=1, Kinematic=2)
 * instead of a single bool. Static bodies never move. Kinematic bodies
 * are user-driven (no force integration).
 *
 * DATA LAYOUT: RigidBodyState (30 scalars = 240 bytes)
 * ┌───────────────────┬──────────┬─────────────────────────────────────┐
 * │ Field             │ Type     │ Purpose                             │
 * ├───────────────────┼──────────┼─────────────────────────────────────┤
 * │ Position          │ Vector3  │ World-space position                │
 * │ Rotation          │ Quaternion│ Orientation                         │
 * │ LinearVelocity    │ Vector3  │ Velocity of center of mass          │
 * │ AngularVelocity   │ Vector3  │ Rotational velocity (rad/s)         │
 * │ Force             │ Vector3  │ Accumulated force (zeroed post-step)│
 * │ Torque            │ Vector3  │ Accumulated torque (zeroed post-step)│
 * │ PreviousPosition  │ Vector3  │ Position at start of step (CCD)     │
 * │ PreviousRotation  │ Quaternion│ Rotation at start of step (CCD)    │
 * └───────────────────┴──────────┴─────────────────────────────────────┘
 *
 * INERTIA FORMULAS (for InertiaComputeBox/Sphere/Capsule):
 *
 * Solid box (half-extents hx, hy, hz, mass m):
 *   Ixx = m * (hy² + hz²) / 3
 *   Iyy = m * (hx² + hz²) / 3
 *   Izz = m * (hx² + hy²) / 3
 *
 * Solid sphere (radius r, mass m):
 *   Ixx = Iyy = Izz = 2/5 * m * r²
 *
 * Solid capsule (cylinder + hemispherical caps, radius r, height h):
 *   Cylinder Ixx = Izz = m_cyl * (3*r² + h²) / 12
 *   Caps Ixx += 2 * m_cap * (2*r²/5 + h²/4 + 3*h*r/8)
 *
 * References:
 * - Bullet Physics: btRigidBody component decomposition
 * - Box2D 3.0: b2Body state management
 * - Erleben, "Physics-Based Animation": Chapter 7
 * - Goldstein, "Classical Mechanics": inertia tensor formulas
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Quaternion.h>
#include <TohruPhysics/Matrix.h>

#include <stdint.h>

// ---------------------------------------------------------------------------
//  BodyType — three-way classification
//  三値分類のボディタイプね。
// ---------------------------------------------------------------------------
typedef enum {
	BodyType_Static    = 0,
	BodyType_Dynamic   = 1,
	BodyType_Kinematic = 2
} BodyType;

// ---------------------------------------------------------------------------
//  BodyFlags — bit flags for body state
//  ボディ状態のビットフラグよ。
// ---------------------------------------------------------------------------
typedef enum {
	BodyFlag_Active       = 1 << 0,
	BodyFlag_BroadphaseDirty = 1 << 1,
	BodyFlag_AllowSleep   = 1 << 2,
	BodyFlag_RequestCcd   = 1 << 3,
	BodyFlag_ForceUpdated = 1 << 4
} BodyFlag;

// ---------------------------------------------------------------------------
//  MaterialCombine — how to combine contact properties
//  接触プロパティの結合方法ね。
// ---------------------------------------------------------------------------
typedef enum {
	MaterialCombine_Multiply = 0,
	MaterialCombine_Average  = 1,
	MaterialCombine_Min      = 2,
	MaterialCombine_Max      = 3
} MaterialCombine;

// ---------------------------------------------------------------------------
//  BodyID — persistent handle (index into body array)
//  永続ハンドル（ボディ配列へのインデックス）ね。
// ---------------------------------------------------------------------------
typedef uint64_t BodyID;

// ---------------------------------------------------------------------------
//  0091: RigidBodyState — pose, momentum, previous pose
// ---------------------------------------------------------------------------
typedef struct {
	Vector3  Position;
	Quaternion Rotation;
	Vector3  LinearVelocity;
	Vector3  AngularVelocity;
	Vector3  Force;
	Vector3  Torque;
	Vector3  PreviousPosition;
	Quaternion PreviousRotation;
} RigidBodyState;

// ---------------------------------------------------------------------------
//  0093: MassProperties — scalar mass + CoM
// ---------------------------------------------------------------------------
typedef struct {
	Real     Mass;
	Real     InverseMass;
	Vector3  CenterOfMass;
} MassProperties;

// ---------------------------------------------------------------------------
//  0094: InertiaTensor — local + world inverse inertia
// ---------------------------------------------------------------------------
typedef struct {
	Matrix3x3 InverseInertiaLocal;
	Matrix3x3 InverseInertiaWorld;
} InertiaTensor;

// ---------------------------------------------------------------------------
//  DampingConfig — linear + angular velocity damping
// ---------------------------------------------------------------------------
typedef struct {
	Real LinearDamping;
	Real AngularDamping;
} DampingConfig;

// ---------------------------------------------------------------------------
//  VelocityCap — max speeds
// ---------------------------------------------------------------------------
typedef struct {
	Real MaxLinearVelocity;
	Real MaxAngularVelocity;
} VelocityCap;

// ---------------------------------------------------------------------------
//  SleepConfig — sleep timer + thresholds
// ---------------------------------------------------------------------------
typedef struct {
	Real  LinearThreshold;
	Real  AngularThreshold;
	Real  DwellTime;
	Real  Timer;  // accumulated time below threshold
} SleepConfig;

// ---------------------------------------------------------------------------
//  GravityField — per-body gravity configuration
//  重力場の設定ね。
// ---------------------------------------------------------------------------
typedef struct {
	Vector3 Acceleration;   // m/s²  (e.g. {0, -9.81, 0})
	Real    DefaultScale;   // default gravity scale for new bodies
} GravityField;

// ---------------------------------------------------------------------------
//  0095–0096: MaterialConfig — surface properties with combine mode
// ---------------------------------------------------------------------------
typedef struct {
	Real           Restitution;
	Real           StaticFriction;
	Real           DynamicFriction;
	MaterialCombine CombineRestitution;
	MaterialCombine CombineFriction;
} MaterialConfig;

// ---------------------------------------------------------------------------
//  KinematicConfig — body type
// ---------------------------------------------------------------------------
typedef struct {
	BodyType Type;
	Real     GravityScale;
} KinematicConfig;

// ===========================================================================
//  Combined body creation and config defaults
// ===========================================================================

typedef struct {
	RigidBodyState  State;
	MassProperties  Mass;
	InertiaTensor   Inertia;
	DampingConfig   Damping;
	VelocityCap     VelCap;
	SleepConfig     Sleep;
	MaterialConfig  Material;
	KinematicConfig Kinematic;
	uint32_t        Flags;
	BodyID          ID;
} BodyConfig;

// ===========================================================================
//  Yui — production body state functions
// ===========================================================================

// ---- RigidBodyState ----
RigidBodyState YuiRigidBodyStateMake(const Vector3 *Position, const Quaternion *Rotation);
RigidBodyState YuiRigidBodyStateZero(void);
void           YuiRigidBodyStateClearForces(RigidBodyState *State);
void           YuiRigidBodyStateSavePrevious(RigidBodyState *State);

// ---- MassProperties ----
MassProperties YuiMassPropertiesMake(Real Mass, const Vector3 *CenterOfMass);
MassProperties YuiMassPropertiesZero(void);

// ---- InertiaTensor ----
InertiaTensor YuiInertiaTensorMake(const Matrix3x3 *InverseInertiaLocal);
InertiaTensor YuiInertiaTensorZero(void);
void          YuiInertiaTensorUpdateWorld(InertiaTensor *Tensor, const Quaternion *Rotation);

// Inertia computation helpers — fills InverseInertiaLocal from shape params
Matrix3x3 YuiInertiaComputeBox(Real Mass, const Vector3 *HalfExtents);
Matrix3x3 YuiInertiaComputeSphere(Real Mass, Real Radius);
Matrix3x3 YuiInertiaComputeCapsule(Real Mass, Real Radius, Real HalfHeight);

// ---- Damping ----
DampingConfig YuiDampingConfigMake(Real Linear, Real Angular);
DampingConfig YuiDampingConfigZero(void);
void          YuiDampingApply(DampingConfig *D, RigidBodyState *S, Real Dt);

// ---- VelocityCap ----
VelocityCap YuiVelocityCapMake(Real MaxLinear, Real MaxAngular);
VelocityCap YuiVelocityCapZero(void);
void        YuiVelocityCapApply(VelocityCap *Cap, RigidBodyState *S);

// ---- SleepConfig ----
SleepConfig YuiSleepConfigMake(Real LinearThreshold, Real AngularThreshold, Real DwellTime);
SleepConfig YuiSleepConfigZero(void);
int         YuiSleepUpdate(SleepConfig *Sleep, const RigidBodyState *State, Real Dt);

// ---- GravityField ----
GravityField YuiGravityFieldMake(Real X, Real Y, Real Z, Real DefaultScale);
GravityField YuiGravityFieldDefault(void);
void         YuiApplyGravity(const GravityField *Field, const KinematicConfig *Kin,
                             RigidBodyState *State, Real Dt);

// ---- MaterialConfig ----
MaterialConfig YuiMaterialConfigMake(
	Real Restitution, Real StaticFriction, Real DynamicFriction);
MaterialConfig YuiMaterialConfigDefault(void);
MaterialConfig YuiMaterialConfigZero(void);

// Combine two materials into one (for contact pair)
MaterialConfig YuiMaterialConfigCombine(
	const MaterialConfig *A, const MaterialConfig *B);

// ---- KinematicConfig ----
KinematicConfig YuiKinematicConfigMake(BodyType Type, Real GravityScale);
KinematicConfig YuiKinematicConfigDefault(void);
KinematicConfig YuiKinematicConfigZero(void);

// ---- BodyConfig ----
BodyConfig YuiBodyConfigDefault(void);
BodyConfig YuiBodyConfigMakeDynamic(
	const Vector3 *Position, const Quaternion *Rotation,
	Real Mass, const Vector3 *HalfExtents);
BodyConfig YuiBodyConfigMakeStatic(const Vector3 *Position);
BodyConfig YuiBodyConfigMakeKinematic(
	const Vector3 *Position, const Quaternion *Rotation);
