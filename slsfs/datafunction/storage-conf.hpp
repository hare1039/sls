#pragma once
#ifndef STORAGE_CONF_HPP__
#define STORAGE_CONF_HPP__

#include <slsfs.hpp>
#include <vector>

namespace df
{

class storage_conf
{
protected:
    std::vector<std::shared_ptr<slsfs::storage::interface>> hostlist_;
public:
    virtual ~storage_conf() {}
    virtual void init() = 0;
    virtual int  blocksize() = 0;

    void connect()
    {
        for (auto& host : hostlist_)
            host->connect();
    }

    auto hosts() -> decltype(hostlist_)& { return hostlist_; };
};

} // namespace df

#endif // STORAGE_CONF_HPP__
