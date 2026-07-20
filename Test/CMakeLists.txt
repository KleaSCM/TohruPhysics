# Unit tests for TohruPhysics
# TohruPhysicsの単体テストね。
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
