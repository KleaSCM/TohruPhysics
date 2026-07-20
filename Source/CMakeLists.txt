add_library(TohruPhysics STATIC)

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
