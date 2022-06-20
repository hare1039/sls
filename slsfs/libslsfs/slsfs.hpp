#pragma once
#ifndef SLSFS_HPP__
#define SLSFS_HPP__

#include "slsfs/uuid-gen.hpp"
#include "slsfs/json.hpp"
#include "slsfs/basetypes.hpp"
#include "slsfs/http-verb.hpp"
#include "slsfs/storage-cassandra.hpp"
#include "slsfs/storage.hpp"
#include "slsfs/switchstring.hpp"
#include "slsfs/debuglog.hpp"

#include <kafka/KafkaConsumer.h>
#include <kafka/KafkaProducer.h>

#include <algorithm>
#include <memory>
#include <thread>

namespace slsfs
{

using off_t = std::uint64_t;

void send_kafka(std::string const& uuid, base::json const& data)
{
    slsfs::log::logstring("send_kafka start");

    kafka::Properties props ({
        {"bootstrap.servers",  "zion01:9092"},
        {"enable.idempotence", "true"},
    });

    kafka::clients::KafkaProducer producer(props);
    producer.setLogLevel(0);

    auto line = std::make_shared<std::string>(base::encode_kafkajson(data));
    auto record = kafka::clients::producer::ProducerRecord(uuid,
                                                           kafka::NullKey,
                                                           kafka::Value(line->c_str(), line->size()));

    slsfs::log::logstring("send_kafka producer.send()");
    producer.send(
        record,
        // The delivery report handler
        // Note: Here we capture the shared_pointer of `line`,
        //       which holds the content for `record.value()`.
        //       It makes sure the memory block is valid until the lambda finishes.
        [line](kafka::clients::producer::RecordMetadata const & metadata, kafka::Error const & error) {
            if (!error)
                std::cerr << "% Message delivered: " << metadata.toString() << "\n";
            else
                std::cerr << "% Message delivery failed: " << error.message() << "\n";
        });
    slsfs::log::logstring("send_kafka end");
}

auto listen_kafka(std::string const& channel) -> base::json
{
    slsfs::log::logstring("listen_kafka start");

    kafka::Properties props ({
        {"bootstrap.servers",  "zion01:9092"},
        {"enable.auto.commit", "true"}
    });

    slsfs::log::logstring("listen_kafka start subscribe channel");
    kafka::clients::KafkaConsumer consumer(props);
    consumer.subscribe({channel});
    consumer.setLogLevel(0);
    std::cerr << "listen on " << channel << "\n";

    slsfs::log::logstring("listen_kafka start poll()");
    std::vector<kafka::clients::consumer::ConsumerRecord> records = consumer.poll(std::chrono::milliseconds(10000));

    //assert(records.size() == 1);

    for (kafka::clients::consumer::ConsumerRecord const& record: records)
    {
        if (record.value().size() == 0)
            return "";

        if (!record.error())
        {
            std::cerr << "% Got a new message..." << std::endl;
            std::cerr << "    Topic    : " << record.topic() << std::endl;
            std::cerr << "    Partition: " << record.partition() << std::endl;
            std::cerr << "    Offset   : " << record.offset() << std::endl;
            std::cerr << "    Timestamp: " << record.timestamp().toString() << std::endl;
            std::cerr << "    Headers  : " << kafka::toString(record.headers()) << std::endl;
            std::cerr << "    Key   [" << record.key().toString() << "]" << std::endl;
            std::cerr << "    Value [" << record.value().toString() << "]" << std::endl;
            slsfs::log::logstring("listen_kafka end");
            return base::decode_kafkajson(record.value().toString());
        }
        else
            std::cerr << record.toString() << std::endl;

    }
    return "";
}


auto write(char const * filename, char const *data, std::size_t size, off_t off, /*struct fuse_file_info*/ void* info )
    -> std::size_t
{
    slsfs::log::logstring("write start");
    std::string const uuid = uuid::get_uuid(filename);
    std::string const rvc_chan = uuid + "-df2src-" + uuid::gen_rand_str(10);

    base::json jsondata;
    jsondata["filename"] = filename;
    jsondata["data"] = data;
    jsondata["size"] = size;
    jsondata["offset"] = off;
    jsondata["type"] = "file";
    jsondata["uuid"] = uuid;
    jsondata["operation"] = "write";
    jsondata["returnchannel"] = rvc_chan;

    std::cerr << "sending json to kafka\n";

    slsfs::log::logstring("write send_kafka");
    send_kafka(uuid, jsondata);

    slsfs::log::logstring("write listen_kafka");
    base::json const ret = listen_kafka(rvc_chan);
    std::cerr << "ret: " << ret << "\n";
    slsfs::log::logstring("write end");
    return 0;
}

auto read(char const * filename, char *data, std::size_t size, off_t off, /*struct fuse_file_info*/ void* info)
    -> std::size_t
{
    slsfs::log::logstring("read start");
    std::string const uuid = uuid::get_uuid(filename);
    std::string const rvc_chan = uuid + "-df2src-" + uuid::gen_rand_str(10);
    base::json jsondata;
    jsondata["filename"] = filename;
    jsondata["size"] = size;
    jsondata["offset"] = off;
    jsondata["type"] = "file";
    jsondata["uuid"] = uuid;
    jsondata["operation"] = "read";
    jsondata["returnchannel"] = rvc_chan;

    std::cerr << "sending read json to kafka\n";

    slsfs::log::logstring("read send_kafka");
    send_kafka(uuid, jsondata);


    slsfs::log::logstring("read listen_kafka");
    base::json const ret = listen_kafka(rvc_chan);
    std::cerr << "ret: " << ret << "\n";

    std::string const read_data = ret["data"].get<std::string>();

    std::size_t readsize = std::min(size, read_data.size());
    std::copy_n(read_data.begin(), readsize, data);

    slsfs::log::logstring("read end");
    return readsize;
}


int create(char const * filename)
{
    slsfs::log::logstring("create start");

    std::string const uuid = uuid::get_uuid(filename);

    using namespace std::literals;
    base::json triggerdata;
    std::string const triggername = "trigger-"s + uuid;
    triggerdata["name"] = triggername;

    base::json k;
    k["key"] = "feed";
    k["value"] = "/whisk.system/messaging/kafkaFeed";

    triggerdata["annotations"] = base::json::array();
    triggerdata["annotations"].push_back(k);

    std::string const triggerurl = "https://zion01/api/v1/namespaces/_/triggers/"s + triggername + "?overwrite=false";

    slsfs::log::logstring("create put trigger");
    httpdo::put(triggerurl, triggerdata.dump());

    std::string const providerurl="https://zion01/api/v1/namespaces/whisk.system/actions/messaging/kafkaFeed?blocking=true&result=false";
    base::json providerjson;

    providerjson["authKey"] = "789c46b1-71f6-4ed5-8c54-816aa4f8c502:abczO3xZCLrMN6v2BKK1dXYFpXlPkccOFqm12CdAsMgRU4VrNZ9lyGVCGuMDGIwP";
    providerjson["brokers"] = base::json::array();
    providerjson["brokers"].push_back("zion01:9092");
    providerjson["isJSONData"] = false;
    providerjson["lifecycleEvent"] = "CREATE";
    providerjson["topic"] = uuid;
    using namespace std::literals;
    providerjson["triggerName"] = "/_/"s + triggername;

    slsfs::log::logstring("create post provider");
    httpdo::post(providerurl, providerjson.dump());
    slsfs::log::logstring("create post provider end");

    std::cerr << providerjson << "post data \n";

    base::json ruledata;
    std::string const rulename = "kaf2df-" + uuid;
    ruledata["name"] = rulename;
    ruledata["status"] = "";
    ruledata["trigger"] = "/whisk.system/trigger-" + uuid;
    ruledata["action"] = "/whisk.system/slsfs-datafunction";

    //"https://zion01/api/v1/namespaces/_/rules/kaf2cpp?overwrite=false";
    std::string const ruleurl = "https://zion01/api/v1/namespaces/_/rules/"s + rulename + "?overwrite=false";

    slsfs::log::logstring("create put rule");
    httpdo::put(ruleurl, ruledata.dump());

    slsfs::log::logstring("create end");
    return 0;
}

} // namespace slsfs

#endif // SLSFS_HPP__
