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
#include <chrono>

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
    std::string name;
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

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

string_view consumeLine(string_view& buffer){
    const auto termi = buffer.find_first_of('\n');
    return buffer.substr(termi+1);
}

int main(int argc, const char * argv[]) {
    auto start = std::chrono::steady_clock::now();
    
    MmappedFile file("measurements.txt");
    const string_view allFile(file.mapped,file.size);

    std::map<std::string, StationData> results;
    string_view currentLine = allFile;
    auto line = 0;
    while (currentLine.size()>0) {
        LineContent lineContent(currentLine);
        results[lineContent.name]+=lineContent.value;
        currentLine = consumeLine(currentLine);
    }
    
    auto endCompute = std::chrono::steady_clock::now();
    for (auto const& [key, val] : results){
        cout <<format("{}={}/{:.1f}/{},",key,val.min/10.0f,(val.sum/val.count)/10.0f,val.max/10.0f);
    }
    

    auto end = std::chrono::steady_clock::now();
    std::cout<<std::endl <<std::endl <<"Compute time:" << std::chrono::duration<double, std::milli>(endCompute-start).count() << " ms" << std::endl;
    std::cout << "Print Time:"<<std::chrono::duration<double, std::milli>(end-endCompute).count() << " ms" << std::endl;
    std::cout<< "Total Time:" << std::chrono::duration<double, std::milli>(end-start).count() << " ms" << std::endl;

    
    return 0;
}
