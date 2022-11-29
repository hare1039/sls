#pragma once

#ifndef SLSFS_STORAGE_SWIFTKV_HPP__
#define SLSFS_STORAGE_SWIFTKV_HPP__

#include "basetypes.hpp"
#include "serializer.hpp"
#include "storage.hpp"
#include "scope-exit.hpp"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/Object.h>

#include <iostream>
#include <string>
#include <vector>


namespace slsfs::storage
{

namespace { using namespace std; }

class swiftkv : public interface
{
private:
    Aws::Client::ClientConfiguration client_config_;
    Aws::S3::S3Client s3_client_;
    Aws::SDKOptions options_;
    Aws::String kvstore_ = nullptr;

public:
    swiftkv(string kvstore_name)
    {
        // Create S3 base URL, credentials, and client
        Aws::InitAPI(options_);

        client_config_.endpointOverride = "https://stack.nerc.mghpcc.org:13808";
        Aws::Auth::AWSCredentials cred{"994dde2d21f24b498455a14611d9dfbd",
                                       "3b3817d6435f4590b252bc99ba95c8d4"};
        s3_client_ = Aws::S3::S3Client(cred,
                                       Aws::MakeShared<Aws::S3::Endpoint::S3EndpointProvider>("DF swift allocationTag"),
                                       client_config_); // 2th constructure
        kvstore_ = Aws::String(kvstore_name.c_str(), kvstore_name.size());;
    }

    ~swiftkv()
    {
        Aws::ShutdownAPI(options_);
    }

    // key interface   [str] -> buf
    auto read_key (pack::key_t const& name, std::size_t partition, std::size_t location, std::size_t size) -> base::buf override
    {
        return {};
    }

    // TODO: Should this return a response, What if this fails?
    void write_key(pack::key_t const& name, std::size_t partition, base::buf const& buffer, std::size_t location, std::uint32_t version) override {}

    bool check_version_ok(pack::key_t const & name, std::size_t partition, std::uint32_t& version) override {return false;}

    // list interface  [str] -> buf
    void append_list_key(pack::key_t const& name, base::buf const& buffer) {}
    void merge_list_key(pack::key_t const& name, std::function<void(std::vector<base::buf> const&)> reduce) {}

    auto get_list_key(pack::key_t const& name) -> base::buf override
    {
        Aws::S3::Model::ListObjectsRequest request;
        request.WithBucket(kvstore_);

        auto outcome = s3_client_.ListObjects(request);

        if (!outcome.IsSuccess()) {
            std::cerr << "Error: ListObjects: " <<
                outcome.GetError().GetMessage() << std::endl;
        }
        else {
            Aws::Vector<Aws::S3::Model::Object> objects =
                outcome.GetResult().GetContents();

            for (Aws::S3::Model::Object &object: objects) {
                std::cout << object.GetKey() << std::endl;
            }
        }
        return {};
    }
};

} // namespace slsfs::storage

#endif // SLSFS_STORAGE_SWIFTKV_HPP__