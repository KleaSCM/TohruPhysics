/**
 * MainWindowBridge — Slint ↔ AppState binding implementation.
 * SlintとAppStateのバインディング実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "MainWindowBridge.h"

void MainWindowBridgeInit(MainWindowBridge *Bridge, AppState *State, slint::ComponentHandle<MainWindow> Window) {
	Bridge->State  = State;
	Bridge->Window = Window;
	MainWindowBridgeRegisterCallbacks(Bridge);
	MainWindowBridgeSync(Bridge);
}

void MainWindowBridgeSync(MainWindowBridge *Bridge) {
	if (!Bridge || !Bridge->State) {
		return;
	}

	AppState *S = Bridge->State;

	/* Sim state */
	(*Bridge->Window).set_IsRunning(S->Sim.PlayMode == SimPlayMode_Running);
	(*Bridge->Window).set_TimeScale(S->Sim.TimeScale);
	(*Bridge->Window).set_GravityY(S->Physics.GravityY);

	/* Physics params */
	(*Bridge->Window).set_LinearDamping(S->Physics.LinearDamping);
	(*Bridge->Window).set_AngularDamping(S->Physics.AngularDamping);
	(*Bridge->Window).set_Restitution(S->Physics.GlobalRestitution);
	(*Bridge->Window).set_Friction(S->Physics.StaticFriction);
	(*Bridge->Window).set_CcdEnabled(S->Physics.CcdEnabled != 0);

	/* Camera state */
	(*Bridge->Window).set_CameraSpeed(S->Camera.MoveSpeed);
	(*Bridge->Window).set_OrthoMode(S->Camera.OrthoMode != 0);
	(*Bridge->Window).set_AutoRotate(S->Camera.AutoRotate != 0);

	/* Stats */
	(*Bridge->Window).set_TotalBodies(S->Stats.TotalBodies);
	(*Bridge->Window).set_ActiveBodies(S->Stats.ActiveBodies);
	(*Bridge->Window).set_SleepingBodies(S->Stats.SleepingBodies);
	(*Bridge->Window).set_ContactCount(S->Stats.ContactCount);
	(*Bridge->Window).set_Fps(S->Stats.FPS);
	(*Bridge->Window).set_SimTimeMs(S->Stats.SimTimeMs);

	/* Selected Inspector */
	(*Bridge->Window).set_SelectedBodyId((int)S->Inspector.Primary.BodyId);
	(*Bridge->Window).set_SelectedPosX(S->Inspector.Primary.PosX);
	(*Bridge->Window).set_SelectedPosY(S->Inspector.Primary.PosY);
	(*Bridge->Window).set_SelectedPosZ(S->Inspector.Primary.PosZ);
	(*Bridge->Window).set_SelectedMass(S->Inspector.Primary.Mass);

	/* Active demo ID */
	(*Bridge->Window).set_ActiveDemoId(S->Demo.ActiveDemoId);
}

void MainWindowBridgeRegisterCallbacks(MainWindowBridge *Bridge) {
	if (!Bridge || !Bridge->State) {
		return;
	}

	AppState *S = Bridge->State;

	Bridge->Window->on_PlayClicked([S]() {
		S->Sim.PlayMode = SimPlayMode_Running;
	});

	Bridge->Window->on_PauseClicked([S]() {
		S->Sim.PlayMode = SimPlayMode_Paused;
	});

	Bridge->Window->on_StepClicked([S]() {
		S->Sim.PlayMode = SimPlayMode_Stepping;
	});

	Bridge->Window->on_ResetClicked([S]() {
		S->Sim.ResetRequested = 1;
	});

	Bridge->Window->on_TimeScaleChanged([S](float V) {
		S->Sim.TimeScale = V;
	});

	Bridge->Window->on_GravityChanged([S](float V) {
		S->Physics.GravityY = V;
	});

	Bridge->Window->on_DampingChanged([S](float L, float A) {
		S->Physics.LinearDamping = L;
		S->Physics.AngularDamping = A;
	});

	Bridge->Window->on_RestitutionChanged([S](float V) {
		S->Physics.GlobalRestitution = V;
	});

	Bridge->Window->on_FrictionChanged([S](float V) {
		S->Physics.StaticFriction = V;
	});

	Bridge->Window->on_CcdToggled([S](bool A) {
		S->Physics.CcdEnabled = A ? 1 : 0;
	});

	Bridge->Window->on_CameraSpeedChanged([S](float V) {
		S->Camera.MoveSpeed = V;
	});

	Bridge->Window->on_OrthoToggled([S](bool A) {
		S->Camera.OrthoMode = A ? 1 : 0;
	});

	Bridge->Window->on_AutoRotateToggled([S](bool A) {
		S->Camera.AutoRotate = A ? 1 : 0;
	});

	Bridge->Window->on_ResetCamera([S]() {
		S->Camera.PosX = 10.0f;
		S->Camera.PosY = 8.0f;
		S->Camera.PosZ = 12.0f;
		S->Camera.TargetX = 0.0f;
		S->Camera.TargetY = 0.0f;
		S->Camera.TargetZ = 0.0f;
	});

	Bridge->Window->on_LoadDemo([S](int Id) {
		S->Demo.ActiveDemoId = Id;
		S->Sim.ResetRequested = 1;
	});

	Bridge->Window->on_BodyMassChanged([S](float M) {
		if (S->Inspector.Primary.BodyId >= 0) {
			S->Inspector.Primary.Mass = M;
		}
	});
}
