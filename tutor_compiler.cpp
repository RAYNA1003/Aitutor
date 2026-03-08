#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <string>

std::string runCommand(const char* cmd) {
    std::array<char, 256> buffer;
    std::string result;

    // Run command and capture stderr
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(cmd, "r"), pclose
    );

    if (!pipe) {
        return "Error: popen() failed!";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

int main() {
    std::string command = "g++ test.cpp 2>&1";
    std::string compilerOutput = runCommand(command.c_str());

    if (compilerOutput.empty()) {
        std::cout << "✅ Compilation successful. No errors.\n";
    } else {
        std::cout << "❌ Compiler Errors:\n";
        std::cout << compilerOutput << std::endl;
    }

    return 0;
}
