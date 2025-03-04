#include "DatabaseMigrator.h"


DatabaseMigrator::DatabaseMigrator(const std::string& connStr)
    : conn(connStr) {
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
