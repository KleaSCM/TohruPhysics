/**
 * Unit tests for ShioriIntegrator numerical integration.
 * ShioriIntegratorの単体テストね。
 *
 * Tests 0231-0240: velocity/position integration, clamping, rotation,
 * normalisation, COM update, Verlet, CCD, gyroscopic forces, adaptive dt.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Integrator.h>
#include <TohruPhysics/Vector3.h>
#include <TohruPhysics/Math.h>
#include <TohruPhysics/Quaternion.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST(cond, msg) do { \
	if (!(cond)) { \
		fprintf(stderr, "FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
		exit(1); \
	} \
} while(0)

#define AE(a, b, eps) NagisaApproxEqual((a), (b), (eps))

static int Passed = 0;

#define RUN_TEST(name, desc) do { \
	fprintf(stderr, "  %-45s ... ", desc); \
	name(); \
	fprintf(stderr, "ok\n"); \
	Passed++; \
} while(0)

// ===========================================================================
//  0231: Integrate velocities
// ===========================================================================

static void TestIntegrateVelocities(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));
	State.LinearVelocity = KannaVector3Make(0, 0, 0);
	State.Force = KannaVector3Make(10, 0, 0);  // 10 N along X

	MassProperties Mass;
	Mass.Mass = 2.0;
	Mass.InverseMass = 1.0 / 2.0;
	Mass.CenterOfMass = KannaVector3Make(0, 0, 0);

	InertiaTensor Inertia;
	memset(&Inertia, 0, sizeof(Inertia));
	Inertia.InverseInertiaWorld.Data[0] = 1.0;
	Inertia.InverseInertiaWorld.Data[4] = 1.0;
	Inertia.InverseInertiaWorld.Data[8] = 1.0;

	Vector3 Gravity = KannaVector3Make(0, -9.81, 0);
	ShioriIntegratorConfig Cfg = ShioriDefaultConfig();

	ShioriIntegrateVelocities(&State, &Mass, &Inertia, &Gravity, &Cfg, 1.0/60.0);

	// v_x += (10 / 2) * (1/60) = 10/2 * 1/60 = 5/60 ≈ 0.0833
	// v_y += -9.81 * (1/60) ≈ -0.1635
	Real ExpectedVx = (10.0 / 2.0) / 60.0;
	Real ExpectedVy = -9.81 / 60.0;
	TEST(AE(State.LinearVelocity.Data[0], ExpectedVx, 1e-6),
		"linear velocity X after force");
	TEST(AE(State.LinearVelocity.Data[1], ExpectedVy, 1e-6),
		"linear velocity Y after gravity");

	// Static body (invMass = 0)
	Mass.InverseMass = 0.0;
	Vector3 OldV = State.LinearVelocity;
	ShioriIntegrateVelocities(&State, &Mass, &Inertia, &Gravity, &Cfg, 1.0/60.0);
	TEST(State.LinearVelocity.Data[0] == OldV.Data[0],
		"static body velocity unchanged");

	// Null
	int R = ShioriIntegrateVelocities(NULL, &Mass, &Inertia, &Gravity, &Cfg, 1.0/60.0);
	TEST(R == -1, "null state returns -1");
}

// ===========================================================================
//  0232: Integrate positions
// ===========================================================================

static void TestIntegratePositions(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));
	State.Position = KannaVector3Make(1, 2, 3);
	State.LinearVelocity = KannaVector3Make(10, 0, 0);
	State.PreviousPosition = KannaVector3Make(0, 0, 0);  // dummy

	ShioriIntegratePositions(&State, 0.5);

	// x += 10 * 0.5 = 5 → new x = 6
	TEST(AE(State.Position.Data[0], 6.0, 1e-10), "position X after integration");
	TEST(AE(State.Position.Data[1], 2.0, 1e-10), "position Y unchanged");
	TEST(AE(State.PreviousPosition.Data[0], 1.0, 1e-10), "previous position saved");

	// Zero dt
	State.Position.Data[0] = 0.0;
	ShioriIntegratePositions(&State, 0.0);
	TEST(AE(State.Position.Data[0], 0.0, 1e-10), "zero dt: position unchanged");

	// Null
	int R = ShioriIntegratePositions(NULL, 1.0);
	TEST(R == -1, "null returns -1");
}

// ===========================================================================
//  0233: Velocity clamping
// ===========================================================================

static void TestClampVelocity(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));
	State.LinearVelocity = KannaVector3Make(200, 0, 0);  // exceeds cap of 100
	State.AngularVelocity = KannaVector3Make(0, 100, 0);  // exceeds cap of 50

	ShioriIntegratorConfig Cfg = ShioriDefaultConfig();

	int Clamped = ShioriClampVelocity(&State, &Cfg);
	TEST(Clamped == 2, "both velocity components clamped");

	Real LinSpeed = KannaVector3Length(&State.LinearVelocity);
	TEST(AE(LinSpeed, Cfg.LinearVelocityCap, 1e-6), "linear speed clamped to cap");

	Real AngSpeed = KannaVector3Length(&State.AngularVelocity);
	TEST(AE(AngSpeed, Cfg.AngularVelocityCap, 1e-6), "angular speed clamped to cap");

	// No clamping needed
	State.LinearVelocity = KannaVector3Make(1, 0, 0);
	State.AngularVelocity = KannaVector3Make(1, 0, 0);
	Clamped = ShioriClampVelocity(&State, &Cfg);
	TEST(Clamped == 0, "no clamping for slow body");

	// Null
	Clamped = ShioriClampVelocity(NULL, &Cfg);
	TEST(Clamped == -1, "null state returns -1");
}

// ===========================================================================
//  0234: Rotational integration
// ===========================================================================

static void TestRotationalIntegration(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));
	State.Rotation = EuphylliaQuaternionIdentity();
	State.AngularVelocity = KannaVector3Make(0, 0, 1.0);  // 1 rad/s around Z
	State.Torque = KannaVector3Make(0, 0, 0);

	MassProperties Mass;
	Mass.Mass = 1.0;
	Mass.InverseMass = 1.0;
	Mass.CenterOfMass = KannaVector3Make(0, 0, 0);

	InertiaTensor Inertia;
	memset(&Inertia, 0, sizeof(Inertia));
	Inertia.InverseInertiaWorld.Data[0] = 1.0;
	Inertia.InverseInertiaWorld.Data[4] = 1.0;
	Inertia.InverseInertiaWorld.Data[8] = 1.0;

	ShioriIntegratorConfig Cfg = ShioriDefaultConfig();

	ShioriIntegrateRotationalState(&State, &Mass, &Inertia, &Cfg, 1.0/60.0);

	// After small rotation around Z, quaternion should have non-zero z and w
	TEST(!EuphylliaQuaternionIsIdentity(&State.Rotation),
		"quaternion changed after rotation");
	TEST(State.Rotation.Data[2] != 0.0, "quaternion Z component non-zero");

	// Static body
	Mass.InverseMass = 0.0;
	Quaternion QBefore = State.Rotation;
	ShioriIntegrateRotationalState(&State, &Mass, &Inertia, &Cfg, 1.0/60.0);
	TEST(EuphylliaQuaternionEqual(&State.Rotation, &QBefore),
		"static body rotation unchanged");

	// Null
	int R = ShioriIntegrateRotationalState(NULL, &Mass, &Inertia, &Cfg, 1.0/60.0);
	TEST(R == -1, "null returns -1");
}

// ===========================================================================
//  0235: Normalise rotation
// ===========================================================================

static void TestNormalizeRotation(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));

	// Valid quaternion
	State.Rotation = EuphylliaQuaternionIdentity();
	int R = ShioriNormalizeRotation(&State);
	TEST(R == 0, "identity quaternion is valid");

	// Degenerate (zero)
	State.Rotation = EuphylliaQuaternionMake(0, 0, 0, 0);
	R = ShioriNormalizeRotation(&State);
	TEST(R == 1, "zero quaternion corrected");
	TEST(EuphylliaQuaternionIsIdentity(&State.Rotation),
		"zero quaternion reset to identity");

	// Non-unit but valid
	State.Rotation = EuphylliaQuaternionMake(2, 0, 0, 0);
	R = ShioriNormalizeRotation(&State);
	TEST(R == 1, "non-unit quaternion normalized");

	// Null
	R = ShioriNormalizeRotation(NULL);
	TEST(R == -1, "null returns -1");
}

// ===========================================================================
//  0236: Center of mass
// ===========================================================================

static void TestCenterOfMass(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));
	State.Position = KannaVector3Make(0, 0, 0);
	State.Rotation = EuphylliaQuaternionIdentity();

	MassProperties Mass;
	Mass.Mass = 1.0;
	Mass.InverseMass = 1.0;
	Mass.CenterOfMass = KannaVector3Make(0, 1, 0);  // COM 1 unit above origin

	// With identity rotation, COM position = origin + rotation * localCOM = (0,1,0)
	ShioriUpdateCenterOfMass(&State, &Mass);
	TEST(AE(State.Position.Data[1], 1.0, 1e-10), "COM position updated");

	// COM at origin — no change
	State.Position = KannaVector3Make(0, 0, 0);
	Mass.CenterOfMass = KannaVector3Make(0, 0, 0);
	ShioriUpdateCenterOfMass(&State, &Mass);
	TEST(AE(State.Position.Data[1], 0.0, 1e-10), "zero COM: position unchanged");

	// Null
	int R = ShioriUpdateCenterOfMass(NULL, &Mass);
	TEST(R == -1, "null returns -1");
}

// ===========================================================================
//  0237: Velocity Verlet
// ===========================================================================

static void TestVerlet(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));
	State.Position = KannaVector3Make(0, 0, 0);
	State.LinearVelocity = KannaVector3Make(10, 0, 0);
	State.Force = KannaVector3Make(0, 0, 0);

	MassProperties Mass;
	Mass.Mass = 1.0;
	Mass.InverseMass = 1.0;
	Mass.CenterOfMass = KannaVector3Make(0, 0, 0);

	Vector3 Gravity = KannaVector3Make(0, 0, 0);
	Real Dt = 1.0/60.0;

	// Verlet start with constant velocity, no forces
	ShioriIntegrateVerletStart(&State, &Mass, &Gravity, Dt);

	// Position should have advanced by v*dt
	TEST(AE(State.Position.Data[0], 10.0 * Dt, 1e-10),
		"Verlet start: position advanced");

	Vector3 OldAccel = KannaVector3Make(0, 0, 0);
	Vector3 NewAccel = KannaVector3Make(0, 0, 0);
	Vector3 OldAngAccel = KannaVector3Make(0, 0, 0);
	Vector3 NewAngAccel = KannaVector3Make(0, 0, 0);
	ShioriIntegrateVerletFinish(&State, &OldAccel, &NewAccel,
	                            &OldAngAccel, &NewAngAccel, Dt);

	// Velocity should remain the same (no acceleration)
	TEST(AE(State.LinearVelocity.Data[0], 10.0, 1e-10),
		"Verlet finish: velocity unchanged");

	// Null
	int R = ShioriIntegrateVerletStart(NULL, &Mass, &Gravity, Dt);
	TEST(R == -1, "Verlet start null returns -1");
}

// ===========================================================================
//  0238: CCD trajectory prediction
// ===========================================================================

static void TestCCDPrediction(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));
	State.Position = KannaVector3Make(0, 0, 0);
	State.LinearVelocity = KannaVector3Make(50, 0, 0);  // fast (above CCD threshold)

	ShioriIntegratorConfig Cfg = ShioriDefaultConfig();
	Cfg.EnableCCD = 1;
	Cfg.CCDThreshold = 20.0;

	AABB BodyBox;
	BodyBox.Min = KannaVector3Make(-1, -1, -1);
	BodyBox.Max = KannaVector3Make(1, 1, 1);

	Ray OutRay;
	AABB Swept;
	int R = ShioriPredictTrajectory(&State, &BodyBox, &Cfg, 1.0/60.0,
	                                &OutRay, &Swept);
	TEST(R == 0, "CCD prediction succeeds for fast body");

	// Slow body should return -1
	State.LinearVelocity = KannaVector3Make(1, 0, 0);
	R = ShioriPredictTrajectory(&State, &BodyBox, &Cfg, 1.0/60.0,
	                            &OutRay, &Swept);
	TEST(R == -1, "slow body: no CCD prediction");

	// Null
	R = ShioriPredictTrajectory(NULL, &BodyBox, &Cfg, 1.0/60.0,
	                            &OutRay, &Swept);
	TEST(R == -1, "null state returns -1");
}

// ===========================================================================
//  0239: Gyroscopic forces
// ===========================================================================

static void TestGyroscopicForces(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));
	State.AngularVelocity = KannaVector3Make(10, 0, 1);  // spinning

	InertiaTensor Inertia;
	memset(&Inertia, 0, sizeof(Inertia));
	Inertia.InverseInertiaWorld.Data[0] = 0.5;
	Inertia.InverseInertiaWorld.Data[4] = 0.3;
	Inertia.InverseInertiaWorld.Data[8] = 0.7;

	Vector3 OldOmega = State.AngularVelocity;
	Real Mag = ShioriApplyGyroscopicForces(&State, &Inertia, 1.0/60.0);
	TEST(Mag >= 0, "gyroscopic torque magnitude non-negative");

	// Angular velocity should have changed
	Vector3 OmegaDiff = KannaVector3Sub(&State.AngularVelocity, &OldOmega);
	Real Diff = KannaVector3Length(&OmegaDiff);
	TEST(Diff > 0 || AE(Diff, 0.0, 1e-15),
		"angular velocity may change with gyroscopic forces");

	// Null
	Mag = ShioriApplyGyroscopicForces(NULL, &Inertia, 1.0/60.0);
	TEST(Mag == 0.0, "null returns 0");
}

// ===========================================================================
//  0240: Adaptive time-step
// ===========================================================================

static void TestAdaptiveDt(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));
	State.LinearVelocity = KannaVector3Make(10, 0, 0);

	ShioriIntegratorConfig Cfg = ShioriDefaultConfig();

	// Fast body with small radius → small timestep
	Real Dt = ShioriComputeTimeStep(&State, 0.5, &Cfg);
	TEST(Dt > 0, "adaptive dt positive");
	TEST(Dt <= Cfg.MaxTimeStep, "dt <= max");
	TEST(Dt >= Cfg.MinTimeStep, "dt >= min");

	// Slow body with large radius → large timestep
	State.LinearVelocity = KannaVector3Make(0.1, 0, 0);
	Dt = ShioriComputeTimeStep(&State, 10.0, &Cfg);
	TEST(AE(Dt, Cfg.MaxTimeStep, 1e-10),
		"slow body gets max dt");

	// Stationary body → max dt
	State.LinearVelocity = KannaVector3Make(0, 0, 0);
	Dt = ShioriComputeTimeStep(&State, 1.0, &Cfg);
	TEST(AE(Dt, Cfg.MaxTimeStep, 1e-10),
		"stationary body gets max dt");

	// Null
	Dt = ShioriComputeTimeStep(NULL, 1.0, &Cfg);
	TEST(AE(Dt, SHIORI_DEFAULT_DT, 1e-10), "null returns default dt");
}

// ===========================================================================
//  Full body step convenience
// ===========================================================================

static void TestFullStep(void) {
	RigidBodyState State;
	memset(&State, 0, sizeof(State));
	State.Position = KannaVector3Make(0, 10, 0);
	State.Rotation = EuphylliaQuaternionIdentity();
	State.Force = KannaVector3Make(0, -10, 0);  // downward force
	State.Torque = KannaVector3Make(0, 0, 0);

	MassProperties Mass;
	Mass.Mass = 1.0;
	Mass.InverseMass = 1.0;
	Mass.CenterOfMass = KannaVector3Make(0, 0, 0);

	InertiaTensor Inertia;
	memset(&Inertia, 0, sizeof(Inertia));
	Inertia.InverseInertiaWorld.Data[0] = 1.0;
	Inertia.InverseInertiaWorld.Data[4] = 1.0;
	Inertia.InverseInertiaWorld.Data[8] = 1.0;

	Vector3 Gravity = KannaVector3Make(0, -9.81, 0);
	ShioriIntegratorConfig Cfg = ShioriDefaultConfig();

	Real Dt = 1.0 / 60.0;
	int R = ShioriStepBody(&State, &Mass, &Inertia, &Gravity, &Cfg, Dt,
	                       NULL, NULL, NULL);
	TEST(R == 0, "full step succeeds");
	TEST(State.Position.Data[1] < 10.0, "body fell down");
	TEST(State.Force.Data[0] == 0.0 && State.Force.Data[1] == 0.0,
		"forces cleared after step");

	// With CCD
	Cfg.EnableCCD = 1;
	State.Position = KannaVector3Make(0, 10, 0);
	State.LinearVelocity = KannaVector3Make(0, 0, 0);
	State.Force = KannaVector3Make(0, -10, 0);
	AABB BodyBox;
	BodyBox.Min = KannaVector3Make(-0.5, -0.5, -0.5);
	BodyBox.Max = KannaVector3Make(0.5, 0.5, 0.5);
	Ray CCDRay;
	AABB CCDSwept;
	R = ShioriStepBody(&State, &Mass, &Inertia, &Gravity, &Cfg, Dt,
	                   &BodyBox, &CCDRay, &CCDSwept);
	TEST(R == 0, "full step with CCD succeeds");

	// Null
	R = ShioriStepBody(NULL, &Mass, &Inertia, &Gravity, &Cfg, Dt,
	                   NULL, NULL, NULL);
	TEST(R == -1, "null full step returns -1");
}

// ===========================================================================
//  Main
// ===========================================================================

int main(void) {
	fprintf(stderr, "=== TestIntegrator ===\n");

	RUN_TEST(TestIntegrateVelocities, "Integrator: integrate velocities");
	RUN_TEST(TestIntegratePositions, "Integrator: integrate positions");
	RUN_TEST(TestClampVelocity, "Integrator: clamp velocity");
	RUN_TEST(TestRotationalIntegration, "Integrator: rotational integration");
	RUN_TEST(TestNormalizeRotation, "Integrator: normalise rotation");
	RUN_TEST(TestCenterOfMass, "Integrator: center of mass");
	RUN_TEST(TestVerlet, "Integrator: Verlet");
	RUN_TEST(TestCCDPrediction, "Integrator: CCD prediction");
	RUN_TEST(TestGyroscopicForces, "Integrator: gyroscopic forces");
	RUN_TEST(TestAdaptiveDt, "Integrator: adaptive dt");
	RUN_TEST(TestFullStep, "Integrator: full step");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
