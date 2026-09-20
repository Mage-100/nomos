add_executable(nomos main.cpp)

target_include_directories(nomos PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_sources(nomos PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/MPSParser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/LPSolver.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/LPSolverBuilder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BasisFactorizationEngine.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utility.cpp"
)

target_link_libraries(nomos PRIVATE Eigen3::Eigen ftxui::ftxui)

target_compile_definitions(nomos PRIVATE
    DATASET_DIR=\"${_DATASET_DIR}\"
)

target_compile_features(nomos PRIVATE
    cxx_std_23
)

set_target_properties(nomos PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_compile_options(nomos PRIVATE
    /W4
)
