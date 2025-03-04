add_executable(Server 
    src/main.cpp
    src/DatabaseMigrator.h
    src/DatabaseMigrator.cpp
)

find_package(libpqxx REQUIRED)

target_link_libraries(Server libpqxx::pqxx)

