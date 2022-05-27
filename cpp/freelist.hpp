#pragma once
#ifndef FREELIST_HPP__
#define FREELIST_HPP__

#include <bitset>
#include <cstdint>

namespace base
{

template<int Size>
class freelist
{
    std::uint32_t base_ = 0;
    std::bitset<Size> data_;

public:
    auto nbegin() -> std::uint32_t { return base_; }
    auto nend()   -> std::uint32_t { return base_ + data_.size(); }
    auto test(std::uint32_t pos) -> bool   { return data_.test(pos - base_); }
    void reset() { data_.reset(); }
    void set (std::uint32_t pos, bool val) { data_.set(pos - base_, val); }
    void set_base(std::uint32_t base)      { base_ = base; }
};

}// namespace base

#endif // FREELIST_HPP__
