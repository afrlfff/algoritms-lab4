#include <iostream>
#include <string>
#include <fstream>

// realization which can encode all symbols including numbers
// also with decode possibility
// but it can't compress more than 9 identical symbols at once
// (like 10A or 21C, only 1A, 9B, 3C etc.)
// it also use negative numbers before unique sequences of symbols
// (like -5ABCDE or -15ABCDEFGIJKLMNOP)
std::string RLE_encode(std::string str)
{
    std::string newStr;

    int countIdent = 1; // current count of repeating identical characters
    int countUnique = 1; // current count of repeating unique characters
    std::string uniqueSeq(1, str[0]); // last sequence of unique characters

    // show if previous character was part of sequence
    bool flag = false;

    char prev = str[0]; // previous character

    // start RLE
    for (int i = 1; i < str.size(); i++)
    {
        if (str[i] == prev) 
        {
            // record last sequence of unique symbols if it exists
            if (countUnique > 1) {
                uniqueSeq.pop_back(); // because "prev" was read as unique
                countUnique--; // because "prev" was read as unique

                countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
                newStr.append(std::to_string(-1 * countUnique) + uniqueSeq);

                countUnique = 1;
            }

            if (flag) { countIdent = 1; flag = false; } 
            else { countIdent++; }
            
            countUnique = 0;
            uniqueSeq = "";
        }
        else 
        {
            // record last sequence of identical symbols if it exists
            if (countIdent > 1) {
                if (countIdent >= 9) {
                    for (int i = 0; i < (countIdent / 9); i++) {
                        newStr.push_back('9');
                        newStr.push_back(prev);
                    }
                }
                if (countIdent % 9 != 0) {
                    newStr.push_back(('0' + (countIdent % 9)));
                    newStr.push_back(prev);
                }
                flag = true;
                countIdent = 1;
            } else if (countIdent == 0) {
                countIdent = 1;
            }

            if (flag) {
                countUnique = 1;
                uniqueSeq = str[i];
                flag = false;
            } else {
                if (countUnique == 0) {
                    countUnique = 1;
                    uniqueSeq = prev;
                }

                countUnique++;
                uniqueSeq += str[i];
            }
            countIdent = 1;

            // limit length of sequence
            if (countUnique == 9) {
                newStr.append(std::to_string(-1 * countUnique) + uniqueSeq);
                flag = true;
                countUnique = 0;
                uniqueSeq = "";
            }
        }
        prev = str[i];
    }

    // record last sequence which was lost in the loop
    if (countIdent > 1) {
        if (countIdent >= 9) {
            for (int i = 0; i < (countIdent / 9); i++) {
                newStr.push_back('9');
                newStr.push_back(prev);
            }
        }
        if (countIdent % 9 != 0) {
            newStr.push_back(('0' + (countIdent % 9)));
            newStr.push_back(prev);
        }
    }
    if (countUnique > 0) { 
        countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
        newStr.append(std::to_string(-1 * countUnique) + uniqueSeq);
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
    // argv[2] - path to save encoded file
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

        std::string encodedStr = RLE_encode(content);

        // specific writing for binary files
        out.write(encodedStr.c_str(), encodedStr.size());
    } else {
        out.write(
            ("Error opening file: " + std::string(argv[1])).c_str(), 
            ("Error opening file: " + std::string(argv[1])).size());
    }
    in.close();
    out.close();

    // test code
    /* std::string exStr = "888888888888888888999999888";

    std::string newStr2 = RLE_encode(exStr);

    std::cout << "Original string: " << exStr << std::endl;
    std::cout << "Encoded string: " << newStr2 << std::endl; */

    return 0;
}