#include <iostream>
#include <memory>

#include "consistent.h"

int main()
{
    std::hash<std::string> std_hash;
    auto c_h = std::make_unique<consistent_hash>(
                    /*virtual_node_num = */4,
                    /*hash_method = */std_hash
                );
    c_h->add_node("10.25.70.187");
    c_h->add_node("10.25.70.188");
    c_h->add_node("10.25.70.189");


    std::vector<std::string> test_keys {"audit_cee_1:12228", "audit_cee_2:12228", "audit_cee_3:12228", "avscan_cee_1:12228"};

    for (const auto& key : test_keys)
    {
        std::cout << "current key is: " << key << " | the cache node is: " << c_h->find_node(key) << std::endl;
    }

    std::cout << "----------------------------------\n";

    c_h->add_node("10.25.70.190");
    c_h->add_node("10.25.70.191");
    c_h->add_node("10.25.70.192");

    for (const auto& key : test_keys)
    {
        std::cout << "current key is: " << key << " | the cache node is: " << c_h->find_node(key) << std::endl;
    }
}