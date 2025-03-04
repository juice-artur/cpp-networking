add_executable(Server 
    src/main.cpp
    src/DatabaseMigrator.h
    src/DatabaseMigrator.cpp
)

set_target_properties(Server PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

find_package(libpqxx REQUIRED)

target_link_libraries(Server libpqxx::pqxx)

