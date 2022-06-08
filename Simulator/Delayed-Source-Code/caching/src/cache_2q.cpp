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
class TwoQCacheSet : public BaseCacheSet {
protected:
    LRUQueue<CacheEntry> queue_; // LRU Queue
    FIFOQueue<CacheEntry> Fifo;
    LRUQueue<CacheEntry> Lru;


public:
    double P = 0.9;
    uint64_t CSize = getNumEntries();
    uint64_t Lru_size = uint64_t(CSize * P);
    uint64_t Fifo_size = CSize - Lru_size;//uint64_t(CSize * (1 - P));

    TwoQCacheSet(const size_t num_entries, const size_t misslat) : BaseCacheSet(num_entries,misslat) {}
    virtual ~TwoQCacheSet() {}

    std::unordered_map<std::string,uint64_t> Sizes1;
    uint64_t UsedSpace_Fifo = 0;
    uint64_t UsedSpace_Lru = 0;

    virtual int
    update_freqs(const std::string& key, uint64_t size){
       //SUPPRESS_UNUSED_WARNING(key);
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
        CacheEntry evicted_entry;
        
        //首先检查Item所在的list
        int incache = -1;
        auto position_iter_0 = Lru.positions().find(key);
        if (position_iter_0 != Lru.positions().end()) {
            incache = 0;
            // LRU Hit
            written_entry = *(position_iter_0->second);

            // Sanity checks
            assert(contains(key));
            assert(written_entry.isValid());
            assert(written_entry.key() == key);
            // If the read was successful, the corresponding entry is
            // the MRU element in the cache. Remove it from the queue.
            Lru.erase(position_iter_0);

            //更新LRU的list
            Lru.insertBack(written_entry);
        }

        auto position_iter = Fifo.positions().find(key);
        if (position_iter != Fifo.positions().end()) {
            incache = 1;
            // FIFO Hit
            written_entry = *(position_iter->second);

            // Sanity checks
            assert(contains(key));
            assert(written_entry.isValid());
            assert(written_entry.key() == key);

            // If the read was successful, the corresponding entry is
            // the MRU element in the cache. Remove it from the queue.
            Fifo.erase(position_iter);
            UsedSpace_Fifo -= Sizes1[written_entry.key()];

            //数据转移到LRU的list
            Lru.insertBack(written_entry);
            UsedSpace_Lru += Sizes1[written_entry.key()];

            // If required, evict the LRU entry
            while(UsedSpace_Lru > Lru_size){
                evicted_entry = Lru.popFront();
                assert(evicted_entry.isValid()); // Sanity check
                UsedSpace_Lru -= Sizes1[evicted_entry.key()];
                occupied_entries_set_.erase(evicted_entry.key());
            }

        }

        if(incache == -1){
            written_entry.update(key);
            written_entry.toggleValid();

            // Update the occupied entries set
            occupied_entries_set_.insert(key);

            Fifo.insertBack(written_entry);
            UsedSpace_Fifo += Sizes1[key];

            while(UsedSpace_Fifo >= Fifo_size){
                evicted_entry = Fifo.popFront();
                assert(evicted_entry.isValid()); // Sanity check
                UsedSpace_Fifo -= Sizes1[evicted_entry.key()];
                occupied_entries_set_.erase(evicted_entry.key());
            }
        }

        // Sanity checks
        assert(occupied_entries_set_.size() <= getNumEntries());
        assert(occupied_entries_set_.size() == (Lru.size()+Fifo.size()));
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
class TwoQCache : public BaseCache {
public:
    TwoQCache(const size_t miss_latency, const size_t cache_set_associativity, const size_t
             num_cache_sets, const bool penalize_insertions, const HashType hash_type, int
             argc, char** argv) : BaseCache(miss_latency, cache_set_associativity,
             num_cache_sets, penalize_insertions, hash_type) {
        SUPPRESS_UNUSED_WARNING(argc);
        SUPPRESS_UNUSED_WARNING(argv);

        // Initialize the cache sets
        for (size_t idx = 0; idx < kMaxNumCacheSets; idx++) {
            cache_sets_.push_back(new TwoQCacheSet(kCacheSetAssociativity,miss_latency));
        }
    }
    virtual ~TwoQCache() {}

    /**
     * Returns the canonical cache name.
     */
    virtual std::string name() const override { return "TwoQCache"; }
};

// Run default benchmarks
int main(int argc, char** argv) {
    BaseCache::defaultBenchmark<TwoQCache>(argc, argv);
}
