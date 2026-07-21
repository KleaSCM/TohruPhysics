# TohruPhysics GUI — Slint 1.17.1 + OpenGL 3.3 application.
# TohruPhysicsのGUIアプリケーション — Slint 1.17.1とOpenGL 3.3ね。
#
# Fetches Slint via FetchContent. GLAD 2 is committed locally under Third/glad/
# so no internet access is required beyond the initial configure step.
#
# Targets produced:
#   TohruPhysicsGui       — main windowed application
#   TohruPhysicsHeadless  — headless demo runner for automated testing

cmake_minimum_required(VERSION 3.28)

# ---------------------------------------------------------------------------
#  Slint — fetched at configure time
#  Slintをコンフィグ時に取得するね。
# ---------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
	Slint
	GIT_REPOSITORY https://github.com/slint-ui/slint.git
	GIT_TAG        v1.17.1
	SOURCE_SUBDIR  api/cpp
)

set(SLINT_FEATURE_RENDERER_WINIT ON  CACHE BOOL "" FORCE)
set(SLINT_FEATURE_BACKEND_WINIT  ON  CACHE BOOL "" FORCE)
set(SLINT_FEATURE_BACKEND_QT     OFF CACHE BOOL "" FORCE)
set(SLINT_FEATURE_INTERPRETER    OFF CACHE BOOL "" FORCE)
set(SLINT_BUILD_TESTING          OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS            OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(Slint)

# ---------------------------------------------------------------------------
#  GLAD — OpenGL 3.3 core profile, committed to Third/glad/
#  GLAD — OpenGL 3.3コアプロファイル、Third/glad/に収録済みね。
# ---------------------------------------------------------------------------
add_library(TpGlad STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/Third/glad/glad.c
)
target_include_directories(TpGlad PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/Third
)
# GLAD uses dlopen on Linux
target_link_libraries(TpGlad PUBLIC ${CMAKE_DL_LIBS})

# ---------------------------------------------------------------------------
#  Slint UI files — all .slint compiled to C++ headers
#  SlintのUIファイルをC++ヘッダーにコンパイルするの。
# ---------------------------------------------------------------------------
set(TP_SLINT_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/Theme/Tokens.slint
	${CMAKE_CURRENT_SOURCE_DIR}/Theme/Icons.slint
	${CMAKE_CURRENT_SOURCE_DIR}/Window/MainWindow.slint
	${CMAKE_CURRENT_SOURCE_DIR}/Controls/SimControls.slint
	${CMAKE_CURRENT_SOURCE_DIR}/Controls/PhysicsPanel.slint
	${CMAKE_CURRENT_SOURCE_DIR}/Controls/CameraPanel.slint
	${CMAKE_CURRENT_SOURCE_DIR}/Controls/StatsPanel.slint
	${CMAKE_CURRENT_SOURCE_DIR}/Inspector/BodyInspector.slint
	${CMAKE_CURRENT_SOURCE_DIR}/Demo/DemoLauncher.slint
	${CMAKE_CURRENT_SOURCE_DIR}/Notification/ToastSystem.slint
	${CMAKE_CURRENT_SOURCE_DIR}/Perf/PerfOverlay.slint
)

slint_target_sources(TohruPhysicsGuiSlint ${TP_SLINT_FILES})

# ---------------------------------------------------------------------------
#  Main GUI sources
#  メインGUIのソースファイルね。
# ---------------------------------------------------------------------------
set(TP_GUI_SOURCES
	# App
	App/AppMain.cpp
	App/AppState.cpp

	# Window bridge
	Window/MainWindowBridge.cpp

	# Controls bridges
	Controls/SimControlsBridge.cpp
	Controls/PhysicsPanelBridge.cpp
	Controls/CameraPanelBridge.cpp
	Controls/StatsPanelBridge.cpp

	# Viewport
	Viewport/ViewportHost.cpp
	Viewport/GlPipeline.cpp
	Viewport/DebugRenderer.cpp
	Viewport/GridRenderer.cpp

	# Inspector bridge
	Inspector/InspectorBridge.cpp

	# Scene management
	Scene/SceneManager.cpp
	Scene/SceneSerializer.cpp
	Scene/SceneJson.cpp

	# Demo launcher + scenes
	Demo/DemoLauncherBridge.cpp
	Demo/WaterSimDemo.cpp
	Demo/RigidStackDemo.cpp
	Demo/SoftBodyDemo.cpp
	Demo/FluidDemo.cpp
	Demo/ConstraintsDemo.cpp
	Demo/ParticleDemo.cpp
	Demo/DestructionDemo.cpp
	Demo/VehicleDemo.cpp
	Demo/CharacterDemo.cpp
	Demo/MultiPhysicsDemo.cpp
	Demo/ScientificDemo.cpp

	# Notifications
	Notification/ToastBridge.cpp

	# Performance overlay
	Perf/PerfBridge.cpp

	# Recording / playback
	Record/SimRecorder.cpp
)

# ---------------------------------------------------------------------------
#  TohruPhysicsGui — main windowed executable
# ---------------------------------------------------------------------------
add_executable(TohruPhysicsGui
	${TP_GUI_SOURCES}
)

target_include_directories(TohruPhysicsGui PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/Include
)

target_link_libraries(TohruPhysicsGui PRIVATE
	TohruPhysics
	Slint::Slint
	TpGlad
	OpenGL::GL
)

# Shader files are embedded as string literals via a CMake-generated header.
# シェーダーファイルはCMake生成のヘッダーに文字列として埋め込むね。
target_compile_definitions(TohruPhysicsGui PRIVATE
	TP_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Viewport/Shaders"
)

find_package(OpenGL REQUIRED)

# Slint compile_slint generates the Slint C++ glue; link it into the target.
# slint_target_sources must be called on a concrete target.
slint_target_sources(TohruPhysicsGui ${TP_SLINT_FILES})

# ---------------------------------------------------------------------------
#  TohruGuiHeadless — automated testing runner (no display required)
#  自動テスト用ランナー（ディスプレイ不要）ね。
# ---------------------------------------------------------------------------
add_executable(TohruGuiHeadless
	App/HeadlessMain.cpp
	App/AppState.cpp
	Scene/SceneManager.cpp
	Scene/SceneSerializer.cpp
	Scene/SceneJson.cpp
	Demo/WaterSimDemo.cpp
	Demo/RigidStackDemo.cpp
	Demo/SoftBodyDemo.cpp
	Demo/FluidDemo.cpp
	Demo/ConstraintsDemo.cpp
	Demo/ParticleDemo.cpp
	Demo/DestructionDemo.cpp
	Demo/VehicleDemo.cpp
	Demo/CharacterDemo.cpp
	Demo/MultiPhysicsDemo.cpp
	Demo/ScientificDemo.cpp
	Record/SimRecorder.cpp
)

target_include_directories(TohruGuiHeadless PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/Include
)

target_link_libraries(TohruGuiHeadless PRIVATE
	TohruPhysics
)

# ---------------------------------------------------------------------------
#  Install rules (§3.43)
#  インストールルールね。
# ---------------------------------------------------------------------------
install(TARGETS TohruPhysicsGui
	RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Viewport/Shaders/
	DESTINATION share/TohruPhysics/Shaders
	FILES_MATCHING PATTERN "*.vert" PATTERN "*.frag"
)
