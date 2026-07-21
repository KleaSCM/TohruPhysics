/**
 * AppState — runtime application state implementation.
 * TohruPhysics GUIのランタイムアプリケーション状態の実装ね。
 *
 * Provides init, default-value population, notification queue management,
 * performance sample ring-buffer, camera auto-rotation tick.
 *
 * All values follow ZII — zero-initialised state is always safe to use.
 * The physics engine sees zero gravity as "no gravity"; zero body count
 * is a valid empty scene; zero perf samples graph as flat zero.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "AppState.h"

#include <string.h>
#include <math.h>

/* -------------------------------------------------------------------------
 *  AppStateInit — zero out entire state
 *  AppStateInit — 全状態をゼロ初期化するね。
 * ---------------------------------------------------------------------- */
void AppStateInit(AppState *S) {
	memset(S, 0, sizeof(AppState));
}

/* -------------------------------------------------------------------------
 *  AppStateApplyDefaults — fill sensible physics / camera / prefs values
 *  AppStateApplyDefaults — 適切なデフォルト値を設定するの。
 *
 *  Called once after AppStateInit. Override any specific zero that would
 *  be meaningless (e.g. FovDeg=0 produces a degenerate projection).
 * ---------------------------------------------------------------------- */
void AppStateApplyDefaults(AppState *S) {
	/* Simulation */
	S->Sim.PlayMode         = SimPlayMode_Paused;
	S->Sim.TimeScale        = 1.0f;
	S->Sim.Dt               = 1.0f / 60.0f;
	S->Sim.SubStepCount     = 4;
	S->Sim.SolverIterations = 10;
	S->Sim.WarmUpFrames     = 3;

	/* Physics — Earth gravity, light damping, moderate friction */
	S->Physics.GravityY             = -9.81f;
	S->Physics.LinearDamping        = 0.02f;
	S->Physics.AngularDamping       = 0.05f;
	S->Physics.GlobalRestitution    = 0.3f;
	S->Physics.StaticFriction       = 0.5f;
	S->Physics.DynamicFriction      = 0.4f;
	S->Physics.SolverTolerance      = 1e-4f;
	S->Physics.MaxLinearVelocity    = 200.0f;
	S->Physics.MaxAngularVelocity   = 200.0f;
	S->Physics.CcdEnabled           = 0;
	S->Physics.CcdThreshold         = 10.0f;
	S->Physics.CollisionGroupMask   = 0xFFFFFFFF;

	/* Camera — isometric-ish starting position */
	S->Camera.PosX    = 10.0f;
	S->Camera.PosY    = 8.0f;
	S->Camera.PosZ    = 12.0f;
	S->Camera.TargetX = 0.0f;
	S->Camera.TargetY = 0.0f;
	S->Camera.TargetZ = 0.0f;
	S->Camera.UpX     = 0.0f;
	S->Camera.UpY     = 1.0f;
	S->Camera.UpZ     = 0.0f;
	S->Camera.FovDeg  = 60.0f;
	S->Camera.NearPlane    = 0.1f;
	S->Camera.FarPlane     = 2000.0f;
	S->Camera.MoveSpeed    = 1.0f;
	S->Camera.OrthoWidth   = 20.0f;
	S->Camera.AutoRotateSpeed = 15.0f;

	/* Theme — dark mode, normal font scale */
	S->Theme.Mode          = ThemeMode_Dark;
	S->Theme.FontScale     = 1.0f;
	S->Theme.AnimationSpeed = 1.0f;
	S->Theme.AccentR       = 0.42f;
	S->Theme.AccentG       = 0.55f;
	S->Theme.AccentB       = 1.00f;
	S->Theme.BackgroundR   = 0.08f;
	S->Theme.BackgroundG   = 0.08f;
	S->Theme.BackgroundB   = 0.10f;

	/* Record */
	S->Record.PlaybackSpeed = 1.0f;

	/* Preferences */
	S->Prefs.Language              = 0; /* English */
	S->Prefs.AutoSaveEnabled       = 1;
	S->Prefs.AutoSaveIntervalSec   = 300.0f;
	S->Prefs.ConfirmOnReset        = 1;
	S->Prefs.ShowSplashScreen      = 1;
	S->Prefs.ShowGrid              = 1;
	S->Prefs.ShowAxis              = 1;
	S->Prefs.ShowFpsCounter        = 1;
	S->Prefs.ShowStatsOverlay      = 0;
	S->Prefs.GridSpacing           = 1.0f;
	S->Prefs.GridOpacity           = 0.35f;
	S->Prefs.ShadowQuality         = 1; /* low */
	S->Prefs.AntiAliasing          = 1;
	S->Prefs.TextureQuality        = 1;
	S->Prefs.AmbientIntensity      = 0.25f;
	S->Prefs.DirectionalIntensity  = 0.85f;
	S->Prefs.FogDistance           = 500.0f;
	S->Prefs.ShowContactPoints     = 0;
	S->Prefs.ShowContactNormals    = 0;
	S->Prefs.ShowVelocityVectors   = 0;
	S->Prefs.ShowAngularVelocity   = 0;
	S->Prefs.ShowAABBs             = 0;
	S->Prefs.ShowBVH               = 0;
	S->Prefs.ShowSleepColors       = 1;
	S->Prefs.ShowJoints            = 1;
	S->Prefs.ShowConstraintLimits  = 0;
	S->Prefs.TargetFPS             = 0; /* unlimited */
	S->Prefs.VSyncEnabled          = 1;
	S->Prefs.MaxBodiesWarning      = 10000;
	S->Prefs.TooltipDelayMs        = 500.0f;
	S->Prefs.KeyboardNavigation    = 1;
	S->Prefs.FocusIndicator        = 1;

	/* Inspector — no selection */
	S->Inspector.Primary.BodyId = -1;

	/* Demo — no active demo */
	S->Demo.ActiveDemoId = -1;

	/* Window defaults */
	S->WindowWidth  = 1600;
	S->WindowHeight = 900;
	S->ViewportWidth  = 1100;
	S->ViewportHeight = 900;
}

/* -------------------------------------------------------------------------
 *  AppStatePushNotification — add a toast to the ring buffer
 *  AppStatePushNotification — リングバッファにトーストを追加するね。
 * ---------------------------------------------------------------------- */
void AppStatePushNotification(AppState *S, int Level, const char *Msg, float DurationSec) {
	NotifQueue *Q = &S->Notif;
	int Idx = Q->Write & (APP_MAX_NOTIF - 1);

	Notification *N = &Q->Items[Idx];
	memset(N, 0, sizeof(Notification));
	N->Level       = Level;
	N->DurationSec = DurationSec;
	N->ElapsedSec  = 0.0f;
	N->Dismissed   = 0;

	/* Copy message safely without strncpy's non-termination risk */
	int I = 0;
	while (Msg[I] && I < (int)(sizeof(N->Message) - 1)) {
		N->Message[I] = Msg[I];
		I++;
	}
	N->Message[I] = '\0';

	Q->Write = (Q->Write + 1) & (APP_MAX_NOTIF - 1);
	if (Q->Count < APP_MAX_NOTIF) {
		Q->Count++;
	} else {
		/* Ring full — advance read pointer, drop oldest */
		Q->Read = (Q->Read + 1) & (APP_MAX_NOTIF - 1);
	}
}

/* -------------------------------------------------------------------------
 *  AppStateTickNotifications — age and expire notifications
 *  AppStateTickNotifications — 通知を時間経過させて期限切れにするの。
 * ---------------------------------------------------------------------- */
void AppStateTickNotifications(AppState *S, float DeltaSec) {
	NotifQueue *Q = &S->Notif;
	int Alive     = 0;

	for (int I = 0; I < APP_MAX_NOTIF; I++) {
		Notification *N = &Q->Items[I];
		if (N->Dismissed) {
			continue;
		}
		if (N->Level == 0 && N->DurationSec == 0.0f && N->Message[0] == '\0') {
			continue; /* empty slot */
		}
		N->ElapsedSec += DeltaSec;
		if (N->DurationSec > 0.0f && N->ElapsedSec >= N->DurationSec) {
			N->Dismissed = 1;
		} else {
			Alive++;
		}
	}
	Q->Count = Alive;
}

/* -------------------------------------------------------------------------
 *  AppStatePushPerfSample — write one frame sample into the ring buffers
 *  AppStatePushPerfSample — フレームサンプルをリングバッファに書き込むの。
 * ---------------------------------------------------------------------- */
void AppStatePushPerfSample(AppState *S, float FPS, float FrameMs,
	float SimMs, float BroadMs, float NarrowMs, float SolverMs,
	float MemMB, float CpuPct)
{
	PerfHistory *P = &S->Perf;
	int H = P->Head & (APP_PERF_HISTORY - 1);

	P->FPS[H]      = FPS;
	P->FrameMs[H]  = FrameMs;
	P->SimMs[H]    = SimMs;
	P->BroadMs[H]  = BroadMs;
	P->NarrowMs[H] = NarrowMs;
	P->SolverMs[H] = SolverMs;
	P->MemoryMB[H] = MemMB;
	P->CpuPct[H]   = CpuPct;

	P->Head = (P->Head + 1) & (APP_PERF_HISTORY - 1);
	if (P->Count < APP_PERF_HISTORY) {
		P->Count++;
	}
}

/* -------------------------------------------------------------------------
 *  AppStateTickCamera — auto-rotate camera around target
 *  AppStateTickCamera — カメラをターゲット周りで自動回転させるね。
 * ---------------------------------------------------------------------- */
void AppStateTickCamera(AppState *S, float DeltaSec) {
	CameraState *C = &S->Camera;
	if (!C->AutoRotate) {
		return;
	}

	/*
	 * Orbit the eye position around the target by rotating in the XZ plane.
	 * カメラの視点をターゲット周りのXZ平面で回転させるの。
	 */
	float AngleRad = C->AutoRotateSpeed * (3.14159265f / 180.0f) * DeltaSec;
	float Dx = C->PosX - C->TargetX;
	float Dz = C->PosZ - C->TargetZ;
	float CosA = cosf(AngleRad);
	float SinA = sinf(AngleRad);
	C->PosX = C->TargetX + Dx * CosA - Dz * SinA;
	C->PosZ = C->TargetZ + Dx * SinA + Dz * CosA;
}
