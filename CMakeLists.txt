cmake_minimum_required(VERSION 3.11)

project("cooperativeLocalization")

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# Find Gurobi (optional)
option(ENABLE_GUROBI "Link to Gurobi libraries" OFF)

# Find CPLEX (optional)
option(ENABLE_CPLEX "Link to Cplex libraries" ON)

# LEMON settings
set(LEMON_INCLUDE_DIR "/home/kaarthik/libraries/lemon/include" CACHE PATH "LEMON include directory")
set(LEMON_INCLUDE_DIRS "${LEMON_INCLUDE_DIR}")

if (UNIX)
    set(LEMON_LIB_NAME "libemon.a")
endif()

set(LEMON_LIBRARY "/home/kaarthik/libraries/lemon/lib/${LEMON_LIB_NAME}" CACHE FILEPATH "LEMON library")
set(LEMON_LIBRARIES "${LEMON_LIBRARY}")

MARK_AS_ADVANCED(LEMON_LIBRARY LEMON_INCLUDE_DIR)
include_directories(${LEMON_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LEMON_LIBRARIES})

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -O -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD -lpthread")
endif()

add_subdirectory(src)
