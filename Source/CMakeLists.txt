add_library(TohruPhysics STATIC)

# KleaSCM: no exceptions, no RTTI, errors on warnings
target_compile_options(TohruPhysics PRIVATE
	-fno-exceptions -fno-rtti
	-Werror -Wno-unused-parameter
)

target_include_directories(TohruPhysics
	PUBLIC
		${PROJECT_SOURCE_DIR}/Include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Arena subsystem
target_sources(TohruPhysics PRIVATE
	Arena/Arena.cpp
)

# Math subsystem — scalar foundations
target_sources(TohruPhysics PRIVATE
	Math/Math.cpp
)

# Vector subsystem
target_sources(TohruPhysics PRIVATE
	Vector3/Vector3.cpp
)

# Matrix subsystem
target_sources(TohruPhysics PRIVATE
	Matrix/Matrix.cpp
)

# Quaternion subsystem
target_sources(TohruPhysics PRIVATE
	Quaternion/Quaternion.cpp
)

# Array subsystem
target_sources(TohruPhysics PRIVATE
	Array/Array.cpp
)

# Transform subsystem
target_sources(TohruPhysics PRIVATE
	Transform/Transform.cpp
)

# Error subsystem
target_sources(TohruPhysics PRIVATE
	Error/Error.cpp
)

# Log subsystem
target_sources(TohruPhysics PRIVATE
	Log/Log.cpp
)

# Geometry subsystem
target_sources(TohruPhysics PRIVATE
	Geometry/Geometry.cpp
)

# BodyState subsystem
target_sources(TohruPhysics PRIVATE
	BodyState/BodyState.cpp
)

# GJK subsystem
target_sources(TohruPhysics PRIVATE
	GJK/GJK.cpp
)

# SAT subsystem
target_sources(TohruPhysics PRIVATE
	SAT/SAT.cpp
)

# Contact Manifold subsystem
target_sources(TohruPhysics PRIVATE
	Collision/ContactManifold.cpp
)

# BroadPhase subsystem
target_sources(TohruPhysics PRIVATE
	BroadPhase/BroadPhase.cpp
)
