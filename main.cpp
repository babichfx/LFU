#include <iostream>
#include <vector>
#include <deque>
#include <chrono>
#include <random>
#include <time.h>

#define DOLOG           false
#define CACHESIZE       10
#define TESTSIZE        1000000
#define DATAPIECES      50
#define LOADPROCESSDUR  10

template <typename T>
class CacheLFU{
    //typename used to clarify that is type and not any other thing
    using vecIt = typename std::vector<T>::iterator;

    class Counter{ //to save informatoin about frequently using
        uint  count;
        vecIt dataIt;
    public:
        Counter(vecIt inputIterator) : dataIt(inputIterator), count(0) {};
        ~Counter(){}
        void  operator++(){++count;}
        int   getCount() const {return count;}
        vecIt getDataIterator() const {return dataIt;}
    };

    size_t cacheSize;
    size_t hits, works; //stat
    std::vector<T> cacheArray;
    std::vector<Counter> counterArray;
    timespec timeSl, tmTmp;

    using cntIt = typename std::vector<Counter>::iterator;

    vecIt lookingForDataInCache(const T& newData){
        if(DOLOG)
            std::cout << "Looking for: " << newData << "... ";
        return find(cacheArray.begin(), cacheArray.end(), newData);
    }
    vecIt addNewDataToCache(const T& newData) {
        cacheArray.push_back(newData);
        works++;
        nanosleep(&timeSl, &tmTmp); //simulate long process
        if(DOLOG)
            std::cout << "Added data: " << *(--(cacheArray.end())) << std::endl;
        return --(cacheArray.end());
    }
    void  replaceDataInCache(const vecIt& vIt, const T& newData) {
        if(DOLOG)
            std::cout << *vIt << " replaced with: " << newData << std::endl;
        *vIt = newData;
        works++;
        nanosleep(&timeSl, &tmTmp); //simulate long process
    }
    cntIt findCounterItForData(const vecIt& vIt){
        for(auto it = counterArray.begin(); it != counterArray.end(); ++it){
            if(it->getDataIterator() == vIt)
                return it;
        }
        return counterArray.end();
    }
    void  addNewCounter(const vecIt& newIt) {
        counterArray.push_back(Counter(newIt));
        incrementCounter(counterArray.end()-1);
    }
    void  replaceCounter(const cntIt& cIt) {
        *cIt = Counter(cIt->getDataIterator());
        incrementCounter(cIt);
    }
    //const is working because iterator save a pointer to memory
    //and const iterator cant allow to changethe pointer itself
    //but const dont defence the memory under pointer if its not cons also
    void  incrementCounter(const cntIt& cIt) {
        ++(*cIt);
    }
public:
    CacheLFU(size_t newSize) : cacheSize(newSize) {
        for(uint i=0; i<cacheSize; ++i){
            cacheArray.push_back(-1);
        }
        cacheArray.shrink_to_fit();
        cacheArray.clear();
        hits = works = 0;
        timeSl.tv_sec = 0;
        timeSl.tv_nsec = LOADPROCESSDUR;
    }
    void  loadNewElement(const T& newData){
        vecIt dataCacheIt = lookingForDataInCache(newData);
        //is data in cache?
        if(dataCacheIt == cacheArray.end()){
            //cahce is full?
            if(cacheArray.size() >= cacheSize){
                //loking for minimum counter
                cntIt counterIt;
                uint tmpCount = UINT32_MAX;
                for(auto it=counterArray.begin(); it!=counterArray.end(); ++it){
                    if( tmpCount > it->getCount() ){
                        tmpCount = it->getCount();
                        counterIt = it;
                    }//if
                }//for
                //replace old data with new
                replaceDataInCache(counterIt->getDataIterator(), newData);
                replaceCounter(counterIt);
            } else { //cache not full
                //add new data to cache and add new counter
                addNewCounter( addNewDataToCache(newData) );
            }//full if
        } else { //if data found in cache
            //increment counter by iterator of data in cache
            incrementCounter(findCounterItForData(dataCacheIt));
            hits++;
            if(DOLOG)
                std::cout << "Hit with: " << newData << std::endl;
        }//in cache if
    }
    void  printCache(){
        std::cout << "Cache capacity: " << cacheArray.capacity()
                  << ". Cache size: " << cacheArray.size() << " --> ";
        for(auto& el : cacheArray){
            std::cout << el << ", ";
        }
        std::cout << std::endl;
    }
    void  printCounters(){
        std::cout << "Size of counters array is: " << counterArray.size() << ".\n";
        for(auto& el : counterArray){
            std::cout << "For: " << *(el.getDataIterator());
            std::cout << ", count is: " << el.getCount() << ".\n";
        }
    }
    void  printStat(){
        std::cout << "Accesses to memory: " << works << ", ";
        std::cout << "hits: " << hits << ".\n";
        std::cout << "---------------------------------------------\n";
    }
};

int main()
{
    using std::cout;
    using std::endl;

    CacheLFU<int> cache1(CACHESIZE);

    std::deque<int> inData1;
    std::srand(15);
    for(int i=0; i<TESTSIZE; i++)
        inData1.push_back(rand() % DATAPIECES);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for(const auto& el : inData1)
        cache1.loadNewElement(el);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    cache1.printCache();
    cache1.printStat();

    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>
                 (end - begin).count()/1000000.0 << "[sec]" << std::endl;
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>
                 (end - begin).count() << "[µs]" << std::endl;

    return 0;
}
