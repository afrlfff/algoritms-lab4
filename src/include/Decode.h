#pragma once

#include <string>

std::string DecodeRLE(std::string str);
std::string Decode(std::string key, std::string str);
std::string Decode(std::string key, std::string inputPath, std::string outputPath);


std::string Decode(std::string key, std::string str)
{
    if (key == "RLE") {
        return DecodeRLE(str);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
        return "";
    }
}

std::string Decode(std::string key, std::string inputPath, std::string outputPath)
{
    std::ifstream in(inputPath, std::ios::binary);
    std::ofstream out(outputPath, std::ios::binary);

    std::string inputStr;
    while(1) {
        char c = (char)in.get();
        if (in.eof()) break;
        inputStr += c;
    }

    std::string outputStr = Decode(key, inputStr);
    for(int i = 0; i < outputStr.size(); i++) {
        out << outputStr[i];
    }

    in.close();
    out.close();
    return outputStr;
}

// Run-length decoding
std::string DecodeRLE(std::string str)
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


