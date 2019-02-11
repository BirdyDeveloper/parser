#ifndef UTILITY_H
#define UTILITY_H

#include "lexer.h"
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <ostream>
#include <unistd.h>
#include <map>

std::string build_json_header(std::string const& header);
std::string build_json_tree(std::vector<std::string> const& tree);
std::string build_json_map(std::map<std::string, std::set<std::string>> const& mp, std::string where);
std::string build_json_map(std::map<std::string, std::map<std::string, std::vector<std::string>>> const& mp, std::string where);
std::ostream& operator << (std::ostream& out, Token token);

std::string trim(std::string str);

std::pair<std::string, std::string> get_browser_arguments();

std::vector<std::string> split(std::string, char);
std::string join(std::vector<std::string>, char);

#endif // UTILITY_H
