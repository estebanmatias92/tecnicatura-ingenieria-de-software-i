/**
 * @file sqlite_component.cpp
 * @author Gabriel Ferreira (gabrielinuz@fi.mdp.edu.ar)
 * @brief Implementación concreta de un componente manejador de base de datos Sqlite.
 * @version 0.1
 * @date 2026-06-16
 * 
 * @copyright Copyright (c) 2026 Released under the MIT license
 * @link https://opensource.org/licenses/MIT @endlink
 * 
 */

#ifndef SQLITE_COMPONENT_HPP
#define SQLITE_COMPONENT_HPP

#include "i_database.hpp"
#include "sqlite3.h"
#include <iostream>

class SqliteComponent : public IDatabase 
{
    private:
        sqlite3* db_{nullptr};

        // Método auxiliar para preparar la sentencia y enlazar variables
        sqlite3_stmt* prepare_and_bind(const std::string& query, const std::vector<std::string>& params) 
        {
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
            {
                return nullptr;
            }

            for (size_t i = 0; i < params.size(); ++i) 
            {
                // SQLITE_TRANSIENT le dice a SQLite que haga su propia copia del string
                sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
            }
            return stmt;
        }

    public:
        SqliteComponent() = default;
        
        ~SqliteComponent() noexcept override 
        {
            disconnect();
        }

        ComponentResult connect(const std::string& connection_string) noexcept override 
        {
            if (db_) return ComponentResult::ERROR_INTERNAL;

            if (sqlite3_open(connection_string.c_str(), &db_) != SQLITE_OK) 
            {
                return ComponentResult::ERROR_INTERNAL;
            }

            // Mitigación de Concurrencia: Esperar hasta 5000ms si hay bloqueos de escritura concurrentes
            sqlite3_busy_timeout(db_, 5000);

            // Optimización Arquitectónica: Activar Write-Ahead Logging para lecturas no bloqueantes
            char* err_msg = nullptr;
            sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, &err_msg);
            if (err_msg) sqlite3_free(err_msg);

            return ComponentResult::SUCCESS;
        }

        void disconnect() noexcept override 
        {
            if (db_) 
            {
                sqlite3_close_v2(db_);
                db_ = nullptr;
            }
        }

        ComponentResult execute(const std::string& query, const std::vector<std::string>& params) noexcept override 
        {
            if (!db_) return ComponentResult::ERROR_INTERNAL;

            sqlite3_stmt* stmt = prepare_and_bind(query, params);
            if (!stmt) return ComponentResult::ERROR_INVALID_ARGUMENT;

            ComponentResult result = ComponentResult::SUCCESS;
            if (sqlite3_step(stmt) != SQLITE_DONE) 
            {
                result = ComponentResult::ERROR_INTERNAL;
            }

            sqlite3_finalize(stmt);
            return result;
        }

        ComponentResult query(const std::string& query, const std::vector<std::string>& params, ResultSet& out_result) noexcept override 
        {
            if (!db_) return ComponentResult::ERROR_INTERNAL;

            sqlite3_stmt* stmt = prepare_and_bind(query, params);
            if (!stmt) return ComponentResult::ERROR_INVALID_ARGUMENT;

            out_result.clear();

            while (sqlite3_step(stmt) == SQLITE_ROW) 
            {
                DataRow row;
                int column_count = sqlite3_column_count(stmt);
                for (int i = 0; i < column_count; ++i) 
                {
                    const char* col_name = sqlite3_column_name(stmt, i);
                    const char* col_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    row[col_name] = col_value ? col_value : "null";
                }
                out_result.push_back(std::move(row));
            }

            sqlite3_finalize(stmt);
            return ComponentResult::SUCCESS;
        }
};

// --- EXPORTACIÓN DE C-API ---
extern "C" int get_api_version() noexcept { return CURRENT_API_VERSION; }
extern "C" IComponent* create_component() noexcept { try { return new SqliteComponent(); } catch (...) { return nullptr; } }
extern "C" void destroy_component(IComponent* instance) noexcept { delete instance; }

#endif