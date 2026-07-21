/**
 * ToastBridge — toast notification helper functions.
 * トースト通知のヘルパー関数ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "../App/AppState.h"

void ToastShowInfo(AppState *State, const char *Msg);
void ToastShowSuccess(AppState *State, const char *Msg);
void ToastShowWarning(AppState *State, const char *Msg);
void ToastShowError(AppState *State, const char *Msg);
