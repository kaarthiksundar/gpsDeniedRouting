include_directories(${PROJECT_SOURCE_DIR}/include)

set(CMAKE_BINARY_DIR "${PROJECT_SOURCE_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_executable(main main.cpp utilities.cpp) 

if (APPLE)
    target_link_libraries(main ${CPLEX_LIBRARIES})
else()
    target_link_libraries(main ${CPLEX_LIBRARIES} ${LEMON_LIBRARIES})
endif()
