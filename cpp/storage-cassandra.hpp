#pragma once

#ifndef STORAGE_CASSANDRA_HPP__
#define STORAGE_CASSANDRA_HPP__

#include <cassandra.h>

#include "storage.hpp"
#include "basetypes.hpp"
#include "scope-exit.hpp"

namespace storage
{

class cassandra : public base::storage_interface
{
    CassCluster* cluster_ = nullptr;
    CassSession* session_ = nullptr;
    CassFuture* connect_future_ = nullptr;
public:
    cassandra()
    {
        state_.optimal_size = 1024;
        state_.priority = 0;

        char const * hosts = "192.168.2.25";
        cluster_ = cass_cluster_new();
        session_ = cass_session_new();
        cass_cluster_set_contact_points(cluster_, hosts);
    }

    ~cassandra()
    {
        cass_future_free(connect_future_);

        CassFuture* close_future = cass_session_close(session_);
        cass_future_wait(close_future);
        cass_future_free(close_future);

        cass_session_free(session_);
        cass_cluster_free(cluster_);
    }

    void connect() override
    {
        connect_future_ = cass_session_connect(session_, cluster_);
    }

    template<typename Callback> // Callback(CassResult const*)
    void run_query(CassStatement * statement, Callback callback)
    {
        if (cass_future_error_code(connect_future_) == CASS_OK)
        {
            CassFuture* result_future = cass_session_execute(session_, statement);
            SCOPE_DEFER([&result_future]() { cass_future_free(result_future); });

            CassResult const * result = cass_future_get_result(result_future);
            SCOPE_DEFER([&result]() { cass_result_free(result); });

            std::invoke(callback, result);
        }
    }

    void create_volume(std::uint32_t const size, std::string const& name) override
    {

        //CREATE TABLE functionkv.tableA (key int, value text, PRIMARY KEY (key));
    }

    auto read_block(std::uint32_t const offset) -> base::buf override
    {
        char const* query = "SELECT value FROM functionkv.tableA WHERE key=?";

        CassStatement* statement = cass_statement_new(query, 1);
        SCOPE_DEFER([&statement]() { cass_statement_free(statement); });

        cass_statement_bind_int32(statement, 0, offset);

        base::buf buf;
        run_query(statement,
                  [&buf] (CassResult const* result) {
                      CassRow const * row = cass_result_first_row(result);
                      if (row)
                      {
                          CassValue const * value = cass_row_get_column_by_name(row, "value");

                          const char* block = nullptr;
                          size_t block_length;
                          cass_value_get_string(value, &block, &block_length);
                          std::string s(block, block_length);

                          buf = base::decode(s);
                      }
                  });
        return buf;
    }

    void write_block(std::uint32_t const offset, base::buf const& buffer) override
    {
        char const* query = "INSERT INTO functionkv.tableA (key, value) VALUES (?, ?);";

        CassStatement* statement = cass_statement_new(query, 2);
        SCOPE_DEFER([&statement]() { cass_statement_free(statement); });

        std::string v = base::encode(buffer);
        cass_statement_bind_int32(statement, 0, offset);
        cass_statement_bind_string(statement, 1, v.c_str());

        run_query(statement, [] (CassResult const* result) {});
    }
};

} // namespace storage

#endif // STORAGE_CASSANDRA_HPP__
