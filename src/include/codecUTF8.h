#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "FileUtils.h"

class CodecUTF8 {
public:
    static std::string EncodeString32ToString(const std::u32string& str);

    static void EncodeChar32ToBinaryFile(FILE* file, const char32_t& code_point);
    static void EncodeString32ToBinaryFile(FILE* file, const std::u32string& str);

    // base utf-8 decode function
    static std::u32string DecodeString32FromBinaryFile(FILE* file, const size_t& size);
    static std::string DecodeString32FromBinaryFileToString(FILE* file, const size_t& size);
    static std::string DecodeChar32FromBinaryFileToString(FILE* file);

    // base utf-8 encode function
    static std::string EncodeChar32ToString(const char32_t& code_point);
private:
    CodecUTF8() = default;
    ~CodecUTF8() = default;
};

// START IMPLEMENTATION

std::string CodecUTF8::EncodeChar32ToString(const char32_t& code_point) {
    std::string str;
    if (code_point <= 0x007F) {
        char ch = static_cast<char>(code_point);
         
        str.push_back(ch);
    } else if (code_point <= 0x07FF) {
        //Use uint8_t or unsigned char. Regular char may be signed
        //and the sign bit may cause defect during bit manipulation
        uint8_t b2 = 0b10000000 | (code_point & 0b111111);
        uint8_t b1 = 0b11000000 | (code_point >> 6);
         
        str.push_back(b1);
        str.push_back(b2);
    } else if (code_point <= 0xFFFF) {
        uint8_t b3 = 0b10000000 | (code_point & 0b111111);
        uint8_t b2 = 0b10000000 | ((code_point >> 6) & 0b111111);
        uint8_t b1 = 0b11100000 | (code_point >> 12);
         
        str.push_back(b1);
        str.push_back(b2);
        str.push_back(b3);
    } else if (code_point <= 0x10FFFF) {
        uint8_t b4 = 0b10000000 | (code_point & 0b111111);
        uint8_t b3 = 0b10000000 | ((code_point >> 6) & 0b111111);
        uint8_t b2 = 0b10000000 | ((code_point >> 12) & 0b111111);
        uint8_t b1 = 0b11110000 | (code_point >> 18);
         
        str.push_back(b1);
        str.push_back(b2);
        str.push_back(b3);
        str.push_back(b4);
    }

    return str;
}

std::string CodecUTF8::EncodeString32ToString(const std::u32string& str)
{
    std::string result;
    for (char32_t code : str) {
        result = EncodeChar32ToString(code);
    }
    return result;
}

void CodecUTF8::EncodeChar32ToBinaryFile(FILE* file, const char32_t& code_point) {
    if (code_point <= 0x007F) {
        uint8_t ch = static_cast<uint8_t>(code_point);
        FileUtils::AppendValueBinary(file, ch);
    } else if (code_point <= 0x07FF) {
        //Use uint8_t or unsigned char. Regular char may be signed
        //and the sign bit may cause defect during bit manipulation
        uint8_t b2 = 0b10000000 | (code_point & 0b111111);
        uint8_t b1 = 0b11000000 | (code_point >> 6);

        FileUtils::AppendValueBinary(file, b1);
        FileUtils::AppendValueBinary(file, b2);
    } else if (code_point <= 0xFFFF) {
        uint8_t b3 = 0b10000000 | (code_point & 0b111111);
        uint8_t b2 = 0b10000000 | ((code_point >> 6) & 0b111111);
        uint8_t b1 = 0b11100000 | (code_point >> 12);
         
        FileUtils::AppendValueBinary(file, b1);
        FileUtils::AppendValueBinary(file, b2);
        FileUtils::AppendValueBinary(file, b3);
    } else if (code_point <= 0x10FFFF) {
        uint8_t b4 = 0b10000000 | (code_point & 0b111111);
        uint8_t b3 = 0b10000000 | ((code_point >> 6) & 0b111111);
        uint8_t b2 = 0b10000000 | ((code_point >> 12) & 0b111111);
        uint8_t b1 = 0b11110000 | (code_point >> 18);
        
        FileUtils::AppendValueBinary(file, b1);
        FileUtils::AppendValueBinary(file, b2);
        FileUtils::AppendValueBinary(file, b3);
        FileUtils::AppendValueBinary(file, b4);
    }
}

void CodecUTF8::EncodeString32ToBinaryFile(FILE* file, const std::u32string& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        EncodeChar32ToBinaryFile(file, str[i]);
    }
}

std::string CodecUTF8::DecodeChar32FromBinaryFileToString(FILE* file) {
    return DecodeString32FromBinaryFileToString(file, 1);
}

std::string CodecUTF8::DecodeString32FromBinaryFileToString(FILE* file, const size_t& size) {
    if (size < 1) {
        return "";
    }

    std::u32string result = DecodeString32FromBinaryFile(file, size);
    std::string resultStr;
    for (char32_t code : result) {
        resultStr = EncodeChar32ToString(code);
    }
 
    return resultStr;
}

std::u32string CodecUTF8::DecodeString32FromBinaryFile(FILE* file, const size_t& size)
{
    if (size == 0) {
        return U"";
    }

    // result after decoding, which will be encoded to a char
    std::u32string resultStr; 

    //For unsigned bytes use uint8_t and not char.
    //char may be signed in some platforms. Using
    //char can actually introduce defects.
    uint8_t bytes[] = {0, 0, 0, 0};
 
    // decode
    for (size_t i = 0; i < size; ++i) {
        bytes[0] = FileUtils::ReadValueBinary<uint8_t>(file);

        if ((bytes[0] & 0b10000000) == 0) {
            char32_t code = bytes[0];
            resultStr.push_back(code);
        } else if ((bytes[0] & 0b11100000) == 0b11000000) {
            bytes[1] = FileUtils::ReadValueBinary<uint8_t>(file);

            if ((bytes[1] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                throw std::runtime_error("Can't decode byte in UTF-8");
            }

            //Clear the UTF-8 marker bits
            bytes[0] = bytes[0] & 0b00011111;
            bytes[1] = bytes[1] & 0b00111111;

            char32_t code = (bytes[0] << 6) | bytes[1];

            resultStr.push_back(code);
        } else if ((bytes[0] & 0b11110000) == 0b11100000) {

            bytes[1] = FileUtils::ReadValueBinary<uint8_t>(file);
            bytes[2] = FileUtils::ReadValueBinary<uint8_t>(file);

            if ((bytes[1] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                throw std::runtime_error("Can't decode byte in UTF-8");
            }

            if ((bytes[2] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                throw std::runtime_error("Can't decode byte in UTF-8");
            }

            //Clear the UTF-8 marker bits
            bytes[0] = bytes[0] & 0b00001111;
            bytes[1] = bytes[1] & 0b00111111;
            bytes[2] = bytes[2] & 0b00111111;

            char32_t code = (bytes[0] << 12) | (bytes[1] << 6) | bytes[2];

            resultStr.push_back(code);
        } else if ((bytes[0] & 0b11111000) == 0b11110000) {
            bytes[1] = FileUtils::ReadValueBinary<uint8_t>(file);
            bytes[2] = FileUtils::ReadValueBinary<uint8_t>(file);
            bytes[3] = FileUtils::ReadValueBinary<uint8_t>(file);

            if ((bytes[1] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                throw std::runtime_error("Can't decode byte in UTF-8");
            } if ((bytes[2] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                throw std::runtime_error("Can't decode byte in UTF-8");
            } if ((bytes[3] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                throw std::runtime_error("Can't decode byte in UTF-8");
            }

            //Clear the UTF-8 marker bits
            bytes[0] = bytes[0] & 0b00000111;
            bytes[1] = bytes[1] & 0b00111111;
            bytes[2] = bytes[2] & 0b00111111;
            bytes[3] = bytes[3] & 0b00111111;

            char32_t code = (bytes[0] << 18) | (bytes[1] << 12) | (bytes[2] << 6) | bytes[3];

            resultStr.push_back(code);
        } else {
            //We can't decode this byte
            throw std::runtime_error("Can't decode byte in UTF-8");
        }
    }

    return resultStr;
}

// END IMPLEMENTATION


// Here are the functions from the internet I used as a template
/* 
void utf8_encode(std::string& str, unsigned long code_point) {
    if (code_point <= 0x007F) {
        char ch = static_cast<char>(code_point);
         
        str.push_back(ch);
    } else if (code_point <= 0x07FF) {
        //Use uint8_t or unsigned char. Regular char may be signed
        //and the sign bit may cause defect during bit manipulation
        uint8_t b2 = 0b10000000 | (code_point & 0b111111);
        uint8_t b1 = 0b11000000 | (code_point >> 6);
         
        str.push_back(b1);
        str.push_back(b2);
    } else if (code_point <= 0xFFFF) {
        uint8_t b3 = 0b10000000 | (code_point & 0b111111);
        uint8_t b2 = 0b10000000 | ((code_point >> 6) & 0b111111);
        uint8_t b1 = 0b11100000 | (code_point >> 12);
         
        str.push_back(b1);
        str.push_back(b2);
        str.push_back(b3);
    } else if (code_point <= 0x10FFFF) {
        uint8_t b4 = 0b10000000 | (code_point & 0b111111);
        uint8_t b3 = 0b10000000 | ((code_point >> 6) & 0b111111);
        uint8_t b2 = 0b10000000 | ((code_point >> 12) & 0b111111);
        uint8_t b1 = 0b11110000 | (code_point >> 18);
         
        str.push_back(b1);
        str.push_back(b2);
        str.push_back(b3);
        str.push_back(b4);
    }
}

bool utf8_decode(const std::string& str, std::vector<unsigned long>& result) {
    if (str.empty()) {
        //Nothing to do
        return true;
    }
 
    //For unsigned bytes use uint8_t and not char.
    //char may be signed in some platforms. Using
    //char can actually introduce defects.
    uint8_t bytes[] = {0, 0, 0, 0};
 
    for (size_t idx = 0; idx < str.length(); ++idx) {
        bytes[0] = str.at(idx);
 
        if ((bytes[0] & 0b10000000) == 0) {
            result.push_back(bytes[0]);
        } else if ((bytes[0] & 0b11100000) == 0b11000000) {
            if (idx + 1 == str.length()) {
                //Error. Not enough bytes.
                return false;
            }
 
            bytes[1] = str.at(++idx);
 
            if ((bytes[1] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                return false;
            }
 
            //Clear the UTF-8 marker bits
            bytes[0] = bytes[0] & 0b00011111;
            bytes[1] = bytes[1] & 0b00111111;
 
            unsigned long code = (bytes[0] << 6) | bytes[1];
 
            result.push_back(code);
        } else if ((bytes[0] & 0b11110000) == 0b11100000) {
            if (idx + 2 == str.length()) {
                //Error. Not enough bytes.
                return false;
            }
 
            bytes[1] = str.at(++idx);
            bytes[2] = str.at(++idx);
 
            if ((bytes[1] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                return false;
            }
 
            if ((bytes[2] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                return false;
            }
 
            //Clear the UTF-8 marker bits
            bytes[0] = bytes[0] & 0b00001111;
            bytes[1] = bytes[1] & 0b00111111;
            bytes[2] = bytes[2] & 0b00111111;
 
            unsigned long code = (bytes[0] << 12) | (bytes[1] << 6) | bytes[2];
 
            result.push_back(code);
        } else if ((bytes[0] & 0b11111000) == 0b11110000) {
            if (idx + 3 == str.length()) {
                //Error. Not enough bytes.
                return false;
            }
 
            bytes[1] = str.at(++idx);
            bytes[2] = str.at(++idx);
            bytes[3] = str.at(++idx);
 
            if ((bytes[1] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                return false;
            }
 
            if ((bytes[2] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                return false;
            }
 
            if ((bytes[3] & 0b11000000) != 0b10000000) {
                //Error. Not a follow-on byte.
                return false;
            }
 
            //Clear the UTF-8 marker bits
            bytes[0] = bytes[0] & 0b00000111;
            bytes[1] = bytes[1] & 0b00111111;
            bytes[2] = bytes[2] & 0b00111111;
            bytes[3] = bytes[3] & 0b00111111;
 
            unsigned long code = (bytes[0] << 18) | (bytes[1] << 12) | (bytes[2] << 6) | bytes[3];
 
            result.push_back(code);
        } else {
            //We can't decode this byte
            return false;
        }
    }
 
    return true;
}
*/