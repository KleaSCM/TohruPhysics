/**
 * MainWindowBridge — Slint ↔ AppState binding implementation.
 * SlintとAppStateのバインディング実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "MainWindowBridge.h"

void MainWindowBridgeInit(MainWindowBridge *Bridge, AppState *State, std::shared_ptr<MainWindow> Window) {
	Bridge->State  = State;
	Bridge->Window = Window;
	MainWindowBridgeRegisterCallbacks(Bridge);
	MainWindowBridgeSync(Bridge);
}

void MainWindowBridgeSync(MainWindowBridge *Bridge) {
	if (!Bridge || !Bridge->State || !Bridge->Window) {
		return;
	}

	AppState *S = Bridge->State;
	MainWindow *W = Bridge->Window.get();

	/* Sim state */
	W->set_IsRunning(S->Sim.PlayMode == SimPlayMode_Running);
	W->set_TimeScale(S->Sim.TimeScale);
	W->set_GravityY(S->Physics.GravityY);

	/* Physics params */
	W->set_LinearDamping(S->Physics.LinearDamping);
	W->set_AngularDamping(S->Physics.AngularDamping);
	W->set_Restitution(S->Physics.GlobalRestitution);
	W->set_Friction(S->Physics.StaticFriction);
	W->set_CcdEnabled(S->Physics.CcdEnabled != 0);

	/* Camera state */
	W->set_CameraSpeed(S->Camera.MoveSpeed);
	W->set_OrthoMode(S->Camera.OrthoMode != 0);
	W->set_AutoRotate(S->Camera.AutoRotate != 0);

	/* Stats */
	W->set_TotalBodies(S->Stats.TotalBodies);
	W->set_ActiveBodies(S->Stats.ActiveBodies);
	W->set_SleepingBodies(S->Stats.SleepingBodies);
	W->set_ContactCount(S->Stats.ContactCount);
	W->set_Fps(S->Stats.FPS);
	W->set_SimTimeMs(S->Stats.SimTimeMs);

	/* Selected Inspector */
	W->set_SelectedBodyId((int)S->Inspector.Primary.BodyId);
	W->set_SelectedPosX(S->Inspector.Primary.PosX);
	W->set_SelectedPosY(S->Inspector.Primary.PosY);
	W->set_SelectedPosZ(S->Inspector.Primary.PosZ);
	W->set_SelectedMass(S->Inspector.Primary.Mass);

	/* Active demo ID */
	W->set_ActiveDemoId(S->Demo.ActiveDemoId);
}

void MainWindowBridgeRegisterCallbacks(MainWindowBridge *Bridge) {
	if (!Bridge || !Bridge->State || !Bridge->Window) {
		return;
	}

	AppState *S = Bridge->State;
	MainWindow *W = Bridge->Window.get();

	W->on_PlayClicked([S]() {
		S->Sim.PlayMode = SimPlayMode_Running;
	});

	W->on_PauseClicked([S]() {
		S->Sim.PlayMode = SimPlayMode_Paused;
	});

	W->on_StepClicked([S]() {
		S->Sim.PlayMode = SimPlayMode_Stepping;
	});

	W->on_ResetClicked([S]() {
		S->Sim.ResetRequested = 1;
	});

	W->on_TimeScaleChanged([S](float V) {
		S->Sim.TimeScale = V;
	});

	W->on_GravityChanged([S](float V) {
		S->Physics.GravityY = V;
	});

	W->on_DampingChanged([S](float L, float A) {
		S->Physics.LinearDamping = L;
		S->Physics.AngularDamping = A;
	});

	W->on_RestitutionChanged([S](float V) {
		S->Physics.GlobalRestitution = V;
	});

	W->on_FrictionChanged([S](float V) {
		S->Physics.StaticFriction = V;
	});

	W->on_CcdToggled([S](bool A) {
		S->Physics.CcdEnabled = A ? 1 : 0;
	});

	W->on_CameraSpeedChanged([S](float V) {
		S->Camera.MoveSpeed = V;
	});

	W->on_OrthoToggled([S](bool A) {
		S->Camera.OrthoMode = A ? 1 : 0;
	});

	W->on_AutoRotateToggled([S](bool A) {
		S->Camera.AutoRotate = A ? 1 : 0;
	});

	W->on_ResetCamera([S]() {
		S->Camera.PosX = 10.0f;
		S->Camera.PosY = 8.0f;
		S->Camera.PosZ = 12.0f;
		S->Camera.TargetX = 0.0f;
		S->Camera.TargetY = 0.0f;
		S->Camera.TargetZ = 0.0f;
	});

	W->on_LoadDemo([S](int Id) {
		S->Demo.ActiveDemoId = Id;
		S->Sim.ResetRequested = 1;
	});

	W->on_BodyMassChanged([S](float M) {
		if (S->Inspector.Primary.BodyId >= 0) {
			S->Inspector.Primary.Mass = M;
		}
	});
}
