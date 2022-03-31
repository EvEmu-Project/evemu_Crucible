cmake_minimum_required(VERSION 2.8.12)

project(OdbCMake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(ODB REQUIRED COMPONENTS qt sqlite OPTIONAL_COMPONENTS mysql pgsql)
include(${ODB_USE_FILE})

set(OdbCMake_SOURCES
	driver.cpp
	database.h)

set(OdbCMake_ODB_HEADERS
	person.h)

odb_compile(OdbCMake_SOURCES FILES ${OdbCMake_ODB_HEADERS} DB sqlite GENERATE_QUERY GENERATE_SESSION)

add_executable(odbcmake
	${OdbCMake_SOURCES}
	${OdbCMake_ODB_HEADERS})
target_link_libraries(odbcmake
	${ODB_LIBRARIES})
target_include_directories(odbcmake
	PRIVATE
		${ODB_INCLUDE_DIRS}
		${ODB_COMPILE_OUTPUT_DIR})
target_compile_definitions(odbcmake
	PRIVATE
		DATABASE_SQLITE)
