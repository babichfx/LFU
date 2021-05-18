#include <iostream>
#include <vector>
#include <array>

#define DOLOG     false

template <typename T>
class CacheLFU{
    //typename used to clarify that is type and not any other thing
    using vecIt = typename std::vector<T>::iterator;

    class Counter{
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

    using cntIt = typename std::vector<Counter>::iterator;

    vecIt lookingForDataInCache(const T& newData){
        if(DOLOG)
            std::cout << "Looking for: " << newData << "... ";
        for(auto it = cacheArray.begin(); it != cacheArray.end(); it++){
            if(*it == newData)
                return it;
        }
        return cacheArray.end();
    }
    vecIt addNewDataToCache(const T& newData) {
        cacheArray.push_back(newData);
        if(DOLOG)
            std::cout << "Added data: " << *(--(cacheArray.end())) << std::endl;
        return --(cacheArray.end());
    }
    void  replaceDataInCache(const vecIt& vIt, const T& newData) {
        if(DOLOG)
            std::cout << *vIt << " replaced with: " << newData << std::endl;
        *vIt = newData;
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
        works++;
    }
    void  replaceCounter(const cntIt& cIt) {
        *cIt = Counter(cIt->getDataIterator());
        incrementCounter(cIt);
        works++;
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
                    if( tmpCount >= it->getCount() ){
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

    CacheLFU<int> cache1(3);
    std::array<int, 15> inData1 = {7,0,1,2,0,3,0,4,2,3,0,3,1,2,0};
    for(const auto& el : inData1)
        cache1.loadNewElement(el);
    cache1.printCounters();
    cache1.printCache();
    cache1.printStat();

    CacheLFU<int> cache2(3);
    std::array<int, 15> inData2 = {0,1,0,0,2,2,4,3,3,1,0,2,2,1,0};
    for(const auto& el : inData2)
        cache2.loadNewElement(el);
    cache2.printCounters();
    cache2.printCache();
    cache2.printStat();

    return 0;
}
