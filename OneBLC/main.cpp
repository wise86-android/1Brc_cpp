//
//  main.cpp
//  OneBLC
//
//  Created by Giovanni Visentini on 19/07/24.
//

#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <algorithm>
#include <array>
#include <map>
#include <format>
#include <thread>
#include <ranges>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

using namespace std;

struct StationData{
    int min = std::numeric_limits<int>::max();
    int max = std::numeric_limits<int>::min();
    int sum = 0;
    uint count = 0;
    
    StationData& operator +=(const StationData& other){
        min = std::min(min,other.min);
        max = std::max(max,other.max);
        sum += sum;
        count += count;
        return *this;
    }
    
    StationData& operator +=(const int value){
        min = std::min(min,value);
        max = std::max(max,value);
        sum += value;
        count += 1;
        return *this;
    }
};

struct LineContent{
    string_view name;
    int value;
    
    LineContent(const string_view& line){
        const auto separator = line.find_first_of(';');
        name = line.substr(0,separator);
        value = extractValue(line.substr(separator+1));
    }
    
    int extractValue(const string_view& strValue){
        int numberIndex = strValue[0] == '-' ? 1 : 0;
        if(strValue[numberIndex+1]=='.'){
            int value = (strValue[numberIndex]-'0')*10+strValue[numberIndex+2]-'0';
            return strValue[0] == '-' ? -value : value;
        }else{
            int value = (strValue[numberIndex]-'0')*100+(strValue[numberIndex+1]-'0')*10+strValue[numberIndex+3]-'0';
            return strValue[0] == '-' ? -value : value;
        }
        
    }
    
};

class MmappedFile{
    int fd;
        
public:
    char *mapped;
    off_t size ;
    
    MmappedFile(const string_view& fileName){
        fd = open(fileName.data(),O_RDONLY);
        struct stat s;
        fstat (fd, & s);
        size = s.st_size;

        /* Memory-map the file. */
        mapped = (char*) mmap (0, size, PROT_READ, MAP_PRIVATE, fd, 0);
        
    }
    inline char operator[](size_t offset){
        return mapped[offset];
    }
    
    ~MmappedFile(){
        munmap(mapped,size);
        close(fd);
    }
};

string_view consumeLine(const string_view& buffer){
    const auto termi = buffer.find_first_of('\n');
    return buffer.substr(termi+1);
}

std::vector<string_view> splitWork(const string_view& fullDataSet,uint chunkNumber){
    const auto chunkSize = fullDataSet.size()/chunkNumber;
    vector<string_view> splitString;
    splitString.reserve(chunkNumber);
    string_view::size_type currentChunkStart = 0;
    string_view::size_type currentChunkEnd = chunkSize;
    for (int i = 0; i < chunkNumber; i++) {
        if(currentChunkEnd != fullDataSet.size()){
            currentChunkEnd += fullDataSet.find_first_of('\n',currentChunkEnd)-currentChunkEnd+1;
        }
        splitString.push_back(fullDataSet.substr(currentChunkStart,currentChunkEnd-currentChunkStart));
        currentChunkStart = currentChunkEnd;
        currentChunkEnd = std::min(currentChunkStart+chunkSize,fullDataSet.size());
    }
    return splitString;
}

void thread_work(const string_view& input,std::unordered_map<std::string_view, StationData> &results){
    string_view currentLine = input;

    while (currentLine.size()>0) {
        LineContent lineContent(currentLine);
        results[lineContent.name]+=lineContent.value;
        currentLine = consumeLine(currentLine);
    }
}

#include <queue>

void format_output(std::ostream &out,
                   const std::unordered_map<std::string_view, StationData>& db) {
    std::priority_queue<std::string_view> names;
    // Grab all the unique station names
    std::ranges::for_each(db | std::views::keys, [&names](const auto &key){names.push(key);});
    // Sorting UTF-8 strings lexicographically is the same
    // as sorting by codepoint value
    //std::ranges::sort(names, std::less<>{});

    out << "{";
    for (; !names.empty(); names.pop()){
        const auto &name = names.top();
        const auto &val = db.find(name)->second;
        out <<format("{}={}/{:.1f}/{}, ",name,val.min/10.0f,(val.sum/val.count)/10.0f,val.max/10.0f);
    }
    out << "}\n";
}

int main(int argc, const char * argv[]) {
    auto start = std::chrono::steady_clock::now();
    
    MmappedFile file("measurements.txt");
    const string_view allFile(file.mapped,file.size);
      
    const auto nThread = std::thread::hardware_concurrency();
    
    const auto splittedString = splitWork(allFile, nThread);
    std::vector<std::unordered_map<std::string_view, StationData>> resultData(nThread);
    std::vector<thread> workingThread;
    std::unordered_map<std::string_view, StationData> totalResult;
    
    for ( auto i = 0 ; i<nThread ; i++){
        workingThread.emplace_back(thread_work,std::ref(splittedString[i]),std::ref(resultData[i]));
    }
    for ( auto& t : workingThread){
        t.join();
    }
    auto endCompute = std::chrono::steady_clock::now();
    
    
    for (const auto & workResult : resultData){
        for (auto const& [key, val] : workResult){
            totalResult[key]+=val;
        }
    }
    
    auto endMerge = std::chrono::steady_clock::now();
 
    format_output(std::cout, totalResult);
 
    auto end = std::chrono::steady_clock::now();
    
    std::cout<<std::endl <<std::endl <<"Compute time:" << std::chrono::duration<double>(endCompute-start).count() << " s" << std::endl;
    std::cout<<std::endl <<std::endl <<"Merge time:" << std::chrono::duration<double,std::milli>(endMerge-endCompute).count() << " s" << std::endl;
    std::cout << "Print Time:"<<std::chrono::duration<double, std::milli>(end-endMerge).count() << " ms" << std::endl;
    std::cout<< "Total Time:" << std::chrono::duration<double, std::milli>(end-start).count() << " ms" << std::endl;

    
    return 0;
}
