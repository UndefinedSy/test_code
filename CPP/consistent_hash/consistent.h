#ifndef _CONSISTENT_HASH_H_
#define _CONSISTENT_HASH_H_

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

class consistent_hash
{
private:
    std::map<uint32_t, std::string> circle_;
    std::map<std::string, bool>     members_;
    std::vector<uint32_t>           sorted_ring_;
    int                             nodes_num_;
    int                             virtual_nodes_num_;
    std::mutex                      rw_mutex_;
    std::function<uint32_t(std::string)> hash_method_;

    void update_sorted_ring();
    int search_node(uint32_t hashed_key);

public:
    consistent_hash(int virtual_nodes_num, std::function<uint32_t(std::string)> hash_method);
    ~consistent_hash();

    void add_node(std::string node_addr);
    std::string find_node(std::string key);
};

#endif