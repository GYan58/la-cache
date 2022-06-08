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
class LRUKCacheSet : public BaseCacheSet {
protected:
    LRUKQueue<CacheEntry> queue_; // LRU Queue
    TQQueue<CacheEntry> History;

public:
    std::unordered_map<std::string,uint64_t> Freqs;//用于记录未被cache的item对应的访问频率

    LRUKCacheSet(const size_t num_entries, const size_t misslat) : BaseCacheSet(num_entries,misslat) {}
    virtual ~LRUKCacheSet() {}

    std::unordered_map<std::string,uint64_t> Sizes1;
    uint64_t UsedSpace = 0;
    uint64_t HisSpace = 0;
    
    virtual int
    update_freqs(const std::string& key, uint64_t size){
        if(Sizes1.find(key) == Sizes1.end()){
           Sizes1[key] = size;
           queue_.Sizes2[key] = size;
        }

        queue_.update_lrts(key);
        
        // update history
        auto position_iter_c = queue_.positions().find(key);
        if (position_iter_c == queue_.positions().end()) {
            History.update_freqs(key);

            CacheEntry written_entry;
            CacheEntry evicted_entry;
            auto position_iter = History.positions().find(key);
            if (position_iter != History.positions().end()) {
                written_entry = *(position_iter->second);
                History.erase(position_iter);
            }
            else{
                written_entry.update(key);
                written_entry.toggleValid();
                while(HisSpace >= 4*getNumEntries()){
                    evicted_entry = History.popFront();
                    assert(evicted_entry.isValid()); // Sanity check
                    HisSpace -= Sizes1[evicted_entry.key()];
               }
                HisSpace += size;
           }
           History.insertBack(written_entry);
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
            queue_.insertBack(written_entry); 
        }
        // The update was unsuccessful, create a new entry to insert
        else {
            written_entry.update(key);
            written_entry.toggleValid();

            if(History.HisFreqs[key] >= 4){
                queue_.insertBack(written_entry); 
                // Update the occupied entries set
                occupied_entries_set_.insert(key);
                UsedSpace += Sizes1[key];
                History.erase_elem(key);
                HisSpace -= Sizes1[key];
            }

            // If required, evict the LRU entry
            while(UsedSpace > getNumEntries()){
                evicted_entry = queue_.popFront();
                assert(evicted_entry.isValid()); // Sanity check
                UsedSpace -= Sizes1[evicted_entry.key()];
                occupied_entries_set_.erase(evicted_entry.key());
            }
        }

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
class LRUKCache : public BaseCache {
public:
    LRUKCache(const size_t miss_latency, const size_t cache_set_associativity, const size_t
             num_cache_sets, const bool penalize_insertions, const HashType hash_type, int
             argc, char** argv) : BaseCache(miss_latency, cache_set_associativity,
             num_cache_sets, penalize_insertions, hash_type) {
        SUPPRESS_UNUSED_WARNING(argc);
        SUPPRESS_UNUSED_WARNING(argv);

        // Initialize the cache sets
        for (size_t idx = 0; idx < kMaxNumCacheSets; idx++) {
            cache_sets_.push_back(new LRUKCacheSet(kCacheSetAssociativity,miss_latency));
        }
    }
    virtual ~LRUKCache() {}

    /**
     * Returns the canonical cache name.
     */
    virtual std::string name() const override { return "LRUKCache"; }
};

// Run default benchmarks
int main(int argc, char** argv) {
    BaseCache::defaultBenchmark<LRUKCache>(argc, argv);
}
