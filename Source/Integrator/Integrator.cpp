/**
 * ShioriIntegrator — rigid body numerical integration implementation.
 * TohruPhysics用の剛体数値積分実装ね。
 *
 * Implements 0231-0240: velocity/position/rotational integration,
 * velocity clamping, quaternion normalisation, COM tracking, Verlet,
 * CCD prediction, gyroscopic forces, and adaptive timestep.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Integrator.h>
#include <TohruPhysics/Math.h>
#include <TohruPhysics/Quaternion.h>
#include <string.h>

// ===========================================================================
//  0231: Integrate velocities
// ===========================================================================

int ShioriIntegrateVelocities(RigidBodyState *State,
                              const MassProperties *Mass,
                              const InertiaTensor *Inertia,
                              const Vector3 *Gravity,
                              const ShioriIntegratorConfig *Config,
                              Real Dt) {
	if (!State || !Mass || !Inertia || !Config) return -1;
	if (Dt <= 0.0) return 0;

	Real InvMass = Mass->InverseMass;
	if (InvMass <= 0.0) return 0;  // static body

	// Linear: v += (F * invMass + gravity) * dt
	Vector3 Accel = KannaVector3Scale(&State->Force, InvMass);
	Accel = KannaVector3Add(&Accel, Gravity);
	Vector3 VelDelta = KannaVector3Scale(&Accel, Dt);
	State->LinearVelocity = KannaVector3Add(
		&State->LinearVelocity, &VelDelta);

	// Angular: ω += I_inv * (τ + gyroscopic) * dt
	// Apply gyroscopic term if enabled
	Vector3 TotalTorque = State->Torque;
	if (Config->EnableGyroscopicForces) {
		Real GMag = ShioriApplyGyroscopicForces(State, Inertia, Dt);
		(void)GMag;
	}

	// Compute angular acceleration: α = I_inv_world * τ
	Vector3 AngAccel = MiorineMatrix3x3VecMul(
		&Inertia->InverseInertiaWorld, &TotalTorque);
	Vector3 AngDelta = KannaVector3Scale(&AngAccel, Dt);
	State->AngularVelocity = KannaVector3Add(
		&State->AngularVelocity, &AngDelta);

	return 0;
}

// ===========================================================================
//  0232: Integrate positions
// ===========================================================================

int ShioriIntegratePositions(RigidBodyState *State, Real Dt) {
	if (!State) return -1;
	if (Dt <= 0.0) return 0;

	// Save previous position
	State->PreviousPosition = State->Position;
	State->PreviousRotation = State->Rotation;

	// x += v * dt
	Vector3 PosDelta = KannaVector3Scale(&State->LinearVelocity, Dt);
	State->Position = KannaVector3Add(
		&State->Position, &PosDelta);

	return 0;
}

// ===========================================================================
//  0233: Velocity clamping
// ===========================================================================

int ShioriClampVelocity(RigidBodyState *State,
                        const ShioriIntegratorConfig *Config) {
	if (!State || !Config) return -1;

	int Clamped = 0;

	Real LinSpeed = KannaVector3Length(&State->LinearVelocity);
	if (LinSpeed > Config->LinearVelocityCap && LinSpeed > 0.0) {
		Real Scale = Config->LinearVelocityCap / LinSpeed;
		State->LinearVelocity = KannaVector3Scale(&State->LinearVelocity, Scale);
		Clamped++;
	}

	Real AngSpeed = KannaVector3Length(&State->AngularVelocity);
	if (AngSpeed > Config->AngularVelocityCap && AngSpeed > 0.0) {
		Real Scale = Config->AngularVelocityCap / AngSpeed;
		State->AngularVelocity = KannaVector3Scale(&State->AngularVelocity, Scale);
		Clamped++;
	}

	return Clamped;
}

// ===========================================================================
//  0234: Rotational integration
// ===========================================================================

int ShioriIntegrateRotationalState(RigidBodyState *State,
                                   const MassProperties *Mass,
                                   const InertiaTensor *Inertia,
                                   const ShioriIntegratorConfig *Config,
                                   Real Dt) {
	if (!State || !Mass || !Inertia || !Config) return -1;
	if (Dt <= 0.0) return 0;

	Real InvMass = Mass->InverseMass;
	if (InvMass <= 0.0) return 0;  // static

	// Angular acceleration from torque (already partially applied in 0231,
	// but apply remaining torque-based acceleration here)
	Vector3 AngAccel = MiorineMatrix3x3VecMul(
		&Inertia->InverseInertiaWorld, &State->Torque);
	Vector3 AngDelta = KannaVector3Scale(&AngAccel, Dt);
	State->AngularVelocity = KannaVector3Add(
		&State->AngularVelocity, &AngDelta);

	// Build pure quaternion from angular velocity: Qω = (0, ωx, ωy, ωz)
	Quaternion OmegaQ = EuphylliaQuaternionMake(
		State->AngularVelocity.Data[0],
		State->AngularVelocity.Data[1],
		State->AngularVelocity.Data[2],
		0.0);

	// dq/dt = 0.5 * Qω * q
	Quaternion DQ = EuphylliaQuaternionMul(&OmegaQ, &State->Rotation);
	DQ.Data[0] *= 0.5 * Dt;
	DQ.Data[1] *= 0.5 * Dt;
	DQ.Data[2] *= 0.5 * Dt;
	DQ.Data[3] *= 0.5 * Dt;

	// q_new = q + dq
	Quaternion NewQ;
	NewQ.Data[0] = State->Rotation.Data[0] + DQ.Data[0];
	NewQ.Data[1] = State->Rotation.Data[1] + DQ.Data[1];
	NewQ.Data[2] = State->Rotation.Data[2] + DQ.Data[2];
	NewQ.Data[3] = State->Rotation.Data[3] + DQ.Data[3];

	// Normalize
	State->Rotation = EuphylliaQuaternionNormalize(&NewQ);

	return 0;
}

// ===========================================================================
//  0235: Orientation verification
// ===========================================================================

int ShioriNormalizeRotation(RigidBodyState *State) {
	if (!State) return -1;

	Real LenSq = EuphylliaQuaternionLengthSq(&State->Rotation);
	int Corrected = 0;

	// Check for NaN/Inf
	int HasNaN = 0;
	for (int I = 0; I < 4; I++) {
		if (MaiIsNaN(State->Rotation.Data[I]) ||
		    MaiIsInf(State->Rotation.Data[I])) {
			HasNaN = 1;
			break;
		}
	}

	if (HasNaN || LenSq < SHIORI_QUATERNION_EPS) {
		State->Rotation = EuphylliaQuaternionIdentity();
		Corrected = 1;
	} else if (NagisaApproxEqual(LenSq, 1.0, 1e-6)) {
		// Close enough — skip explicit normalize
	} else {
		State->Rotation = EuphylliaQuaternionNormalize(&State->Rotation);
		Corrected = 1;
	}

	return Corrected;
}

// ===========================================================================
//  0236: Center-of-mass position adjustment
// ===========================================================================

int ShioriUpdateCenterOfMass(RigidBodyState *State,
                             const MassProperties *Mass) {
	if (!State || !Mass) return -1;

	// If COM is at origin, no adjustment needed
	Real ComLen = KannaVector3Length(&Mass->CenterOfMass);
	if (ComLen < 1e-10) return 0;

	// Position = BodyOrigin + Rotation * CenterOfMassLocal
	Vector3 WorldCOM = EuphylliaQuaternionRotateVector(
		&State->Rotation, &Mass->CenterOfMass);
	State->Position = KannaVector3Add(&State->Position, &WorldCOM);

	return 0;
}

// ===========================================================================
//  0237: Velocity Verlet
// ===========================================================================

int ShioriIntegrateVerletStart(RigidBodyState *State,
                               const MassProperties *Mass,
                               const Vector3 *Gravity,
                               Real Dt) {
	if (!State || !Mass || !Gravity) return -1;
	if (Dt <= 0.0) return 0;

	Real InvMass = Mass->InverseMass;
	if (InvMass <= 0.0) return 0;

	// Store current acceleration for the Verlet finish step
	// (caller must pass these to Finish)

	// x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt²
	Vector3 Accel = KannaVector3Scale(&State->Force, InvMass);
	Accel = KannaVector3Add(&Accel, Gravity);

	Vector3 HalfAccelDt2 = KannaVector3Scale(&Accel, 0.5 * Dt * Dt);
	Vector3 VelDt = KannaVector3Scale(&State->LinearVelocity, Dt);
	State->PreviousPosition = State->Position;
	State->Position = KannaVector3Add(&State->Position, &VelDt);
	State->Position = KannaVector3Add(&State->Position, &HalfAccelDt2);

	return 0;
}

void ShioriIntegrateVerletFinish(RigidBodyState *State,
                                 const Vector3 *OldAcceleration,
                                 const Vector3 *NewAcceleration,
                                 const Vector3 *OldAngularAcceleration,
                                 const Vector3 *NewAngularAcceleration,
                                 Real Dt) {
	if (!State || !OldAcceleration || !NewAcceleration ||
	    !OldAngularAcceleration || !NewAngularAcceleration) return;
	if (Dt <= 0.0) return;

	// v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt
	Vector3 AvgAccel = KannaVector3Add(OldAcceleration, NewAcceleration);
	AvgAccel = KannaVector3Scale(&AvgAccel, 0.5);
	Vector3 VelDelta = KannaVector3Scale(&AvgAccel, Dt);
	State->LinearVelocity = KannaVector3Add(
		&State->LinearVelocity, &VelDelta);

	// ω(t+dt) = ω(t) + 0.5*(α(t) + α(t+dt))*dt
	Vector3 AvgAngAccel = KannaVector3Add(OldAngularAcceleration, NewAngularAcceleration);
	AvgAngAccel = KannaVector3Scale(&AvgAngAccel, 0.5);
	Vector3 AngDelta = KannaVector3Scale(&AvgAngAccel, Dt);
	State->AngularVelocity = KannaVector3Add(
		&State->AngularVelocity, &AngDelta);
}

// ===========================================================================
//  0238: CCD trajectory prediction
// ===========================================================================

int ShioriPredictTrajectory(const RigidBodyState *State,
                            const AABB *BodyAABB,
                            const ShioriIntegratorConfig *Config,
                            Real Dt,
                            Ray *OutRay,
                            AABB *OutSweptAABB) {
	if (!State || !BodyAABB || !Config || !OutRay || !OutSweptAABB)
		return -1;

	Real Speed = KannaVector3Length(&State->LinearVelocity);
	if (Speed < Config->CCDThreshold) return -1;

	// Ray from current position to predicted position
	Vector3 Direction = State->LinearVelocity;
	Real DirLen = KannaVector3Length(&Direction);
	if (DirLen < 1e-10) return -1;

	Direction = KannaVector3Scale(&Direction, 1.0 / DirLen);
	*OutRay = SabinaRayMake(&State->Position, &Direction);

	// Swept AABB = AABB expanded to include both current and target positions
	Vector3 VelDt = KannaVector3Scale(&State->LinearVelocity, Dt);
	Vector3 TargetPos = KannaVector3Add(
		&State->Position, &VelDt);

	AABB CurBox = *BodyAABB;
	AABB TargetBox = CurBox;
	Vector3 Offset = KannaVector3Sub(&TargetPos, &State->Position);
	TargetBox.Min = KannaVector3Add(&TargetBox.Min, &Offset);
	TargetBox.Max = KannaVector3Add(&TargetBox.Max, &Offset);

	// Merge both AABBs
	OutSweptAABB->Min.Data[0] = YuuMin(CurBox.Min.Data[0], TargetBox.Min.Data[0]);
	OutSweptAABB->Min.Data[1] = YuuMin(CurBox.Min.Data[1], TargetBox.Min.Data[1]);
	OutSweptAABB->Min.Data[2] = YuuMin(CurBox.Min.Data[2], TargetBox.Min.Data[2]);
	OutSweptAABB->Max.Data[0] = YuuMax(CurBox.Max.Data[0], TargetBox.Max.Data[0]);
	OutSweptAABB->Max.Data[1] = YuuMax(CurBox.Max.Data[1], TargetBox.Max.Data[1]);
	OutSweptAABB->Max.Data[2] = YuuMax(CurBox.Max.Data[2], TargetBox.Max.Data[2]);

	return 0;
}

// ===========================================================================
//  0239: Gyroscopic forces
// ===========================================================================

Real ShioriApplyGyroscopicForces(RigidBodyState *State,
                                 const InertiaTensor *Inertia,
                                 Real Dt) {
	if (!State || !Inertia) return 0.0;
	if (Dt <= 0.0) return 0.0;

	// I · ω: invert the world-space inverse inertia tensor to get the
	// world-space inertia tensor, then multiply by angular velocity.
	// I_world = (I_inv_world)^{-1}
	Matrix3x3 IWorld = MiorineMatrix3x3Inverse(&Inertia->InverseInertiaWorld);
	Vector3 IWorldOmega = MiorineMatrix3x3VecMul(&IWorld, &State->AngularVelocity);

	// Gyroscopic torque: τ_gyro = -ω × (I · ω)
	Vector3 GyroTorque = KannaVector3Cross(&State->AngularVelocity, &IWorldOmega);
	GyroTorque = KannaVector3Scale(&GyroTorque, -1.0);

	// Apply as acceleration on angular velocity
	Vector3 GyroAccel = MiorineMatrix3x3VecMul(
		&Inertia->InverseInertiaWorld, &GyroTorque);
	Vector3 GyroDelta = KannaVector3Scale(&GyroAccel, Dt);
	State->AngularVelocity = KannaVector3Add(
		&State->AngularVelocity, &GyroDelta);

	return KannaVector3Length(&GyroTorque);
}

// ===========================================================================
//  0240: Adaptive time-step
// ===========================================================================

Real ShioriComputeTimeStep(const RigidBodyState *State,
                           Real BodyRadius,
                           const ShioriIntegratorConfig *Config) {
	if (!State || !Config) return SHIORI_DEFAULT_DT;

	Real Speed = KannaVector3Length(&State->LinearVelocity);
	if (Speed < 1e-10 || BodyRadius < 1e-10)
		return Config->MaxTimeStep;

	// CFL-like: dt = bodyRadius / (speed * safetyFactor)
	// With safety factor of ~2, a body moves at most half its radius per step
	const Real Safety = 2.0;
	Real Dt = BodyRadius / (Speed * Safety);

	// Clamp
	if (Dt < Config->MinTimeStep) Dt = Config->MinTimeStep;
	if (Dt > Config->MaxTimeStep) Dt = Config->MaxTimeStep;

	return Dt;
}

// ===========================================================================
//  Convenience: full body step
// ===========================================================================

int ShioriStepBody(RigidBodyState *State,
                   const MassProperties *Mass,
                   const InertiaTensor *Inertia,
                   const Vector3 *Gravity,
                   const ShioriIntegratorConfig *Config,
                   Real Dt,
                   const AABB *BodyAABB,
                   Ray *CCDRayOut,
                   AABB *CCDSweptOut) {
	if (!State || !Mass || !Inertia || !Gravity || !Config) return -1;

	int R;

	// 1. Apply forces → update velocities
	R = ShioriIntegrateVelocities(State, Mass, Inertia, Gravity, Config, Dt);
	if (R != 0) return R;

	// 2. Clamp velocities
	ShioriClampVelocity(State, Config);

	// 3. Update positions
	R = ShioriIntegratePositions(State, Dt);
	if (R != 0) return R;

	// 4. Integrate rotation
	R = ShioriIntegrateRotationalState(State, Mass, Inertia, Config, Dt);
	if (R != 0) return R;

	// 5. Normalize rotation
	ShioriNormalizeRotation(State);

	// 6. Update COM
	ShioriUpdateCenterOfMass(State, Mass);

	// 7. CCD prediction (optional)
	if (Config->EnableCCD && BodyAABB && CCDRayOut && CCDSweptOut) {
		ShioriPredictTrajectory(State, BodyAABB, Config, Dt,
		                        CCDRayOut, CCDSweptOut);
	}

	// Clear forces for next frame
	State->Force = KannaVector3Make(0, 0, 0);
	State->Torque = KannaVector3Make(0, 0, 0);

	return 0;
}
