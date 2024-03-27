// main file

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // C++ 17 and more
#include <cstdlib>

#include "include/FileCodec.h"
#include "include/TextTools.h"
#include "include/EncodingDecodingRatios.h"

// temporary
#include "include/FileCodecTxtOnly.h"

namespace fs = std::filesystem;
const fs::path INPUT_DIR = fs::current_path() / "..\\input";
const fs::path OUTPUT_DIR = fs::current_path() / "..\\output";

// HELPER FUNCTIONS
void ImageToText(const fs::path& inputPath);
void AllImagesToText();
void EncodeAll(FileCodec& codec);
void DecodeAll(FileCodec& codec);
void MakeResultsFile(const std::string& codecName);
void MakeGraphics(const std::string& codecName);

int main()
{
    std::cout << "Start" << std::endl;

    // TEST CODE

    CodecAFMTxtOnly codec;
    codec.Encode("..\\input\\txt\\temp.txt", "..\\output\\encoded\\temp_encoded.txt");
    codec.Decode("..\\output\\encoded\\temp_encoded.txt", "..\\output\\decoded\\temp_decoded.txt");

    //MakeResultsFile("RLE");

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
    // create directory if it doesn't exist
    fs::create_directory(OUTPUT_DIR / "encoded");
    
    for (const auto& entry : fs::directory_iterator(INPUT_DIR / "txt")) {
        fs::path inputPath = entry.path();

        fs::path outputPath = OUTPUT_DIR / "encoded" / (inputPath.stem().string() + "_encoded.bin"); 

        codec.Encode(inputPath.string(), outputPath.string());
    }
}

void DecodeAll(FileCodec& codec)
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

        codec.Decode(inputPath.string(), outputPath.string());
    }
}

void MakeResultsFile(const std::string& codecName)
{
    // BIN IMPLEMENTATION
    std::ofstream file((OUTPUT_DIR / ("results_"+ codecName + ".txt")));
    file << "fileName entropyRatio startSize[kb] encodedSize[kb] EncodingRatio decodingRatio";

    for (const auto& entry : fs::directory_iterator(INPUT_DIR / ("txt"))) {
        fs::path inputPath = entry.path();
        std::string fileName = inputPath.stem().string(); // name with no extension

        fs::path pathToOriginal = inputPath;
        fs::path pathToEncoded = (OUTPUT_DIR / "encoded" / (fileName + "_encoded.bin"));
        fs::path pathToDecoded = (OUTPUT_DIR / "decoded" / (fileName + "_decoded.txt"));

        MyFile fileOriginal(pathToOriginal.string(), "r");
        std::wstring textOriginal = fileOriginal.ReadWideContent();

        file << '\n';
        file << fileName + ' ' + 
                std::to_string(TextEntropy(textOriginal)) + ' ' + 
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
        std::cout << "Error: File " 
            << "results_" + codecName + ".txt" 
            << " doesn't exist" << std::endl;
        return;
    }

    std::string command = "python include/plot.py " + inputPath.string();
    system(command.c_str());
}


// END IMPLEMENTATION