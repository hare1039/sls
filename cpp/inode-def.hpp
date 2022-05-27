#pragma once
#ifndef INODE_DEF_HPP__
#define INODE_DEF_HPP__

namespace inode
{

using inode_ptr = std::uint32_t;
using block_ptr = inode_ptr;

constexpr int block_list_size = 16;
constexpr inode_ptr null_inode_ptr = -1;
constexpr block_ptr null_block_ptr = -1;

} // namespace inode

#endif // INODE_DEF_HPP__
