// A simple compiler in C++ for a toy language that supports expressions like: x = 2 + 3 * (4 - 1);
// This example includes lexing, parsing (recursive descent), and evaluation.

#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <map>
#include <memory>

// ==== TOKENIZER ====
enum class TokenType {
    IDENTIFIER, NUMBER,
    PLUS, MINUS, MUL, DIV,
    ASSIGN, LPAREN, RPAREN,
    SEMICOLON, END
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
    std::string input;
    size_t pos = 0;

public:
    explicit Lexer(const std::string &src) : input(src) {}

    Token getNextToken() {
        while (pos < input.length() && isspace(input[pos])) pos++;
        if (pos >= input.length()) return {TokenType::END, ""};

        char current = input[pos];
        if (isalpha(current)) {
            std::string id;
            while (isalnum(input[pos])) id += input[pos++];
            return {TokenType::IDENTIFIER, id};
        } else if (isdigit(current)) {
            std::string num;
            while (isdigit(input[pos])) num += input[pos++];
            return {TokenType::NUMBER, num};
        } else {
            pos++;
            switch (current) {
                case '+': return {TokenType::PLUS, "+"};
                case '-': return {TokenType::MINUS, "-"};
                case '*': return {TokenType::MUL, "*"};
                case '/': return {TokenType::DIV, "/"};
                case '=': return {TokenType::ASSIGN, "="};
                case '(': return {TokenType::LPAREN, "("};
                case ')': return {TokenType::RPAREN, ")"};
                case ';': return {TokenType::SEMICOLON, ";"};
                default:
                    std::cerr << "Unknown token: " << current << std::endl;
                    exit(1);
            }
        }
    }
};

// ==== PARSER ====
class ASTNode {
public:
    virtual int eval(std::map<std::string, int> &vars) = 0;
    virtual ~ASTNode() = default;
};

class NumberNode : public ASTNode {
    int value;
public:
    explicit NumberNode(int val) : value(val) {}
    int eval(std::map<std::string, int> &) override { return value; }
};

class VariableNode : public ASTNode {
    std::string name;
public:
    explicit VariableNode(std::string n) : name(std::move(n)) {}
    int eval(std::map<std::string, int> &vars) override { return vars[name]; }
};

class BinaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> left, right;
public:
    BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
    int eval(std::map<std::string, int> &vars) override {
        int lval = left->eval(vars);
        int rval = right->eval(vars);
        switch (op) {
            case '+': return lval + rval;
            case '-': return lval - rval;
            case '*': return lval * rval;
            case '/': return lval / rval;
            default: exit(1);
        }
    }
};

class AssignmentNode : public ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> expr;
public:
    AssignmentNode(std::string n, std::unique_ptr<ASTNode> e)
        : name(std::move(n)), expr(std::move(e)) {}
    int eval(std::map<std::string, int> &vars) override {
        int val = expr->eval(vars);
        vars[name] = val;
        return val;
    }
};

class Parser {
    Lexer lexer;
    Token currentToken;

    void eat(TokenType type) {
        if (currentToken.type == type) {
            currentToken = lexer.getNextToken();
        } else {
            std::cerr << "Unexpected token: " << currentToken.value << std::endl;
            exit(1);
        }
    }

    std::unique_ptr<ASTNode> factor() {
        if (currentToken.type == TokenType::NUMBER) {
            int val = std::stoi(currentToken.value);
            eat(TokenType::NUMBER);
            return std::make_unique<NumberNode>(val);
        } else if (currentToken.type == TokenType::IDENTIFIER) {
            std::string name = currentToken.value;
            eat(TokenType::IDENTIFIER);
            return std::make_unique<VariableNode>(name);
        } else if (currentToken.type == TokenType::LPAREN) {
            eat(TokenType::LPAREN);
            auto node = expr();
            eat(TokenType::RPAREN);
            return node;
        } else {
            std::cerr << "Unexpected factor: " << currentToken.value << std::endl;
            exit(1);
        }
    }

    std::unique_ptr<ASTNode> term() {
        auto node = factor();
        while (currentToken.type == TokenType::MUL || currentToken.type == TokenType::DIV) {
            char op = currentToken.value[0];
            eat(currentToken.type);
            node = std::make_unique<BinaryOpNode>(op, std::move(node), factor());
        }
        return node;
    }

    std::unique_ptr<ASTNode> expr() {
        auto node = term();
        while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS) {
            char op = currentToken.value[0];
            eat(currentToken.type);
            node = std::make_unique<BinaryOpNode>(op, std::move(node), term());
        }
        return node;
    }

    std::unique_ptr<ASTNode> assignment() {
        if (currentToken.type == TokenType::IDENTIFIER) {
            std::string name = currentToken.value;
            eat(TokenType::IDENTIFIER);
            eat(TokenType::ASSIGN);
            auto rhs = expr();
            eat(TokenType::SEMICOLON);
            return std::make_unique<AssignmentNode>(name, std::move(rhs));
        } else {
            std::cerr << "Expected identifier in assignment" << std::endl;
            exit(1);
        }
    }

public:
    explicit Parser(const std::string &src) : lexer(src) {
        currentToken = lexer.getNextToken();
    }

    std::unique_ptr<ASTNode> parse() {
        return assignment();
    }
};

// ==== MAIN ====
int main() {
    std::map<std::string, int> vars;
    std::string line;

    std::cout << "Enter a statement (e.g., x = 2 + 3 * (4 - 1);):\n> ";
    std::getline(std::cin, line);

    Parser parser(line);
    auto tree = parser.parse();
    int result = tree->eval(vars);

    std::cout << "Result: " << result << "\n";
    for (const auto &kv : vars) {
        std::cout << kv.first << " = " << kv.second << "\n";
    }
    return 0;
}