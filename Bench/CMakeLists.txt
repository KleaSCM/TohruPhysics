# Benchmarks for TohruPhysics.
# TohruPhysicsのベンチマークよ。

option(TP_BUILD_BENCHMARKS "Build benchmarks" OFF)

if(TP_BUILD_BENCHMARKS)
	add_executable(BenchCore
		Source/BenchCore.cpp
	)
	target_link_libraries(BenchCore PRIVATE TohruPhysics)
endif()
