//#include <boost/asio.hpp>

#include "datafunction.hpp"
#include "metadatafunction.hpp"

#include <slsfs.hpp>

#include <iostream>
#include <memory>
#include <thread>
#include <sstream>


auto perform(slsfs::base::storage_interface &datastorage, slsfs::base::json const& single_input)
    -> slsfs::base::json
{
    slsfs::log::logstring("perform start");
    auto const datatype = single_input["type"].get<std::string>();

    switch (slsfs::sswitch::hash(datatype))
    {
        using namespace slsfs::sswitch;

    case "file"_:
    {
        return datafunction::perform_single_request(datastorage, single_input);
        break;
    }

    case "metadata"_:
    {
        return metadata::perform_single_request(datastorage, single_input);
        break;
    }
    }
    slsfs::log::logstring("perform end");
    return {};
}

int main(int argc, char *argv[])
{
    char const * name = "datafunction";
    slsfs::log::init(name);
    slsfs::log::logstring("data function start");

    using json = slsfs::base::json;
    using namespace std::literals;
    json input;
    std::cin >> input;
    std::cerr << "get request: " << input << "\n";
    slsfs::log::logstring("parsed json");

    json output;
    SCOPE_DEFER([&output]{ std::cout << output.dump() << "\n"; });
    output["original-request"] = input;
    output["response"] = json::array();

    slsfs::storage::cassandra datastorage{"192.168.2.25"};
    datastorage.connect();
    slsfs::log::logstring("connected to datastorage");

    try
    {
        if (input.contains("messages"))
        {
            for (json & single_input : input["messages"])
            {
                json const converted = slsfs::base::decode_kafkajson(single_input["value"].get<std::string>());
                output["response"].push_back(perform(datastorage, converted));
            }
        }
        else
        {
            output["response"].push_back(perform(datastorage, input));
        }
    }
    catch (std::exception const & e)
    {
        output["exception thrown"] = e.what();
        std::cerr << "exception thrown" << e.what() << "\n";
    }
    std::cout << output.dump() << "\n";

    return 0;
}
