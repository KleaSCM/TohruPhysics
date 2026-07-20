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

# Math subsystem (coming next)
# target_sources(TohruPhysics PRIVATE
# 	Math/Math.cpp
# )
