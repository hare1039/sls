#pragma once

#ifndef SLSFS_STORAGE_CASSANDRA_HPP__
#define SLSFS_STORAGE_CASSANDRA_HPP__

#include "storage.hpp"
#include "basetypes.hpp"
#include "scope-exit.hpp"

#include <cassandra.h>
#include <vector>

namespace slsfs::storage
{

class cassandra : public base::storage_interface
{
    CassCluster* cluster_ = nullptr;
    CassSession* session_ = nullptr;
    CassFuture* connect_future_ = nullptr;
public:
    cassandra(char const * hosts)
    {
        state_.optimal_size = 1024;
        state_.priority = 0;

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

                          char const * block = nullptr;
                          std::size_t block_length;
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

    auto read_key(std::string const name) -> base::buf override
    {
        char const* query = "SELECT value FROM functionkv.tableB WHERE key=?";

        CassStatement* statement = cass_statement_new(query, 1);
        SCOPE_DEFER([&statement]() { cass_statement_free(statement); });

        cass_statement_bind_string(statement, 0, name.c_str());

        base::buf buf;
        run_query(statement,
                  [&buf] (CassResult const* result) {
                      CassRow const * row = cass_result_first_row(result);
                      if (row)
                      {
                          CassValue const * value = cass_row_get_column_by_name(row, "value");

                          char const * block = nullptr;
                          std::size_t block_length;
                          cass_value_get_string(value, &block, &block_length);
                          std::string s(block, block_length);

                          buf = base::decode(s);
                      }
                  });
        return buf;
    }

    void write_key(std::string const name, base::buf const& buffer) override
    {
        char const* query = "INSERT INTO functionkv.tableB (key, value) VALUES (?, ?);";

        CassStatement* statement = cass_statement_new(query, 2);
        SCOPE_DEFER([&statement]() { cass_statement_free(statement); });

        std::string v = base::encode(buffer);
        cass_statement_bind_string(statement, 0, name.c_str());
        cass_statement_bind_string(statement, 1, v.c_str());

        run_query(statement, [] (CassResult const* result) {});
    }

    void append_list_key(std::string const name, base::buf const& buffer) override
    {
        // CREATE TABLE functionkv.tableC (key text, value list<text>, PRIMARY KEY (key));
        char const* query = "UPDATE functionkv.tableC SET value = value + ? WHERE key=?;";

        CassStatement* statement = cass_statement_new(query, 2);
        SCOPE_DEFER([&statement]() { cass_statement_free(statement); });

        std::string const buffer_encoded = base::encode(buffer);

        CassCollection* list = cass_collection_new(CASS_COLLECTION_TYPE_LIST, 1);
        SCOPE_DEFER([&list] { cass_collection_free(list); });
        cass_collection_append_string(list, buffer_encoded.c_str());

        cass_statement_bind_collection(statement, 0, list);
        cass_statement_bind_string(statement, 1, name.c_str());

        run_query(statement, [] (CassResult const*) {});
    }

    void merge_list_key(std::string const name, std::function<void(std::vector<base::buf> const&)> reduce) override
    {
        char const* query = "SELECT value FROM functionkv.tableC WHERE key=?";

        CassStatement* statement = cass_statement_new(query, 1);
        SCOPE_DEFER([&statement]() { cass_statement_free(statement); });

        cass_statement_bind_string(statement, 0, name.c_str());

        run_query(statement,
                  [&reduce] (CassResult const* result) {
                      CassRow const * row = cass_result_first_row(result);
                      if (row)
                      {
                          CassValue const * value = cass_row_get_column_by_name(row, "value");
                          CassIterator * iter = cass_iterator_from_collection(value);
                          SCOPE_DEFER([&iter]{ cass_iterator_free(iter); });

                          std::vector<base::buf> items;

                          while (cass_iterator_next(iter))
                          {
                              CassValue const * v = cass_iterator_get_value(iter);
                              char const * block = nullptr;
                              std::size_t block_length;
                              cass_value_get_string(v, &block, &block_length);
                              std::string s(block, block_length);

                              items.push_back(base::decode(s));
                          }
                          std::invoke(reduce, items);
                      }
                  });
    }

    auto  get_list_key(std::string const name) -> base::buf override
    {
        return {};
    }
};

} // namespace storage

#endif // SLSFS_STORAGE_CASSANDRA_HPP__
