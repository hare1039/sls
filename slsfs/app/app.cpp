
#include <slsfs.hpp>

#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

int main(int argc, char *argv[])
{
    char const * name = "app";
    slsfs::log::init(name);
    slsfs::log::logstring("app start");

    using json = slsfs::base::json;
    json input;
    std::cin >> input;

    json output;
    SCOPE_DEFER([&output]{
                    std::cout << output.dump() << "\n";
                    std::cerr << output.dump() << "\n";
                });
    output["response"] = "ok";

    auto const data = input["data"].get<std::string>();
    using namespace std::chrono_literals;


    slsfs::log::logstring<slsfs::log::level::info>("create /");
    slsfs::create("/");
    slsfs::log::logstring<slsfs::log::level::info>("create /r/");
    slsfs::create("/restaurant/");
    slsfs::log::logstring<slsfs::log::level::info>("create /r/g");
    slsfs::create("/restaurant/burgerking.txt");
    slsfs::log::logstring<slsfs::log::level::info>("write /r/b");
    slsfs::write("/restaurant/burgerking.txt", data.data(), data.size(), 0, nullptr);

    {
        std::array<char, 512> readbuf;
        slsfs::log::logstring<slsfs::log::level::info>("read /r/b");
        std::size_t readsize = slsfs::read("/restaurant/burgerking.txt", readbuf.data(), readbuf.size(), 0, nullptr);
        std::string readfile;
        std::copy_n(readbuf.begin(), readsize, std::back_inserter(readfile));
        output["read"] = readfile;
    }

//    {
//        std::array<char, 512> readbuf;
//        std::size_t readsize = slsfs::read("/hello.txt", readbuf.data(), readbuf.size(), 0, nullptr);
//        std::string readfile;
//        std::copy_n(readbuf.begin(), readsize, std::back_inserter(readfile));
//        output["readhello"] = readfile;
//    }

    return 0;
}
