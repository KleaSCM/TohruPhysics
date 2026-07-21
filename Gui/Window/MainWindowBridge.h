/**
 * MainWindowBridge — bridge connecting Slint MainWindow to AppState.
 * Slint MainWindowとAppStateを接続するブリッジね。
 *
 * Handles bidirectional binding: syncs AppState values into Slint UI
 * properties each frame, and dispatches Slint callback events back to
 * physics simulation routines.
 *
 * DESIGN PHILOSOPHY:
 * Push model — AppState is updated by physics tick or UI events, then
 * MainWindowBridgeSync() copies fields to Slint. No exceptions or null checks.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "../App/AppState.h"

#include <memory>
#include "MainWindow.h"

struct MainWindowBridge {
	std::shared_ptr<MainWindow> Window;
	AppState                   *State;
};

void MainWindowBridgeInit(MainWindowBridge *Bridge, AppState *State, std::shared_ptr<MainWindow> Window);
void MainWindowBridgeSync(MainWindowBridge *Bridge);
void MainWindowBridgeRegisterCallbacks(MainWindowBridge *Bridge);
