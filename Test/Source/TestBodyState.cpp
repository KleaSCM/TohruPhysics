/**
 * Unit tests for production rigid body state components.
 * プロダクション剛体状態コンポーネントの単体テストね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/BodyState.h>
#include <stdio.h>
#include <stdlib.h>

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
//  RigidBodyState
// ===========================================================================

static void TestRigidBodyStateMake(void) {
	Vector3 P = KannaVector3Make(1, 2, 3);
	Quaternion R = EuphylliaQuaternionMake(0, 0, 0.382683, 0.92388);
	RigidBodyState S = YuiRigidBodyStateMake(&P, &R);
	TEST(AE(S.Position.Data[0], 1, 1e-12), "pos x");
	TEST(AE(S.Rotation.Data[2], 0.382683, 1e-6), "rot z");
	TEST(AE(S.LinearVelocity.Data[0], 0, 1e-12), "vel zero");
	TEST(AE(S.PreviousPosition.Data[0], 1, 1e-12), "prev pos matches");
}

static void TestRigidBodyStateSavePrevious(void) {
	RigidBodyState S = YuiRigidBodyStateZero();
	S.Position = KannaVector3Make(10, 20, 30);
	YuiRigidBodyStateSavePrevious(&S);
	TEST(AE(S.PreviousPosition.Data[0], 10, 1e-12), "prev saved");
}

// ===========================================================================
//  MassProperties
// ===========================================================================

static void TestMassPropertiesMake(void) {
	Vector3 CoM = KannaVector3Make(0.5, 0, 0);
	MassProperties P = YuiMassPropertiesMake(10.0, &CoM);
	TEST(AE(P.Mass, 10.0, 1e-12), "mass = 10");
	TEST(AE(P.InverseMass, 0.1, 1e-12), "inv mass = 0.1");
	Vector3 ZeroVec = KannaVector3Zero();
	MassProperties Z = YuiMassPropertiesMake(0.0, &ZeroVec);
	TEST(AE(Z.InverseMass, 0.0, 1e-12), "zero mass = infinite");
}

// ===========================================================================
//  InertiaTensor
// ===========================================================================

static void TestInertiaTensorComputeBox(void) {
	Vector3 HE = KannaVector3Make(1, 2, 3);
	Matrix3x3 InvI = YuiInertiaComputeBox(12.0, &HE);
	// Ixx = 12 * (4+9)/3 = 12*13/3 = 52
	// Iyy = 12 * (1+9)/3 = 12*10/3 = 40
	// Izz = 12 * (1+4)/3 = 12*5/3 = 20
	TEST(AE(InvI.Data[0], 1.0 / 52.0, 1e-6), "box inv ixx");
	TEST(AE(InvI.Data[4], 1.0 / 40.0, 1e-6), "box inv iyy");
	TEST(AE(InvI.Data[8], 1.0 / 20.0, 1e-6), "box inv izz");
}

static void TestInertiaTensorComputeSphere(void) {
	Matrix3x3 InvI = YuiInertiaComputeSphere(10.0, 2.0);
	// I = 0.4 * 10 * 4 = 16
	TEST(AE(InvI.Data[0], 1.0 / 16.0, 1e-6), "sphere inv i");
	TEST(AE(InvI.Data[4], 1.0 / 16.0, 1e-6), "sphere inv i yy");
}

static void TestInertiaTensorComputeCapsule(void) {
	Matrix3x3 InvI = YuiInertiaComputeCapsule(10.0, 1.0, 1.0);
	TEST(!MaiIsInf(InvI.Data[0]), "capsule inv i finite");
	TEST(!MaiIsInf(InvI.Data[4]), "capsule inv i yy finite");
	TEST(InvI.Data[0] > REAL_ZERO, "capsule inv i positive");
}

// ===========================================================================
//  Damping
// ===========================================================================

static void TestDampingApply(void) {
	RigidBodyState S = YuiRigidBodyStateZero();
	S.LinearVelocity = KannaVector3Make(10, 0, 0);
	DampingConfig D = YuiDampingConfigMake(1.0, 0.0);
	YuiDampingApply(&D, &S, 1.0);
	// 10 * 1/(1+1*1) = 10 * 0.5 = 5
	TEST(AE(S.LinearVelocity.Data[0], 5.0, 1e-6), "damping halved");
}

// ===========================================================================
//  VelocityCap
// ===========================================================================

static void TestVelocityCapApply(void) {
	RigidBodyState S = YuiRigidBodyStateZero();
	S.LinearVelocity = KannaVector3Make(100, 0, 0);
	VelocityCap Cap = YuiVelocityCapMake(10.0, 0.0);
	YuiVelocityCapApply(&Cap, &S);
	TEST(AE(S.LinearVelocity.Data[0], 10.0, 1e-6), "vel capped to 10");
}

// ===========================================================================
//  MaterialConfig
// ===========================================================================

static void TestMaterialConfigCombine(void) {
	MaterialConfig A = YuiMaterialConfigMake(0.5, 0.4, 0.3);
	MaterialConfig B = YuiMaterialConfigMake(0.8, 0.7, 0.6);
	MaterialConfig C = YuiMaterialConfigCombine(&A, &B);
	// Default combine mode is Multiply
	TEST(AE(C.Restitution, 0.4, 1e-6), "combined rest = 0.5*0.8");
	TEST(AE(C.StaticFriction, 0.28, 1e-6), "combined sf = 0.4*0.7");
	TEST(AE(C.DynamicFriction, 0.18, 1e-6), "combined df = 0.3*0.6");
}

// ===========================================================================
//  KinematicConfig
// ===========================================================================

static void TestKinematicConfig(void) {
	KinematicConfig K = YuiKinematicConfigMake(BodyType_Kinematic, 2.0);
	TEST(K.Type == BodyType_Kinematic, "kinematic type");
	TEST(AE(K.GravityScale, 2.0, 1e-12), "gravity scale = 2");
}

// ===========================================================================
//  BodyConfig
// ===========================================================================

static void TestBodyConfigMakeDynamic(void) {
	Vector3 Pos = KannaVector3Make(0, 5, 0);
	Quaternion Rot = EuphylliaQuaternionIdentity();
	Vector3 HE = KannaVector3Make(1, 1, 1);
	BodyConfig C = YuiBodyConfigMakeDynamic(&Pos, &Rot, 10.0, &HE);
	TEST(AE(C.State.Position.Data[1], 5.0, 1e-12), "dynamic pos y=5");
	TEST(AE(C.Mass.Mass, 10.0, 1e-12), "dynamic mass=10");
	TEST(C.Kinematic.Type == BodyType_Dynamic, "dynamic type");
	TEST(C.Flags & BodyFlag_AllowSleep, "dynamic allows sleep");
}

static void TestBodyConfigMakeStatic(void) {
	Vector3 Pos = KannaVector3Make(0, 0, 0);
	BodyConfig C = YuiBodyConfigMakeStatic(&Pos);
	TEST(C.Kinematic.Type == BodyType_Static, "static type");
	TEST(AE(C.Mass.InverseMass, 0.0, 1e-12), "static inv mass = 0");
}

static void TestSleepUpdate(void) {
	SleepConfig Sleep = YuiSleepConfigMake(0.5, 0.5, 2.0);
	RigidBodyState S = YuiRigidBodyStateZero();
	S.LinearVelocity = KannaVector3Make(100, 0, 0);

	// High velocity — should not sleep
	int ShouldSleep = YuiSleepUpdate(&Sleep, &S, 1.0);
	TEST(ShouldSleep == 0, "no sleep at high velocity");
	TEST(AE(Sleep.Timer, 0.0, 1e-12), "timer reset");

	// Zero velocity — should accumulate timer
	S.LinearVelocity = KannaVector3Zero();
	ShouldSleep = YuiSleepUpdate(&Sleep, &S, 1.0);
	TEST(ShouldSleep == 0, "not yet: 1s < 2s dwell");
	TEST(AE(Sleep.Timer, 1.0, 1e-6), "timer = 1s");

	ShouldSleep = YuiSleepUpdate(&Sleep, &S, 1.0);
	TEST(ShouldSleep == 1, "sleep after 2s");
}

static void TestApplyGravity(void) {
	KinematicConfig Kin = YuiKinematicConfigMake(BodyType_Dynamic, 1.0);
	GravityField Grav = YuiGravityFieldMake(0, -9.81, 0, 1.0);
	RigidBodyState S = YuiRigidBodyStateZero();
	YuiApplyGravity(&Grav, &Kin, &S, 1.0);
	TEST(AE(S.LinearVelocity.Data[1], -9.81, 1e-6), "gravity applied");

	// Kinematic body — no gravity
	Kin.Type = BodyType_Kinematic;
	S.LinearVelocity = KannaVector3Zero();
	YuiApplyGravity(&Grav, &Kin, &S, 1.0);
	TEST(AE(S.LinearVelocity.Data[1], 0.0, 1e-12), "kinematic no gravity");
}

int main(void) {
	fprintf(stderr, "=== TestBodyState ===\n");

	RUN_TEST(TestRigidBodyStateMake, "RigidBodyState: make");
	RUN_TEST(TestRigidBodyStateSavePrevious, "RigidBodyState: save previous");
	RUN_TEST(TestMassPropertiesMake, "MassProperties: make");
	RUN_TEST(TestInertiaTensorComputeBox, "InertiaTensor: compute box");
	RUN_TEST(TestInertiaTensorComputeSphere, "InertiaTensor: compute sphere");
	RUN_TEST(TestInertiaTensorComputeCapsule, "InertiaTensor: compute capsule");
	RUN_TEST(TestDampingApply, "Damping: apply");
	RUN_TEST(TestVelocityCapApply, "VelocityCap: apply");
	RUN_TEST(TestMaterialConfigCombine, "MaterialConfig: combine");
	RUN_TEST(TestKinematicConfig, "KinematicConfig: make");
	RUN_TEST(TestBodyConfigMakeDynamic, "BodyConfig: dynamic");
	RUN_TEST(TestBodyConfigMakeStatic, "BodyConfig: static");
	RUN_TEST(TestSleepUpdate, "SleepConfig: update");
	RUN_TEST(TestApplyGravity, "GravityField: apply");

	fprintf(stderr, "\n=== %d passed, 0 failed ===\n", Passed);
	return 0;
}
