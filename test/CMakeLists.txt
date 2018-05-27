set(TestName BenchmarkTest)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${CONAN_INCLUDE_DIRS}
)

add_executable(
    ${TestName}
    benchmark.cpp
    main.cpp
)
    
target_link_libraries(
    ${TestName}
    ${CONAN_LIBS}
    -pthread
)

add_test(
    NAME ${TestName}
    COMMAND ${TestName} --gtest_output=xml:${CMAKE_BINARY_DIR}/Testing/${TestName}.xml
)

