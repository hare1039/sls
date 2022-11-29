#include "libslsfs/slsfs/storage-swiftkv.hpp"
#include "libslsfs/slsfs/serializer.hpp"


int main(int argc, char const *argv[])
{
    slsfs::storage::swiftkv swift_client = slsfs::storage::swiftkv("moc-kvstore");

    slsfs::pack::key_t key = slsfs::pack::key_t{
        7, 8, 7, 8, 7, 8, 7, 8,
        7, 8, 7, 8, 7, 8, 7, 8,
        7, 8, 7, 8, 7, 8, 7, 8,
        7, 8, 7, 8, 7, 8, 7, 8};

    swift_client.get_list_key(key);

    return 0;
}
