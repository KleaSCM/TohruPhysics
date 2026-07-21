/**
 * AppMain — GUI application entry point.
 * GUIアプリケーションのエントリポイントね。
 *
 * DESIGN PHILOSOPHY:
 * Slint owns the window and event loop. A repeating slint::Timer drives
 * the fixed-timestep physics tick and pushes AppState → Slint UI via the
 * MainWindowBridge. OpenGL rendering for the 3D viewport runs in the
 * rendering notifier's BeforeRendering callback, where Slint guarantees
 * that the OpenGL context is current.
 *
 * LIFECYCLE:
 * 1. Init AppState (zero + defaults)
 * 2. Create MainWindow (Slint)
 * 3. Init MainWindowBridge
 * 4. Register rendering notifier (GL setup / render / teardown)
 * 5. Start repeating physics timer
 * 6. Show window → run event loop
 * 7. Destroy viewport, return
 *
 * ┌──────────┐   Timer(16ms)   ┌─────────────┐   BeforeRendering   ┌──────────────┐
 * │ Physics  │ ──────────────> │ MainWindow  │ ──────────────────> │ ViewportHost │
 * │ Tick     │   Sync Bridge   │ Bridge      │   OpenGL Render     │ (Grid+Debug) │
 * └──────────┘                 └─────────────┘                     └──────────────┘
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "AppState.h"
#include "Window/MainWindowBridge.h"
#include <slint.h>

int main() {
	/* ---- 1. Application state ---- */
	AppState State;
	AppStateInit(&State);
	AppStateApplyDefaults(&State);

	/* ---- 2. Slint main window ---- */
	auto Window = MainWindow::create();

	/* ---- 3. Bridge (AppState ↔ Slint properties) ---- */
	MainWindowBridge Bridge {
		.Window = Window,
		.State = &State
	};
	MainWindowBridgeInit(&Bridge, &State, Window);

	/* ---- 4. Rendering notifier (GL context lifecycle) ---- */
	Window->window().set_rendering_notifier([&](slint::RenderingState RS, slint::GraphicsAPI) {
		(void)RS;
	});

	/* ---- 6. Physics / UI sync timer (≈60 fps) ---- */
	slint::Timer MainTimer;
	MainTimer.start(slint::TimerMode::Repeated,
		std::chrono::milliseconds(16),
		[&]() {
			float DtRaw = State.Sim.Dt * State.Sim.TimeScale;

			/* Fixed-timestep accumulator */
			State.Sim.AccumTime += DtRaw;
			while (State.Sim.AccumTime >= State.Sim.Dt) {
				if (State.Sim.PlayMode == SimPlayMode_Running) {
					/* TODO: StepPhysics(&State, State.Sim.Dt) */
				} else if (State.Sim.PlayMode == SimPlayMode_Stepping) {
					/* TODO: StepPhysics(&State, State.Sim.Dt) */
					State.Sim.PlayMode = SimPlayMode_Paused;
				}

				if (State.Sim.ResetRequested) {
					/* TODO: SceneManagerReset(&State) */
					State.Sim.ResetRequested = 0;
				}

				State.Sim.AccumTime -= State.Sim.Dt;
				State.Sim.FrameCount++;
			}

			/* Fill frame stats */
			State.Sim.SimTimeMs = DtRaw * 1000.0f;
			State.Stats.FPS = (DtRaw > 0.0f) ? (1.0f / DtRaw) : 60.0f;

			/* Camera auto-orbit */
			AppStateTickCamera(&State, DtRaw);

			/* Age notification ring-buffer */
			AppStateTickNotifications(&State, DtRaw);

			/* Push one performance sample */
			AppStatePushPerfSample(&State,
				State.Stats.FPS,
				State.Sim.SimTimeMs,
				State.Sim.SimTimeMs,
				0.0f, 0.0f, 0.0f,
				0.0f, 0.0f);

			/* Push state to Slint properties */
			MainWindowBridgeSync(&Bridge);

			/* Request a frame render so BeforeRendering fires */
			Window->window().request_redraw();
		});

	/* ---- 7. Show and run ---- */
	Window->show();
	slint::run_event_loop();

	return 0;
}
