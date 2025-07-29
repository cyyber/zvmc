# EVMC: Ethereum Client-VM Connector API.
# Copyright 2020 The EVMC Authors.
# Licensed under the Apache License, Version 2.0.

include(UseJava)

find_package(Java REQUIRED COMPONENTS Development)
find_package(JNI REQUIRED)

add_jar(qrvmc-jar
    OUTPUT_NAME qrvmc-java
    VERSION ${PROJECT_VERSION}
    SOURCES
    java/src/main/java/org/theqrl/qrvmc/QrvmcLoaderException.java
    java/src/main/java/org/theqrl/qrvmc/QrvmcVm.java
    java/src/main/java/org/theqrl/qrvmc/Host.java
    java/src/main/java/org/theqrl/qrvmc/HostContext.java
    GENERATE_NATIVE_HEADERS qrvmc-java-native-headers)

add_library(qrvmc-java SHARED)
target_sources(qrvmc-java PRIVATE
    c/qrvmc-vm.c
    c/host.c
    c/host.h)
target_include_directories(qrvmc-java PRIVATE ${JNI_INCLUDE_DIRS})
target_link_libraries(qrvmc-java PRIVATE qrvmc-java-native-headers qrvmc::qrvmc qrvmc::loader ${JNI_LIBRARIES})

if(QRVMC_INSTALL)
    install(TARGETS qrvmc-java EXPORT qrvmcTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    install_jar(qrvmc-jar DESTINATION ${CMAKE_INSTALL_DATADIR}/java)
endif()