/**
 * ViewportHost — OpenGL viewport manager integration.
 * OpenGLビューポートマネージャーの統合ね。
 *
 * Owns OpenGL renderers (GridRenderer, DebugRenderer), matrices computation,
 * viewport rendering per frame.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "../App/AppState.h"
#include "GridRenderer.h"
#include "DebugRenderer.h"

struct ViewportHost {
	GridRenderer  Grid;
	DebugRenderer Debug;
	int           Initialized;
};

void ViewportHostInit(ViewportHost *Host);
void ViewportHostDestroy(ViewportHost *Host);
void ViewportHostRender(ViewportHost *Host, const AppState *State, int Width, int Height);
