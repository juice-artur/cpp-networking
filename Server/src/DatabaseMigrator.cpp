#include "DatabaseMigrator.h"

#include <filesystem>
#include <fstream>
#include <iostream>

static std::string readSqlFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) throw std::runtime_error("Cannot open file: " + path);
  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

DatabaseMigrator::DatabaseMigrator(const std::string& connStr) : conn(connStr) {
  InitMigrationsTable();
}

void DatabaseMigrator::InitMigrationsTable() {
  pqxx::work txn(conn);
  txn.exec(R"(
            CREATE TABLE IF NOT EXISTS migrations (
                id SERIAL PRIMARY KEY,
                name TEXT UNIQUE NOT NULL,
                applied_at TIMESTAMP DEFAULT NOW()
            )
        )");
  txn.commit();
}

void DatabaseMigrator::ApplyMigrations() {
  namespace fs = std::filesystem;

  pqxx::work txn(conn);

  pqxx::result appliedMigrations = txn.exec("SELECT name FROM migrations");
  std::vector<std::string> applied;
  for (const auto& row : appliedMigrations) {
    applied.push_back(row[0].as<std::string>());
  }

  for (const auto& entry : fs::directory_iterator(migrationsDir)) {
    std::string filename = entry.path().filename().string();
    if (std::find(applied.begin(), applied.end(), filename) != applied.end()) {
      std::cout << "Skipping applied migration: " << filename << std::endl;
      continue;
    }

    std::cout << "Applying migration: " << filename << std::endl;
    std::string sql = readSqlFile(entry.path().string());

    try {
      txn.exec(sql);
      txn.exec("INSERT INTO migrations (name) VALUES ('" + txn.esc(filename) +
               "')");
      txn.commit();
      std::cout << "Migration applied successfully!\n";
    } catch (const std::exception& e) {
      std::cerr << "Error applying migration " << filename << ": " << e.what()
                << std::endl;
    }
  }
}