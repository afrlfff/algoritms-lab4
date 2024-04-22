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
#include "include/EncodeToString.h"

namespace fs = std::filesystem;
const fs::path INPUT_DIR = fs::current_path() / "..\\input";
const fs::path OUTPUT_DIR = fs::current_path() / "..\\output";

// HELPER FUNCTIONS
void ClearOutputDirectory();
void ImageToText(const fs::path& inputPath);
void AllImagesToText();
void EncodeAll(FileCodec& codec);
void DecodeAll(FileCodec& codec);
void MakeResultsFile();
void MakeGraphics(const std::string& codecName);

int main()
{
    //ClearOutputDirectory();
    std::cout << "Start" << std::endl;

    // TEST CODE

    CodecHA codec;

    //EncodeAll(codec);
    //DecodeAll(codec);
    //MakeResultsFile();

    //std::string text = "abcd";
    //std::string encodedText = EncodeHA_toString(text);
    //std::cout << encodedText << std::endl;


    codec.Encode("..\\input\\txt\\enwik7.txt", "..\\output\\encoded\\enwik7_encoded.bin");
    codec.Decode("..\\output\\encoded\\enwik7_encoded.bin", "..\\output\\decoded\\enwik7_decoded.txt");

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

        codec.Encode(inputPath.string().c_str(), outputPath.string().c_str());
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

        codec.Decode(inputPath.string().c_str(), outputPath.string().c_str());
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
        std::cout << "Error: File " 
            << "results_" + codecName + ".txt" 
            << " doesn't exist" << std::endl;
        return;
    }

    std::string command = "python include/plot.py " + inputPath.string();
    system(command.c_str());
}


// END IMPLEMENTATION