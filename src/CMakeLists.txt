add_executable(nomos main.cpp)

target_include_directories(nomos PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_sources(nomos PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/MPSParser.cpp"
)

target_compile_definitions(nomos PRIVATE
    DATASET_DIR=\"${_DATASET_DIR}\"
)
