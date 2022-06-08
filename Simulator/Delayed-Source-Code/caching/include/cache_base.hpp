#ifndef cache_base_h
#define cache_base_h

// STD headers
#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Boost headers
#include <boost/bimap.hpp>
#include <boost/program_options.hpp>

// Custom headers
#include "utils.hpp"
#include "cache_common.hpp"

template<class T>
std::vector<std::string> split(const std::string& s, const T& separator, bool ignore_empty = 0, bool split_empty = 0)
{
  struct {
    auto len(const std::string&             s) { return s.length(); }
    auto len(const std::string::value_type* p) { return p ? std::char_traits<std::string::value_type>::length(p) : 0; }
    auto len(const std::string::value_type  c) { return c == std::string::value_type() ? 0 : 1; /*return 1;*/ }
  } util;
  
  if (s.empty()) { /// empty string ///
    if (!split_empty || util.len(separator)) return {""};
    return {};
  }
  
  auto v = std::vector<std::string>();
  auto n = static_cast<std::string::size_type>(util.len(separator));
  if (n == 0) {    /// empty separator ///
    if (!split_empty) return {s};
    for (auto&& c : s) v.emplace_back(1, c);
    return v;
  }
  
  auto p = std::string::size_type(0);
  while (1) {      /// split with separator ///
    auto pos = s.find(separator, p);
    if (pos == std::string::npos) {
      if (ignore_empty && p - n + 1 == s.size()) break;
      v.emplace_back(s.begin() + p, s.end());
      break;
    }
    if (!ignore_empty || p != pos)
      v.emplace_back(s.begin() + p, s.begin() + pos);
    p = pos + n;
  }
  return v;
}


namespace caching {

/**
 * Abstract base class representing a generic cache-set.
 */
class BaseCacheSet {
protected:
    const size_t kNumEntries; // The number of cache entries in this set
    const size_t MissLatency;
    std::unordered_set<std::string> occupied_entries_set_; // Set of currently
                                                           // cached flow IDs.
    std::vector<std::string> TraceIds;

public:
    BaseCacheSet(const size_t num_entries, const size_t misslat) : kNumEntries(num_entries),MissLatency(misslat) {}
    virtual ~BaseCacheSet() {}

    
    virtual void inittrace(std::vector<std::string> Ids) {SUPPRESS_UNUSED_WARNING(Ids);}

    virtual int
    update_freqs(const std::string& key, uint64_t size)=0;

    // Membership test (internal use only)
    bool contains(const std::string& flow_id) const {
        return (occupied_entries_set_.find(flow_id) !=
                occupied_entries_set_.end());
    }
    /**
     * Returns the number of cache entries in this set
     */
    size_t getNumEntries() const { return kNumEntries; }

    /**
     * Records arrival of a new packet.
     */
    virtual void recordPacketArrival(const utils::Packet& packet) {
        SUPPRESS_UNUSED_WARNING(packet);
    }

    /**
     * Simulates a cache write.
     *
     * @param key The key corresponding to this write request.
     * @param packet The packet corresponding to this write request.
     * @return The written CacheEntry instance.
     */
    virtual CacheEntry
    write(const std::string& key, const utils::Packet& packet) = 0;

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
    writeq(const std::list<utils::Packet>& queue) {
        CacheEntry written_entry;
        for (const auto& packet : queue) {
            written_entry = write(packet.getFlowId(), packet);
        }
        return written_entry;
    }
};

/**
 * Abstract base class representing a single-tiered cache.
 */
class BaseCache {
protected:
    const size_t kCacheMissLatency;         // Cost (in cycles) of an L1 cache miss
    const size_t kMaxNumCacheSets;          // Maximum number of sets in the L1 cache
    const size_t kMaxNumCacheEntries;       // Maximum number of entries in the L1 cache
    const size_t kCacheSetAssociativity;    // Set-associativity of the L1 cache
    const size_t kIsPenalizeInsertions;     // Whether insertions should incur an L1 cache miss
    const HashFamily kHashFamily;           // A HashFamily instance

    size_t clk_ = 0; // Time in clock cycles
    size_t total_latency_ = 0; // Total packet latency
    std::vector<BaseCacheSet*> cache_sets_; // Fixed-sized array of CacheSet instances
    std::unordered_set<std::string> memory_entries_; // Set of keys in the global store
    boost::bimap<size_t, std::string> completed_reads_; // A dictionary mapping clk values to the keys
                                                        // whose blocking reads complete on that cycle.

    std::unordered_map<std::string, std::list<utils::Packet>>
    packet_queues_; // Dictionary mapping keys to queued requests. Each queue
                    // contains zero or more packets waiting to be processed.

    std::vector<std::string> TraceIds;
    size_t Misses = 0;
    size_t FHits = 0;
    size_t DHits = 0;
    std::string HITRecd = "";
    std::string LATRecd = "";
    double BWidth = 104857600;
    std::unordered_map<std::string, double> Arrives;

public:
    BaseCache(const size_t miss_latency, const size_t cache_set_associativity, const size_t
              num_cache_sets, const bool penalize_insertions, const HashType hash_type) :
              kCacheMissLatency(miss_latency), kMaxNumCacheSets(num_cache_sets),
              kMaxNumCacheEntries(num_cache_sets * cache_set_associativity),
              kCacheSetAssociativity(cache_set_associativity),
              kIsPenalizeInsertions(penalize_insertions),
              kHashFamily(1, hash_type) {}

    virtual ~BaseCache() {
        // Deallocate the cache sets
        assert(cache_sets_.size() == kMaxNumCacheSets);
        for (size_t idx = 0; idx < kMaxNumCacheSets; idx++) { delete(cache_sets_[idx]); }
    }

    /**
     * Returns the canonical cache name.
     */
    virtual std::string name() const = 0;

    /**
     * Records arrival of a new packet.
     */
    virtual void recordPacketArrival(const utils::Packet& packet) {
        SUPPRESS_UNUSED_WARNING(packet);
    }


    bool Init = 0;

    /**
     * Returns the cache miss latency.
     */
    size_t getCacheMissLatency() const { return kCacheMissLatency; }

    /**
     * Returns the number of entries in the memory at any instant.
     */
    size_t getNumMemoryEntries() const { return memory_entries_.size(); }

    size_t getCacheNumEntries() const { return kMaxNumCacheEntries; }

    /**
     * Returns the current time in clock cycles.
     */
    size_t clk() const { return clk_; }

    /**
     * Returns the total packet latency for this simulation.
     */
    size_t getTotalLatency() const { return total_latency_; }

    /**
     * 返回 Hit Probability
    */
    std::vector<size_t> getHits(){
         std::vector<size_t> Res = {FHits, DHits, Misses};
         return Res;
    }

    std::vector<std::string> getDynRecd(){
        std::vector<std::string> Res = {HITRecd,LATRecd};
        return Res;
    }


    /**
     * Returns the cache index corresponding to the given key.
     */
    size_t getCacheIndex(const std::string& key) const {
        return (kMaxNumCacheSets == 1) ?
            0 : kHashFamily.hash(0, key) % kMaxNumCacheSets;
    }

    /**
     * Increments the internal clock.
     */
    void incrementClk() { clk_++; }

    void setTraceIds(std::vector<std::string> Ids){TraceIds = Ids;}

    /**
     * Handles any blocking read completions on this cycle.
     */
    void processAll(std::list<utils::Packet>& processed_packets) {

        // A blocking read completed on this cycle
        auto completed_read = completed_reads_.left.find(clk());
        if (completed_read != completed_reads_.left.end()) {
            const std::string& key = completed_read->second;
            std::list<utils::Packet>& queue = packet_queues_.at(key);
            assert(!queue.empty()); // Sanity check: Queue may not be empty

            // Fetch the cache set corresponding to this key
            size_t cache_idx = getCacheIndex(key);
            BaseCacheSet& cache_set = *cache_sets_[cache_idx];

            // Sanity checks
            assert(!cache_set.contains(key));
            assert(queue.front().getTotalLatency() == kCacheMissLatency);

            // Commit the queued entries
            cache_set.writeq(queue);
            processed_packets.insert(processed_packets.end(),
                                     queue.begin(), queue.end());

            // Purge the queue, as well as the bidict mapping
            queue.clear();
            packet_queues_.erase(key);
            completed_reads_.left.erase(completed_read);
        }

        // Finally, increment the clock
        incrementClk();
    }



    void processAriv(std::list<utils::Packet>& processed_packets) {
        std::unordered_map<std::string, double>::iterator Iter;
        double TimeNow = clk();
        std::vector<std::string> DelKeys;

        for(Iter = Arrives.begin();Iter != Arrives.end();Iter++){
            double ariv_time = Iter->second;
            std::string key = Iter->first;
            if(ariv_time <= TimeNow){
                std::list<utils::Packet>& queue = packet_queues_.at(key);
            assert(!queue.empty()); // Sanity check: Queue may not be empty

                // Fetch the cache set corresponding to this key
                size_t cache_idx = getCacheIndex(key);
                BaseCacheSet& cache_set = *cache_sets_[cache_idx];

                //std::cout<<gsize<<std::endl;
		
                // Sanity checks
                assert(!cache_set.contains(key));

                // Commit the queued entries
                cache_set.writeq(queue);
                processed_packets.insert(processed_packets.end(),
                                     queue.begin(), queue.end());

                // Purge the queue, as well as the bidict mapping
                queue.clear();
                packet_queues_.erase(key);
                DelKeys.push_back(key);
            }
         }
         
         std::vector<std::string>::iterator Vcer;
         for(Vcer=DelKeys.begin();Vcer!=DelKeys.end();Vcer++){
                Arrives.erase(*Vcer);
         }
         incrementClk();
    }



    /**
     * Processes the parameterized packet.
     */
    void process(utils::Packet& packet, std::list<
                 utils::Packet>& processed_packets) {
        packet.setArrivalClock(clk());

        const std::string& key = packet.getFlowId();
        auto queue_iter = packet_queues_.find(key);
        BaseCacheSet& cache_set = *cache_sets_.at(
            getCacheIndex(key));
 
        if(Init == 0){
            cache_set.inittrace(TraceIds);
            Init = 1;
        }
        uint64_t size = packet.getFlowSize();
        cache_set.update_freqs(key,size);

        // Record arrival of the packet at
        // the cache and cache-set levels.
        recordPacketArrival(packet);
        cache_set.recordPacketArrival(packet);

        // If this packet corresponds to a new flow, allocate its context
        if (memory_entries_.find(key) == memory_entries_.end()) {
            assert(!cache_set.contains(key));
            memory_entries_.insert(key);

            if (!kIsPenalizeInsertions) {
                cache_set.write(key, packet);
            }
        }
        // First, if the flow is cached, process the packet immediately.
        // This implies that the packet queue must be non-existent.
        std::string hittype = "2";
        std::string lattype = "0\n";
        if (cache_set.contains(key)) {
            FHits += 1;

            assert(queue_iter == packet_queues_.end());

            cache_set.write(key, packet);

            packet.finalize();
            processed_packets.push_back(packet);
            total_latency_ += packet.getTotalLatency();
        }
        // Else, we must either: a) perform a blocking read from memory,
        // or b) wait for an existing blocking read to complete. Insert
        // this packet into the corresponding packet queue.
        else {
            if (queue_iter == packet_queues_.end()) {
                Misses += 1;
                hittype = "0";

                double misslatnow = kCacheMissLatency + size * 1000 / (BWidth / 1.0);
                double target_clk = clk() + misslatnow + 1;

                Arrives[key] = target_clk;
                packet.addLatency(misslatnow);
                packet.finalize();

                // Initialize a new queue for this flow
                packet_queues_[key].push_back(packet);

                lattype = std::to_string(int(misslatnow)) + "\n";
            }
            // Update the flow's packet queue
            else {
                DHits += 1;
                hittype = "1"; 

                double target_clk = Arrives[key];//completed_reads_.right.at(key);
                packet.setQueueingDelay(queue_iter->second.size());
                packet.addLatency(target_clk - clk());
                packet.finalize();

                // Add this packet to the existing flow queue
                queue_iter->second.push_back(packet);

                lattype = std::to_string(int(target_clk-clk())) + "\n";
            }
            assert(packet.isFinalized()); // Sanity check
            total_latency_ += packet.getTotalLatency();
        }
        // Process any completed reads
        processAriv(processed_packets);
        HITRecd += hittype;
        LATRecd += lattype;
    }

    /**
     * Indicates completion of the warmup period.
     */
    void warmupComplete() {
        total_latency_ = 0;
        packet_queues_.clear();
        completed_reads_.clear();
    }

    /**
     * Indicates completion of the simulation.
     */
    void teardown(std::list<utils::Packet>& processed_packets) {
        while (!packet_queues_.empty()) {
            processAriv(processed_packets);
        }
    }

    /**
     * Save the raw packet data to file.
     */
    static void savePackets(std::list<utils::Packet>& packets,
                            const std::string& packets_fp) {
        if (!packets_fp.empty()) {
            std::ofstream file(packets_fp, std::ios::out |
                                           std::ios::app);
            // Save the raw packets to file
            for (const utils::Packet& packet : packets) {
                file << packet.getFlowId() << ";"
                     << static_cast<size_t>(packet.getTotalLatency()) << ";"
                     << static_cast<size_t>(packet.getQueueingDelay()) << std::endl;
            }
        }
        packets.clear();
    }

    /**
     * Generate and output model benchmarks.
     */
    static void benchmark(BaseCache& model, const std::string& trace_fp, const std::
                          string& packets_fp, const size_t num_warmup_cycles, std::string root_fp) {
        std::list<utils::Packet> packets; // List of processed packets
        size_t num_counted_packets = 0; // Post-warmup packet count
        size_t num_total_packets = 0; // Total packet count
        size_t num_total_cycles = 0; // Total cycle count

        if (!packets_fp.empty()) {
            std::ofstream file(packets_fp, std::ios::out |
                                           std::ios::trunc);
            // Write the header
            file << model.name() << ";" << model.kCacheSetAssociativity << ";"
                 << model.kMaxNumCacheSets << ";" << model.kMaxNumCacheEntries
                 << std::endl;
        }

        //get all trace information
        if(1>0){
            // Process the trace
            std::string line;
            std::ifstream trace_ifs(trace_fp);
            std::vector<std::string> TraceIds;
            while (std::getline(trace_ifs, line)) {
                std::string timestamp, flow_id;

                // Nonempty packet
                if (!line.empty()) {
                    std::stringstream linestream(line);
                    std::string field;
                    std::istringstream sin(line);
                    std::getline(sin,field);
                
                    std::string Data = (std::string)field;
                    std::vector<std::string> GetData = split(Data,";");
                    timestamp = GetData[0];
                    flow_id = GetData[1];
                    TraceIds.push_back(flow_id);
                 }
            }
            model.setTraceIds(TraceIds);
        }


        // Process the trace
        std::string line;
        std::ifstream trace_ifs(trace_fp);
        while (std::getline(trace_ifs, line)) {
            std::string timestamp, flow_id;
            uint64_t flow_size = 1;

            /****************************************
             * Important note: Currently, we ignore *
             * packet timestamps and inject at most *
             * 1 packet into the system each cycle. *
             ****************************************/

            // Nonempty packet
            if (!line.empty()) {
                std::stringstream linestream(line);

                std::string field;
                std::istringstream sin(line);
                std::getline(sin,field);
                
                std::string Data = (std::string)field;
                std::vector<std::string> GetData = split(Data,";");
                timestamp = GetData[0];
                flow_id = GetData[1];
                flow_size = std::stoi(GetData[2]);
            }
            // Cache warmup completed
            if (num_total_cycles == num_warmup_cycles) {
                model.warmupComplete(); packets.clear();
                num_counted_packets = 0;
            }
            // Periodically save packets to file
            if (num_counted_packets > 0 &&
                (num_counted_packets + 1) % 100000 == 0) {
                if (num_total_cycles >= num_warmup_cycles) {
                    savePackets(packets, packets_fp);
                }
                std::cout << "Processing: " << num_counted_packets + 1 << std::endl;
            }
            // Process the packet
            if (!flow_id.empty()) {
                num_total_packets++;
                num_counted_packets++;
                utils::Packet packet(flow_id,flow_size);
                model.process(packet, packets);
            }
            else { model.processAriv(packets); }
            num_total_cycles++;
        }

        // Perform teardown
        model.teardown(packets);
        savePackets(packets, packets_fp);

        // Debug: Print trace and simulation statistics
        std::cout << std::endl;
        std::cout<< "Results:" << std::endl;
        std::cout<< "Algorithm: " << model.name() << std::endl;
        std::cout << "Total latency: " << model.getTotalLatency() << std::endl;
        std::vector<size_t> HITs = model.getHits();
        std::cout << "Full Hit: " << HITs[0] << std::endl;
	std::cout << "Delayed Hit: " << HITs[1] << std::endl;
        std::cout << "Miss: " << HITs[2] << std::endl;
        std::cout<<"--------------------"<<std::endl;
        std::cout << std::endl;

        // Records all Results into a File

        std::string Root = root_fp;
        std::string PathW = Root + model.name() + "_" + std::to_string(int(model.getCacheNumEntries()/1024/1024)) + "c_" + std::to_string(model.getCacheMissLatency()) + "l.txt";
        std::ofstream WFile;
        std::vector<std::string> DynRes = model.getDynRecd();
        WFile.open(PathW);
        WFile << "Total latency is:" << model.getTotalLatency() << std::endl;
        WFile << "Full Hit:" << HITs[0] << std::endl;
        WFile << "Delayed Hit:" << HITs[1] << std::endl;
        WFile << "Miss:" << HITs[2] << std::endl;
        WFile << "Latency:\n" << DynRes[1] << std::endl;
        WFile.close();
    }

    /**
     * Run default benchmarks.
     */
    template<class T>
    static void defaultBenchmark(uint64_t argc, char** argv) {
        using namespace boost::program_options;

        // Parameters
        size_t z;
        double c_scale;
        std::string trace_fp;
        std::string packets_fp;
        std::string root_fp;
        size_t set_associativity;
        size_t num_warmup_cycles;

        // Program options
        variables_map variables;
        options_description desc{"A caching simulator that models Delayed Hits"};

        try {
            // Command-line arguments
            desc.add_options()
                ("help",        "Prints this message")
                ("trace",       value<std::string>(&trace_fp)->required(),            "Input trace file path")
                ("outpath",       value<std::string>(&root_fp)->required(),            "output path")
                ("csize",      value<double>(&c_scale)->required(),                  "Parameter: Cache size (%Concurrent Flows)")
                ("latency",     value<size_t>(&z)->required(),                        "Parameter: u")
                ("packets",     value<std::string>(&packets_fp)->default_value(""),   "[Optional] Output packets file path")
                ("csa",         value<size_t>(&set_associativity)->default_value(0),  "[Optional] Parameter: Cache set-associativity")
                ("warmup",      value<size_t>(&num_warmup_cycles)->default_value(0),  "[Optional] Parameter: Number of cache warm-up cycles");

            // Parse model parameters
            store(parse_command_line(argc, argv, desc), variables);

            // Handle help flag
            if (variables.count("help")) {
                std::cout << desc << std::endl;
                return;
            }
            store(command_line_parser(argc, argv).options(
                desc).allow_unregistered().run(), variables);
            notify(variables);
        }
        // Flag argument errors
        catch(const required_option& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return;
        }
        catch(...) {
            std::cerr << "Unknown Error." << std::endl;
            return;
        }

        // Compute the set associativity and set count
        double cache_size = c_scale * 1024 * 1024;//(num_cfs * c_scale) / 100.0;
        if (set_associativity == 0) { set_associativity = std::max<size_t>(
            1, static_cast<size_t>(round(cache_size)));
        }
        size_t num_cache_sets = std::max<size_t>(1,
            static_cast<size_t>(round(cache_size / set_associativity)));

        // Debug: Print the cache and trace parameters
        std::cout << std::endl;
        std::cout << "Parameters: c=" << c_scale << ", l=" << z << std::endl;

        // Instantiate the model
        T model(z, set_associativity, num_cache_sets, true,
                HashType::MURMUR_HASH, argc, argv);

        std::cout << "Starting:" << std::endl;
        BaseCache::benchmark(model, trace_fp, packets_fp, num_warmup_cycles,root_fp);
    }
};

} // namespace caching

#endif // cache_base_h
