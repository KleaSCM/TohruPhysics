/**
 * Japanese String Table for TohruPhysics GUI.
 * TohruPhysics GUI用の日本語文字列テーブルね。
 *
 * Uses natural feminine speech patterns without theatrical expressions.
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include "StringTable.h"

static const char *const GJaStrings[StrKey_Count] = {
	"トールフィジックス メイドエンジン", /* StrKey_AppName */
	"再生",                               /* StrKey_Play */
	"一時停止",                           /* StrKey_Pause */
	"コマ送り",                           /* StrKey_Step */
	"リセット",                           /* StrKey_Reset */
	"タイムスケール",                     /* StrKey_TimeScale */
	"重力ベクトル",                       /* StrKey_Gravity */
	"線形減衰",                           /* StrKey_Damping */
	"反発係数",                           /* StrKey_Restitution */
	"摩擦係数",                           /* StrKey_Friction */
	"ソルバー反復回数",                   /* StrKey_SolverIter */
	"サブステップ数",                     /* StrKey_SubSteps */
	"総ボディ数",                         /* StrKey_BodyCount */
	"アクティブボディ",                   /* StrKey_ActiveBodies */
	"スリープ中ボディ",                   /* StrKey_SleepingBodies */
	"FPS",                                /* StrKey_Fps */
	"シミュレーション時間",               /* StrKey_SimTime */
	"メモリ使用量",                       /* StrKey_Memory */
	"デモショーケース"                    /* StrKey_DemoTitle */
};
