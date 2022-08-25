#pragma once
#ifndef STORAGE_CONF_CASS_HPP__
#define STORAGE_CONF_CASS_HPP__

#include "storage-conf.hpp"

namespace slsfsdf
{

class storage_conf_cass : public storage_conf
{
public:
    virtual void init() override {
        hostlist_.push_back(std::make_shared<slsfs::storage::cassandra>("192.168.2.27"));
    }

    virtual int blocksize() override { return 4096; } // byte
};

} // namespace slsfsdf

#endif // STORAGE_CONF_CASS_HPP__
