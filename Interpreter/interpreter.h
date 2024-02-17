#pragma once

#include <vector>
#include <string>

std::vector<std::string> Split(std::string str, char delimiter);
std::vector<std::string> Lexer(std::string content);
std::vector<std::vector<std::string>> Parse(const char* filename);
