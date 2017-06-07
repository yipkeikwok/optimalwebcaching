//#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <tuple>
#include "parse_trace.h"

using namespace lemon;

uint64_t parseTraceFile(std::vector<trEntry> & trace, std::string & path) {
    std::ifstream traceFile(path);
    uint64_t time, id, size, reqc=0, uniqc=0;
    std::map<std::pair<uint64_t, uint64_t>, uint64_t> lastSeen;

    while(traceFile >> time >> id >> size) {
        if(lastSeen.count(std::make_pair(id,size))>0) {
            trace[lastSeen[std::make_pair(id,size)]].hasNext = true;
        } else {
            uniqc++;
        }
        trace.emplace_back(id,size,time);
        lastSeen[std::make_pair(id,size)]=reqc++;
    }
    return uniqc;
}
                    
void createMCF(SmartDigraph & g, std::vector<trEntry> & trace, uint64_t cacheSize, SmartDigraph::ArcMap<int64_t> & cap, SmartDigraph::ArcMap<double> & cost, SmartDigraph::NodeMap<int64_t> & supplies) {

    // we consider (id,size) as unique identification of an object (sizes can change, but then it's a different object)
    // lastSeen maps (id,size) to (nodeId,traceIndex) of the last time this object was seen
    std::map<std::pair<uint64_t, uint64_t>, std::pair<uint64_t, int> > lastSeen;
    SmartDigraph::Arc curArc;
    SmartDigraph::Node curNode = g.addNode(); // initial node
    SmartDigraph::Node prevNode;

    // iterate over trace
    for(uint64_t i=0; i<trace.size(); i++) {
        const trEntry thisTrEntry = trace[i];
        const uint64_t id = thisTrEntry.id;
        const uint64_t size = thisTrEntry.size;
        const bool nextRequest = thisTrEntry.hasNext;
        // first: check if previous interval ended here
        if(lastSeen.count(std::make_pair(id,size))>0) {
            // create "outer" request arc
            const SmartDigraph::Node lastReq = g.nodeFromId(lastSeen[std::make_pair(id,size)].second);
            curArc = g.addArc(lastReq,curNode);
            cap[curArc] = size;
            cost[curArc] = 1/static_cast <double>(size);
            supplies[lastReq] += size;
            supplies[curNode] -= size;
            trace[lastSeen[std::make_pair(id,size)].first].arcId = g.id(curArc);
        }
        // second: if there is another request for this object
        if(nextRequest) {
            // save prev node as anchor for future arcs
            prevNode = curNode;
            lastSeen[std::make_pair(id,size)]=std::make_pair(i,g.id(prevNode));
            // create another node, "inner" capacity arc
            curNode = g.addNode(); // next node
            curArc = g.addArc(prevNode,curNode);
            cap[curArc] = cacheSize; 
            cost[curArc] = 0;
        }
    }
}
