/**
 * ToastBridge implementation.
 * トーストブリッジの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "ToastBridge.h"

void ToastShowInfo(AppState *State, const char *Msg) {
	AppStatePushNotification(State, NotifLevel_Info, Msg, 3.0f);
}

void ToastShowSuccess(AppState *State, const char *Msg) {
	AppStatePushNotification(State, NotifLevel_Success, Msg, 3.0f);
}

void ToastShowWarning(AppState *State, const char *Msg) {
	AppStatePushNotification(State, NotifLevel_Warning, Msg, 4.0f);
}

void ToastShowError(AppState *State, const char *Msg) {
	AppStatePushNotification(State, NotifLevel_Error, Msg, 5.0f);
}
