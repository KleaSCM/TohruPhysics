# Demo applications for TohruPhysics.
# TohruPhysicsのデモアプリケーションよ。

add_executable(DemoPhysics
	Source/Main.cpp
)
target_link_libraries(DemoPhysics PRIVATE TohruPhysics)
