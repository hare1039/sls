#pragma once
#ifndef STORAGE_CONF_HPP__
#define STORAGE_CONF_HPP__

#include <slsfs.hpp>
#include <vector>

namespace slsfsdf
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
        for (std::shared_ptr<slsfs::storage::interface> host : hostlist_)
            host->connect();
    }

    template<typename DoFunction>
    void foreach(DoFunction f)
    {
        for (std::shared_ptr<slsfs::storage::interface> host : hostlist_)
            std::invoke(f, host);
    }
};

} // namespace slsfsdf

#endif // STORAGE_CONF_HPP__
