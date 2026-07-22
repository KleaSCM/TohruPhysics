# Unit tests for TohruPhysics
# TohruPhysicsの単体テストね。

# KleaSCM: no exceptions, no RTTI, errors on warnings
add_compile_options(-fno-exceptions -fno-rtti)
add_compile_options(-Werror -Wno-unused-parameter)

add_executable(TestArena
	Source/TestArena.cpp
)
target_link_libraries(TestArena PRIVATE TohruPhysics)

add_executable(TestMath
	Source/TestMath.cpp
)
target_link_libraries(TestMath PRIVATE TohruPhysics)

add_executable(TestVector3
	Source/TestVector3.cpp
)
target_link_libraries(TestVector3 PRIVATE TohruPhysics)

add_executable(TestMatrix
	Source/TestMatrix.cpp
)
target_link_libraries(TestMatrix PRIVATE TohruPhysics)

add_executable(TestQuaternion
	Source/TestQuaternion.cpp
)
target_link_libraries(TestQuaternion PRIVATE TohruPhysics)

add_executable(TestArray
	Source/TestArray.cpp
)
target_link_libraries(TestArray PRIVATE TohruPhysics)

add_executable(TestTransform
	Source/TestTransform.cpp
)
target_link_libraries(TestTransform PRIVATE TohruPhysics)

add_executable(TestError
	Source/TestError.cpp
)
target_link_libraries(TestError PRIVATE TohruPhysics)

add_executable(TestLog
	Source/TestLog.cpp
)
target_link_libraries(TestLog PRIVATE TohruPhysics)

add_executable(TestGeometry
	Source/TestGeometry.cpp
)
target_link_libraries(TestGeometry PRIVATE TohruPhysics)

add_executable(TestBodyState
	Source/TestBodyState.cpp
)
target_link_libraries(TestBodyState PRIVATE TohruPhysics)

add_executable(TestGJK
	Source/TestGJK.cpp
)
target_link_libraries(TestGJK PRIVATE TohruPhysics)

add_executable(TestSAT
	Source/TestSAT.cpp
)
target_link_libraries(TestSAT PRIVATE TohruPhysics)

add_executable(TestContactManifold
	Source/TestContactManifold.cpp
)
target_link_libraries(TestContactManifold PRIVATE TohruPhysics)

add_executable(TestBroadPhase
	Source/TestBroadPhase.cpp
)
target_link_libraries(TestBroadPhase PRIVATE TohruPhysics)

add_executable(TestSpatialGrid
	Source/TestSpatialGrid.cpp
)
target_link_libraries(TestSpatialGrid PRIVATE TohruPhysics)

add_executable(TestThreadPool
	Source/TestThreadPool.cpp
)
target_link_libraries(TestThreadPool PRIVATE TohruPhysics)

add_executable(TestBVH
	Source/TestBVH.cpp
)
target_link_libraries(TestBVH PRIVATE TohruPhysics)

add_executable(TestSAP
	Source/TestSAP.cpp
)
target_link_libraries(TestSAP PRIVATE TohruPhysics)
