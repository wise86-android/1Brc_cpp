# 1Brc in c++

this repot try to solve this challenge in c++:
https://1brc.dev/

This solution use:
- mmap
- threads
- lot of strings_view to avoid copying data
- unordered_map to efficelty store the place data

things to improve:
- implement a custom map structure to store the data
- use some simd to parse the temperature ? 
- copy some city name to avoid reload the file ?
- split definition in multiple files
- handler open file errors
- use more renge libs to print/itereate the results

this project was done with xcode and tested on mac M1