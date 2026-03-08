    #include <iostream>
    #include <vector>
    #include <string>
    #include <set>
    #include <cctype>
    #include<sstream>
   #include <map>
    #include <unordered_set>
    #include <unordered_map>
    #include <cstdlib>

    using namespace std;

    /* =======================
    TOKEN DEFINITIONS
    ======================= */

    enum TokenType {
        KEYWORD,
        IDENTIFIER,
        NUMBER,
        OPERATOR,
        STRING_LITERAL,
        DELIMITER,
        END_OF_FILE,
        INVALID
    };

    struct Token {
        TokenType type;
        string value;
        int line;
    };
    struct CompilerError {
    string type;          // Syntax / Semantic / Type
    string message;       // What happened
    string explanation;   // Why it happened
    string hint;          // How to fix it
    int line;
};
struct CompilationResult {
    bool success;
    vector<CompilerError> errors;
    vector<string> irInstructions;
    map<string, string> symbolTableSnapshot;
};
    enum ASTNodeType {
        AST_NUMBER,
        AST_VARIABLE,
        AST_STRING,
        AST_BINARY,
        AST_ASSIGN,
        AST_PROGRAM,
        AST_IF,
        AST_PRINT,
        AST_WHILE

    };
    struct ASTNode {
        ASTNodeType type;
        string value;          // operator, variable name, or number
        ASTNode* left;
        ASTNode* elseBranch;

        vector<ASTNode*> children; 
        ASTNode* right;

        ASTNode(ASTNodeType t, string v = "")
            : type(t), value(v), left(nullptr), right(nullptr) {}
    };


    /* =======================
    TOKEN TYPE TO STRING
    ======================= */

    string tokenTypeToString(TokenType type) {
        switch (type) {
            case KEYWORD: return "KEYWORD";
            case IDENTIFIER: return "IDENTIFIER";
            case NUMBER: return "NUMBER";
            case OPERATOR: return "OPERATOR";
            case DELIMITER: return "DELIMITER";
            case END_OF_FILE: return "EOF";
            case INVALID: return "INVALID";
            default: return "UNKNOWN";
        }
    }

    /* =======================
    LEXER
    ======================= */

set<string> keywords = {"int","string","bool","if","while","else","print","true","false"};

    class Lexer {
    private:
        string source;
        int pos;
        int line;

    public:
        Lexer(const string& src) : source(src), pos(0), line(1) {}

        vector<Token> tokenize() {
            vector<Token> tokens;

            while (pos < source.length()) {
                char current = source[pos];

                // Ignore spaces and tabs
                if (current == ' ' || current == '\t') {
                    pos++;
                    continue;
                }

                // Newline
                if (current == '\n') {
                    line++;
                    pos++;
                    continue;
                }
                // String literal
    if (current == '"') {
        pos++; // skip opening quote
        string value;

        while (pos < source.length() && source[pos] != '"') {
            value += source[pos++];
        }

        if (pos >= source.length()) {
            cout << "Unterminated string at line " << line << endl;
            exit(1);
        }

        pos++; // skip closing quote
        tokens.push_back({STRING_LITERAL, value, line});
        continue;
    }


                // Identifier or keyword
                if (isalpha(current)) {
                    string value;
                    while (pos < source.length() &&
                        (isalnum(source[pos]) || source[pos] == '_')) {
                        value += source[pos++];
                    }

                    if (keywords.count(value))
                        tokens.push_back({KEYWORD, value, line});
                    else
                        tokens.push_back({IDENTIFIER, value, line});

                    continue;
                }

                // Number
                if (isdigit(current)) {
                    string value;
                    while (pos < source.length() && isdigit(source[pos])) {
                        value += source[pos++];
                    }
                    tokens.push_back({NUMBER, value, line});
                    continue;
                }

                // Operator
                // Multi-character operators
if (pos + 1 < source.length()) {
    string twoChar = source.substr(pos, 2);

    if (twoChar == "==" || twoChar == "!=" ||
        twoChar == "<=" || twoChar == ">=" ||
        twoChar == "&&" || twoChar == "||") {

        tokens.push_back({OPERATOR, twoChar, line});
        pos += 2;
        continue;
    }
}

// Single-character operators
if (current == '=' || current == '+' || current == '-' ||
    current == '*' || current == '/' ||
    current == '>' || current == '<') {

    tokens.push_back({OPERATOR, string(1, current), line});
    pos++;
    continue;
}

                // Delimiter
            // Delimiter
                if (current == ';' || current == '(' || current == ')' || current == ',' || current == '{' || current == '}') {
                tokens.push_back({DELIMITER, string(1, current), line});
                pos++;
                continue;
                }


                // Invalid character
                tokens.push_back({INVALID, string(1, current), line});
                pos++;
            }

            tokens.push_back({END_OF_FILE, "EOF", line});
            return tokens;
        }
    };



    /* =======================
    PARSER
    ======================= */
    struct Symbol {
    string type;
    bool initialized;
};

class SymbolTable {
private:
    vector<unordered_map<string, Symbol>> scopes;

public:
    bool exists(const string& name) {
    return lookup(name) != nullptr;
}

string getType(const string& name) {
    Symbol* sym = lookup(name);
    if (sym) return sym->type;
    return "";
}
    void enterScope() {
        scopes.push_back({});
    }

    void exitScope() {
        if (!scopes.empty())
            scopes.pop_back();
    }

    bool declare(const string& name, const string& type) {
        if (scopes.empty())
            enterScope();

        if (scopes.back().count(name))
            return false;

        scopes.back()[name] = {type, false};
        return true;
    }

    Symbol* lookup(const string& name) {
        for (int i = scopes.size() - 1; i >= 0; i--) {
            if (scopes[i].count(name))
                return &scopes[i][name];
        }
        return nullptr;
    }

    void markInitialized(const string& name) {
        Symbol* sym = lookup(name);
        if (sym)
            sym->initialized = true;
    }
};

    void printAST(ASTNode* node, int depth = 0) {
        if (!node) return;

        for (int i = 0; i < depth; i++)
            cout << "  ";

        cout << node->value << endl;

        printAST(node->left, depth + 1);
        printAST(node->right, depth + 1);
    }

    struct Value {
        string type;  // "int" or "string"
        int intValue;
        string strValue;
        bool boolValue;
    };



    class Parser {
    private:
        vector<Token> tokens;
        int pos;
        SymbolTable symTable; 
        vector<CompilerError> errors;
    
    unordered_map<string, Value> memory;


    Token current() {
        if (pos >= tokens.size())
            return tokens.back(); // EOF
        return tokens[pos];
    }


        void advance() {
            if (pos < tokens.size())
                pos++;
        }

       void error(const string& msg) {
   errors.push_back({
    "Syntax",
    msg,
    "The parser found an unexpected token while reading the program.",
    "Check the syntax near this line and ensure correct structure (missing symbols, brackets, or semicolons).",
    current().line
});

    advance();  // skip token
}

        void expect(TokenType type, const string& msg) {
            if (current().type == type)
                advance();
            else
                error(msg);
        }

    public:
        Parser(const vector<Token>& toks) : tokens(toks), pos(0) {
            symTable.enterScope(); 
        }
vector<CompilerError> getErrors() {
    return errors;
}
    ASTNode* parseProgram() {
        ASTNode* program = new ASTNode(AST_PROGRAM, "PROGRAM");
        int res=0;
        while (current().type != END_OF_FILE) {
            ASTNode* stmt = parseStatement();
            if (stmt){
            
                program->children.push_back(stmt);

            }
        }
         
        
        return program;
      
    }
   ASTNode* parseCondition() {
    ASTNode* left = parseExpression();

    if (current().type == OPERATOR &&
        (current().value == ">"  || current().value == "<"  ||
         current().value == ">=" || current().value == "<=" ||
         current().value == "==" || current().value == "!=")) {

        string op = current().value;
        advance();

        ASTNode* right = parseExpression();

        ASTNode* node = new ASTNode(AST_BINARY, op);
        node->left = left;
        node->right = right;

        return node;
    }

    return left;
}

    ASTNode* parsePrint() {
        expect(KEYWORD, "Expected 'print'");
        expect(DELIMITER, "Expected '('");

        ASTNode* expr = parseExpression();

        expect(DELIMITER, "Expected ')'");
        expect(DELIMITER, "Expected ';'");

        ASTNode* printNode = new ASTNode(AST_PRINT, "print");
        printNode->left = expr;

        return printNode;
    }

    ASTNode* parseWhile() {
        expect(KEYWORD, "Expected 'while'");

        expect(DELIMITER, "Expected '('");

        ASTNode* condition = parseCondition();

        expect(DELIMITER, "Expected ')'");

        expect(DELIMITER, "Expected '{'");
        symTable.enterScope();
        ASTNode* whileNode = new ASTNode(AST_WHILE, "while");
        whileNode->left = condition;

        while (!(current().type == DELIMITER && current().value == "}")) {
            ASTNode* stmt = parseStatement();
            if (stmt)
                whileNode->children.push_back(stmt);
        }
        symTable.exitScope();
       if (!(current().type == DELIMITER && current().value == "}")) {
    errors.push_back({
        "Syntax",
        "Missing closing brace '}'",
        "The block was not properly closed.",
        "Add '}' to close the block.",
        current().line
    });
} else {
    advance();
}

        return whileNode;
    }


    ASTNode* parseIf() {
        expect(KEYWORD, "Expected 'if'");
        expect(DELIMITER, "Expected '('");

        ASTNode* condition = parseCondition();

        expect(DELIMITER, "Expected ')'");
        expect(DELIMITER, "Expected '{'");

        ASTNode* ifNode = new ASTNode(AST_IF, "if");
        ifNode->left = condition;

        // THEN block
        while (!(current().type == DELIMITER && current().value == "}")) {
            ASTNode* stmt = parseStatement();
            if (stmt)
                ifNode->children.push_back(stmt);
        }

       if (!(current().type == DELIMITER && current().value == "}")) {
    errors.push_back({
        "Syntax",
        "Missing closing brace '}'",
        "The block was not properly closed.",
        "Add '}' to close the block.",
        current().line
    });
} else {
    advance();
}
        symTable.enterScope();
        // 🔥 Check for else
        if (current().type == KEYWORD && current().value == "else") {
            advance();
            expect(DELIMITER, "Expected '{'");
            symTable.enterScope();
            ASTNode* elseNode = new ASTNode(AST_PROGRAM, "else");

            while (!(current().type == DELIMITER && current().value == "}")) {
                ASTNode* stmt = parseStatement();
                if (stmt)
                    elseNode->children.push_back(stmt);
            }
            symTable.exitScope();
            expect(DELIMITER, "Expected '}'");

            ifNode->elseBranch = elseNode;
        }

        return ifNode;
    }




   ASTNode* parseStatement() {

    // 🔥 FIRST: catch invalid tokens explicitly
    if (current().type == INVALID) {
        errors.push_back({
            "Syntax",
            "Unexpected symbol '" + current().value + "'",
            "The symbol is not valid in this language.",
            "Remove or replace this symbol.",
            current().line
        });
        advance();
        return nullptr;
    }

    if (current().type == KEYWORD &&
        (current().value == "int" ||
         current().value == "string" ||
         current().value == "bool")) {

        parseDeclaration();
        return nullptr;
    }

    else if (current().type == KEYWORD && current().value == "if") {
        return parseIf();
    }

    else if (current().type == KEYWORD && current().value == "while") {
        return parseWhile();
    }

    else if (current().type == KEYWORD && current().value == "print") {
        return parsePrint();
    }

    else if (current().type == IDENTIFIER) {
        return parseAssignment();
    }

    else if (current().type == DELIMITER && current().value == "}") {
        errors.push_back({
            "Syntax",
            "Unmatched closing brace '}'",
            "A closing brace was found without a matching opening brace.",
            "Remove this brace or add a matching '{' before it.",
            current().line
        });
        advance();
        return nullptr;
    }

    else {
        errors.push_back({
            "Syntax",
            "Unrecognized statement",
            "The statement does not match declaration, assignment, print, if, or while.",
            "Ensure the statement starts with a valid keyword or variable name.",
            current().line
        });
        advance();
        return nullptr;
    }
}

    ASTNode* parseTerm() {
        ASTNode* node = nullptr;

        if (current().type == NUMBER) {
            node = new ASTNode(AST_NUMBER, current().value);
            advance();
        }
        else if (current().type == KEYWORD && 
        (current().value == "true" || current().value == "false")) {

    string boolVal = current().value;
    node = new ASTNode(AST_NUMBER, boolVal);  // reuse for now
    advance();
}

        else if (current().type == IDENTIFIER) {
    string varName = current().value;

    Symbol* sym = symTable.lookup(varName);
if (!sym) {
   errors.push_back({
    "Semantic",
    "Variable '" + varName + "' used without declaration",
    "You are trying to use a variable that was never declared in this scope.",
    "Declare the variable before using it. Example: int " + varName + ";",
    current().line
});
    advance();   // move forward to avoid infinite loop
    return new ASTNode(AST_NUMBER, "0");  // dummy node
}
  if (!sym->initialized) {
    errors.push_back({
    "Semantic",
    "Variable '" + varName + "' used before initialization",
    "The variable was declared but no value was assigned before using it.",
    "Assign a value before using it. Example: " + varName + " = 10;",
    current().line
});
}

    node = new ASTNode(AST_VARIABLE, varName);
    advance();
}else if (current().type == STRING_LITERAL) {
        node = new ASTNode(AST_STRING, current().value);
        advance();
    }

        else if (current().type == DELIMITER && current().value == "(") {
            advance(); // '('
            node = parseExpression();
           if (current().type == DELIMITER && current().value == ")") {
    advance();
} else {
    errors.push_back({
        "Syntax",
        "Missing closing parenthesis ')'",
        "Every '(' must have a matching ')'.",
        "Add ')' to close the expression.",
        current().line
    });
}
        }
        else {
            error("Expected number or variable");
        }

        while (current().type == OPERATOR &&
            (current().value == "*" || current().value == "/")) {

            string op = current().value;
            advance();

            ASTNode* right = parseTerm();

            ASTNode* parent = new ASTNode(AST_BINARY, op);
            parent->left = node;
            parent->right = right;

            node = parent;
        }

        return node;
    }



    ASTNode* parseExpression() {
        ASTNode* node = parseTerm();

        while (current().type == OPERATOR &&
            (current().value == "+" || current().value == "-")) {

            string op = current().value;
            advance();

            ASTNode* right = parseTerm();

            ASTNode* parent = new ASTNode(AST_BINARY, op);
            parent->left = node;
            parent->right = right;

            node = parent;
        }

        return node;
    }




    void parseDeclaration() {
    string varType = current().value;

    if (current().type == KEYWORD &&
    (varType == "int" || varType == "string" || varType == "bool")) {
        advance();
    } else {
        error("Expected type declaration");
    }


        // At least one variable
        while (true) {
            if (current().type != IDENTIFIER) {
                error("Expected variable name");
            }

            string varName = current().value;

            // 🔴 Redeclaration check
         if (!symTable.declare(varName, varType)) {
   errors.push_back({
    "Semantic",
    "Variable '" + varName + "' redeclared in same scope",
    "A variable with the same name already exists in the current block.",
    "Use a different variable name or remove the duplicate declaration.",
    current().line
});
}
          

            advance(); // consume identifier

            // If comma, continue parsing variables
            if (current().type == DELIMITER && current().value == ",") {
                advance(); // consume ','
                continue;
            }

            // End of declaration
            break;
        }
if (!(current().type == DELIMITER && current().value == ";")) {
    errors.push_back({
        "Syntax",
        "Missing semicolon",
        "Every declaration must end with ';'.",
        "Add ';' at the end of the declaration.",
        current().line
    });
} else {
    advance();
}
    }



    ASTNode* parseAssignment() {
        string varName = current().value;
        expect(IDENTIFIER, "Expected variable name");

       Symbol* sym = symTable.lookup(varName);
if (!sym) {
   errors.push_back({
    "Semantic",
    "Variable '" + varName + "' used without declaration",
    "You are assigning a value to a variable that has not been declared.",
    "Declare it first. Example: int " + varName + ";",
    current().line
});

    advance();
    return new ASTNode(AST_NUMBER, "0"); // dummy
}

        if (current().type == OPERATOR && current().value == "=")
            advance();
            if (current().type == DELIMITER && current().value == ";") {
    errors.push_back({
        "Syntax",
        "Incomplete assignment",
        "An expression is required after '='.",
        "Provide a number or variable after '='.",
        current().line
    });
}
        else
            error("Expected '='");

        ASTNode* expr = parseCondition();
        string varType = symTable.getType(varName);

    // Determine expression type
    string exprType;

if (expr->type == AST_NUMBER) {
    if (expr->value == "true" || expr->value == "false")
        exprType = "bool";
    else
        exprType = "int";
}
else if (expr->type == AST_STRING) {
    exprType = "string";
}
else if (expr->type == AST_VARIABLE) {
    exprType = symTable.getType(expr->value);
}
else if (expr->type == AST_BINARY) {

    // relational + equality operators produce bool
    if (expr->value == ">"  || expr->value == "<"  ||
        expr->value == ">=" || expr->value == "<=" ||
        expr->value == "==" || expr->value == "!=")
        exprType = "bool";
    else
        exprType = "int";
}
else {
    exprType = "int";
}


   if (varType != exprType) {
   errors.push_back({
    "Type",
    "Cannot assign " + exprType + " to " + varType,
    "The datatype of the expression does not match the datatype of the variable.",
    "Ensure both sides have the same datatype. Example: int x = 5;",
    current().line
});
}else{
    symTable.markInitialized(varName);
}

      if (!(current().type == DELIMITER && current().value == ";")) {
    errors.push_back({
        "Syntax",
        "Missing semicolon",
        "Every assignment must end with ';'.",
        "Add ';' at the end of the assignment.",
        current().line
    });
} else {
    advance();
}

        ASTNode* assign = new ASTNode(AST_ASSIGN, "=");
        assign->left = new ASTNode(AST_VARIABLE, varName);
        assign->right = expr;
        
        return assign;
    }



    };
    class IRGenerator {
    private:
        int tempCount;
        vector<string> instructions;
        int labelCount = 0;

    string newLabel() {
        return "L" + to_string(++labelCount);
    }

        string newTemp() {
            return "t" + to_string(++tempCount);
        }

    public:
        IRGenerator() : tempCount(0) {}

        string generate(ASTNode* node) {
            if (!node) return "";

            switch (node->type) {

                case AST_NUMBER:
                    return node->value;

                case AST_VARIABLE:
                    return node->value;

                case AST_STRING:
        return "\"" + node->value + "\"";

                case AST_BINARY: {
                    string left = generate(node->left);
                    string right = generate(node->right);

                    string temp = newTemp();
                    instructions.push_back(
                        temp + " = " + left + " " +
                        node->value + " " + right
                    );

                    return temp;
                }
                case AST_PRINT: {
                string value = generate(node->left);
                instructions.push_back("PRINT " + value);
                return "";
            }


                case AST_ASSIGN: {
                    string rhs = generate(node->right);
                    string lhs = node->left->value;

                    instructions.push_back(lhs + " = " + rhs);
                    return lhs;
                }

                case AST_PROGRAM: {
                    for (ASTNode* stmt : node->children) {
                        generate(stmt);
                    }
                    return "";
                }
            case AST_IF: {
        string conditionTemp = generate(node->left);

        string labelElse = newLabel();
        string labelEnd  = newLabel();

        instructions.push_back("IF_FALSE " + conditionTemp + " GOTO " + labelElse);

        // THEN block
        for (ASTNode* stmt : node->children) {
            generate(stmt);
        }

        instructions.push_back("GOTO " + labelEnd);
        instructions.push_back(labelElse + ":");

        // ELSE block
        if (node->elseBranch) {
            for (ASTNode* stmt : node->elseBranch->children) {
                generate(stmt);
            }
        }

        instructions.push_back(labelEnd + ":");

        return "";
    }

    case AST_WHILE: {
        string labelStart = newLabel();
        string labelEnd = newLabel();

        instructions.push_back(labelStart + ":");

        string conditionTemp = generate(node->left);

        instructions.push_back("IF_FALSE " + conditionTemp + " GOTO " + labelEnd);

        for (ASTNode* stmt : node->children) {
            generate(stmt);
        }

        instructions.push_back("GOTO " + labelStart);
        instructions.push_back(labelEnd + ":");

        return "";
    }


            }

            return "";
        }

        void printIR() {
            cout << "\nGenerated IR (Three Address Code):\n";
            for (auto& inst : instructions) {
                cout << inst << endl;
            }
        }

        vector<string> getInstructions() {
            return instructions;
        }
    };
    class IRInterpreter {
    private:
        unordered_map<string, Value> memory;

    public:
        void execute(const vector<string>& instructions) {
            for (int i = 0; i < instructions.size(); i++) {
            string inst = instructions[i];
            if (inst.find("GOTO") == 0) {
        string gotoWord, label;
        stringstream ss(inst);
        ss >> gotoWord >> label;

        for (int j = 0; j < instructions.size(); j++) {
            if (instructions[j] == label + ":") {
                i = j-1;
                break;
            }
        }
        continue;
    }

            if (inst.find("IF_FALSE") == 0) {
        string temp, cond, gotoWord, label;
        stringstream ss(inst);
        ss >> temp >> cond >> gotoWord >> label;
            
        Value condition = memory[cond];

bool isFalse = false;

if (condition.type == "int")
    isFalse = (condition.intValue == 0);
else if (condition.type == "bool")
    isFalse = (!condition.boolValue);

if (isFalse) {
    for (int j = 0; j < instructions.size(); j++) {
        if (instructions[j] == label + ":") {
            i = j - 1;
            break;
        }
    }
}

        continue;
    }
    if (inst.find("PRINT") == 0) {
        string cmd, value;
        stringstream ss(inst);
        ss >> cmd >> value;

    Value val = getValue(value);

   if (val.type == "int")
    cout << val.intValue << endl;
else if (val.type == "string")
    cout << val.strValue << endl;
else if (val.type == "bool")
    cout << (val.boolValue ? "true" : "false") << endl;


        continue;
    }


            if (inst.back() == ':')
                continue;

                // Split instruction
                string lhs, eq, op1, op, op2;
                stringstream ss(inst);

                ss >> lhs >> eq >> op1;

                // If simple assignment
        Value val1 = getValue(op1);

    if (!(ss >> op)) {
        memory[lhs] = val1;
    }
    else {
        ss >> op2;
        Value val2 = getValue(op2);

        if (val1.type == "int" && val2.type == "int") {
            Value result;
            result.type = "int";

            if (op == "+") result.intValue = val1.intValue + val2.intValue;
            if (op == "-") result.intValue = val1.intValue - val2.intValue;
            if (op == "*") result.intValue = val1.intValue * val2.intValue;
            if (op == "/") result.intValue = val1.intValue / val2.intValue;
           if (op == ">") {
    result.type = "bool";
    result.boolValue = val1.intValue > val2.intValue;
}
else if (op == "<") {
    result.type = "bool";
    result.boolValue = val1.intValue < val2.intValue;
}
else if (op == ">=") {
    result.type = "bool";
    result.boolValue = val1.intValue >= val2.intValue;
}
else if (op == "<=") {
    result.type = "bool";
    result.boolValue = val1.intValue <= val2.intValue;
}
else if (op == "==") {
    result.type = "bool";
    result.boolValue = val1.intValue == val2.intValue;
}
else if (op == "!=") {
    result.type = "bool";
    result.boolValue = val1.intValue != val2.intValue;
}


            memory[lhs] = result;
        }
        else if (val1.type == "string" && val2.type == "string" && op == "+") {
            Value result;
            result.type = "string";
            result.strValue = val1.strValue + val2.strValue;
            memory[lhs] = result;
        }
        else {
            cout << "Type Error: Invalid operation between "
                << val1.type << " and " << val2.type << endl;
            exit(1);
        }
    }


            }

            
        }

    private:
       Value getValue(string s) {

    Value v;

    // String literal
    if (s[0] == '"') {
        v.type = "string";
        v.strValue = s.substr(1, s.length() - 2);
        return v;
    }

    // Boolean literal
    if (s == "true") {
        v.type = "bool";
        v.boolValue = true;
        return v;
    }

    if (s == "false") {
        v.type = "bool";
        v.boolValue = false;
        return v;
    }

    // Number
    if (isdigit(s[0]) || (s[0] == '-' && s.size() > 1)) {
        v.type = "int";
        v.intValue = stoi(s);
        return v;
    }

    // Variable lookup
    return memory[s];
}


    };
class AITutorCompiler {
public:
    CompilationResult compile(string code) {
        CompilationResult result;

        Lexer lexer(code);
        vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        auto program = parser.parseProgram();

        result.errors = parser.getErrors();

        if (!result.errors.empty()) {
            result.success = false;
            return result;
        }

        IRGenerator irGen;
        irGen.generate(program);

        result.irInstructions = irGen.getInstructions();
        result.success = true;

        return result;
    }
};



    /* =======================
    MAIN (TEST DRIVER)
    ======================= */

    int main() {

    // Read entire input until EOF
    string code;
    getline(cin, code, '\0');   // read full input

    AITutorCompiler compiler;
    CompilationResult result = compiler.compile(code);

    if (!result.success) {
        for (auto &e : result.errors) {
            cout << "Line: " << e.line << endl;
            cout << "Type: " << e.type << endl;
            cout << "Error: " << e.message << endl;
            cout << "Explanation: " << e.explanation << endl;
            cout << "Hint: " << e.hint << endl;
            cout << "----" << endl;
        }
        return 0;
    }

    for (auto &inst : result.irInstructions)
        cout << inst << endl;

    return 0;
}