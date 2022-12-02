//#include "libslsfs/slsfs/storage-swiftkv-minio.hpp"
//#include "libslsfs/slsfs/serializer.hpp"

#include <miniocpp/client.h>

#include <filesystem>


int main(int argc, char const *argv[])
{
//    Aws::SDKOptions options_;
//    Aws::InitAPI(options_);

    minio::s3::BaseUrl base_url("stack.nerc.mghpcc.org:13808");;

    // Create credential provider.
    minio::creds::StaticProvider provider("994dde2d21f24b498455a14611d9dfbd", "3b3817d6435f4590b252bc99ba95c8d4");

    // Create S3 client.
    minio::s3::Client client(base_url, &provider);

    std::string bucket_name = "moc-kvstore";

    // Check 'asiatrip' bucket exist or not.
    bool exist = false;
    {
        minio::s3::BucketExistsArgs args;
        args.bucket = bucket_name;

        minio::s3::BucketExistsResponse resp = client.BucketExists(args);
        if (!resp) {
            std::cout << "unable to do bucket existence check; " << resp.Error().String()
                      << std::endl;
            return EXIT_FAILURE;
        }

        exist = resp.exist;
    }

    if (exist)
        std::cout << "exist\n";
    else
        std::cout << "notexist\n";

    // Create put object arguments.
    std::ifstream file("conanfile.txt");

    auto size = std::filesystem::file_size("conanfile.txt");

    minio::s3::PutObjectArgs args(file, size, 0);
    args.bucket = "moc-kvstore";
    args.object = "conan.txt";

    minio::s3::PutObjectResponse resp2 = client.PutObject(args);

    // Handle response.
    if (resp2) {
        std::cout << "my-object is successfully created" << std::endl;
    } else {
        std::cout << "unable to do put object; " << resp2.Error().String()
                  << std::endl;
    }

    // Create get object arguments.
    minio::s3::GetObjectArgs args3;
    args3.bucket = "moc-kvstore";
    args3.object = "conan.txt";
    args3.datafunc = [](minio::http::DataFunctionArgs args) -> bool {
                         std::cout << args.datachunk;
                         return true;
                     };
    minio::s3::GetObjectResponse resp3 = client.GetObject(args3);

    if (resp3) {
        std::cout << std::endl
                  << "data of my-object is received successfully" << std::endl;

    } else {
        std::cout << "unable to get object; " << resp3.Error().String() << std::endl;
    }





//    slsfs::storage::swiftkv swift_client = slsfs::storage::swiftkv("moc-kvstore");
//
//    slsfs::pack::key_t key = slsfs::pack::key_t{
//        7, 8, 7, 8, 7, 8, 7, 8,
//        7, 8, 7, 8, 7, 8, 7, 8,
//        7, 8, 7, 8, 7, 8, 7, 8,
//        7, 8, 7, 8, 7, 8, 7, 8};
//
//    swift_client.get_list_key(key);
//    Aws::ShutdownAPI(options_);

    return 0;
}
