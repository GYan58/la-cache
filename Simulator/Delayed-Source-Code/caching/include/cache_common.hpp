#ifndef cache_common_h
#define cache_common_h

// STD headers
#include <assert.h>
#include <string>
#include <unordered_map>
#include <numeric>

// Custom headers
#include "MurmurHash3.h"

namespace caching {

/**
 * Represents a single cache entry.
 */
class CacheEntry {
private:
    std::string key_; // Tag used to uniquely identify objects
    bool is_valid_ = false; // Whether this cache entry is valid
    uint64_t objsize_;

public:
    // Accessors
    bool isValid() const { return is_valid_; }
    const std::string& key() const { return key_; }

    // Mutators
    void toggleValid() { is_valid_ = !is_valid_; }
    void update(const std::string& key) { key_ = key; }
};



/**
 * Implements a parameterizable LRU queue.
 */
template<class T> class LRUQueue {
private:
    typedef typename std::list<T>::iterator Iterator;
    typedef typename std::unordered_map<std::string, Iterator>::iterator PositionIterator;
    std::unordered_map<std::string, Iterator> positions_; // A dict mapping keys to iterators
    std::list<T> entries_; // An ordered list of T instances. The list is ordered such that, at
                           // any time, the element at the front of the queue is the LRU entry.
    // Helper method
    const std::string& getKey(const T& entry) const { return entry.key(); }

public:
    // Accessors
    std::list<T>& entries() { return entries_; }
    size_t size() const { return entries_.size(); }
    const std::list<T>& entries() const { return entries_; }
    std::unordered_map<std::string, Iterator>& positions() { return positions_; }
    const std::unordered_map<std::string, Iterator>& positions() const { return positions_; }

     std::unordered_map<std::string,uint64_t> Sizes2;

    /**
     * Membership test.
     */
    bool contains(const std::string& key) const {
        return (positions_.find(key) != positions_.end());
    }

    /**
     * Erase the given queue entry.
     */
    void erase(const PositionIterator& position_iter) {
        entries_.erase(position_iter->second);
        positions_.erase(position_iter);
    }

    /**
     * Pop the entry at the front of the queue.
     */
    T popFront() {
        T entry = entries_.front();
        positions_.erase(getKey(entry));
        entries_.pop_front();
        return entry;
    }

    /**
     * Insert the given entry at the back of the queue.
     */
    void insertBack(const T& entry) {
        const std::string& key = getKey(entry);
        assert(positions_.find(key) == positions_.end());

        entries_.push_back(entry);
        positions_[key] = std::prev(entries_.end());
    }
};

// Template specializations
template<> const std::string&
LRUQueue<std::string>::getKey(const std::string& entry) const { return entry; }


/**
 * Implements a parameterizable LFU queue.
 */
template<class T> class LFUQueue {
private:
    typedef typename std::list<T>::iterator Iterator;
    typedef typename std::unordered_map<std::string, Iterator>::iterator PositionIterator;
    std::unordered_map<std::string, Iterator> positions_; // A dict mapping keys to iterators
    std::list<T> entries_; // An ordered list of T instances. The list is ordered such that, at
                           // any time, the element at the front of the queue is the LFU entry.
    // Helper method
    const std::string& getKey(const T& entry) const { return entry.key(); }

public:
    // Accessors
    std::list<T>& entries() { return entries_; }
    size_t size() const { return entries_.size(); }
    const std::list<T>& entries() const { return entries_; }
    std::unordered_map<std::string, Iterator>& positions() { return positions_; }
    const std::unordered_map<std::string, Iterator>& positions() const { return positions_; }

    std::unordered_map<std::string, uint64_t> Freqs;
    std::unordered_map<std::string,uint64_t> Sizes2;


    /**
     * Membership test.
     */
    bool contains(const std::string& key) const {
        return (positions_.find(key) != positions_.end());
    }

    /**
     * Erase the given queue entry.
     */
    void erase(const PositionIterator& position_iter) {
        entries_.erase(position_iter->second);
        positions_.erase(position_iter);
    }

    /**
     * Pop the entry at the front of the queue.
     */
    T popFreq() {
        class std::list<T>::iterator It;
        class std::list<T>::iterator Loc;
        uint64_t find_val = 1000000000;
        //uint64_t find_loc = 0;
        T entry;
        for(It = entries_.begin(); It != entries_.end();It++){
            std::string gkey = getKey(*It);
            uint64_t Val = Freqs[gkey];
            if(Val < find_val){
                find_val = Val;
                entry = *It;
                Loc = It;
            }
        }

        positions_.erase(getKey(entry));
        entries_.erase(Loc);
        return entry;
    }

    /**
     * Insert the given entry at the back of the queue.
     */
    void insertBack(const T& entry) {
        const std::string& key = getKey(entry);
        assert(positions_.find(key) == positions_.end());

        entries_.push_back(entry);
        positions_[key] = std::prev(entries_.end());
        
    }
};

// Template specializations
template<> const std::string&
LFUQueue<std::string>::getKey(const std::string& entry) const { return entry; }



/**
 * Implements a parameterizable FIFO queue.
 */
template<class T> class FIFOQueue {
private:
    typedef typename std::list<T>::iterator Iterator;
    typedef typename std::unordered_map<std::string, Iterator>::iterator PositionIterator;
    std::unordered_map<std::string, Iterator> positions_; // A dict mapping keys to iterators
    std::list<T> entries_; // An ordered list of T instances. The list is ordered such that, at
                           // any time, the element at the front of the queue is the FIFO entry.
    // Helper method
    const std::string& getKey(const T& entry) const { return entry.key(); }

public:
    // Accessors
    std::list<T>& entries() { return entries_; }
    size_t size() const { return entries_.size(); }
    const std::list<T>& entries() const { return entries_; }
    std::unordered_map<std::string, Iterator>& positions() { return positions_; }
    const std::unordered_map<std::string, Iterator>& positions() const { return positions_; }

    std::unordered_map<std::string,uint64_t> Sizes2;

    /**
     * Membership test.
     */
    bool contains(const std::string& key) const {
        return (positions_.find(key) != positions_.end());
    }

    /**
     * Erase the given queue entry.
     */
    void erase(const PositionIterator& position_iter) {
        entries_.erase(position_iter->second);
        positions_.erase(position_iter);
    }

    /**
     * Pop the entry at the front of the queue.
     */
    T popFront() {
        T entry = entries_.front();
        positions_.erase(getKey(entry));
        entries_.pop_front();
        return entry;
    }

    /**
     * Insert the given entry at the back of the queue.
     */
    void insertBack(const T& entry) {
        const std::string& key = getKey(entry);
        assert(positions_.find(key) == positions_.end());

        entries_.push_back(entry);
        positions_[key] = std::prev(entries_.end());
    }
};

// Template specializations
template<> const std::string&
FIFOQueue<std::string>::getKey(const std::string& entry) const { return entry; }



/*
Class for PB-LRU
*/
class InTimes{
private:
    double num_windows_ = 0;
    double cumulative_itimes_ = 0;

public:
    uint64_t L = 20;
    std::list<double> CTimes;
    /**
     * Records a packet arrival corresponding to this flow.
     */
    void recordArrivTimes(double itime) {
        //num_windows_++;
        //cumulative_itimes_ += itime;
        CTimes.push_back(itime);
        if(CTimes.size() > L){
            CTimes.pop_front();
        }
    }
    /**
     * Returns the payoff for this flow.
     */
    double getLambda() {
        double Lamb = 0.00000001;
        if(CTimes.size() >= 3){
            double Sum = std::accumulate(CTimes.begin(),CTimes.end(),0);
            Lamb = 1.0 / (Sum / (double)CTimes.size());
        }
        return Lamb;
    }
};



/**
 * Implements a parameterizable PBLRU2 queue.
 */
template<class T> class PBSQueue {
private:
    typedef typename std::list<T>::iterator Iterator;
    typedef typename std::unordered_map<std::string, Iterator>::iterator PositionIterator;
    std::unordered_map<std::string, Iterator> positions_; // A dict mapping keys to iterators
    std::list<T> entries_; // An ordered list of T instances. The list is ordered such that, at
                           // any time, the element at the front of the queue is the LFU entry.
    // Helper method
    const std::string& getKey(const T& entry) const { return entry.key(); }

public:
    // Accessors
    std::list<T>& entries() { return entries_; }
    size_t size() const { return entries_.size(); }
    const std::list<T>& entries() const { return entries_; }
    std::unordered_map<std::string, Iterator>& positions() { return positions_; }
    const std::unordered_map<std::string, Iterator>& positions() const { return positions_; }

    std::unordered_map<std::string, uint64_t> Freqs;//记录元素的频率
    uint64_t MissLatency;
    uint64_t Timer = 0;
    double BWidth = 104857600.0;
    std::unordered_map<std::string,uint64_t> LRTs;
    std::unordered_map<std::string,InTimes> InterTimes;
    std::unordered_map<std::string,double> Lambdas;
    std::unordered_map<std::string,uint64_t> Sizes2;
    std::unordered_map<std::string,double> EvictRules;
    bool use2 = false;


    void set_Z(const size_t misslat){MissLatency = misslat; if(misslat <= 1000000){use2=true;}}

    /**
     * Membership test.
     */
    bool contains(const std::string& key) const {
        return (positions_.find(key) != positions_.end());
    }

    /**
     * Erase the given queue entry.
     */
    void erase(const PositionIterator& position_iter) {
        entries_.erase(position_iter->second);
        positions_.erase(position_iter);
    }

     /*
     update eviction rules
     */
    void update_evict(){
         class std::list<T>::iterator It;
         for(It = entries_.begin(); It != entries_.end();It++){
            std::string gkey = getKey(*It);
            double lrt = Timer - LRTs[gkey] + 1.0;
            double size = Sizes2[gkey] + 1.0;
            double glambda = Lambdas[gkey];
            if(use2 && lrt >= 12.0 * 1.0 / Lambdas[gkey]){ glambda = 1.0 / lrt;}
            double LT = glambda * (MissLatency + size * 1000 / BWidth);
            double Val = LT * (LT + 1) / (LT + 2) / 1.0 / size;
            //if(use2){Val = Val / lrt / 1.0;}
            EvictRules[gkey] = Val;
          }
     }
    

    /**
     * Pop the entry at the front of the queue.
     */
    T popMin() {
        class std::list<T>::iterator It;
        class std::list<T>::iterator Loc;
        double find_val = std::numeric_limits<double>::max();
        T entry;
        for(It = entries_.begin(); It != entries_.end();It++){
            std::string gkey = getKey(*It);
            double Val = EvictRules[gkey];
            if(Val < find_val){
                find_val = Val;
                entry = *It;
                Loc = It;
            }
        }

        positions_.erase(getKey(entry));
        entries_.erase(Loc);
        return entry;
    }

    /**
     * Insert the given entry at the back of the queue.
     */
    void insertBack(const T& entry) {
        const std::string& key = getKey(entry);
        assert(positions_.find(key) == positions_.end());

        entries_.push_back(entry);
        positions_[key] = std::prev(entries_.end());
    }
};

// Template specializations
template<> const std::string&
PBSQueue<std::string>::getKey(const std::string& entry) const { return entry; }



/**
 * Implements a parameterizable PBLRU2 queue.
 */
template<class T> class PBLQueue {
private:
    typedef typename std::list<T>::iterator Iterator;
    typedef typename std::unordered_map<std::string, Iterator>::iterator PositionIterator;
    std::unordered_map<std::string, Iterator> positions_; // A dict mapping keys to iterators
    std::list<T> entries_; // An ordered list of T instances. The list is ordered such that, at
                           // any time, the element at the front of the queue is the LFU entry.
    // Helper method
    const std::string& getKey(const T& entry) const { return entry.key(); }

public:
    // Accessors
    std::list<T>& entries() { return entries_; }
    size_t size() const { return entries_.size(); }
    const std::list<T>& entries() const { return entries_; }
    std::unordered_map<std::string, Iterator>& positions() { return positions_; }
    const std::unordered_map<std::string, Iterator>& positions() const { return positions_; }
    
    uint64_t MissLatency;
    uint64_t Timer = 0;
    double BWidth = 104857600.0;
    std::unordered_map<std::string,uint64_t> LRTs;
    std::unordered_map<std::string,InTimes> InterTimes;
    std::unordered_map<std::string,double> Lambdas;
    std::unordered_map<std::string,uint64_t> Sizes2;
    std::unordered_map<std::string,double> EvictRules;


    void set_Z(const size_t misslat){MissLatency = misslat;}

    /**
     * Membership test.
     */
    bool contains(const std::string& key) const {
        return (positions_.find(key) != positions_.end());
    }

    /**
     * Erase the given queue entry.
     */
    void erase(const PositionIterator& position_iter) {
        entries_.erase(position_iter->second);
        positions_.erase(position_iter);
    }

     /*
     update eviction rules
     */
    void update_evict(){
         class std::list<T>::iterator It;
         for(It = entries_.begin(); It != entries_.end();It++){
            std::string gkey = getKey(*It);
            double lrt = Timer - LRTs[gkey] + 1.0;
            double size = Sizes2[gkey] + 1.0;
            double LT = Lambdas[gkey] * (MissLatency + size * 1000 / BWidth);
            double Val = LT * (LT + 1) / (LT + 2) / lrt / 1.0 / size;
            EvictRules[gkey] = Val;
          }
     }
    

    /**
     * Pop the entry at the front of the queue.
     */
    T popMin() {
        class std::list<T>::iterator It;
        class std::list<T>::iterator Loc;
        double find_val = std::numeric_limits<double>::max();
        T entry;
        for(It = entries_.begin(); It != entries_.end();It++){
            std::string gkey = getKey(*It);
            double Val = EvictRules[gkey];
            if(Val < find_val){
                find_val = Val;
                entry = *It;
                Loc = It;
            }
        }

        positions_.erase(getKey(entry));
        entries_.erase(Loc);
        return entry;
    }

    /**
     * Insert the given entry at the back of the queue.
     */
    void insertBack(const T& entry) {
        const std::string& key = getKey(entry);
        assert(positions_.find(key) == positions_.end());

        entries_.push_back(entry);
        positions_[key] = std::prev(entries_.end());
    }
};

// Template specializations
template<> const std::string&
PBLQueue<std::string>::getKey(const std::string& entry) const { return entry; }



/**
 * Implements a parameterizable Belady queue.
 */
template<class T> class BeladyQueue {
private:
    typedef typename std::list<T>::iterator Iterator;
    typedef typename std::unordered_map<std::string, Iterator>::iterator PositionIterator;
    std::unordered_map<std::string, Iterator> positions_; // A dict mapping keys to iterators
    std::list<T> entries_; // An ordered list of T instances. The list is ordered such that, at
                           // any time, the element at the front of the queue is the LFU entry.
    // Helper method
    const std::string& getKey(const T& entry) const { return entry.key(); }

public:
    // Accessors
    std::list<T>& entries() { return entries_; }
    size_t size() const { return entries_.size(); }
    const std::list<T>& entries() const { return entries_; }
    std::unordered_map<std::string, Iterator>& positions() { return positions_; }
    const std::unordered_map<std::string, Iterator>& positions() const { return positions_; }

    std::vector<std::string> Trace;
    std::unordered_map<std::string,uint64_t> Counter;
    std::unordered_map<std::string,std::vector<uint64_t>> ReqTimes;
    std::unordered_map<std::string,uint64_t> NRTs;
    std::unordered_map<std::string,uint64_t> Sizes;
    uint64_t MaxLim = 1000000000;
    uint64_t Timenow = -1;

    void setTrace(std::vector<std::string> Ids){Trace = Ids;}
 
    void process(){
        uint64_t gtime = 0;
        for(uint64_t i=0;i<Trace.size();i++){
            std::string gky = Trace[i];
            if(ReqTimes.find(gky) != ReqTimes.end()){
                ReqTimes[gky].push_back(gtime);
            }
            else{
                Counter[gky] = 0;
                std::vector<uint64_t> GT;
                GT.push_back(gtime);
                ReqTimes[gky] = GT;
            }
            gtime++;
        }

        std::unordered_map<std::string,std::vector<uint64_t>>::iterator It;
        for(It = ReqTimes.begin();It != ReqTimes.end(); It++){
            std::string gky = It->first;
            ReqTimes[gky].push_back(MaxLim);
        }
    }

    void updateNRTs(std::string Ky){
        Timenow++;
        Counter[Ky]++;
        uint64_t ky_nrt = ReqTimes[Ky][Counter[Ky]] - Timenow + 1;
        NRTs[Ky] = ky_nrt;
        
        std::unordered_map<std::string,uint64_t>::iterator It;
        for(It = NRTs.begin();It != NRTs.end(); It++){
            std::string gky = It->first;
            uint64_t gval = NRTs[gky];
            if(gval != MaxLim){
                NRTs[gky] = gval - 1;
            }
         }
        //std::cout<<"Update Finished..."<<std::endl;
    }

    /**
     * Membership test.
     */
    bool contains(const std::string& key) const {
        return (positions_.find(key) != positions_.end());
    }

    /**
     * Erase the given queue entry.
     */
    void erase(const PositionIterator& position_iter) {
        entries_.erase(position_iter->second);
        positions_.erase(position_iter);
    }

     /**
     * Pop the entry at the front of the queue.
     */
    T popMax() {
        class std::list<T>::iterator It;
        class std::list<T>::iterator Loc;
        double find_val = -1.0;
        T entry;
        for(It = entries_.begin(); It != entries_.end();It++){
            std::string gkey = getKey(*It);
            double Val = NRTs[gkey] / 1.0;
            if(Val > find_val){
                find_val = Val;
                entry = *It;
                Loc = It;
            }
            if(Val <= 0){
                std::cout<<gkey<<" "<<Val<<std::endl;
            }
        }

        positions_.erase(getKey(entry));
        NRTs.erase(getKey(entry));
        entries_.erase(Loc);
        return entry;
    }


     /**
     * Insert the given entry at the back of the queue.
     */
    void insertBack(const T& entry) {
        const std::string& key = getKey(entry);
        assert(positions_.find(key) == positions_.end());

        entries_.push_back(entry);
        positions_[key] = std::prev(entries_.end());
    }
};

// Template specializations
template<> const std::string&
BeladyQueue<std::string>::getKey(const std::string& entry) const { return entry; }



/**
 * Implements a parameterizable Belady-Size queue.
 */
template<class T> class BeladySQueue {
private:
    typedef typename std::list<T>::iterator Iterator;
    typedef typename std::unordered_map<std::string, Iterator>::iterator PositionIterator;
    std::unordered_map<std::string, Iterator> positions_; // A dict mapping keys to iterators
    std::list<T> entries_; // An ordered list of T instances. The list is ordered such that, at
                           // any time, the element at the front of the queue is the LFU entry.
    // Helper method
    const std::string& getKey(const T& entry) const { return entry.key(); }

public:
    // Accessors
    std::list<T>& entries() { return entries_; }
    size_t size() const { return entries_.size(); }
    const std::list<T>& entries() const { return entries_; }
    std::unordered_map<std::string, Iterator>& positions() { return positions_; }
    const std::unordered_map<std::string, Iterator>& positions() const { return positions_; }

    std::vector<std::string> Trace;
    std::unordered_map<std::string,uint64_t> Counter;
    std::unordered_map<std::string,std::vector<uint64_t>> ReqTimes;
    std::unordered_map<std::string,uint64_t> NRTs;
    std::unordered_map<std::string,uint64_t> Sizes;
    uint64_t MaxLim = 1000000000;
    uint64_t Timenow = -1;

    void setTrace(std::vector<std::string> Ids){Trace = Ids;}
 
    void process(){
        uint64_t gtime = 0;
        for(uint64_t i=0;i<Trace.size();i++){
            std::string gky = Trace[i];
            if(ReqTimes.find(gky) != ReqTimes.end()){
                ReqTimes[gky].push_back(gtime);
            }
            else{
                Counter[gky] = 0;
                std::vector<uint64_t> GT;
                GT.push_back(gtime);
                ReqTimes[gky] = GT;
            }
            gtime++;
        }

        std::unordered_map<std::string,std::vector<uint64_t>>::iterator It;
        for(It = ReqTimes.begin();It != ReqTimes.end(); It++){
            std::string gky = It->first;
            ReqTimes[gky].push_back(MaxLim);
        }
    }

    void updateNRTs(std::string Ky){
        Timenow++;
        Counter[Ky]++;
        uint64_t ky_nrt = ReqTimes[Ky][Counter[Ky]] - Timenow + 1;
        NRTs[Ky] = ky_nrt;
        
        std::unordered_map<std::string,uint64_t>::iterator It;
        for(It = NRTs.begin();It != NRTs.end(); It++){
            std::string gky = It->first;
            uint64_t gval = NRTs[gky];
            if(gval != MaxLim){
                NRTs[gky] = gval - 1;
            }
         }
        //std::cout<<"Update Finished..."<<std::endl;
    }

    /**
     * Membership test.
     */
    bool contains(const std::string& key) const {
        return (positions_.find(key) != positions_.end());
    }

    /**
     * Erase the given queue entry.
     */
    void erase(const PositionIterator& position_iter) {
        entries_.erase(position_iter->second);
        positions_.erase(position_iter);
    }

     /**
     * Pop the entry at the front of the queue.
     */
    T popMax() {
        class std::list<T>::iterator It;
        class std::list<T>::iterator Loc;
        double find_val = -1.0;
        T entry;
        for(It = entries_.begin(); It != entries_.end();It++){
            std::string gkey = getKey(*It);
            double Val = NRTs[gkey] / 1.0 * Sizes[gkey];
            if(Val > find_val){
                find_val = Val;
                entry = *It;
                Loc = It;
            }
            if(Val <= 0){
                std::cout<<gkey<<" "<<Val<<std::endl;
            }
        }

        positions_.erase(getKey(entry));
        NRTs.erase(getKey(entry));
        entries_.erase(Loc);
        return entry;
    }


     /**
     * Insert the given entry at the back of the queue.
     */
    void insertBack(const T& entry) {
        const std::string& key = getKey(entry);
        assert(positions_.find(key) == positions_.end());

        entries_.push_back(entry);
        positions_[key] = std::prev(entries_.end());
    }
};

// Template specializations
template<> const std::string&
BeladySQueue<std::string>::getKey(const std::string& entry) const { return entry; }



/**
 * Implements a parameterizable FIFO queue.
 */
template<class T> class LRUKQueue {
private:
    typedef typename std::list<T>::iterator Iterator;
    typedef typename std::unordered_map<std::string, Iterator>::iterator PositionIterator;
    std::unordered_map<std::string, Iterator> positions_; // A dict mapping keys to iterators
    std::list<T> entries_; // An ordered list of T instances. The list is ordered such that, at
                           // any time, the element at the front of the queue is the FIFO entry.
    // Helper method
    const std::string& getKey(const T& entry) const { return entry.key(); }

public:
    // Accessors
    std::list<T>& entries() { return entries_; }
    size_t size() const { return entries_.size(); }
    const std::list<T>& entries() const { return entries_; }
    std::unordered_map<std::string, Iterator>& positions() { return positions_; }
    const std::unordered_map<std::string, Iterator>& positions() const { return positions_; }

    std::unordered_map<std::string,uint64_t> Sizes2;
    std::unordered_map<std::string,std::vector<uint64_t>> LRTs;
    uint64_t Timer = 0;
    uint64_t K = 4;

    /**
     * Membership test.
     */
    bool contains(const std::string& key) const {
        return (positions_.find(key) != positions_.end());
    }

    /**
     * Erase the given queue entry.
     */
    void erase(const PositionIterator& position_iter) {
        entries_.erase(position_iter->second);
        positions_.erase(position_iter);
    }

    void update_lrts(std::string key){
        if(LRTs.find(key) == LRTs.end()){
            std::vector<uint64_t> lrts;
            lrts.push_back(Timer);
            LRTs[key] = lrts;
        }
        else{
           LRTs[key].push_back(Timer);
        }
        Timer++;
    }

    /**
     * Pop the entry at the front of the queue.
     */
    T popFront() {
        class std::list<T>::iterator It;
        class std::list<T>::iterator Loc;
        uint64_t find_val = 0;
        T entry;
        for(It = entries_.begin(); It != entries_.end();It++){
            std::string gkey = getKey(*It);
            std::vector<uint64_t> glrts = LRTs[gkey];
            uint64_t L = glrts.size() - K;
            uint64_t Val = Timer - glrts[L];
            if(Val > find_val){
                find_val = Val;
                entry = *It;
                Loc = It;
            }
        }

        positions_.erase(getKey(entry));
        entries_.erase(Loc);
        return entry;
    }

    /**
     * Insert the given entry at the back of the queue.
     */
    void insertBack(const T& entry) {
        const std::string& key = getKey(entry);
        assert(positions_.find(key) == positions_.end());

        entries_.push_back(entry);
        positions_[key] = std::prev(entries_.end());
    }
};

// Template specializations
template<> const std::string&
LRUKQueue<std::string>::getKey(const std::string& entry) const { return entry; }





/**
 * Implements a parameterizable FIFO queue.
 */
template<class T> class TQQueue {
private:
    typedef typename std::list<T>::iterator Iterator;
    typedef typename std::unordered_map<std::string, Iterator>::iterator PositionIterator;
    std::unordered_map<std::string, Iterator> positions_; // A dict mapping keys to iterators
    std::list<T> entries_; // An ordered list of T instances. The list is ordered such that, at
                           // any time, the element at the front of the queue is the FIFO entry.
    // Helper method
    const std::string& getKey(const T& entry) const { return entry.key(); }

public:
    // Accessors
    std::list<T>& entries() { return entries_; }
    size_t size() const { return entries_.size(); }
    const std::list<T>& entries() const { return entries_; }
    std::unordered_map<std::string, Iterator>& positions() { return positions_; }
    const std::unordered_map<std::string, Iterator>& positions() const { return positions_; }

    std::unordered_map<std::string,uint64_t> Sizes2;
    std::unordered_map<std::string,uint64_t> HisFreqs;

    /**
     * Membership test.
     */
    bool contains(const std::string& key) const {
        return (positions_.find(key) != positions_.end());
    }

    /**
     * Erase the given queue entry.
     */
    void erase(const PositionIterator& position_iter) {
        entries_.erase(position_iter->second);
        positions_.erase(position_iter);
    }


    void update_freqs(std::string key){
        if(HisFreqs.find(key) == HisFreqs.end()){
            HisFreqs[key] = 1;
        }
        else{
            HisFreqs[key] += 1;
        }
    }

    void erase_elem(std::string Ky){
        class std::list<T>::iterator It;
        class std::list<T>::iterator Loc;
        T entry;
        for(It = entries_.begin(); It != entries_.end();It++){
            std::string gkey = getKey(*It);
            if(gkey == Ky){
                entry = *It;
                Loc = It;
                break;
            }
        }

        positions_.erase(getKey(entry));
        entries_.erase(Loc);
        HisFreqs.erase(Ky);
    }

    /**
     * Pop the entry at the front of the queue.
     */
    T popFront() {
        T entry = entries_.front();
        positions_.erase(getKey(entry));
        entries_.pop_front();
        HisFreqs.erase(getKey(entry));
        return entry;
    }

    /**
     * Insert the given entry at the back of the queue.
     */
    void insertBack(const T& entry) {
        const std::string& key = getKey(entry);
        assert(positions_.find(key) == positions_.end());

        entries_.push_back(entry);
        positions_[key] = std::prev(entries_.end());
    }
};

// Template specializations
template<> const std::string&
TQQueue<std::string>::getKey(const std::string& entry) const { return entry; }



/**
 * Represents a min-heap entry.
 */
template<class T> class MinHeapEntry {
private:
    std::string key_; // Cache tag corresponding to this min-heap entry
    size_t last_ref_time_; // Time (in clock cycles) of last reference
    size_t insertion_time_; // Time (in clock cycles) of insertion
    T primary_metric_; // The primary priority metric

public:
    MinHeapEntry(const std::string& key, const T& metric, const size_t lr_time,
                 const size_t in_time) : key_(key), last_ref_time_(lr_time),
                 insertion_time_(in_time), primary_metric_(metric) {}
    // Accessors
    const std::string& key() const { return key_; }
    T getPrimaryMetric() const { return primary_metric_; }
    size_t getLastRefTime() const { return last_ref_time_; }
    size_t getInsertionTime() const { return insertion_time_; }

    // Comparator
    bool operator<(const MinHeapEntry& other) const {
        // Sort by the primary metric
        if (primary_metric_ != other.primary_metric_) {
            return primary_metric_ >= other.primary_metric_;
        }
        // Sort by last reference time
        else if (last_ref_time_ != other.last_ref_time_) {
            return last_ref_time_ >= other.last_ref_time_;
        }
        // Finally, sort by insertion time
        return insertion_time_ >= other.insertion_time_;
    }
};

/**
 * Available hash functions.
 */
enum HashType {
    NO_HASH = 0,
    MURMUR_HASH,
};

/**
 * Represents a family of hash functions, H = {h_0, h_1, ...}.
 */
class HashFamily {
private:
    const size_t kNumHashes;
    const HashType kHashType;

public:
    HashFamily(const size_t num_hashes, const HashType type) :
        kNumHashes(num_hashes), kHashType(type) {}

    /**
     * Computes the h_{idx}'th hash for the given key.
     */
    size_t hash(const size_t idx, const std::string& key) const {
        assert(idx < kNumHashes && kHashType == MURMUR_HASH);
        uint64_t hashes[2]; // The computed, 128-bit hash

        MurmurHash3_x64_128(key.data(), static_cast<int>(key.size()), idx + 1, &hashes);
        return hashes[0]; // Return the lower 64 bits
    }
};

} // namespace caching

#endif // cache_common_h
