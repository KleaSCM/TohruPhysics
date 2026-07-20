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
