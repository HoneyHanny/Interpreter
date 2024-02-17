#include <fstream>
#include <sstream>
#include <iostream>

#include "interpreter.h"

std::vector<std::string> Split(std::string str, char delimiter = '\n')
{
    std::vector<std::string> tokens;

    std::istringstream iss(str);

    std::string token;
    while (std::getline(iss, token, delimiter))
    {
        tokens.push_back(token);
    }

    return tokens;
}

std::vector<std::string> Lexer(std::string content)
{
    std::vector<std::string> tokens = Split(content);
    return tokens;
}

std::vector<std::vector<std::string>> Parse(const char* filename)
{
    // Start READ FILE
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file." << "\n";
        return std::vector<std::vector<std::string>>();
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }
    file.close();
    // End READ FILE

    // Start LEXER
    std::vector<std::vector<std::string>> tokenized_lines;
    std::vector<std::string> tokens;
    for (const std::string& li : lines)
    {
        const char* chars = li.c_str();
        std::string tempStr = "";
        int quoteCount = 2;
        bool inQuotes;
        tokens.clear();
        for (int i = 0; chars[i] != '\0'; i++)
        {
            if (chars[i] == '"' || chars[i] == '\'')
                quoteCount++;
            if (quoteCount % 2 == 0)
                inQuotes = false;
            else
                inQuotes = true;

            if (chars[i] == ' ' && !inQuotes)
            {
                tokens.push_back(tempStr);
                tempStr = "";
            }
            else
            {
                tempStr += chars[i];
            }
        }
        tokens.push_back(tempStr);
        tokenized_lines.push_back(tokens);
    }
    // End LEXER

    // TODO(hans): add types for tokens
    return tokenized_lines;
}

// Token types
// String - " "
// Char - ' '
// Symbol - print function var
// Expression +-/*
// Number - 0-9
