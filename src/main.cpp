#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // C++ 17 and more
#include <cstdlib>
#include <windows.h>
#include <chrono>

#include "include/EncodeTxt.h"
#include "include/DecodeTxt.h"
#include "include/FileCodec.h"

#include "include/Entropy.h"
#include "include/DecodingRatio.h"
#include "include/CompressionRatio.h"

namespace fs = std::filesystem;
const fs::path INPUT_DIR = fs::current_path() / "..\\input";
const fs::path OUTPUT_DIR = fs::current_path() / "..\\output";

// START HELPER FUNCTIONS

void ImageToText(fs::path inputPath)
{
    std::string fileName = inputPath.stem().string();
    fs::path outputPath = (INPUT_DIR / "txt" / (fileName + ".txt"));

    std::string command = "python include/img_to_text.py " + inputPath.string() + " " + outputPath.string();
    system(command.c_str());
}

void AllImagesToText()
{
    for (const auto& entry : fs::directory_iterator(INPUT_DIR / "img")) {
        fs::path inputPath = entry.path();

        ImageToText(inputPath);
    }
}

void EncodeAll(FileCodec& codec)
{
    // BIN IMPLEMENTATION
    for (const auto& entry : fs::directory_iterator(INPUT_DIR / "txt")) {
        fs::path inputPath = entry.path();

        fs::path outputPath = OUTPUT_DIR / codec.getKey() / (inputPath.stem().string() + "_encoded.bin"); 
        fs::create_directory(OUTPUT_DIR / codec.getKey()); // create directory if it doesn't exist  

        codec.EncodeBin(inputPath.string(), outputPath.string());
    }
}

void DecodeAll(FileCodec& codec)
{
    // BIN IMPLEMENTATION
    for (const auto& entry : fs::directory_iterator(OUTPUT_DIR / codec.getKey())) {
        fs::path inputPath = entry.path();
        std::string fileName = inputPath.stem().string();
        if ((fileName.find("_encoded") == std::string::npos)) continue;

        std::string clearFileName = "";
        for (int i = 0; i < (fileName.size() - 8); i++) {
            clearFileName += fileName[i];
        }

        fs::path outputPath = OUTPUT_DIR / codec.getKey() / (clearFileName + "_decoded.txt");
        fs::create_directory(OUTPUT_DIR / codec.getKey()); // create directory if it doesn't exist

        codec.DecodeBin(inputPath.string(), outputPath.string());
    }
}

void CreateResultsFile(std::string key)
{
    // BIN IMPLEMENTATION
    std::ofstream file((OUTPUT_DIR / ("results_"+ key + ".txt")));
    file << "fileName entropyRatio startSize[kb] encodedSize[kb] compressionRatio decodingRatio";

    for (const auto& entry : fs::directory_iterator(INPUT_DIR / ("txt"))) {
        fs::path inputPath = entry.path();
        std::string fileName = inputPath.stem().string();

        file << '\n';
        std::string pathToOriginal = (inputPath).string();
        std::string pathToEncoded = (OUTPUT_DIR / key / (fileName + "_encoded.bin")).string();
        std::string pathToDecoded = (OUTPUT_DIR / key / (fileName + "_decoded.txt")).string();

        file << fileName + ' ' + 
                std::to_string(Entropy(pathToOriginal)) + ' ' + 
                std::to_string(fs::file_size(pathToOriginal) / static_cast<double>(1024)) + ' ' + 
                std::to_string(fs::file_size(pathToEncoded) / static_cast<double>(1024)) + ' ' + 
                std::to_string(CompressionRatio(pathToOriginal, pathToEncoded)) + ' ' + 
                std::to_string(DecodingRatio(pathToOriginal, pathToDecoded));
    }
    file.close();
}

void CreateGraphics(std::string key)
{
    fs::path inputPath = OUTPUT_DIR / ("results_" + key + ".txt");
    if (!fs::exists(inputPath)) {
        std::cout << "Error: File " 
            << "results_" + key + ".txt" 
            << " doesn't exist" << std::endl;
        return;
    }

    std::string command = "python include/plot.py " + inputPath.string();
    system(command.c_str());
}

// END HELPER FUNCTIONS

int main()
{
    std::cout << "Start" << std::endl;

    // TEST CODE
    //AllImagesToText();
    //EncodeAll("BWT");
    //DecodeAll("BWT");
    //CreateResultsFile("BWT");

    EncodeTxt("AFM", "../input/txt/temp.txt", "../output/AFM/temp_encoded.txt");
    //DecodeTxt("AFM", "../output/AFM/temp_encoded.txt", "../output/AFM/temp_decoded.txt");

    //EncodeBin("BWT", "..\\input\\txt\\temp.txt", "..\\output\\BWT\\temp_encoded.bin");
    //DecodeBin("BWT", "..\\output\\BWT\\temp_encoded.bin", "..\\output\\BWT\\temp_decoded.txt");

    return 0;
}