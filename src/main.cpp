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
        throw std::runtime_error("Fatal error opening file " + filePath);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Fatal error reading file " + filePath);
    }
    return buffer;
}

void writeFileContents(const std::string& filePath, const std::vector<char>& contents) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("\033[31mFailed opening file " + filePath + "\033[0m");
    }
    file.write(contents.data(), contents.size());
}

void insertFile(std::ofstream& outputFile, const std::string& filePath) {
    std::string fileName = fs::path(filePath).stem().string();
    std::string fileExtension = fs::path(filePath).extension().string().substr(1);
    if (fileExtension == ".fdir") {
        std::cout << "\033[33m]" << fileName << " was skipped because packing .fdir files is not currently supported\n\033[0m";
    } else if (fileExtension != ".fdir") {
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

        std::cout << "\033[32mSuccessfully wrote " << fileName << " to FDIR\033[0m" << std::endl;
    }
}

void insertMode(const std::string& inputPath, const std::string& outputFileName) {
    std::string outputFilePath = outputFileName + ".fdir";

    std::ios_base::openmode mode = std::ios::binary | std::ios::app;
    if (!fs::exists(outputFilePath)) {
        mode = std::ios::binary;
    }

    std::ofstream outputFile(outputFilePath, mode);
    if (!outputFile.is_open()) {
        throw std::runtime_error("\033[31mFailed opening file " + outputFilePath + "\033[0m");
    }

    if (fs::is_directory(inputPath)) {
        for (const auto& entry : fs::recursive_directory_iterator(inputPath)) {
            if (entry.is_regular_file()) {
                insertFile(outputFile, entry.path().string());
            }
        }
    } else if (fs::is_regular_file(inputPath)) {
        insertFile(outputFile, inputPath);
    } else {
        throw std::runtime_error("\033[31mInvalid input path " + inputPath + "\033[0m");
    }

    outputFile.close();
    std::cout << "\033[32mAll files were successfully written to " << outputFilePath << "\033[0m" << std::endl;
}

void extractMode(const std::string& inputFilePath, const std::string& outputDir) {
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile.is_open()) {
        throw std::runtime_error("\033[31mError opening input file: " + inputFilePath + "\033[0m");
    }

    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    } else if (!fs::is_directory(outputDir)) {
        throw std::runtime_error("\033[33mOutput path is not a directory: " + outputDir + "\033[0m");
    }

    while (inputFile.peek() != EOF) {
        std::string delimiter(FILE_START.size(), '\0');
        inputFile.read(&delimiter[0], FILE_START.size());

        if (delimiter != FILE_START) {
            throw std::runtime_error("\033[31mYour FDIR may be corrupt. Invalid FDIR file format: missing [FILE] start delimiter\033[0m");
        }

        auto readDelimitedValue = [&inputFile](const std::string& startDelim, const std::string& endDelim) {
            std::string startTag(startDelim.size(), '\0');
            inputFile.read(&startTag[0], startDelim.size());

            if (startTag != startDelim) {
                throw std::runtime_error("\033[31mFatal error: Your FDIR may be corrupt. Invalid FDIR file format: missing " + startDelim + " start delimiter\033[0m");
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

        std::string outputFilePath = (fs::path(outputDir) / (fileName + "." + fileExtension)).string();
        writeFileContents(outputFilePath, std::vector<char>(fileContents.begin(), fileContents.end()));

        std::string endDelimiter(FILE_END.size(), '\0');
        inputFile.read(&endDelimiter[0], FILE_END.size());

        if (endDelimiter != FILE_END) {
            throw std::runtime_error("\033[31mFatal error: Your FDIR may be corrupt. Invalid FDIR file format: missing [/FILE] end delimiter\033[0m");
        }
    }

    inputFile.close();
    std::cout << "\033[32mAll files were successfully extracted from FDIR to " << outputDir << "\033[0m" << std::endl;
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
            std::cerr << "\033[32mInvalid arguments.\n\033[0m";
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
        std::cerr << "\033[31mFatal error: Exception was thrown: " << e.what() << "\033[0m" << std::endl;
        return 1;
    }

    return 0;
}
