
set(TEST_EXECUTABLE_NAME "qnet_test")

add_executable(${TEST_EXECUTABLE_NAME} main.cpp)

target_link_libraries(qnet_test PRIVATE
    quicknet
)