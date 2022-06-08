#ifndef cache_belady_h
#define cache_belady_h

// STD headers
#include <assert.h>
#include <list>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

// Custom headers
#include "cache_base.hpp"
#include "cache_common.hpp"
#include "utils.hpp"

namespace caching {

/**
 * Represents a single set (row) in a Belady-based cache.
 *
 * The template argument should a BaseCache-derived class
 * that exposes a getFlowIdToEvict() method. The return
 * value is used to decide which object is evicted from
 * the cache. See src/cache_belady.cpp for an example.
 */
template<class T>
class BeladyCacheSet : public BaseCacheSet {
protected:
    T& cacheImpl; // Reference to the cache implementation
    std::unordered_map<std::string, CacheEntry> entries_; // Dict mapping flow
                                                          // IDs to CacheEntries.
public:
    BeladyCacheSet(const size_t num_entries, const size_t misslat, T& cache) :
        BaseCacheSet(num_entries, misslat), cacheImpl(cache) {}
    virtual ~BeladyCacheSet() {}

    std::unordered_map<std::string,uint64_t> Sizes1;
    uint64_t UsedSpace = 0;

    virtual int
    update_freqs(const std::string& key, uint64_t size){
       //SUPPRESS_UNUSED_WARNING(key);
       //SUPPRESS_UNUSED_WARNING(size);
       if(Sizes1.find(key) == Sizes1.end()){
           Sizes1[key] = size;
       }
       return 0;
    }


    /**
     * Simulates a cache write.
     *
     * @param key The key corresponding to this write request.
     * @param packet The packet corresponding to this write request.
     * @return The written CacheEntry instance.
     */
    virtual CacheEntry
    write(const std::string& key, const utils::Packet& packet) override {
        SUPPRESS_UNUSED_WARNING(packet);
        CacheEntry written_entry;

        //std::cout<<"key: "<<key<<" "<<UsedSpace<<std::endl;

        // If a corresponding entry exists, update it
        auto iter = entries_.find(key);
        if (iter != entries_.end()) {
            written_entry = iter->second;

            // Sanity checks
            assert(contains(key));
            assert(written_entry.isValid());
            assert(written_entry.key() == key);
        }
        // The update was unsuccessful, create a new entry to insert
        else {
            assert(!contains(key));
            written_entry.update(key);
            written_entry.toggleValid();

            bool evicted = 0;

            // If required, evict an existing entry
            //if (entries_.size() == getNumEntries()) {
            uint64_t count = 0;
            while(UsedSpace >= getNumEntries()){
                const std::string evicted_key = (
                    cacheImpl.getFlowIdToEvict(entries_, key, evicted));
                assert(!evicted_key.empty());

                if(key == evicted_key){
                    evicted = 1;
                }

                // Evict an existing cache entry
                auto evicted_iter = entries_.find(evicted_key);
                if (evicted_iter != entries_.end()) {

                    occupied_entries_set_.erase(evicted_key);
                    assert(evicted_iter->second.isValid());
                    entries_.erase(evicted_iter);
                    UsedSpace -= Sizes1[evicted_key];
                    //std::cout<<"1"<<std::endl;
                }
                // Else, reject the contender
                else {
                    assert(evicted_key == key);
                    written_entry.toggleValid();
                    //std::cout<<count<<" 2 "<<evicted_key<<std::endl;
                }
                count++;
            }
            // If required, update the cache
            if (written_entry.isValid()) {
                entries_[key] = written_entry;
                occupied_entries_set_.insert(key);
                UsedSpace += Sizes1[key];
            }
        }
        // Sanity checks
        assert(occupied_entries_set_.size() <= getNumEntries());
        assert(occupied_entries_set_.size() == entries_.size());
        return written_entry;
    }

    /**
     * Simulates a sequence of cache writes for a particular flow's packet queue.
     * Invoking this method should be functionally equivalent to invoking write()
     * on every queued packet; this simply presents an optimization opportunity
     * for policies which do not distinguish between single/multiple writes.
     *
     * @param queue The queued write requests.
     * @return The written CacheEntry instance.
     */
    virtual CacheEntry
    writeq(const std::list<utils::Packet>& queue) override {
        const utils::Packet& packet = queue.back();
        return write(packet.getFlowId(), packet);
    }
};

} // namespace caching

#endif // cache_belady_h
