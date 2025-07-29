hunter_add_package(CLI11)
find_package(CLI11 REQUIRED)

add_executable(qrvmc-tool main.cpp)
add_executable(qrvmc::tool ALIAS qrvmc-tool)
set_target_properties(qrvmc-tool PROPERTIES OUTPUT_NAME qrvmc)
set_source_files_properties(main.cpp PROPERTIES
    COMPILE_DEFINITIONS PROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(qrvmc-tool PRIVATE qrvmc::tooling qrvmc::loader CLI11::CLI11)
