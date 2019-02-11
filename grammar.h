#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <string>
#include <map>
#include <vector>
#include <cstdlib>
#include <set>
#include <algorithm>
#include <sstream>
#include "utility.h"
#include <iostream>
#include <cassert>

struct Grammar {

    Grammar() = default;

    Grammar(std::vector<std::string> rules_list,
            std::set<std::string> nonTerminals,
            std::set<std::string> terminals,
            std::string const& start);

	Grammar(Grammar const&) = default;

    auto const& data() const;

    Grammar& operator = (Grammar const&) = default;
    Grammar& operator = (Grammar&& ) = default;

    std::map<std::string, std::set<std::string>> build_follow_set();
    std::map<std::string, std::set<std::string>> build_first_set();
    std::map<std::string, std::map<std::string, std::vector<std::string>>> build_ll1_table();
	std::map<std::string, std::map<std::string, std::vector<std::string>>> build_slr_table();

    bool is_terminal(std::string const&);
    bool is_nonTerminal(std::string const&);

    std::string get_start();
    std::vector<std::string> get_terminals();
    std::vector<std::string> get_nonTerminals();
    std::vector<std::vector<std::string>> get_rules_for(std::string nt);

    std::set<std::string> get_first(std::vector<std::string> const& s);
    std::set<std::string> get_follow(std::string const& nt);

	// get set of rules and produces block for automata for slr tables
	std::map<std::string, std::set<std::string>> get_closure(
			std::map<std::string, std::set<std::string>> mp);


	std::map<std::string, std::set<std::string>> get_goto(
			std::map<std::string, std::set<std::string>> const& block,
			std::string const& symbol);

	std::map<std::size_t, std::map<std::string, std::set<std::string>>> build_goto_table();

	void prepare_parse_table();
	std::pair<std::size_t, std::string> get_action(std::size_t id, std::string symbol);




private:

    std::set<std::size_t> get_first_raw(std::vector<std::string> const& s);
    std::map<std::size_t, std::set<std::size_t>>  build_follow_set_raw();
    std::map<std::size_t, std::set<std::size_t>>  build_first_set_raw();

    std::set<std::string> transform_index_by_rule(std::set<std::size_t> st);
    std::map<std::string, std::set<std::string>> transform_index_by_rule(std::map<std::size_t, std::set<std::size_t>>);

	size_t index_by_rule(std::vector<std::string> const&);
    size_t index_by_rule(std::string const& entity);

    std::vector<std::string> rule_by_index(std::size_t id);

    bool unite(std::set<std::size_t>& st1,
               std::set<std::size_t> const& st2);

    std::map<std::vector<std::string>, std::size_t> rule_id;
    std::map<std::size_t, std::vector<std::string>> rule_str;
    std::map<std::size_t, std::vector<size_t>> rules;
    std::set<std::size_t> nonTerminals;
    std::set<std::size_t> terminals;
    std::string start;

    std::map<std::size_t, std::set<std::size_t>> first_set;
    std::map<std::size_t, std::set<std::size_t>> follow_set;

	std::map<std::pair<
				std::map<std::string, std::set<std::string>>,
				std::string
				>,std::map<std::string, std::set<std::string>>> goto_table_cache;

	std::map<std::size_t, std::map<std::string, std::set<std::string>>> goto_table;
	std::map<std::map<std::string, std::set<std::string>>, std::size_t> goto_table_inv;
	std::map<std::pair<std::size_t, std::string>, std::pair<std::size_t, std::string>> parse_table;
};

#endif // GRAMMAR_H
