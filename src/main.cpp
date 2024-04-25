// main file

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // C++ 17 and more
#include <cstdlib>

#include "include/TextTools.h"
#include "include/EncodingDecodingRatios.h"

// temporary
#include "include/CodecTestPrototypes.h"
#include "include/CodecRLE.h"
#include "include/CodecMTF.h"

namespace fs = std::filesystem;
const fs::path INPUT_DIR = fs::current_path() / "..\\input";
const fs::path OUTPUT_DIR = fs::current_path() / "..\\output";

// HELPER FUNCTIONS
void ClearOutputDirectory();
void MakeResultsFile();
void MakeGraphics(const std::string& codecName);
template <typename CodecType>
void EncodeAll();
template <typename CodecType>
void DecodeAll(FileCodec& codec);


int main()
{
    //ClearOutputDirectory();
    std::cout << "Start" << std::endl;

    // TEST CODE

    //EncodeAll(codec);
    //DecodeAll(codec);
    //MakeResultsFile();

    //std::string text = "abcd";
    //std::string encodedText = EncodeHA_toString(text);
    //std::cout << encodedText << std::endl;

    CodecMTF::Encode("..\\input\\txt\\temp.txt", "..\\output\\encoded\\temp_encoded.bin");
    CodecMTF::Decode("..\\output\\encoded\\temp_encoded.bin", "..\\output\\decoded\\temp_decoded.txt");

    return 0;
}

// START IMPLEMETATION

void ClearOutputDirectory()
{
    fs::remove_all(OUTPUT_DIR / "encoded");
    fs::remove_all(OUTPUT_DIR / "decoded");
    fs::remove_all(OUTPUT_DIR / "graphics");

    fs::create_directory(OUTPUT_DIR / "encoded");
    fs::create_directory(OUTPUT_DIR / "decoded");
    fs::create_directory(OUTPUT_DIR / "graphics");
}

template <typename CodecType>
void EncodeAll()
{
    // create directory if it doesn't exist
    fs::create_directory(OUTPUT_DIR / "encoded");
    
    for (const auto& entry : fs::directory_iterator(INPUT_DIR / "txt")) {
        fs::path inputPath = entry.path();

        fs::path outputPath = OUTPUT_DIR / "encoded" / (inputPath.stem().string() + "_encoded.bin"); 

        CodecType::Encode(inputPath.string().c_str(), outputPath.string().c_str());
    }
}

template <typename CodecType>
void DecodeAll()
{
    // create directory if it doesn't exist
    fs::create_directory(OUTPUT_DIR / "decoded");
    char decodedPart[12 + 1] = "_decoded.txt";

    for (const auto& entry : fs::directory_iterator(OUTPUT_DIR / "encoded")) {
        fs::path inputPath = entry.path(); // "..._encoded.bin"

        std::string fileName = inputPath.stem().string(); // ".._encoded"

        // -8 to remove "_encoded"
        // +12  to add "_decoded.txt"
        int newFileNameSize = fileName.size() - 8 + 12 + 1;

        // make new file name and output path
        int i = 0;
        std::string newFileName(newFileNameSize, '\0');
        for (i = 0; i < (newFileNameSize - 13); ++i) {
            newFileName[i] = fileName[i];
        } for (int j = 0; j < 12; ++j) {
            newFileName[i + j] = decodedPart[j];
        }
        newFileName[newFileNameSize] = '\0';
        fs::path outputPath = OUTPUT_DIR / "decoded" / newFileName; // "..._decoded.txt"

        CodecType::Decode(inputPath.string().c_str(), outputPath.string().c_str());
    }
}

void MakeResultsFile()
{
    // BIN IMPLEMENTATION
    std::ofstream file((OUTPUT_DIR / ("results.txt")));
    file << "fileName entropyRatio startSize[kb] encodedSize[kb] EncodingRatio decodingRatio";

    for (const auto& entry : fs::directory_iterator(INPUT_DIR / ("txt"))) {
        fs::path inputPath = entry.path();
        std::string fileName = inputPath.stem().string(); // name with no extension

        fs::path pathToOriginal = inputPath;
        fs::path pathToEncoded = (OUTPUT_DIR / "encoded" / (fileName + "_encoded.bin"));
        fs::path pathToDecoded = (OUTPUT_DIR / "decoded" / (fileName + "_decoded.txt"));

        std::u32string textOriginal = FileUtils::ReadContentToU32String(pathToOriginal.string().c_str());

        file << '\n';
        file << fileName + ' ' + 
                std::to_string(GetTextEntropy(textOriginal)) + ' ' + 
                std::to_string(fs::file_size(pathToOriginal) / 1024.0) + ' ' + 
                std::to_string(fs::file_size(pathToEncoded) / 1024.0) + ' ' + 
                std::to_string(EncodingRatio(pathToOriginal, pathToEncoded)) + ' ' + 
                std::to_string(DecodingRatio(pathToOriginal, pathToDecoded));
    }
    file.close();
}

void MakeGraphics(const std::string& codecName)
{
    fs::path inputPath = OUTPUT_DIR / ("results_" + codecName + ".txt");
    if (!fs::exists(inputPath)) {
        throw std::runtime_error(
            "Error: File results_" + codecName + ".txt doesn't exist"
        );
    }

    std::string command = "python include/plot.py " + inputPath.string();
    system(command.c_str());
}

// END IMPLEMENTATION