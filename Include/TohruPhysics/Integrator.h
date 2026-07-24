/**
 * ShioriIntegrator — rigid body numerical integration for TohruPhysics.
 * TohruPhysics用の剛体数値積分ね。
 *
 * Implements symplectic Euler (first-order) and Velocity Verlet (second-order)
 * integration schemes for linear and angular motion, with velocity clamping,
 * quaternion normalisation, gyroscopic force terms, CCD trajectory prediction,
 * and adaptive time-stepping.
 *
 * DESIGN PHILOSOPHY:
 * Symplectic Euler (semi-implicit Euler) is the default integrator because it
 * conserves energy better than explicit Euler for Hamiltonian systems. The
 * update order is: forces → velocity → position (for linear) and torque →
 * angular velocity → orientation (for angular). This gives first-order accuracy
 * with excellent stability for the typical 60 Hz physics timestep.
 *
 * Velocity Verlet (0237) trades per-frame cost for second-order accuracy in
 * systems with smooth force functions.
 *
 * INTEGRATION ORDER (symplectic Euler per frame):
 *   1. Accumulate forces (external) into RigidBodyState.Force/Torque
 *   2. IntegrateVelocities: v += (F * invMass + gravity) * dt
 *   3. ClampVelocity: limit |v| and |ω| to configured caps
 *   4. IntegratePositions: x += v * dt
 *   5. IntegrateRotationalState: ω += I_inv * τ * dt, q += 0.5*ω*dt*q, normalize
 *   6. NormalizeRotation: correct quaternion drift
 *   7. UpdateCenterOfMass: adjust position for COM offset
 *
 * References:
 *   - Hairer, Lubich, Wanner, "Geometric Numerical Integration" (2006)
 *   - David Eberly, "Rigid Body Simulation" (Game Physics, 2010)
 *   - Witkin & Baraff, "Physically Based Modeling" (SIGGRAPH 97)
 *   - Erin Catto, "Numerical Methods for Physics" (GDC 2013)
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <TohruPhysics/BodyState.h>
#include <TohruPhysics/Geometry.h>

// ===========================================================================
//  Constants
// ===========================================================================

#define SHIORI_DEFAULT_DT           (1.0 / 60.0)
#define SHIORI_DEFAULT_LINEAR_CAP   100.0   // m/s
#define SHIORI_DEFAULT_ANGULAR_CAP  50.0    // rad/s
#define SHIORI_DEFAULT_MIN_DT       1e-6
#define SHIORI_DEFAULT_MAX_DT       0.1
#define SHIORI_QUATERNION_EPS       1e-12

// ===========================================================================
//  Integration configuration
// ===========================================================================

typedef enum {
	SHIORI_SCHEME_SYMPLECTIC_EULER = 0,
	SHIORI_SCHEME_VELOCITY_VERLET  = 1,
	SHIORI_SCHEME_RK4              = 2
} ShioriIntegrationScheme;

typedef struct {
	Real LinearVelocityCap;
	Real AngularVelocityCap;
	Real MinTimeStep;
	Real MaxTimeStep;
	int  EnableGyroscopicForces;  // 1 = include ω × I·ω term
	int  EnableCCD;               // 1 = enable continuous collision detection
	ShioriIntegrationScheme Scheme;
	Real CCDThreshold;            // velocity above which CCD is used (m/s)
} ShioriIntegratorConfig;

// Default configuration.
static inline ShioriIntegratorConfig ShioriDefaultConfig(void) {
	ShioriIntegratorConfig C;
	C.LinearVelocityCap  = SHIORI_DEFAULT_LINEAR_CAP;
	C.AngularVelocityCap = SHIORI_DEFAULT_ANGULAR_CAP;
	C.MinTimeStep        = SHIORI_DEFAULT_MIN_DT;
	C.MaxTimeStep        = SHIORI_DEFAULT_MAX_DT;
	C.EnableGyroscopicForces = 0;
	C.EnableCCD              = 0;
	C.Scheme                 = SHIORI_SCHEME_SYMPLECTIC_EULER;
	C.CCDThreshold           = 20.0;
	return C;
}

// ===========================================================================
//  0231: Integrate velocities (apply accumulated forces)
// ===========================================================================

// Apply accumulated Force and Torque to update LinearVelocity and
// AngularVelocity using the current integration scheme.
//   v += (F * invMass + gravity) * dt
//   ω += I_inv * τ * dt  (optionally with gyroscopic correction)
//
// Returns 0 on success, -1 on invalid input.
int ShioriIntegrateVelocities(RigidBodyState *State,
                              const MassProperties *Mass,
                              const InertiaTensor *Inertia,
                              const Vector3 *Gravity,
                              const ShioriIntegratorConfig *Config,
                              Real Dt);

// ===========================================================================
//  0232: Integrate positions
// ===========================================================================

// Update Position from LinearVelocity.
//   x += v * dt
// Saves current position to PreviousPosition for interpolation/CCD.
//
// Returns 0 on success, -1 on invalid input.
int ShioriIntegratePositions(RigidBodyState *State,
                             Real Dt);

// ===========================================================================
//  0233: Velocity clamping
// ===========================================================================

// Clamp LinearVelocity and AngularVelocity magnitudes to configured caps.
// Returns number of components clamped (0 if none).
int ShioriClampVelocity(RigidBodyState *State,
                        const ShioriIntegratorConfig *Config);

// ===========================================================================
//  0234: Rotational integration
// ===========================================================================

// Integrate orientation from AngularVelocity and Torque.
//   ω += I_inv * τ * dt      (torque contribution)
//   Δq = 0.5 * ω_quat * q * dt
//   q += Δq
//   q = normalize(q)
//
// The angluar velocity is treated as a pure quaternion (0, ωx, ωy, ωz).
// Returns 0 on success, -1 on invalid input.
int ShioriIntegrateRotationalState(RigidBodyState *State,
                                   const MassProperties *Mass,
                                   const InertiaTensor *Inertia,
                                   const ShioriIntegratorConfig *Config,
                                   Real Dt);

// ===========================================================================
//  0235: Orientation verification
// ===========================================================================

// Normalize quaternion and check for NaN/Inf or excessive drift.
// If quaternion is degenerate (zero-length or NaN), reset to identity.
// Returns 0 if quaternion was valid, 1 if it was corrected, -1 on error.
int ShioriNormalizeRotation(RigidBodyState *State);

// ===========================================================================
//  0236: Center-of-mass position adjustment
// ===========================================================================

// Update the world-space position to account for center-of-mass offset
// relative to the body frame origin. For bodies where the COM is not at
// the origin, rotation changes shift the COM position.
//   PositionWorld = BodyOrigin + Rotation * CenterOfMassLocal
//
// Returns 0 on success, -1 on invalid input.
int ShioriUpdateCenterOfMass(RigidBodyState *State,
                             const MassProperties *Mass);

// ===========================================================================
//  0237: Second-order integration (Velocity Verlet)
// ===========================================================================

// Velocity Verlet full step:
//   1. x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt²
//   2. Store old acceleration
//   3. Compute new acceleration from forces at t+dt
//   4. v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt
//
// State contains the current force/torque at t (pre-step). After calling
// this, the caller should compute new forces for t+dt and call
// ShioriIntegrateVelocitiesVerletFinish.
//
// Returns 0 on success, -1 on error.
int ShioriIntegrateVerletStart(RigidBodyState *State,
                               const MassProperties *Mass,
                               const Vector3 *Gravity,
                               Real Dt);

// Finish the Verlet step: v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt.
// Caller must compute a(t+dt) from forces at the new position and pass
// it as NewAcceleration / NewAngularAcceleration.
void ShioriIntegrateVerletFinish(RigidBodyState *State,
                                 const Vector3 *OldAcceleration,
                                 const Vector3 *NewAcceleration,
                                 const Vector3 *OldAngularAcceleration,
                                 const Vector3 *NewAngularAcceleration,
                                 Real Dt);

// ===========================================================================
//  0238: CCD trajectory prediction
// ===========================================================================

// Predict the swept AABB of a body moving at its current velocity over dt.
// Returns a ray from the current position to the predicted position, and
// the expanded AABB that encloses both current and swept volumes.
//
// Returns 0 on success, -1 if velocity is below CCD threshold.
int ShioriPredictTrajectory(const RigidBodyState *State,
                            const AABB *BodyAABB,
                            const ShioriIntegratorConfig *Config,
                            Real Dt,
                            Ray *OutRay,
                            AABB *OutSweptAABB);

// ===========================================================================
//  0239: Angular momentum integration (gyroscopic forces)
// ===========================================================================

// Compute and apply the gyroscopic torque term: -ω × (I · ω).
// This torque arises from the rotating reference frame of the body and
// causes precession/nutation. For most games this term is small enough
// to ignore (Config.EnableGyroscopicForces = 0).
//
// Returns the gyroscopic torque magnitude (0 if disabled).
Real ShioriApplyGyroscopicForces(RigidBodyState *State,
                                 const InertiaTensor *Inertia,
                                 Real Dt);

// ===========================================================================
//  0240: Adaptive time-step computation
// ===========================================================================

// Compute a safe time-step based on current velocity state and body size.
// Uses a CFL-like condition: dt = cellSize / (maxVelocity * safetyFactor).
//
// Returns the computed dt, clamped to [MinTimeStep, MaxTimeStep].
Real ShioriComputeTimeStep(const RigidBodyState *State,
                           Real BodyRadius,
                           const ShioriIntegratorConfig *Config);

// ===========================================================================
//  Convenience: full single-body step
// ===========================================================================

// Run one complete integration step (symplectic Euler) for a single body:
//   velocities → clamp → positions → rotation → normalize → COM
//
// Returns 0 on success, -1 on error.
int ShioriStepBody(RigidBodyState *State,
                   const MassProperties *Mass,
                   const InertiaTensor *Inertia,
                   const Vector3 *Gravity,
                   const ShioriIntegratorConfig *Config,
                   Real Dt,
                   const AABB *BodyAABB,   // for CCD (may be NULL)
                   Ray *CCDRayOut,          // for CCD result (may be NULL)
                   AABB *CCDSweptOut);      // for CCD result (may be NULL)
