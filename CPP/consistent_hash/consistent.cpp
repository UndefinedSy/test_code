#include <algorithm>
#include <iostream>
#include <stdexcept>

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

    if (members_.find(node_addr) != members_.end())
    {
        std::cout << "the input node: " << node_addr << " has already been set on the ring." << std::endl;
        return;
    }

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

    if (nodes_num_ == 0)
    {
        std::cout << "hash_map has not been established" << std::endl;
        return nullptr;
    }
    int target_ring_index = search_node(hash_method_(key));

    const std::string& target_node_addr = circle_[sorted_ring_[target_ring_index]];

    if (members_.find(target_node_addr) != members_.end() && !members_[target_node_addr])
    {
        // this node has been failed, can add some failover logic here.
        // like return an specific err code to tell requestor to use the backup ring.
        // or just make the backup node in-place when detect a node fails.
    }

    return circle_[sorted_ring_[target_ring_index]];
}


void
consistent_hash::remove_node(std::string node_addr)
{
    if (members_.find(node_addr) == members_.end())
    {
        std::cout << "the input node: " << node_addr << " does not exist on the ring." << std::endl;
        return;
    }

    const std::lock_guard<std::mutex> rlock(rw_mutex_);

    for (int i = 0; i < virtual_nodes_num_; ++i)
    {
        // maybe should check the members_ existence here to avoid hash conflict.
        circle_.erase(hash_method_(node_addr + "_" + std::to_string(i)));
    }
    nodes_num_--;
    members_.erase(node_addr);
    update_sorted_ring();
}

std::vector<std::string>
consistent_hash::find_next_n_nodes(std::string key, int n)
{
    std::vector<std::string> result;

    const std::lock_guard<std::mutex> rlock(rw_mutex_);

    if (nodes_num_ < n)
    {
        std::cout << "do not have enough node" << std::endl;
        return result;
    }
    int target_ring_index = search_node(hash_method_(key));
    const std::string& target_node_addr = circle_[sorted_ring_[target_ring_index++]];

    result.emplace_back(target_node_addr);

    int ring_length = sorted_ring_.size();
    for (int i = 1; i < n;)
    {
        const std::string& next_entry = circle_[sorted_ring_[target_ring_index++]];
        if (find(result.begin(), result.end(), next_entry) == result.end())
        {
            result.emplace_back(next_entry);
            ++i;
        }
        if (target_ring_index >= ring_length)
            target_ring_index = 0;
    }

    return result;
}