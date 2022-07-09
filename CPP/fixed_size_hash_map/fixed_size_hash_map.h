#ifndef FIXED_SIZE_HASH_MAP_H
#define FIXED_SIZE_HASH_MAP_H

#include <stdio.h>
#include <stdint.h>

#include <exception>
#include <functional>
#include <vector>

#define KVPair std::pair<KeyType, ValueType>

template <typename KeyType, typename ValueType, typename KeyComparator>
class FixSizeHashMap
{
public:
    FixSizeHashMap(int size)
        : total_size_(size)
        , hash_func_(std::hash<KeyType>())
    {
        if (size % 8)
            throw std::runtime_error("size must be multiple of 8");
        occupied_.resize(size/8);
        readable_.resize(size/8);
        data_.resize(size);
    }
    FixSizeHashMap() = delete;
    ~FixSizeHashMap()
    {
        occupied_.clear();
        readable_.clear();
        data_.clear();
    }

public:
    void get_all(std::vector<ValueType>& result)
    {
        result.clear();
        for (int i = 0; i < total_size_; ++i)
        {
            if (is_readable(i))
            {
                result.push_back(data_[i].second);
            }
        }
    }

    bool insert(KeyType key, ValueType value, KeyComparator cmp)
    {
        int index;
        probe_insert(key, cmp, index);
        if (index == -1)
        {
            return false;
        }

        data_[index].first = key;
        data_[index].second = value;
        set_occupied(index);
        set_readable(index, true);
        return true;
    }

    bool remove(KeyType key, KeyComparator cmp)
    {
        int index;
        if (!probe_get(key, cmp, index))
        {
            return false;
        }
        set_readable(index, false);
        return true;
    }

    bool get(KeyType key, KeyComparator cmp, ValueType& value)
    {
        int index;
        if (!probe_get(key, cmp, index))
        {
            return false;
        }
        value = data_[index].second;
        return true;
    }

private:
    bool is_occupied(uint32_t index) const
    {
        return occupied_[index/8] & (1 << (index % 8));
    }
    void set_occupied(uint32_t index)
    {
        occupied_[index/8] |= (1 << (index % 8));
    }
    bool is_readable(uint32_t index) const
    {
        return readable_[index/8] & (1 << (index % 8));
    }
    void set_readable(uint32_t index, bool is_readable)
    {
        if (is_readable)
        {
            readable_[index/8] |= (1 << (index % 8));
        }
        else
        {
            readable_[index/8] &= ~(1 << (index % 8));
        }
    }

private:
    int hash(const KeyType key)
    {
        return hash_func_(key) % total_size_;
    }

    /**
     * @brief Linear probing a slot for inserting the input key.
     * 
     * @param key The key to insert.
     * @param cmp The comparator to compare the key.
     * @param index[OUT] The index of the slot stored the key. -1 if not avail slot.
     * @return true If the key exists.
     * @return false If the key does not exist.
     */
    bool probe_insert(const KeyType key, KeyComparator cmp, int& index)
    {
        uint32_t hash_value = hash_func_(key);
        uint32_t index_ = hash_value % total_size_;
        for (int i = 0; i < total_size_; ++i)
        {
            // empty slot
            if (!is_occupied(index_))
            {
                index = index_;
                return false;
            }

            // slot with tombstone
            if (!is_readable(index_))
            {
                index = index_;
                return false;
            }

            // compare the existing key
            if (cmp(key, data_[index_].first))
            {
                index = index_;
                return true;
            }
            index_ = (index_ + 1) % total_size_;
        }
        index = -1;
        return false;
    }

    /**
     * @brief Linear probing a slot to get the input key.
     * 
     * @param key The key to insert.
     * @param cmp The comparator to compare the key.
     * @param index[OUT] The index of the slot stored the key if found.
     * @return true If the key exists.
     * @return false If the key does not exist.
     */
    bool probe_get(const KeyType key, KeyComparator cmp, int& index)
    {
        uint32_t hash_value = hash_func_(key);
        uint32_t index_ = hash_value % total_size_;
        for (int i = 0; i < total_size_; ++i)
        {
            // empty slot
            if (!is_occupied(index_))
            {
                index = -1;
                return false;
            }
            // compare the existing key
            if (is_readable(index_) && cmp(key, data_[index_].first))
            {
                index = index_;
                return true;
            }
            index_ = (index_ + 1) % total_size_;
        }
        index = -1;
        return false;
    }

private:
    int total_size_;
    std::hash<KeyType> hash_func_;
    std::vector<uint32_t> occupied_;
    std::vector<uint32_t> readable_;
    std::vector<KVPair> data_;
};

#endif