# Unit tests for TohruPhysics
# TohruPhysicsの単体テストね。
add_executable(TestArena
	Source/TestArena.cpp
)
target_link_libraries(TestArena PRIVATE TohruPhysics)
