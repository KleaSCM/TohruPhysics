/**
 * AppState — runtime application state for TohruPhysics GUI.
 * TohruPhysics GUIのランタイムアプリケーション状態ね。
 *
 * Flat C++ structs holding every piece of mutable state that crosses
 * the C++ ↔ Slint boundary. All structs are zero-initialisable (ZII).
 * No virtual functions, no inheritance, no optional<>, no shared_ptr.
 *
 * DESIGN PHILOSOPHY:
 * Slint's data model works best when driven by a single authoritative
 * C++ state tree that is pushed to the UI each frame. This avoids two-
 * way binding complexity and keeps the physics thread free to update
 * state without holding UI locks. The bridge reads AppState once per
 * frame and writes the Slint model properties in one pass.
 *
 * STATE TREE:
 * ┌─────────────────┬────────────────────────────────────────────┐
 * │ SimState        │ play/pause/step flags, time scale, dt       │
 * │ PhysicsParams   │ gravity, damping, restitution, friction …  │
 * │ CameraState     │ position, target, fov, ortho toggle         │
 * │ StatsState      │ body counts, contacts, memory, frame time  │
 * │ InspectorState  │ selected body properties                   │
 * │ DemoState       │ active demo ID, loading flag, category      │
 * │ ThemeState      │ dark/light, font scale, colour overrides    │
 * │ NotifQueue      │ circular buffer of pending notifications    │
 * │ PerfHistory     │ ring buffers for FPS, frame-time, memory   │
 * │ RecordState     │ recording / playback cursor                 │
 * │ PrefsState      │ all user preferences                        │
 * └─────────────────┴────────────────────────────────────────────┘
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

/* -------------------------------------------------------------------------
 *  Fundamental limits
 * ---------------------------------------------------------------------- */
#define APP_MAX_BODIES         65536
#define APP_MAX_NOTIF          32
#define APP_PERF_HISTORY       512   /* ring-buffer depth for graphs      */
#define APP_MAX_DEMO_SCENES    64
#define APP_MAX_CAMERA_BOOKMARKS 8
#define APP_MAX_PLUGIN_SLOTS   32

/* -------------------------------------------------------------------------
 *  SimState — simulation playback control (§3.3)
 *  シミュレーション再生制御の状態ね。
 * ---------------------------------------------------------------------- */
enum SimPlayMode {
	SimPlayMode_Paused   = 0,
	SimPlayMode_Running  = 1,
	SimPlayMode_Stepping = 2    /* advance exactly one frame then pause */
};

struct SimState {
	int       PlayMode;          /* SimPlayMode                          */
	float     TimeScale;         /* 0.1 – 10.0 (real-time multiplier)   */
	float     Dt;                /* current step delta-time (seconds)    */
	float     AccumTime;         /* accumulated simulation time (s)      */
	int       SubStepCount;      /* 1–16                                 */
	int       SolverIterations;  /* 1–100                                */
	int       WarmUpFrames;      /* warm-up frames before recording      */
	int       FrameCount;        /* total frames simulated               */
	float     SimTimeMs;         /* last step wall-clock duration (ms)   */
	int       ResetRequested;    /* set to 1 triggers scene reset        */
};

/* -------------------------------------------------------------------------
 *  PhysicsParams — global physics parameters (§3.4)
 *  グローバル物理パラメーターの状態ね。
 * ---------------------------------------------------------------------- */
struct PhysicsParams {
	float GravityX, GravityY, GravityZ;   /* m/s² — default 0,-9.81,0  */
	float LinearDamping;                   /* 0–1                         */
	float AngularDamping;                  /* 0–1                         */
	float GlobalRestitution;               /* 0–1 coefficient of restitution */
	float StaticFriction;                  /* 0–1                         */
	float DynamicFriction;                 /* 0–1                         */
	float SolverTolerance;                 /* convergence threshold        */
	float MaxLinearVelocity;               /* m/s cap                      */
	float MaxAngularVelocity;              /* rad/s cap                    */
	int   CcdEnabled;                      /* continuous collision detect  */
	float CcdThreshold;                    /* velocity threshold for CCD   */
	int   CollisionGroupMask;              /* 32-bit bitmask               */
};

/* -------------------------------------------------------------------------
 *  CameraState — 3D viewport camera (§3.5)
 *  3Dビューポートカメラの状態ね。
 * ---------------------------------------------------------------------- */
struct CameraBookmark {
	float PosX, PosY, PosZ;
	float TargetX, TargetY, TargetZ;
	char  Name[32];
};

struct CameraState {
	float PosX, PosY, PosZ;            /* world-space eye position        */
	float TargetX, TargetY, TargetZ;   /* look-at point                   */
	float UpX, UpY, UpZ;               /* up vector (normally 0,1,0)      */
	float FovDeg;                       /* field of view (degrees)         */
	float NearPlane, FarPlane;          /* clip distances                  */
	float MoveSpeed;                    /* orbit/pan speed multiplier      */
	int   OrthoMode;                    /* 0=perspective 1=orthographic    */
	float OrthoWidth;                   /* orthographic half-width (m)     */
	int   AutoRotate;                   /* auto-orbit around target        */
	float AutoRotateSpeed;              /* degrees per second              */
	CameraBookmark Bookmarks[APP_MAX_CAMERA_BOOKMARKS];
	int   BookmarkCount;
};

/* -------------------------------------------------------------------------
 *  StatsState — per-frame simulation statistics (§3.14)
 *  フレームごとのシミュレーション統計ね。
 * ---------------------------------------------------------------------- */
struct StatsState {
	int   TotalBodies;
	int   ActiveBodies;
	int   SleepingBodies;
	int   StaticBodies;
	int   KinematicBodies;
	int   ConstraintCount;
	int   ContactCount;
	int   CollisionPairsPerFrame;
	size_t ArenaUsedBytes;
	size_t ArenaTotalBytes;
	float SimTimeMs;
	float BroadPhaseMs;
	float NarrowPhaseMs;
	float SolverMs;
	float FPS;
};

/* -------------------------------------------------------------------------
 *  InspectorState — selected body / multi-selection (§3.9–§3.10)
 *  選択済みボディ／複数選択の状態ね。
 * ---------------------------------------------------------------------- */
enum BodyTypeEnum {
	BodyTypeEnum_Static    = 0,
	BodyTypeEnum_Dynamic   = 1,
	BodyTypeEnum_Kinematic = 2
};

enum ShapeTypeEnum {
	ShapeType_None      = 0,
	ShapeType_Sphere    = 1,
	ShapeType_Box       = 2,
	ShapeType_Capsule   = 3,
	ShapeType_Cylinder  = 4,
	ShapeType_Cone      = 5,
	ShapeType_ConvexHull = 6,
	ShapeType_Mesh      = 7,
	ShapeType_Compound  = 8
};

struct InspectorBody {
	int64_t  BodyId;               /* -1 = no selection                   */
	float    PosX, PosY, PosZ;
	float    RotX, RotY, RotZ, RotW;   /* quaternion                     */
	float    LinVelX, LinVelY, LinVelZ;
	float    AngVelX, AngVelY, AngVelZ;
	float    Mass;
	float    InertiaXX, InertiaYY, InertiaZZ;
	float    Restitution;
	float    StaticFriction;
	float    DynamicFriction;
	int      Type;                 /* BodyTypeEnum                        */
	int      ShapeType;            /* ShapeTypeEnum                       */
	float    ShapeDimA;            /* radius / half-extent X              */
	float    ShapeDimB;            /* half-extent Y / half-height         */
	float    ShapeDimC;            /* half-extent Z                       */
	int      IsSleeping;
	int      CcdEnabled;
	uint32_t Flags;
};

struct InspectorState {
	int           SelectionCount;
	InspectorBody Primary;          /* primary selected body               */
	int64_t       SelectedIds[256]; /* all selected IDs                    */
};

/* -------------------------------------------------------------------------
 *  DemoState — demo launcher and scene management (§3.32)
 *  デモランチャーとシーン管理の状態ね。
 * ---------------------------------------------------------------------- */
enum DemoCategory {
	DemoCategory_All        = 0,
	DemoCategory_Rigid      = 1,
	DemoCategory_Soft       = 2,
	DemoCategory_Fluid      = 3,
	DemoCategory_Particles  = 4,
	DemoCategory_Constraints = 5,
	DemoCategory_Destruction = 6,
	DemoCategory_Vehicle    = 7,
	DemoCategory_Character  = 8,
	DemoCategory_Science    = 9,
	DemoCategory_Game       = 10,
	DemoCategory_Multi      = 11
};

struct DemoEntry {
	int   Id;
	char  Name[64];
	char  Description[256];
	int   Category;           /* DemoCategory                            */
	int   IsFavourite;
	float LastRunFPS;
	float LastRunMs;
};

struct DemoState {
	int       ActiveDemoId;        /* -1 = no demo loaded                 */
	int       LoadingDemo;         /* 1 while async load in progress      */
	float     LoadProgress;        /* 0.0–1.0                             */
	int       FilterCategory;      /* DemoCategory filter                 */
	char      SearchQuery[128];
	DemoEntry Entries[APP_MAX_DEMO_SCENES];
	int       EntryCount;
	int       BenchmarkMode;
	float     BenchmarkDurationSec;
	float     BenchmarkElapsedSec;
};

/* -------------------------------------------------------------------------
 *  ThemeState — visual theme tokens (§3.28)
 *  ビジュアルテーマトークンの状態ね。
 * ---------------------------------------------------------------------- */
enum ThemeMode {
	ThemeMode_Dark   = 0,
	ThemeMode_Light  = 1,
	ThemeMode_Custom = 2
};

struct ThemeState {
	int   Mode;              /* ThemeMode                               */
	float FontScale;         /* 0.75–2.0                                */
	float AnimationSpeed;    /* multiplier for all UI animations         */
	int   HighContrast;      /* 0=normal 1=high-contrast                */
	int   ColourBlindMode;   /* 0=off 1=protanopia 2=deuteranopia 3=tritanopia */
	/* Custom overrides (RGBA 0–1 each) */
	float AccentR, AccentG, AccentB;
	float BackgroundR, BackgroundG, BackgroundB;
};

/* -------------------------------------------------------------------------
 *  NotifQueue — toast notification ring buffer (§3.30)
 *  トースト通知リングバッファね。
 * ---------------------------------------------------------------------- */
enum NotifLevel {
	NotifLevel_Info    = 0,
	NotifLevel_Success = 1,
	NotifLevel_Warning = 2,
	NotifLevel_Error   = 3
};

struct Notification {
	int   Level;             /* NotifLevel                               */
	char  Message[256];
	float DurationSec;       /* 0 = persistent until dismissed           */
	float ElapsedSec;
	int   Dismissed;
};

struct NotifQueue {
	Notification Items[APP_MAX_NOTIF];
	int Write;               /* next write index (ring)                  */
	int Read;                /* next read index (ring)                   */
	int Count;
};

/* -------------------------------------------------------------------------
 *  PerfHistory — ring buffers for performance graphs (§3.15, §3.41)
 *  パフォーマンスグラフ用リングバッファね。
 * ---------------------------------------------------------------------- */
struct PerfHistory {
	float FPS[APP_PERF_HISTORY];
	float FrameMs[APP_PERF_HISTORY];
	float SimMs[APP_PERF_HISTORY];
	float BroadMs[APP_PERF_HISTORY];
	float NarrowMs[APP_PERF_HISTORY];
	float SolverMs[APP_PERF_HISTORY];
	float MemoryMB[APP_PERF_HISTORY];
	float CpuPct[APP_PERF_HISTORY];
	int   Head;              /* current write position (ring)            */
	int   Count;             /* number of valid samples                  */
};

/* -------------------------------------------------------------------------
 *  RecordState — simulation recording and playback (§3.42)
 *  シミュレーション録画と再生の状態ね。
 * ---------------------------------------------------------------------- */
enum RecordMode {
	RecordMode_Idle      = 0,
	RecordMode_Recording = 1,
	RecordMode_Playing   = 2,
	RecordMode_Paused    = 3
};

struct RecordState {
	int   Mode;              /* RecordMode                               */
	int   FrameCount;        /* total frames in recording                */
	int   CurrentFrame;      /* playback cursor                          */
	float PlaybackSpeed;     /* 1.0 = real-time, 0.25 = slow            */
	int   Loop;              /* 1 = loop playback                        */
	char  FilePath[512];
	int   HasAnnotations;
};

/* -------------------------------------------------------------------------
 *  PrefsState — user preferences (§3.31)
 *  ユーザー設定の状態ね。
 * ---------------------------------------------------------------------- */
struct PrefsState {
	/* General */
	int   Language;          /* 0=English 1=Japanese                    */
	int   AutoSaveEnabled;
	float AutoSaveIntervalSec;
	int   ConfirmOnReset;
	int   ShowSplashScreen;
	/* Viewport */
	int   ShowGrid;
	int   ShowAxis;
	int   ShowFpsCounter;
	int   ShowStatsOverlay;
	float GridSpacing;
	float GridOpacity;
	/* Rendering */
	int   ShadowQuality;     /* 0=off 1=low 2=med 3=high                */
	int   AntiAliasing;
	int   TextureQuality;    /* 0=low 1=med 2=high                       */
	float AmbientIntensity;
	float DirectionalIntensity;
	int   Fog;
	float FogDistance;
	int   EnvironmentMap;
	/* Physics */
	int   ShowContactPoints;
	int   ShowContactNormals;
	int   ShowVelocityVectors;
	int   ShowAngularVelocity;
	int   ShowAABBs;
	int   ShowBVH;
	int   ShowSleepColors;
	int   ShowJoints;
	int   ShowConstraintLimits;
	/* Performance */
	int   TargetFPS;         /* 0 = unlimited                           */
	int   VSyncEnabled;
	int   MaxBodiesWarning;  /* warn when body count exceeds this        */
	/* Accessibility */
	float TooltipDelayMs;
	int   KeyboardNavigation;
	int   FocusIndicator;
};

/* -------------------------------------------------------------------------
 *  PluginSlot — one loaded plugin (§3.36)
 *  ロード済みプラグインのスロットね。
 * ---------------------------------------------------------------------- */
struct PluginSlot {
	void *Handle;            /* dlopen handle                            */
	char  Name[64];
	char  Version[16];
	char  Description[256];
	int   Enabled;
};

struct PluginRegistry {
	PluginSlot Slots[APP_MAX_PLUGIN_SLOTS];
	int        Count;
};

/* -------------------------------------------------------------------------
 *  AppState — root state container
 *  ルート状態コンテナね。
 * ---------------------------------------------------------------------- */
struct AppState {
	SimState       Sim;
	PhysicsParams  Physics;
	CameraState    Camera;
	StatsState     Stats;
	InspectorState Inspector;
	DemoState      Demo;
	ThemeState     Theme;
	NotifQueue     Notif;
	PerfHistory    Perf;
	RecordState    Record;
	PrefsState     Prefs;
	PluginRegistry Plugins;

	/* Viewport dimensions (pixels) */
	int ViewportWidth;
	int ViewportHeight;

	/* Window state */
	int WindowWidth;
	int WindowHeight;
	int IsFullscreen;

	/* Scene file path currently loaded */
	char SceneFilePath[512];
	int  SceneDirty;       /* unsaved changes flag                      */
};

/* -------------------------------------------------------------------------
 *  AppState lifecycle functions
 *  AppState ライフサイクル関数ね。
 * ---------------------------------------------------------------------- */
void AppStateInit(AppState *S);
void AppStateApplyDefaults(AppState *S);
void AppStatePushNotification(AppState *S, int Level, const char *Msg, float DurationSec);
void AppStatePushPerfSample(AppState *S, float FPS, float FrameMs, float SimMs,
	float BroadMs, float NarrowMs, float SolverMs, float MemMB, float CpuPct);
void AppStateTickNotifications(AppState *S, float DeltaSec);
void AppStateTickCamera(AppState *S, float DeltaSec);
