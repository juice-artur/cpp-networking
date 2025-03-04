#pragma once

#include <string>
#include <pqxx/pqxx>


class DatabaseMigrator {
 public:
  explicit DatabaseMigrator(const std::string& conn_str);

  void InitMigrationsTable();

 private: 
      pqxx::connection conn;

      const std::string migrationsDir = "migrations";
};