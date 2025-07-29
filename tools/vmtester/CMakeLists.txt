include(GNUInstallDirs)

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

# Disable support for std::tr1::tuple in GTest. This causes problems in Visual Studio 2015.
set_target_properties(GTest::gtest PROPERTIES INTERFACE_COMPILE_DEFINITIONS GTEST_HAS_TR1_TUPLE=0)

add_executable(qrvmc-vmtester vmtester.hpp vmtester.cpp tests.cpp)
target_link_libraries(qrvmc-vmtester PRIVATE qrvmc::loader qrvmc::mocked_host GTest::gtest)
set_source_files_properties(vmtester.cpp PROPERTIES COMPILE_DEFINITIONS PROJECT_VERSION="${PROJECT_VERSION}")
add_executable(qrvmc::qrvmc-vmtester ALIAS qrvmc-vmtester)

if(QRVMC_INSTALL)
    install(TARGETS qrvmc-vmtester EXPORT qrvmcTargets RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
