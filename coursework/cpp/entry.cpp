//#include <boost/asio.hpp>

#include <iostream>
#include <memory>
#include <thread>
#include <sstream>

#include "json.hpp"
#include "scope-exit.hpp"
#include "storage.hpp"
#include "storage-cassandra.hpp"
#include "indexstorage.hpp"
#include "inode.hpp"
#include "switchstring.hpp"


int main(int argc, char *argv[])
{
    using json = nlohmann::json;
    json input;
    std::cin >> input;

    json output;
    SCOPE_DEFER([&output]{ std::cout << output.dump() << "\n"; });
    //output["original-request"] = input;
    output["response"] = "failed";

    try
    {
        auto const operation = input["operation"].get<std::string>();
        indexstorage::indexstorage istorage {std::make_unique<storage::cassandra>()};
        storage::cassandra datastorage;
        datastorage.connect();

        switch (sswitch::hash(operation))
        {
            using namespace sswitch;
            case "format"_:
            {
                istorage.format();
                output["response"] = "ok";
                std::cout << output.dump() << "\n";
                break;
            }
            case "ls"_:
            {
                auto const filename = input["filename"].get<std::string>();
                output["list"] = istorage.list_from_root(filename);
                output["response"] = "ok";
                std::cout << output.dump() << "\n";
                break;
            }
            case "touch"_:
            {
                auto const filename = input["filename"].get<std::string>();
                istorage.touch_from_root(filename);
                output["response"] = "ok";
                std::cout << output.dump() << "\n";
                break;
            }
            case "mkdir"_:
            {
                std::cerr << "mkdir_from_root\n";
                auto const dirname = input["filename"].get<std::string>();
                istorage.mkdir_from_root(dirname);
                output["response"] = "ok";
                std::cout << output.dump() << "\n";
                break;
            }
            case "write"_:
            {
                auto const filename = input["filename"].get<std::string>();
                auto const data = input["data"].get<std::string>();
                base::buf const write_buf = base::to_buf(data);

                istorage.write_file(filename, write_buf, &datastorage);
                output["response"] = "ok";
//                std::uint32_t block_index = 0, reader = 0;
//                for (; block_index < i.blocksize(); block_index++)
//                {
//                    std::cerr << "block" << std::endl;
//                    auto b = std::get<inode::data_block>(i.blocks.at(block_index));
//
//                    std::cerr << "block: " << b << std::endl;
//                    if (reader  <= position and
//                        position < reader + datastorage.current_state().optimal_size)
//                    {
//                        std::cerr << "read " << reader << " p " << position << std::endl;
//                        base::buf original_data = datastorage.read_block(b.offset);
//
//                        std::cerr << "unlink " << std::endl;
//                        istorage.unlink(b.offset);
//
//                        std::cerr << "insert " << position - reader + write_buf.size() << " " << b.size << std::endl;
//
//                        original_data.resize(std::max<std::size_t>(
//                                                 position - reader + write_buf.size(),
//                                                 b.size));
//
//                        std::copy(write_buf.begin(), write_buf.end(),
//                                  std::next(original_data.begin(), position - reader));
//
//                        write_buf = original_data;
//                        break;
//                    }
//                    reader += datastorage.current_state().optimal_size;
//                }
//
//                std::cerr << "fileinode" << operation << std::endl;
//                inode::block_ptr place = istorage.getfree_block();
//                std::cerr << "blockss " << operation << std::endl;
//
//                inode::data_block b {
//                    .vol_id = 0,
//                    .offset = place,
//                    .size = static_cast<std::uint32_t>(write_buf.size()),
//                };
//                i.set(block_index, b);
//
//                std::cerr << "write " << operation << std::endl;
//                datastorage.write_block(place, write_buf);
//                istorage.save_inode(i_ptr, i);
//
                break;
            }

            case "read"_:
            {
                auto const filename = input["filename"].get<std::string>();
                auto v = base::to_string(istorage.readall(filename));
                output["read"] = v;
                output["response"] = "ok";
                std::cout << output.dump() << std::endl;
                break;
            }
        }
    }
    catch (std::exception const & e)
    {
        output["exception thrown"] = e.what();
        std::cerr << "exception thrown" << e.what() << "\n";
    }
    return 0;
}
