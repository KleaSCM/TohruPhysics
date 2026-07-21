# Demo applications for TohruPhysics.
# TohruPhysicsのデモアプリケーションよ。

# KleaSCM: no exceptions, no RTTI, errors on warnings
add_compile_options(-fno-exceptions -fno-rtti)
add_compile_options(-Werror -Wno-unused-parameter)

add_executable(DemoPhysics
	Source/Main.cpp
)
target_link_libraries(DemoPhysics PRIVATE TohruPhysics)
