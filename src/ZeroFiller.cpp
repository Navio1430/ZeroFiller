#include <iostream>
#include <fstream>
#include <optional>
#include <map>
#include <filesystem>
#include "args.hxx"

std::map<std::string, long> units = {
    { "KB", pow(1024, 1) },
    { "MB", pow(1024, 2) },
};

std::optional<long> parseSize(std::string arg) {

    if (arg.size() == 0) {
        return std::nullopt;
    }

    if (!isdigit(arg[0])) {
        return std::nullopt;
    }
    
    int index = -1;

    for (int i = 0; i < arg.size(); i++) {
        if (isdigit(arg[i])) {
            continue;
        }

        index = i;
        break;
    }

    // Unit not specified
    if (index == -1) {
        return std::nullopt;
    }

    int size = std::stoi(arg.substr(0, index));
    std::string unitStr = arg.substr(index, arg.size());
    
    auto unit = units.find(unitStr);
    // Element not found
    if (unit == units.end()) {
        return std::nullopt;
    }

    return size * unit->second;
}

int main(int argc, char* argv[])
{

    args::ArgumentParser parser("Simple application that fills files with zeros into desired size.");
    args::HelpFlag help(parser, "help", "Displays this menu", { 'h', "help" });
    
    args::ValueFlag<std::string> directoryArg(parser, "file path", "Target file path", { 'd' });
    args::ValueFlag<std::string> desiredSizeArg(parser, "desired size", "Desired size. [number][MB/KB] e.g. 600MB or 600KB", { 's' });

    try {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Help&) {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

#pragma region DIRECTORY
    if (!directoryArg) {
        std::cerr << "Directory not specified! Type --help for help" << std::endl;
        return 1;
    }

    std::string filePath = args::get(directoryArg);

    if (!std::filesystem::exists(filePath)) {
        std::cerr << "Specified file does not exists! (" << filePath << ") Type --help for help" << std::endl;
        return 1;
    }
    
    std::ofstream out(filePath, std::ios::app);

    if (!out) {
        std::cerr << "Failed to open the file: " << filePath << std::endl;
        return 1;
    }

    long fileSize = std::filesystem::file_size(filePath);
#pragma endregion DIRECTORY

#pragma region SIZE
    if (!desiredSizeArg) {
        std::cerr << "Desired size is not specified! Type --help for help" << std::endl;
        return 1;
    }

    std::optional<long> parsedSize = parseSize(args::get(desiredSizeArg));
    if (!parsedSize) {
        std::cerr << "Could not parse desired size parameter! Type --help for help" << std::endl;
        return 1;
    }

    long desiredSize = parsedSize.value();
#pragma endregion SIZE

    if (desiredSize < fileSize) {
        std::cerr << "Desired size must be higher than target file size." << std::endl;
        return 1;
    }

    long fillSize = desiredSize - fileSize;

    char zeros[1024] = { 0 };

    long f = fillSize / 1024;
    long d = fillSize - (f * 1024);

    for (int i = 0; i < f; ++i) {
        out.write(zeros, 1024);
    }

    char zero = '\0';

    for (int i = 0; i < d; i++) {
        out.write(&zero, sizeof(char));
    }

    std::cout << "File was successfully filled with " << fillSize << " bytes." << std::endl;

}