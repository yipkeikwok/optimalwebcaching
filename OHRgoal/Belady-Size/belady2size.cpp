#include <cassert>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "lib/parse_trace.h"
#include "lib/solve_mcf.h"

int main(int argc, char* argv[]) {

    if (argc != 4) {
        std::cerr << argv[0] << " traceFile cacheSize sampleSize" << std::endl;
        return 1;
    }

    std::string path(argv[1]);
    uint64_t cacheSize(std::stoull(argv[2]));
    size_t sampleSize(std::stoull(argv[3]));

    // parse trace file
    std::vector<trEntry> trace;
    parseTraceFile(trace, path);
    uint64_t totalReqc = trace.size();
    std::cerr << "scanned trace n=" << totalReqc << " cs " << cacheSize << std::endl;


    // get nextSeen indices
    std::unordered_map<std::pair<uint64_t, uint64_t>, size_t> lastSeen;
    for (size_t i = trace.size(); i--> 0 ;) {
        trEntry & cur = trace[i];
        if(lastSeen.count(std::make_pair(cur.id,cur.size)) > 0) {
            cur.hasNext = true;
            cur.nextSeen = lastSeen[std::make_pair(cur.id, cur.size)];
        }
        lastSeen[std::make_pair(cur.id, cur.size)] = i;
    }


    // actual caching algorithm
    cacheAlg(trace, cacheSize, sampleSize);

    // print results
    printRes(trace, "Belady2SizeForward "+std::to_string(cacheSize)+" "+std::to_string(sampleSize));


    // backward
    LOG("\n--------------------------\n\n",0,0,0);


    // reset
    lastSeen.clear();
    // compute backwards
    for(size_t i=0; i<trace.size(); i++) {
        trEntry & cur = trace[i];
        // reset
        cur.hasNext = true;
        cur.nextSeen = i;
        cur.hit = 0;
    }


    // actual caching algorithm
    cacheAlg(trace, cacheSize, sampleSize);

    // print results
    printRes(trace, "Belady2SizeBackward "+std::to_string(cacheSize)+" "+std::to_string(sampleSize));
    
    return 0;
}


