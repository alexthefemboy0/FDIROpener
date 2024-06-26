#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>
#include <regex>

namespace fs  = std::filesystem;


std::string readFileContents(const std::string& filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writeFileContents(const std::string filePath, const std::string &contents) {
    std::ofstream file(filePath);
    file << contents;
}

std::string getFileExtension(const std::string& fileName) {
    size_t pos = fileName.find_last_of(".");
    if (pos != std::string::npos) {
        return fileName.substr(pos + 1);
    }
    return "";
}

int insertMode(const std::string& folderPath, const std::string& outputFilePath) {
    std::ofstream outputFile(outputFilePath);

    if (!outputFile.is_open()) {
        std::cerr << "Fatal error opening file. Stop." << std::endl;
        return 1;
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().filename().string();
            std::string fileExtension = getFileExtension(fileName);
            std::string fileContents = readFileContents(filePath);

            outputFile << "[FILE]\n";
            outputFile << "[NAME]" << fileName << "[/NAME]\n";
            outputFile << "[EXT]" << fileExtension << "[/EXT]\n";
            outputFile << "[CON]" << fileContents << "[/CON]\n";
            outputFile << "[/FILE]\n";
            std::cout << "Wrote file " << fileName << "." << fileExtension << " with contents " << fileContents << std::endl; 
        }
    }

    outputFile.close();
    std::cout << "Files were successfully written to " << outputFilePath << "." << std::endl;
    return 0;
}

int extractMode(const std::string& inputFilePath) {
    std::ifstream inputFile(inputFilePath);

    if (!inputFile.is_open()) {
        std::cerr << "Fatal error opening file. Stop." << std::endl;
        return 1;
    }


    std::string line;
    std::string fileName, fileContents;
    bool inFile = false;

    while (std::getline(inputFile, line)) {
        if (line == "[FILE]") {
            inFile = true;
            fileName.clear();
            fileContents.clear();
        } else if (line == "[/FILE]") {
            if (!fileName.empty() && fileContents.empty()) {
                writeFileContents(fileName, fileContents);
                std::cout << "Extracted file " << fileName << " with the contents " << fileContents << std::endl;
            }
            inFile = false;
        } else if (inFile) {
            if (line.rfind("[NAME]", 0) == 0) {
                fileName = line.substr(6, line.size() - 13);
            } else if (line.rfind("[CON]", 0) == 0) {
                fileContents = line.substr(5, line.size() - 11);
            }
        }
    }

    inputFile.close();
    std::cout << "Files were sucessfully written to disk." << std::endl;
}

int Init() {
    std::string ModeFilePath = "../MODE";
    std::string folderPath = "../input";
    std::string outputFilePath = "../output/output_files.fdir";

    std::ifstream modeFile(ModeFilePath);
    std::string mode;
    if (modeFile.is_open()) {
        std::getline(modeFile, mode);
        modeFile.close();
    } else {
        std::cerr << "Fatal error getting mode. Stop." << std::endl;
        return 1;
    }

    for (char& c : mode) {
        c = std::toupper(c);
    }

    if (mode == "INSERT") {
        insertMode(folderPath, outputFilePath);
    } else if (mode == "EXTRACT") {
        extractMode(outputFilePath);
    } else {
        std::cerr << "Invalid mode specified in your MODE file. It should be either 'INSERT' or 'EXTRACT' but it was " << mode << ". Stop." << std::endl;
        return 1;
    }

    return 0;
}