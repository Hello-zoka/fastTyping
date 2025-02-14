cmake_minimum_required(VERSION 3.0)
project(fasttyping_server)

set(CMAKE_CXX_STANDARD 17)
find_package(Boost 1.71 REQUIRED system log locale filesystem)
find_package(nlohmann_json 3.7.3 REQUIRED)
find_package(Threads)
include(FetchContent)
set(SKIP_BUILD_TEST on)
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
        libpqxx
        GIT_REPOSITORY https://github.com/jtv/libpqxx.git
        GIT_TAG a6b1d60e74c1427c8ac2324b85cd4a0dc2068332
)
set(PQXX_LIBRARIES pqxx_static)

FetchContent_MakeAvailable(libpqxx)


include_directories(include ../common/include)

add_executable(server src/main.cpp
        src/generator.cpp
        src/user.cpp
        src/database.cpp
        src/server.cpp
        src/game.cpp
        include/user.h
        include/database.h
        src/dictionaries.cpp
        include/dictionaries.h
        src/dictionaryDB.cpp
        src/statisticsDB.cpp
        include/statisticsDB.h)
target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS} include)
target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} ${NETWORKING_LIBS} ${PQXX_LIBRARIES})
target_link_libraries(server nlohmann_json::nlohmann_json)

add_executable(server-tests src/database.cpp
        src/doctest_main.cpp
        src/databaseTests.cpp
        src/user.cpp
        src/game.cpp
        src/dictionaries.cpp
        include/dictionaries.h
        include/database.h
        src/dictionaryDB.cpp
        src/statisticsDB.cpp
        include/statisticsDB.h)
target_include_directories(server-tests PRIVATE ${Boost_INCLUDE_DIRS} include)
target_link_libraries(server-tests ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${NETWORKING_LIBS} ${PQXX_LIBRARIES})

add_executable(generator-tests src/generator.cpp
        src/doctest_main.cpp
        src/generatorTests.cpp)
target_include_directories(generator-tests PRIVATE ${Boost_INCLUDE_DIRS} include)
target_link_libraries(generator-tests ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${NETWORKING_LIBS} ${PQXX_LIBRARIES})

add_executable(dictionaries-interface src/dictionaryDB.cpp
        src/dictionaries.cpp
        include/dictionaries.h
        src/dictionary-interface.cpp
        src/database.cpp
        include/database.h)
target_include_directories(dictionaries-interface PRIVATE ${Boost_INCLUDE_DIRS} include)
target_link_libraries(dictionaries-interface ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${NETWORKING_LIBS} ${PQXX_LIBRARIES})


add_library(test SHARED src/test_so.cpp)
set_target_properties(server PROPERTIES PREFIX "fastytpign_")
install(TARGETS server
        DESTINATION bin
        )