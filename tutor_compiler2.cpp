// tutor_compiler.cpp  -- Week 2 version
#include <bits/stdc++.h>
using namespace std;

struct ErrorInfo {
    string file;
    int line = -1;
    int column = -1;
    string category;
    string message;
};

string runCommand(const string &cmd) {
    array<char, 256> buffer;
    string result;
    // popen reads command stdout+stderr when we use "2>&1"
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "Error: popen() failed to start the compiler.\n";
    while (fgets(buffer.data(), (int)buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

static inline string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

bool parseErrorLine(const string &line, ErrorInfo &err) {
    // We only try to parse lines that contain "error:"
    if (line.find("error:") == string::npos) return false;

    // g++ typical format: file:line:column: error: message
    // Find the first three ':' positions
    size_t p1 = line.find(':');
    if (p1 == string::npos) return false;
    size_t p2 = line.find(':', p1 + 1);
    if (p2 == string::npos) return false;
    size_t p3 = line.find(':', p2 + 1);
    if (p3 == string::npos) return false;

    // Extract file, line, column
    err.file = trim(line.substr(0, p1));
    string lineStr = trim(line.substr(p1 + 1, p2 - p1 - 1));
    string colStr  = trim(line.substr(p2 + 1, p3 - p2 - 1));

    try {
        err.line = stoi(lineStr);
    } catch (...) { err.line = -1; }
    try {
        err.column = stoi(colStr);
    } catch (...) { err.column = -1; }

    // The rest after p3 is like " error: expected ';' before 'int'"
    string rest = line.substr(p3 + 1);
    // remove leading spaces
    rest = trim(rest);

    // Usually it starts with "error: MESSAGE" but we've already checked "error:"
    // So skip initial "error" token if present
    size_t posErr = rest.find("error:");
    if (posErr != string::npos) {
        err.message = trim(rest.substr(posErr + strlen("error:")));
    } else {
        // fallback
        err.message = rest;
    }

    return true;
}

string classifyError(const string &msg) {
    string m = msg;
    // lower-case copy to make matching simpler
    transform(m.begin(), m.end(), m.begin(), ::tolower);

    if (m.find("expected ';'") != string::npos || m.find("missing ';'") != string::npos)
        return "Missing Semicolon";
    if (m.find("was not declared in this scope") != string::npos)
        return "Undeclared Identifier";
    if (m.find("no matching function for call") != string::npos ||
        m.find("candidate function not viable") != string::npos)
        return "Function Call Mismatch";
    if (m.find("cannot convert") != string::npos || m.find("invalid conversion") != string::npos)
        return "Type Mismatch";
    if (m.find("expected ';' before") != string::npos)
        return "Syntax Error";
    return "Unknown Error";
}

vector<string> splitLines(const string &text) {
    vector<string> lines;
    stringstream ss(text);
    string line;
    while (getline(ss, line)) lines.push_back(line);
    return lines;
}

int main(int argc, char* argv[]) {
    string filename = "test2.cpp";
    if (argc > 1) filename = argv[1];

    // Build compile command: redirect stderr to stdout so we can read it
    string cmd = "g++ " + filename + " 2>&1";

    cout << "Compiling: " << filename << "  (running: " << cmd << ")\n\n";
    string output = runCommand(cmd);

    if (output.empty()) {
        cout << "✅ Compilation successful: no errors.\n";
        return 0;
    }

    auto lines = splitLines(output);
    vector<ErrorInfo> errors;

    // parse each line for errors
    for (const auto &ln : lines) {
        ErrorInfo e;
        if (parseErrorLine(ln, e)) {
            e.category = classifyError(e.message);
            errors.push_back(e);
        }
    }

    if (errors.empty()) {
        cout << "No parsable 'error:' lines found. Raw compiler output:\n";
        cout << output << "\n";
        return 0;
    }

    // Print structured errors
    for (size_t i = 0; i < errors.size(); ++i) {
        auto &e = errors[i];
        cout << "---------------------------\n";
        cout << "Error #" << (i+1) << "\n";
        cout << "File   : " << e.file << "\n";
        cout << "Line   : " << e.line << ", Column: " << e.column << "\n";
        cout << "Class  : " << e.category << "\n";
        cout << "Message: " << e.message << "\n";
    }
    cout << "---------------------------\n";
    return 0;
}
