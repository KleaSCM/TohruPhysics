/**
 * BodyState — production rigid body physical state implementations.
 * TohruPhysics用のプロダクション剛体物理状態の実装ね。
 *
 * Inertia tensor formulas for box/sphere/capsule, damping integration,
 * velocity capping, material combination, and body configuration builders.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/BodyState.h>

// ===========================================================================
//  RigidBodyState
// ===========================================================================

RigidBodyState YuiRigidBodyStateMake(const Vector3 *Position, const Quaternion *Rotation) {
	RigidBodyState S;
	S.Position = *Position;
	S.Rotation = *Rotation;
	S.LinearVelocity = KannaVector3Zero();
	S.AngularVelocity = KannaVector3Zero();
	S.Force = KannaVector3Zero();
	S.Torque = KannaVector3Zero();
	S.PreviousPosition = *Position;
	S.PreviousRotation = *Rotation;
	return S;
}

RigidBodyState YuiRigidBodyStateZero(void) {
	Vector3 ZeroPos = KannaVector3Zero();
	Quaternion IdRot = EuphylliaQuaternionIdentity();
	return YuiRigidBodyStateMake(&ZeroPos, &IdRot);
}

void YuiRigidBodyStateClearForces(RigidBodyState *State) {
	State->Force = KannaVector3Zero();
	State->Torque = KannaVector3Zero();
}

void YuiRigidBodyStateSavePrevious(RigidBodyState *State) {
	State->PreviousPosition = State->Position;
	State->PreviousRotation = State->Rotation;
}

// ===========================================================================
//  MassProperties
// ===========================================================================

MassProperties YuiMassPropertiesMake(Real Mass, const Vector3 *CenterOfMass) {
	MassProperties P;
	P.Mass = Mass > REAL_ZERO ? Mass : REAL_ZERO;
	P.InverseMass = Mass > REAL_ZERO ? 1.0 / Mass : REAL_ZERO;
	P.CenterOfMass = *CenterOfMass;
	return P;
}

MassProperties YuiMassPropertiesZero(void) {
	MassProperties P;
	P.Mass = REAL_ZERO;
	P.InverseMass = REAL_ZERO;
	P.CenterOfMass = KannaVector3Zero();
	return P;
}

// ===========================================================================
//  InertiaTensor
// ===========================================================================

InertiaTensor YuiInertiaTensorMake(const Matrix3x3 *InverseInertiaLocal) {
	InertiaTensor T;
	T.InverseInertiaLocal = *InverseInertiaLocal;
	T.InverseInertiaWorld = *InverseInertiaLocal;
	return T;
}

InertiaTensor YuiInertiaTensorZero(void) {
	Matrix3x3 Zero = MiorineMatrix3x3Zero();
	return YuiInertiaTensorMake(&Zero);
}

void YuiInertiaTensorUpdateWorld(InertiaTensor *Tensor, const Quaternion *Rotation) {
	// I_world = R * I_local * R^T
	// ワールド慣性 = R * ローカル慣性 * R^T
	Matrix3x3 R = EuphylliaQuaternionToMatrix3x3(Rotation);
	Matrix3x3 RT = MiorineMatrix3x3Transpose(&R);
	Matrix3x3 Temp = MiorineMatrix3x3Mul(&R, &Tensor->InverseInertiaLocal);
	Tensor->InverseInertiaWorld = MiorineMatrix3x3Mul(&Temp, &RT);
}

// ---------------------------------------------------------------------------
//  Inertia computation — box
//  箱の慣性計算ね。
// ---------------------------------------------------------------------------
Matrix3x3 YuiInertiaComputeBox(Real Mass, const Vector3 *HalfExtents) {
	Real Hx = HalfExtents->Data[0];
	Real Hy = HalfExtents->Data[1];
	Real Hz = HalfExtents->Data[2];

	// Ixx = m * (hy² + hz²) / 3
	// Iyy = m * (hx² + hz²) / 3
	// Izz = m * (hx² + hy²) / 3
	Real Ixx = Mass * (Hy * Hy + Hz * Hz) / 3.0;
	Real Iyy = Mass * (Hx * Hx + Hz * Hz) / 3.0;
	Real Izz = Mass * (Hx * Hx + Hy * Hy) / 3.0;

	// Return InverseInertiaLocal (diagonal, zero off-diagonals for AABB-aligned)
	// 対角成分の逆慣性を返すの（AABB一致なら非対角成分はゼロ）。
	Matrix3x3 M;
	M.Data[0] = NagisaIsZero(Ixx) ? REAL_ZERO : 1.0 / Ixx;
	M.Data[1] = REAL_ZERO; M.Data[2] = REAL_ZERO;
	M.Data[3] = REAL_ZERO;
	M.Data[4] = NagisaIsZero(Iyy) ? REAL_ZERO : 1.0 / Iyy;
	M.Data[5] = REAL_ZERO;
	M.Data[6] = REAL_ZERO; M.Data[7] = REAL_ZERO;
	M.Data[8] = NagisaIsZero(Izz) ? REAL_ZERO : 1.0 / Izz;
	return M;
}

// ---------------------------------------------------------------------------
//  Inertia computation — sphere
//  球の慣性計算ね。
// ---------------------------------------------------------------------------
Matrix3x3 YuiInertiaComputeSphere(Real Mass, Real Radius) {
	// I = 2/5 * m * r² for all axes
	// 全軸で I = 2/5 * m * r²
	Real I = 0.4 * Mass * Radius * Radius;
	Real InvI = NagisaIsZero(I) ? REAL_ZERO : 1.0 / I;

	Matrix3x3 M;
	M.Data[0] = InvI; M.Data[1] = REAL_ZERO; M.Data[2] = REAL_ZERO;
	M.Data[3] = REAL_ZERO; M.Data[4] = InvI; M.Data[5] = REAL_ZERO;
	M.Data[6] = REAL_ZERO; M.Data[7] = REAL_ZERO; M.Data[8] = InvI;
	return M;
}

// ---------------------------------------------------------------------------
//  Inertia computation — capsule
//  カプセルの慣性計算ね。
// ---------------------------------------------------------------------------
Matrix3x3 YuiInertiaComputeCapsule(Real Mass, Real Radius, Real HalfHeight) {
	// Capsule = cylinder + two hemispherical caps
	// カプセル = 円柱 + 半球キャップ2個
	Real H = HalfHeight * 2.0; // full cylinder height
	Real CylMass = Mass * H / (H + 2.0 * HalfHeight);  // approximate
	Real CapMass = (Mass - CylMass) * 0.5;

	// Cylinder Ixx = Izz = m * (3*r² + h²) / 12,  Iyy = m * r² / 2
	Real CylIxx = CylMass * (3.0 * Radius * Radius + H * H) / 12.0;
	Real CylIyy = CylMass * Radius * Radius * 0.5;

	// Each hemispherical cap (parallel axis theorem from center):
	// Ixx_cap = m * (2*r²/5 + h²/4 + 3*h*r/8)
	// For half-height h (= HalfHeight from center to cap center):
	Real Hc = HalfHeight;
	Real CapIxx = CapMass * (0.4 * Radius * Radius + 0.25 * Hc * Hc + 0.375 * Hc * Radius);
	Real CapIyy = CapMass * (0.4 * Radius * Radius);

	Real TotalIxx = CylIxx + 2.0 * CapIxx;
	Real TotalIyy = CylIyy + 2.0 * CapIyy;
	Real TotalIzz = TotalIxx; // rotational symmetry about Y

	Real InvIxx = NagisaIsZero(TotalIxx) ? REAL_ZERO : 1.0 / TotalIxx;
	Real InvIyy = NagisaIsZero(TotalIyy) ? REAL_ZERO : 1.0 / TotalIyy;
	Real InvIzz = NagisaIsZero(TotalIzz) ? REAL_ZERO : 1.0 / TotalIzz;

	Matrix3x3 M;
	M.Data[0] = InvIxx; M.Data[1] = REAL_ZERO; M.Data[2] = REAL_ZERO;
	M.Data[3] = REAL_ZERO; M.Data[4] = InvIyy; M.Data[5] = REAL_ZERO;
	M.Data[6] = REAL_ZERO; M.Data[7] = REAL_ZERO; M.Data[8] = InvIzz;
	return M;
}

// ===========================================================================
//  Damping
// ===========================================================================

DampingConfig YuiDampingConfigMake(Real Linear, Real Angular) {
	DampingConfig D;
	D.LinearDamping = YuuClamp01(Linear);
	D.AngularDamping = YuuClamp01(Angular);
	return D;
}

DampingConfig YuiDampingConfigZero(void) {
	return YuiDampingConfigMake(REAL_ZERO, REAL_ZERO);
}

void YuiDampingApply(DampingConfig *D, RigidBodyState *S, Real Dt) {
	// Exponential damping: v *= 1 / (1 + damping * dt)
	// 指数減衰: v *= 1 / (1 + 減衰 * dt)
	Real LinFactor = 1.0 / (1.0 + D->LinearDamping * Dt);
	Real AngFactor = 1.0 / (1.0 + D->AngularDamping * Dt);
	S->LinearVelocity = KannaVector3Scale(&S->LinearVelocity, LinFactor);
	S->AngularVelocity = KannaVector3Scale(&S->AngularVelocity, AngFactor);
}

// ===========================================================================
//  VelocityCap
// ===========================================================================

VelocityCap YuiVelocityCapMake(Real MaxLinear, Real MaxAngular) {
	VelocityCap C;
	C.MaxLinearVelocity = MaxLinear > REAL_ZERO ? MaxLinear : REAL_ZERO;
	C.MaxAngularVelocity = MaxAngular > REAL_ZERO ? MaxAngular : REAL_ZERO;
	return C;
}

VelocityCap YuiVelocityCapZero(void) {
	return YuiVelocityCapMake(REAL_ZERO, REAL_ZERO);
}

void YuiVelocityCapApply(VelocityCap *Cap, RigidBodyState *S) {
	Real MaxLin = Cap->MaxLinearVelocity;
	if (MaxLin > REAL_ZERO) {
		Real LinLen = KannaVector3Length(&S->LinearVelocity);
		if (LinLen > MaxLin) {
			Real Scale = MaxLin / LinLen;
			S->LinearVelocity = KannaVector3Scale(&S->LinearVelocity, Scale);
		}
	}
	Real MaxAng = Cap->MaxAngularVelocity;
	if (MaxAng > REAL_ZERO) {
		Real AngLen = KannaVector3Length(&S->AngularVelocity);
		if (AngLen > MaxAng) {
			Real Scale = MaxAng / AngLen;
			S->AngularVelocity = KannaVector3Scale(&S->AngularVelocity, Scale);
		}
	}
}

// ===========================================================================
//  SleepConfig
// ===========================================================================

SleepConfig YuiSleepConfigMake(Real LinearThreshold, Real AngularThreshold, Real DwellTime) {
	SleepConfig S;
	S.LinearThreshold = LinearThreshold >= REAL_ZERO ? LinearThreshold : REAL_ZERO;
	S.AngularThreshold = AngularThreshold >= REAL_ZERO ? AngularThreshold : REAL_ZERO;
	S.DwellTime = DwellTime >= REAL_ZERO ? DwellTime : REAL_ZERO;
	return S;
}

SleepConfig YuiSleepConfigZero(void) {
	return YuiSleepConfigMake(REAL_ZERO, REAL_ZERO, REAL_ZERO);
}

// ===========================================================================
//  MaterialConfig
// ===========================================================================

MaterialConfig YuiMaterialConfigMake(
	Real Restitution, Real StaticFriction, Real DynamicFriction)
{
	MaterialConfig M;
	M.Restitution = YuuClamp01(Restitution);
	M.StaticFriction = YuuClamp(StaticFriction, REAL_ZERO, 1.0);
	M.DynamicFriction = YuuClamp(DynamicFriction, REAL_ZERO, 1.0);
	M.CombineRestitution = MaterialCombine_Multiply;
	M.CombineFriction = MaterialCombine_Multiply;
	return M;
}

MaterialConfig YuiMaterialConfigDefault(void) {
	return YuiMaterialConfigMake(0.5, 0.6, 0.4);
}

MaterialConfig YuiMaterialConfigZero(void) {
	return YuiMaterialConfigMake(REAL_ZERO, REAL_ZERO, REAL_ZERO);
}

// ---------------------------------------------------------------------------
//  Material combination — for contact pair
//  素材の組み合わせ — 接触ペア用ね
// ---------------------------------------------------------------------------
static Real CombineValue(Real A, Real B, MaterialCombine Mode) {
	switch (Mode) {
	case MaterialCombine_Multiply: return A * B;
	case MaterialCombine_Average:  return (A + B) * 0.5;
	case MaterialCombine_Min:      return A < B ? A : B;
	case MaterialCombine_Max:      return A > B ? A : B;
	default:                       return A * B;
	}
}

MaterialConfig YuiMaterialConfigCombine(
	const MaterialConfig *A, const MaterialConfig *B)
{
	MaterialConfig M;
	M.Restitution = CombineValue(A->Restitution, B->Restitution, A->CombineRestitution);
	M.StaticFriction = CombineValue(A->StaticFriction, B->StaticFriction, A->CombineFriction);
	M.DynamicFriction = CombineValue(A->DynamicFriction, B->DynamicFriction, A->CombineFriction);
	M.CombineRestitution = A->CombineRestitution;
	M.CombineFriction = A->CombineFriction;
	return M;
}

// ===========================================================================
//  KinematicConfig
// ===========================================================================

KinematicConfig YuiKinematicConfigMake(BodyType Type, Real GravityScale) {
	KinematicConfig K;
	K.Type = Type;
	K.GravityScale = GravityScale;
	return K;
}

KinematicConfig YuiKinematicConfigDefault(void) {
	return YuiKinematicConfigMake(BodyType_Dynamic, 1.0);
}

KinematicConfig YuiKinematicConfigZero(void) {
	return YuiKinematicConfigMake(BodyType_Static, 1.0);
}

// ===========================================================================
//  BodyConfig builders
// ===========================================================================

BodyConfig YuiBodyConfigDefault(void) {
	BodyConfig C;
	C.State = YuiRigidBodyStateZero();
	C.Mass = YuiMassPropertiesZero();
	C.Inertia = YuiInertiaTensorZero();
	C.Damping = YuiDampingConfigMake(0.1, 0.05);
	C.VelCap = YuiVelocityCapMake(500.0, 100.0);
	C.Sleep = YuiSleepConfigMake(0.5, 0.5, 2.0);
	C.Material = YuiMaterialConfigDefault();
	C.Kinematic = YuiKinematicConfigDefault();
	C.Flags = BodyFlag_Active | BodyFlag_AllowSleep;
	C.ID = 0;
	return C;
}

BodyConfig YuiBodyConfigMakeDynamic(
	const Vector3 *Position, const Quaternion *Rotation,
	Real Mass, const Vector3 *HalfExtents)
{
	Vector3 ZeroVec = KannaVector3Zero();
	BodyConfig C = YuiBodyConfigDefault();
	C.State = YuiRigidBodyStateMake(Position, Rotation);
	C.Mass = YuiMassPropertiesMake(Mass, &ZeroVec);
	C.Inertia.InverseInertiaLocal = YuiInertiaComputeBox(Mass, HalfExtents);
	C.Inertia.InverseInertiaWorld = C.Inertia.InverseInertiaLocal;
	C.Kinematic.Type = BodyType_Dynamic;
	return C;
}

BodyConfig YuiBodyConfigMakeStatic(const Vector3 *Position) {
	Quaternion Id = EuphylliaQuaternionIdentity();
	BodyConfig C = YuiBodyConfigDefault();
	C.State = YuiRigidBodyStateMake(Position, &Id);
	C.Mass = YuiMassPropertiesZero();
	C.Kinematic.Type = BodyType_Static;
	C.Flags &= ~(uint32_t)BodyFlag_AllowSleep;
	return C;
}

BodyConfig YuiBodyConfigMakeKinematic(
	const Vector3 *Position, const Quaternion *Rotation)
{
	Vector3 ZeroVec = KannaVector3Zero();
	Vector3 OneVec = KannaVector3Make(1,1,1);
	BodyConfig C = YuiBodyConfigDefault();
	C.State = YuiRigidBodyStateMake(Position, Rotation);
	C.Mass = YuiMassPropertiesMake(1.0, &ZeroVec);
	C.Inertia.InverseInertiaLocal = YuiInertiaComputeBox(1.0, &OneVec);
	C.Inertia.InverseInertiaWorld = C.Inertia.InverseInertiaLocal;
	C.Kinematic.Type = BodyType_Kinematic;
	C.Flags &= ~(uint32_t)BodyFlag_AllowSleep;
	return C;
}
