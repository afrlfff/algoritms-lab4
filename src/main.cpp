// main file

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // C++ 17 and more
#include <cstdlib>
//#include <windows.h>

#include "include/FileCodec.h"
#include "include/TextTools.h"
#include "include/EncodingDecodingRatios.h"

// temporary
#include "include/EncodeTxt.h"
#include "include/DecodeTxt.h"

namespace fs = std::filesystem;
const fs::path INPUT_DIR = fs::current_path() / "..\\input";
const fs::path OUTPUT_DIR = fs::current_path() / "..\\output";

// HELPER FUNCTIONS
void ImageToText(const fs::path& inputPath);
void AllImagesToText();
void EncodeAll(FileCodec& codec);
void DecodeAll(FileCodec& codec);
void MakeResultsFile(const std::string& key);
void MakeGraphics(const std::string& key);

int main()
{
    std::cout << "Start" << std::endl;

    FileCodec codecRLE("RLE");

    // TEST CODE

    //EncodeTxt("AFM", "../input/txt/temp.txt", "../output/AFM/temp_encoded.txt");
    //DecodeTxt("AFM", "../output/AFM/temp_encoded.txt", "../output/AFM/temp_decoded.txt");

    //codecRLE.Encode("..\\input\\txt\\temp.txt", "..\\output\\RLE\\temp_encoded.bin");
    //codecRLE.Decode("..\\output\\RLE\\temp_encoded.bin", "..\\output\\RLE\\temp_decoded.txt");

    MakeResultsFile("RLE");

    return 0;
}

// START IMPLEMETATION

void ImageToText(const fs::path& inputPath)
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

        codec.Encode(inputPath.string(), outputPath.string());
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

        codec.Decode(inputPath.string(), outputPath.string());
    }
}

void MakeResultsFile(const std::string& key)
{
    // BIN IMPLEMENTATION
    std::ofstream file((OUTPUT_DIR / ("results_"+ key + ".txt")));
    file << "fileName entropyRatio startSize[kb] encodedSize[kb] EncodingRatio decodingRatio";

    for (const auto& entry : fs::directory_iterator(INPUT_DIR / ("txt"))) {
        fs::path inputPath = entry.path();
        std::string fileName = inputPath.stem().string();

        fs::path pathToOriginal = inputPath;
        fs::path pathToEncoded = (OUTPUT_DIR / key / (fileName + "_encoded.bin"));
        fs::path pathToDecoded = (OUTPUT_DIR / key / (fileName + "_decoded.txt"));

        MyFile fileOriginal(pathToOriginal.string(), "r");
        std::wstring textOriginal = fileOriginal.ReadWideContent();

        file << '\n';
        file << fileName + ' ' + 
                std::to_string(TextEntropy(textOriginal)) + ' ' + 
                std::to_string(fs::file_size(pathToOriginal) / static_cast<double>(1024)) + ' ' + 
                std::to_string(fs::file_size(pathToEncoded) / static_cast<double>(1024)) + ' ' + 
                std::to_string(EncodingRatio(pathToOriginal, pathToEncoded)) + ' ' + 
                std::to_string(DecodingRatio(pathToOriginal, pathToDecoded));
    }
    file.close();
}

void MakeGraphics(const std::string& key)
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


// END IMPLEMENTATION