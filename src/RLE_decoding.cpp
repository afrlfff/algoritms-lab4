#include <iostream>
#include <string>
#include <fstream>

std::string RLE_decode(std::string str)
{
    std::string newStr;

    int i = 0, count;
    std::string stemp;
    while (i < str.size())
    {
        // if starts with negative number
        // (sequence of unqiue symbols)
        if (str[i] == '-')
        {
            count = str[i + 1] - '0'; // convert char to int
            i += 2;
            for (int j = i; j < (i + count); j++) {
                newStr += str[j];
            }
            i += count;
        }
        // if starts with positive number
        // (sequence of identical symbols)
        else
        {
            count = str[i] - '0'; // convert char to int
            newStr.append(count, str[i + 1]);
            i += 2;
        }
    }

    return newStr;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "No parameters was given" << std::endl;
        return 0;
    }
    
    // pass through all given files paths
    // argv[1] - path to open original file
    // argv[2] - path to save decoded file
    // use binary mode to save specific encoding
    // like "utf-8"
    std::ifstream in(argv[1], std::ios::binary);
    std::ofstream out(argv[2], std::ios::binary);
    if (in.is_open()){
        std::string content;
        in.seekg(0, std::ios::end);
        content.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&content[0], content.size());

        std::string decodedStr = RLE_decode(content);

        // specific writing for binary files
        out.write(decodedStr.c_str(), decodedStr.size());
    } else {
        out.write(
            ("Error opening file: " + std::string(argv[1])).c_str(), 
            ("Error opening file: " + std::string(argv[1])).size());
    }
    in.close();
    out.close();

    // test code
    /* std::string exStr = "1a-4bcde5A";

    std::string newStr = RLE_decode(exStr);

    std::cout << "Original string: " << exStr << std::endl;
    std::cout << "basic RLE: " << newStr << std::endl; */

    return 0;
}