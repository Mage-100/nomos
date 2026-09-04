add_executable(nomos main.cpp)

target_include_directories(nomos PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_sources(nomos PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/MPSParser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/LPSolver.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BuildSolver.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BasisFactorizationEngine.cpp"
)

target_link_libraries(nomos PRIVATE Eigen3::Eigen)

target_compile_definitions(nomos PRIVATE
    DATASET_DIR=\"${_DATASET_DIR}\"
)
