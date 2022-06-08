// STD headers
#include <assert.h>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

// Custom headers
#include "cache_base.hpp"
#include "cache_common.hpp"
#include "utils.hpp"

using namespace caching;

/**
 * Represents a single set (row) in an LRU-based cache.
 */
class LFUCacheSet : public BaseCacheSet {
protected:
    LFUQueue<CacheEntry> queue_; // LRU Queue
    std::unordered_map<std::string, uint64_t> Freqs;
    std::unordered_map<std::string, uint64_t> Caches;

public:
    LFUCacheSet(const size_t num_entries, const size_t misslat) : BaseCacheSet(num_entries,misslat) {}
    virtual ~LFUCacheSet() {}

    std::unordered_map<std::string,uint64_t> Sizes1;
    uint64_t UsedSpace = 0;

    virtual int
    update_freqs(const std::string& key, uint64_t size){
       if(queue_.Freqs.find(key) != queue_.Freqs.end()){
           queue_.Freqs[key] += 1;
       }
       else{
           queue_.Freqs[key] = 1;
       }
       if(Sizes1.find(key) == Sizes1.end()){
           Sizes1[key] = size;
           queue_.Sizes2[key] = size;
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
        CacheEntry evicted_entry;

        //std::cout<<UsedSpace<<" "<<getNumEntries()<<"\n";

        // If a corresponding entry exists, update it
        auto position_iter = queue_.positions().find(key);
        if (position_iter != queue_.positions().end()) {
            written_entry = *(position_iter->second);

            // Sanity checks
            assert(contains(key));
            assert(written_entry.isValid());
            assert(written_entry.key() == key);

            // If the read was successful, the corresponding entry is
            // the MRU element in the cache. Remove it from the queue.
            queue_.erase(position_iter);
        }
        // The update was unsuccessful, create a new entry to insert
        else {
            written_entry.update(key);
            written_entry.toggleValid();

            // If required, evict the LRU entry
            //if (queue_.size() == getNumEntries()) {
            while(UsedSpace >= getNumEntries()){
                evicted_entry = queue_.popFreq();
                assert(evicted_entry.isValid()); // Sanity check
                UsedSpace -= Sizes1[evicted_entry.key()];
                occupied_entries_set_.erase(evicted_entry.key());
            }

            // Update the occupied entries set
            occupied_entries_set_.insert(key);
            UsedSpace += Sizes1[key];
        }
	
        // Finally, (re-)insert the written entry at the back
        queue_.insertBack(written_entry);

        // Sanity checks
        assert(occupied_entries_set_.size() <= getNumEntries());
        assert(occupied_entries_set_.size() == queue_.size());
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

/**
 * Implements a single-tiered LRU cache.
 */
class LFUCache : public BaseCache {
public:
    LFUCache(const size_t miss_latency, const size_t cache_set_associativity, const size_t
             num_cache_sets, const bool penalize_insertions, const HashType hash_type, int
             argc, char** argv) : BaseCache(miss_latency, cache_set_associativity,
             num_cache_sets, penalize_insertions, hash_type) {
        SUPPRESS_UNUSED_WARNING(argc);
        SUPPRESS_UNUSED_WARNING(argv);

        // Initialize the cache sets
        for (size_t idx = 0; idx < kMaxNumCacheSets; idx++) {
            cache_sets_.push_back(new LFUCacheSet(kCacheSetAssociativity,miss_latency));
        }
    }
    virtual ~LFUCache() {}

    /**
     * Returns the canonical cache name.
     */
    virtual std::string name() const override { return "LFUCache"; }
};

// Run default benchmarks
int main(int argc, char** argv) {
    BaseCache::defaultBenchmark<LFUCache>(argc, argv);
}
