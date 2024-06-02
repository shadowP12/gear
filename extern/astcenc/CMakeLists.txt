cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

project(astcenc)

add_library(astcenc STATIC
    astcenc_averages_and_directions.cpp
    astcenc_block_sizes.cpp
    astcenc_color_quantize.cpp
    astcenc_color_unquantize.cpp
    astcenc_compress_symbolic.cpp
    astcenc_compute_variance.cpp
    astcenc_decompress_symbolic.cpp
    astcenc_diagnostic_trace.cpp
    astcenc_entry.cpp
    astcenc_find_best_partitioning.cpp
    astcenc_ideal_endpoints_and_weights.cpp
    astcenc_image.cpp
    astcenc_integer_sequence.cpp
    astcenc_mathlib.cpp
    astcenc_mathlib_softfloat.cpp
    astcenc_partition_tables.cpp
    astcenc_percentile_tables.cpp
    astcenc_pick_best_endpoint_format.cpp
    astcenc_quantization.cpp
    astcenc_symbolic_physical.cpp
    astcenc_weight_align.cpp
    astcenc_weight_quant_xfer_tables.cpp)

target_include_directories(astcenc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})