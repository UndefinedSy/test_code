#include <algorithm>
#include <iostream>

#include "consistent.h"

consistent_hash::consistent_hash(int virtual_nodes_num,
                                 std::function<uint32_t(std::string)> hash_method)
    : virtual_nodes_num_(virtual_nodes_num),
      hash_method_(hash_method)
{}


consistent_hash::~consistent_hash()
{}

void
consistent_hash::update_sorted_ring()
{
    std::vector<uint32_t> hash_ring;

    for(const auto& map_iter : circle_)
    {
        hash_ring.push_back(map_iter.first);
    }

    sort(hash_ring.begin(), hash_ring.end());

    sorted_ring_ = hash_ring;
    // maybe just check insert within the loop inside func `add_node` will be more efficent.
}

void
consistent_hash::add_node(std::string node_addr)
{
    const std::lock_guard<std::mutex> wlock(rw_mutex_);

    for (int i = 0; i < virtual_nodes_num_; ++i)
    {
        // maybe should check the members_ existence here to avoid hash conflict.
        circle_[hash_method_(node_addr + "_" + std::to_string(i))] = node_addr;
    }
    nodes_num_++;
    members_[node_addr] = true;
    update_sorted_ring();
}

int
consistent_hash::search_node(uint32_t hashed_key)
{
	const auto target_position =
        std::upper_bound(sorted_ring_.begin(), sorted_ring_.end(), hashed_key);
	if (target_position == sorted_ring_.end())
    {
        return 0;
	}
    else
    {
        return target_position - sorted_ring_.begin();
    }
}

std::string
consistent_hash::find_node(std::string key)
{
    const std::lock_guard<std::mutex> rlock(rw_mutex_);

    if (circle_.size() == 0)
    {
        std::cout << "hash_map has not been established" << std::endl;
    }
    int target_ring_index = search_node(hash_method_(key));

    return circle_[sorted_ring_[target_ring_index]];
}


