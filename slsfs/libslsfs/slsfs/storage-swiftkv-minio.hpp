#pragma once

#ifndef SLSFS_STORAGE_SWIFTKV_HPP__
#define SLSFS_STORAGE_SWIFTKV_HPP__

#include "basetypes.hpp"
#include "serializer.hpp"
#include "storage.hpp"
#include "scope-exit.hpp"
#include "uuid-gen.hpp"

#include <miniocpp/client.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


namespace slsfs::storage
{

namespace { using namespace std; }

class swiftkv : public interface
{
private:
    minio::s3::BaseUrl base_url = minio::s3::BaseUrl("stack.nerc.mghpcc.org:13808");
    minio::creds::StaticProvider provider = minio::creds::StaticProvider("994dde2d21f24b498455a14611d9dfbd", "3b3817d6435f4590b252bc99ba95c8d4");
    minio::s3::Client client = minio::s3::Client(base_url, &provider);
    string kv_store = ""; 


public:
    swiftkv(string kvstore_name)
    {
        // Check if kvstore exists
        bool exist = false;
        {
            minio::s3::BucketExistsArgs args;
            args.bucket = kvstore_name;

            minio::s3::BucketExistsResponse resp = client.BucketExists(args);
            if (!resp) {
                std::cout << "unable to do bucket existence check; " << resp.Error().String()
                        << std::endl;
                throw runtime_error("unable to check if kvstore exists");
            }

            exist = resp.exist;
        }
        if (!exist) {
            throw runtime_error("kvstore does not exist on server");
        }

        kv_store = kvstore_name;
    }

    ~swiftkv()
    {

    }

    // key interface   [str] -> buf
    auto read_key (pack::key_t const& name, std::size_t partition, std::size_t location, std::size_t size) -> base::buf override
    {
        minio::s3::GetObjectArgs args;
        args.bucket = kv_store;
        args.object = uuid::encode_base64(name);

        args.datafunc = [](minio::http::DataFunctionArgs args) -> bool {
                            std::cout << args.datachunk;
                            return true;
                        };

        minio::s3::GetObjectResponse resp = client.GetObject(args);

        if (resp) {
            std::cout << std::endl
                    << "data of my-object is received successfully" << std::endl;

        } else {
            std::cout << "unable to get object; " << resp.Error().String() << std::endl;
        }

        return {};
    }

    // TODO: Should this return a response, What if this fails?
    void write_key(pack::key_t const& name, std::size_t partition, base::buf const& buffer, std::size_t location, std::uint32_t version) override 
    {
        string temp_filename = "temp_write" + uuid::encode_base64(name);

        ofstream out_file(temp_filename);

        out_file.write((const char*) &buffer[0], buffer.size());
        out_file.close();

        ifstream file(temp_filename);

        auto size = std::filesystem::file_size(temp_filename);

        minio::s3::PutObjectArgs args(file, size, 0);
        args.bucket = kv_store;
        args.object = uuid::encode_base64(name);

        minio::s3::PutObjectResponse resp = client.PutObject(args);

        // Handle response.
        if (resp) {
            std::cout << "my-object is successfully created" << std::endl;
        } else {
            std::cout << "unable to do put object; " << resp.Error().String()
                    << std::endl;
        }

        remove(temp_filename.c_str());

    }

    bool check_version_ok(pack::key_t const & name, std::size_t partition, std::uint32_t& version) override {return false;}

    // list interface  [str] -> buf
    void append_list_key(pack::key_t const& name, base::buf const& buffer) {}
    void merge_list_key(pack::key_t const& name, std::function<void(std::vector<base::buf> const&)> reduce) {}

    auto get_list_key(pack::key_t const& name) -> base::buf override
    {
        minio::s3::ListObjectsArgs args;
        args.bucket = kv_store;

        // Call list objects.
        minio::s3::ListObjectsResult result = client.ListObjects(args);
        for (; result; result++) {
            minio::s3::Item item = *result;
            if (item) {
                std::cout << "Name: " << item.name << std::endl;
                std::cout << "Version ID: " << item.version_id << std::endl;
                std::cout << "ETag: " << item.etag << std::endl;
                std::cout << "Size: " << item.size << std::endl;
                std::cout << "Last Modified: " << item.last_modified << std::endl;
                std::cout << "Delete Marker: "
                            << minio::utils::BoolToString(item.is_delete_marker)
                            << std::endl;
                std::cout << "User Metadata: " << std::endl;
                for (auto& [key, value] : item.user_metadata) {
                    std::cout << "  " << key << ": " << value << std::endl;
                }
                std::cout << "Owner ID: " << item.owner_id << std::endl;
                std::cout << "Owner Name: " << item.owner_name << std::endl;
                std::cout << "Storage Class: " << item.storage_class << std::endl;
                std::cout << "Is Latest: " << minio::utils::BoolToString(item.is_latest)
                            << std::endl;
                std::cout << "Is Prefix: " << minio::utils::BoolToString(item.is_prefix)
                            << std::endl;
                std::cout << "---" << std::endl;
            } else {
            std::cout << "unable to list keys; " << item.Error().String()
                            << std::endl;
            break;
            }
        }
        return {};
    }
};

} // namespace slsfs::storage

#endif // SLSFS_STORAGE_SWIFTKV_HPP__
