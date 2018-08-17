cmake_minimum_required(VERSION 3.11)

project("cooperativeLocalization")

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# Find Gurobi (optional)
option(ENABLE_GUROBI "Link to Gurobi libraries" OFF)

# Find CPLEX (optional)
option(ENABLE_CPLEX "Link to Cplex libraries" ON)

FIND_PACKAGE(lemon)

# Find GUROBI?
if (ENABLE_GUROBI)
    message(STATUS "Enable GUROBI")
    add_definitions(-DUSE_GUROBI)
    find_package(GUROBI)
    include_directories(${GUROBI_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${GUROBI_LIBRARIES})
endif()

# Find CPLEX?
if (ENABLE_CPLEX)
    message(STATUS "Enable CPLEX")
    add_definitions(-DUSE_CPLEX)
    find_package(CPLEX)
    include_directories(${CPLEX_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${CPLEX_LIBRARIES})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DNDEBUG -DIL_STD")
endif()

add_subdirectory(src)