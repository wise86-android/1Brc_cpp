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
    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::min();
    float sum = 0.0f;
    uint count = 0;
    
    StationData& operator +=(const StationData& other){
        min = std::min(min,other.min);
        max = std::max(max,other.max);
        sum += sum;
        count += count;
        return *this;
    }
    
    StationData& operator +=(const float value){
        min = std::min(min,value);
        max = std::max(max,value);
        sum += value;
        count += 1;
        return *this;
    }
};

struct LineContent{
    std::string name;
    float value;
    
    LineContent(const string_view& line){
        const auto separator = line.find_first_of(';');
        name = line.substr(0,separator);
        value = std::stof(string(line.substr(separator+1)));
    }
};


int main(int argc, const char * argv[]) {
    auto start = std::chrono::steady_clock::now();

    ifstream mesurement("measurements.txt");
    array<char,256> lineBuffer{};
    std::map<std::string, StationData> results;

    while (mesurement.getline(lineBuffer.data(), lineBuffer.size())) {
        LineContent lineContent((string_view(lineBuffer.data())));
        results[lineContent.name]+=lineContent.value;
    }

    for (auto const& [key, val] : results){
        cout <<format("{}={}/{:.1f}/{},",key,val.min,val.sum/val.count,val.max);
    }
    
    // Close the file
    mesurement.close();
    
    auto end = std::chrono::steady_clock::now();
    std::cout<<std::endl <<std::endl << std::chrono::duration<double, std::milli>(end-start).count() << " ms" << std::endl;

    
    return 0;
}
