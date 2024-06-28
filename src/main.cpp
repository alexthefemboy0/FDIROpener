#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <string.h>

namespace fs = std::filesystem;

const std::string FILE_START = "[FILE]";
const std::string FILE_END = "[/FILE]";
const std::string NAME_START = "[NAME]";
const std::string NAME_END = "[/NAME]";
const std::string EXT_START = "[EXT]";
const std::string EXT_END = "[/EXT]";
const std::string CON_START = "[CON]";
const std::string CON_END = "[/CON]";

std::vector<char> readFileContents(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Fatal error opening file " + filePath + ". Stop.");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Fatal error reading file " + filePath + ". Stop.");
    }
    return buffer;
}

void writeFileContents(const std::string& filePath, const std::vector<char>& contents) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Fatal error opening file " + filePath + ". Stop.");
    }
    file.write(contents.data(), contents.size());
}

void insertFile(std::ofstream& outputFile, const std::string& filePath) {
    std::string fileName = fs::path(filePath).filename().string();
    std::string fileExtension = fs::path(filePath).extension().string();
    std::vector<char> fileContents = readFileContents(filePath);

    outputFile.write(FILE_START.c_str(), FILE_START.size());

    outputFile.write(NAME_START.c_str(), NAME_START.size());
    outputFile.write(fileName.c_str(), fileName.size());
    outputFile.write(NAME_END.c_str(), NAME_END.size());

    outputFile.write(EXT_START.c_str(), EXT_START.size());
    outputFile.write(fileExtension.c_str(), fileExtension.size());
    outputFile.write(EXT_END.c_str(), EXT_END.size());

    outputFile.write(CON_START.c_str(), CON_START.size());
    outputFile.write(fileContents.data(), fileContents.size());
    outputFile.write(CON_END.c_str(), CON_END.size());

    outputFile.write(FILE_END.c_str(), FILE_END.size());

    std::cout << "Successfully wrote " << fileName << " to FDIR." << std::endl; 
}

void insertMode(const std::string& inputPath, const std::string& outputFileName) {
    std::string outputFilePath = outputFileName + ".fdir";
    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile.is_open()) {
        throw std::runtime_error("Fatal error opening output file " + outputFilePath + ". Stop.");
    }

    if (fs::is_directory(inputPath)) {
        for (const auto& entry : fs::directory_iterator(inputPath)) {
            if (entry.is_regular_file()) {
                insertFile(outputFile, entry.path().string());
            }
        }
    } else if (fs::is_regular_file(inputPath)) {
        insertFile(outputFile, inputPath);
    } else {
        throw std::runtime_error("Fatal error invalid input path " + inputPath + ". Stop.");
    }

    outputFile.close();
    std::cout << "Successfully inserted all files to " << outputFilePath << std::endl;
}

void extractMode(const std::string& inputFilePath, const std::string& outputDir) {
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Error opening input file: " + inputFilePath);
    }

    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    } else if (!fs::is_directory(outputDir)) {
        throw std::runtime_error("Output path is not a directory: " + outputDir);
    }

    while (inputFile.peek() != EOF) {
        std::string delimiter(FILE_START.size(), '\0');
        inputFile.read(&delimiter[0], FILE_START.size());

        if (delimiter != FILE_START) {
            throw std::runtime_error("Invalid FDIR file format: missing [FILE] start delimiter");
        }

        auto readDelimitedValue = [&inputFile](const std::string& startDelim, const std::string& endDelim) {
            std::string startTag(startDelim.size(), '\0');
            inputFile.read(&startTag[0], startDelim.size());

            if (startTag != startDelim) {
                throw std::runtime_error("Invalid FDIR file format: missing " + startDelim + " start delimiter");
            }

            std::string value;
            char ch;
            while (inputFile.get(ch)) {
                value.push_back(ch);
                if (value.size() >= endDelim.size() &&
                    std::equal(endDelim.rbegin(), endDelim.rend(), value.rbegin())) {
                    value.erase(value.end() - endDelim.size(), value.end());
                    break;
                }
            }

            return value;
        };

        std::string fileName = readDelimitedValue(NAME_START, NAME_END);
        std::string fileExtension = readDelimitedValue(EXT_START, EXT_END);
        std::string fileContents = readDelimitedValue(CON_START, CON_END);

        std::string outputFilePath = (fs::path(outputDir) / fileName).string();
        std::cout << "Writing to file: " << outputFilePath << std::endl; // Debug output
        writeFileContents(outputFilePath, std::vector<char>(fileContents.begin(), fileContents.end()));

        std::string endDelimiter(FILE_END.size(), '\0');
        inputFile.read(&endDelimiter[0], FILE_END.size());

        if (endDelimiter != FILE_END) {
            throw std::runtime_error("Invalid FDIR file format: missing [/FILE] end delimiter");
        }
    }

    inputFile.close();
    std::cout << "All files were successfully extracted from FDIR to " << outputDir << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 5) {
            std::cerr << "Usage: -i <input file> OR <input directory> -o <output file name>\nOR\n       -e <fdir file> -o <output_directory>\n";
            return 1;
        }

        std::string mode;
        std::string inputPath;
        std::string outputPath;

        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
                inputPath = argv[++i];
                mode = "INSERT";
            } else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
                inputPath = argv[++i];
                mode = "EXTRACT";
            } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
                outputPath = argv[++i];
            }
        }

        if (inputPath.empty() || outputPath.empty()) {
            std::cerr << "Fatal error: Invalid arguments.\n";
            return 1;
        }

        if (mode == "INSERT") {
            insertMode(inputPath, outputPath);
        } else if (mode == "EXTRACT") {
            extractMode(inputPath, outputPath);
        } else {
            std::cerr << "Invalid mode.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: Exception was thrown: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
