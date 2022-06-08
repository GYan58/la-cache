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
class LRUCacheSet : public BaseCacheSet {
protected:
    BeladySQueue<CacheEntry> bqueue_;

public:
    LRUCacheSet(const size_t num_entries, const size_t misslat) : BaseCacheSet(num_entries,misslat) {}
    virtual ~LRUCacheSet() {}

    std::unordered_map<std::string,uint64_t> Sizes1;
    uint64_t UsedSpace = 0;
    std::vector<std::string> Trace;
    uint64_t TimeProc = 0;

    virtual void
    inittrace(std::vector<std::string> Ids){
        Trace = Ids;
        std::cout<<"Trace Size:"<<Trace.size()<<std::endl;
        bqueue_.setTrace(Trace);
        bqueue_.process();
    }

    virtual int
    update_freqs(const std::string& key, uint64_t size){
       //SUPPRESS_UNUSED_WARNING(key);
       //SUPPRESS_UNUSED_WARNING(size);
       bqueue_.updateNRTs(key);
       if(Sizes1.find(key) == Sizes1.end()){
           Sizes1[key] = size;
           bqueue_.Sizes[key] = size;
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

        //std::cout<<UsedSpace<<" "<<getNumEntries()<<std::endl;

        // If a corresponding entry exists, update it
        auto position_iter = bqueue_.positions().find(key);
        if (position_iter != bqueue_.positions().end()) {
            written_entry = *(position_iter->second);

            // Sanity checks
            assert(contains(key));
            assert(written_entry.isValid());
            assert(written_entry.key() == key);

            // If the read was successful, the corresponding entry is
            // the MRU element in the cache. Remove it from the queue.
            bqueue_.erase(position_iter);
        }
        // The update was unsuccessful, create a new entry to insert
        else {
            written_entry.update(key);
            written_entry.toggleValid();

            // If required, evict the LRU entry
            //if (queue_.size() == getNumEntries()) {
            while(UsedSpace >= getNumEntries()){
                evicted_entry = bqueue_.popMax();
                assert(evicted_entry.isValid()); // Sanity check
                UsedSpace -= Sizes1[evicted_entry.key()];
                occupied_entries_set_.erase(evicted_entry.key());
            }

            // Update the occupied entries set
            occupied_entries_set_.insert(key);
            UsedSpace += Sizes1[key];
        }

        // Finally, (re-)insert the written entry at the back
        bqueue_.insertBack(written_entry);

        TimeProc++;

        // Sanity checks
        assert(occupied_entries_set_.size() <= getNumEntries());
        assert(occupied_entries_set_.size() == bqueue_.size());
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
class LRUCache : public BaseCache {
public:
    LRUCache(const size_t miss_latency, const size_t cache_set_associativity, const size_t
             num_cache_sets, const bool penalize_insertions, const HashType hash_type, int
             argc, char** argv) : BaseCache(miss_latency, cache_set_associativity,
             num_cache_sets, penalize_insertions, hash_type) {
        SUPPRESS_UNUSED_WARNING(argc);
        SUPPRESS_UNUSED_WARNING(argv);

        // Initialize the cache sets
        for (size_t idx = 0; idx < kMaxNumCacheSets; idx++) {
            cache_sets_.push_back(new LRUCacheSet(kCacheSetAssociativity,miss_latency));
        }
    }
    virtual ~LRUCache() {}

    /**
     * Returns the canonical cache name.
     */
    virtual std::string name() const override { return "BeladySCache"; }
};

// Run default benchmarks
int main(int argc, char** argv) {
    BaseCache::defaultBenchmark<LRUCache>(argc, argv);
}
