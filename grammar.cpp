#include "grammar.h"

Grammar::Grammar(std::vector<std::string> rules_list,
                 std::set<std::string> nTerminals,
                 std::set<std::string> trminals,
                 std::string const& strt)
    : start(strt) {

    for (auto const& nt : nTerminals) {
        nonTerminals.insert(index_by_rule(nt));
    }

    for (auto const& t : trminals) {
        terminals.insert(index_by_rule(t));
    }

    for (auto const& rule : rules_list) {
        size_t pos = rule.find("->");
        auto nonTerminal = trim(rule.substr(0, pos));
        auto rhs_string = trim(rule.substr(pos + 2));
        std::vector<std::string> rhs = split(rhs_string, ' ');
        rules[index_by_rule(nonTerminal)].push_back(index_by_rule(rhs));
    }
}

size_t Grammar::index_by_rule(std::string const& entity) {
    return index_by_rule(std::vector{entity});
}

size_t Grammar::index_by_rule(std::vector<std::string> const& entity) {
    if (!rule_id.count(entity)) {
        rule_id[entity] = rule_id.size();
        rule_str[rule_id.size() - 1] = entity;
    }
    return rule_id[entity];
}

auto const& Grammar::data() const {
    return rules;
}

bool Grammar::unite(std::set<std::size_t>& st1,
                    std::set<std::size_t> const& st2) {
    auto len = st1.size();
    std::set<std::size_t> x;
    std::set_union(st1.begin(), st1.end(),
                   st2.begin(), st2.end(),
                   std::inserter(x, x.begin()));
    st1 = x;
    return len != st1.size();
}


std::vector<std::string> Grammar::rule_by_index(std::size_t id) {
    return rule_str[id];
}

std::map<std::size_t, std::set<std::size_t>> Grammar::build_first_set_raw() {

    std::map<std::size_t, std::set<std::size_t>> first;


    ///////////////////////////////// processing epsilon starts /////////////////////////////////

    for (auto const& nt_id : nonTerminals) {
        for (auto const& rul_id : rules[nt_id]) {
            std::vector<std::string> rule = rule_by_index(rul_id);
            if (terminals.count(index_by_rule(rule[0]))) {
                first[nt_id].insert(index_by_rule(rule[0]));
            }
            if (rule.size() == 1 && rule[0] == "eps") {
                first[nt_id].insert(index_by_rule("eps"));
            }
        }
    }

    while (true) {
        bool updated {false};

        for (auto const& rule : rules) {
            std::size_t lhs_id = rule.first;
            std::vector<size_t> const& rhs_id_list = rule.second;
            for (std::size_t rhs_id : rhs_id_list) {
                std::vector<std::string> rhs = rule_by_index(rhs_id);

                auto pos = std::find_if(rhs.begin(), rhs.end(), [this](auto const& entity) {
                    return terminals.count(index_by_rule(entity));
                });
                bool all_has_eps {true};
                std::for_each(rhs.begin(), pos, [&all_has_eps, this, &first, lhs_id](auto const& entity) {
                    all_has_eps &= first[index_by_rule(entity)].count(index_by_rule("eps")) != 0;
                    if (all_has_eps) {
                        std::size_t entity_id = index_by_rule(entity);
                        bool has = first[entity_id].count(index_by_rule("eps")) != 0;
                        if (has) first[entity_id].erase(index_by_rule("eps"));
                        unite(first[lhs_id], first[entity_id]);
                        if (has) first[entity_id].insert(index_by_rule("eps"));
                    }
                });
                if (all_has_eps) {
                    if (pos != rhs.end()) {
                        first[lhs_id].insert(index_by_rule(*pos));
                    } else {
                        first[lhs_id].insert(index_by_rule("eps"));
                    }
                }
            }
        }

        if (!updated) {
            break;
        }
    }

    ///////////////////////////////// processing epsilon end /////////////////////////////////


    for (size_t x : terminals) {
        first[x].insert(x);
    }

    while (true) {
        bool updated {false};
        for (auto const& nt_id : nonTerminals) {
            for (auto const& rul_id : rules[nt_id]) {
                std::vector<std::string> Mrule = rule_by_index(rul_id);

                for (auto it_left = Mrule.begin(); it_left != Mrule.end(); ++it_left) {
                    for (auto it_right = it_left; it_right != Mrule.end(); ++it_right) {
                        std::vector<std::string> rule(it_left, std::next(it_right));
                        if (terminals.count(index_by_rule(rule[0]))) {
                            std::set<std::size_t> st; st.insert(index_by_rule(rule[0]));
                            updated |= unite(first[index_by_rule(rule)], st);
                        } else {
                            if (!first[index_by_rule(rule[0])].count(index_by_rule("eps"))) {
                                updated |= unite(first[index_by_rule(rule)], first[index_by_rule(rule[0])]);
                            } else {
                                std::size_t rule0_id = index_by_rule(rule[0]);
                                bool has = first[rule0_id].count(index_by_rule("eps")) != 0;
                                if (has) first[rule0_id].erase(index_by_rule("eps"));
                                updated |= unite(first[index_by_rule(rule)], first[rule0_id]);
                                updated |= unite(first[index_by_rule(rule)], first[index_by_rule(std::vector<std::string>(std::next(rule.begin()), rule.end()))]);
                                if (has) first[rule0_id].insert(index_by_rule("eps"));
                            }
                        }
                    }
                }
            }
        }

        for (std::size_t nt_id : nonTerminals) {
            for (auto const& rul_id : rules[nt_id]) {
                updated |= unite(first[nt_id], first[rul_id]);
            }
        }

        if (!updated) {
            return first;
        }
    }
}

std::map<std::string, std::set<std::string>> Grammar::transform_index_by_rule(std::map<std::size_t, std::set<std::size_t>> mp) {
    std::map<std::string, std::set<std::string>> res;
    for (std::size_t nt_id : nonTerminals) {
        std::string nt = rule_by_index(nt_id).back();
        for (auto const& fst : mp[nt_id]) {
            res[nt].insert(rule_by_index(fst).back());
        }
    }
    return res;
}

std::set<std::string> Grammar::transform_index_by_rule(std::set<std::size_t> st) {
    std::set<std::string> res;
    for (auto const& t : st) {
        res.insert(rule_by_index(t)[0]);
    }
    return res;
}

std::map<std::size_t, std::set<std::size_t>> Grammar::build_follow_set_raw() {
    std::map<std::size_t, std::set<std::size_t>> follow;
    auto first = build_first_set_raw();
    follow[index_by_rule(start)].insert(index_by_rule("$"));

    for (std::size_t nt_id : nonTerminals) {
        for (auto const& rul_id : rules[nt_id]) {
            std::vector<std::string> rule = rule_by_index(rul_id);
            for (auto it = rule.begin(); std::next(it) != rule.end(); ++it) {
                if (nonTerminals.count(index_by_rule(*it)) &&
                        terminals.count(index_by_rule(*(std::next(it)))) &&
                            *(std::next(it)) != "eps") {
                    follow[index_by_rule(*it)].insert(index_by_rule(*(std::next(it))));
                }
            }
        }
    }

    while (true) {
        bool updated {false};

        for (std::size_t nt_id : nonTerminals) {
            for (auto const& rul_id : rules[nt_id]) {
                std::vector<std::string> rule = rule_by_index(rul_id);
                std::vector<std::string> suffix;
                for (auto it = rule.rbegin(); it != rule.rend(); ++it) {
                    if (nonTerminals.count(index_by_rule(*it))) {
                        bool has = first[index_by_rule(suffix)].count(index_by_rule("eps")) != 0;
                        if (has) first[index_by_rule(suffix)].erase(index_by_rule("eps"));
                        updated |= unite(follow[index_by_rule(*it)], first[index_by_rule(suffix)]);
                        if (has) first[index_by_rule(suffix)].insert(index_by_rule("eps"));
                        if (has || suffix.empty()) {
                            updated |= unite(follow[index_by_rule(*it)], follow[nt_id]);
                        }
                    }
                    suffix.insert(suffix.begin(), *it);
                }
            }
        }

        if (!updated) {
            return follow;
        }
    }
}

std::map<std::string, std::set<std::string>> Grammar::build_follow_set() {
    if (follow_set.empty()) {
        follow_set = build_follow_set_raw();
    }
    return transform_index_by_rule(follow_set);
}

std::map<std::string, std::set<std::string>> Grammar::build_first_set() {
    if (first_set.empty()) {
        first_set = build_first_set_raw();
    }
    return transform_index_by_rule(first_set);
}

bool Grammar::is_nonTerminal(std::string const& str) {
    return nonTerminals.count(index_by_rule(str));
}

bool Grammar::is_terminal(std::string const& str) {
    return terminals.count(index_by_rule(str));
}

std::string Grammar::get_start() {
    return start;
}

std::vector<std::string> Grammar::get_terminals() {
    auto temp = transform_index_by_rule(terminals);
    return std::vector<std::string>(temp.begin(), temp.end());
}

std::vector<std::string> Grammar::get_nonTerminals() {
    auto temp = transform_index_by_rule(nonTerminals);
    return std::vector<std::string>(temp.begin(), temp.end());
}


std::vector<std::vector<std::string>> Grammar::get_rules_for(std::string nt) {
    std::vector<std::vector<std::string>> res;
    for (auto const& v : rules[index_by_rule(nt)]) {
        res.push_back(rule_by_index(v));
    }
    return res;
}

std::set<std::string> Grammar::get_follow(std::string const& nt) {
    if (follow_set.empty()) {
        follow_set = build_follow_set_raw();
    }
    return transform_index_by_rule(follow_set[index_by_rule(nt)]);
}

std::set<std::string> Grammar::get_first(std::vector<std::string> const& s) {
    if (first_set.empty()) {
        first_set = build_follow_set_raw();
    }
    if (!first_set.count(index_by_rule(s))) {
        if (is_terminal(s[0])) {
            first_set[index_by_rule(s)].insert(index_by_rule(s[0]));
        } else {
            if (first_set[index_by_rule(s[0])].count(index_by_rule("eps"))) {
                bool has = first_set[index_by_rule(s)].count(index_by_rule("eps")) != 0;
                if (has) first_set[index_by_rule(s)].erase(index_by_rule("eps"));
                unite(first_set[index_by_rule(s)], first_set[index_by_rule(s[0])]);
                unite(first_set[index_by_rule(s)], get_first_raw(std::vector<std::string>(std::next(s.begin()), s.end())));
                if (has) first_set[index_by_rule(s)].insert(index_by_rule("eps"));
            } else {
                first_set[index_by_rule(s)] = first_set[index_by_rule(s[0])];
            }
        }
    }
    return transform_index_by_rule(first_set[index_by_rule(s)]);
}

std::set<std::size_t> Grammar::get_first_raw(std::vector<std::string> const& s) {
    if (!first_set.count(index_by_rule(s))) {
        if (is_terminal(s[0])) {
            first_set[index_by_rule(s)].insert(index_by_rule(s[0]));
        } else {
            if (first_set[index_by_rule(s[0])].count(index_by_rule("eps"))) {
                bool has = first_set[index_by_rule(s)].count(index_by_rule("eps")) != 0;
                if (has) first_set[index_by_rule(s)].erase(index_by_rule("eps"));
                unite(first_set[index_by_rule(s)], first_set[index_by_rule(s[0])]);
                unite(first_set[index_by_rule(s)], get_first_raw(std::vector<std::string>(std::next(s.begin()), s.end())));
                if (has) first_set[index_by_rule(s)].insert(index_by_rule("eps"));
            } else {
                first_set[index_by_rule(s)] = first_set[index_by_rule(s[0])];
            }
        }
    }
    return first_set[index_by_rule(s[0])];
}

std::map<std::string, std::map<std::string, std::vector<std::string>>> Grammar::build_ll1_table() {
    std::map<std::string, std::map<std::string, std::vector<std::string>>> res;
    auto fst = build_first_set_raw();
    build_follow_set_raw();

    for (auto const& rule : rules) {
        auto nt = rule_by_index(rule.first)[0];
        for (auto const& rhs_id : rule.second) {

            std::vector<std::string> rhs = rule_by_index(rhs_id);
            auto fst_suf = get_first(rhs);
            for (auto const& t : fst_suf) {
                if (t == "eps") {
                    for (std::size_t tt_id : follow_set[index_by_rule(nt)]) {
						if (!res[nt][rule_by_index(tt_id)[0]].empty() && res[nt][rule_by_index(tt_id)[0]] != rhs)
							throw std::runtime_error("NOT LL(1) Grammar: Conflict in {" + nt + ", " + rule_by_index(tt_id)[0] + "} move");
						res[nt][rule_by_index(tt_id)[0]] = rhs;
                    }
				}
				if (!res[nt][t].empty() && res[nt][t] != rhs)
					throw  std::runtime_error("NOT LL(1) Grammar: Conflict in {" + nt + ", " + t + "} move");
                res[nt][t] = rhs;
            }

            std::vector<std::string> w_follow(rhs);
            for (size_t flw : follow_set[index_by_rule(nt)]) {
                if (flw == index_by_rule("esp")) {
                    continue;
                }
                w_follow.push_back(rule_by_index(flw)[0]);
                for (auto const& terminal : get_first(w_follow)) {
                    if (res[nt].count(terminal)) {
                        if (res[nt][terminal] != rhs) {
							throw std::runtime_error("NOT LL(1) Grammar: Conflict in {" + nt + ", " + terminal + "} move");
                        }
                    }
					if (!res[nt][terminal].empty() && res[nt][terminal] != rhs)
						throw  std::runtime_error("NOT LL(1) Grammar: Conflict in {" + nt + ", " + terminal + "} move");
                    res[nt][terminal] = rhs;
                }
                w_follow.pop_back();
            }
        }
    }

	return res;
}

std::map<std::string, std::map<std::string, std::vector<std::string>>> Grammar::build_slr_table() {
	using namespace std;
	prepare_parse_table();
	std::map<std::string, std::map<std::string, std::vector<std::string>>> result;
	for (auto const& entitiy : parse_table) {
		if (entitiy.second.first)
			result[std::to_string(entitiy.first.first)][entitiy.first.second] = {(entitiy.second.first == 1 ? " reduce"s + " : " + entitiy.second.second + " "
																					: entitiy.second.first == 2 ? " shift"s  + " : " + entitiy.second.second + " "
																						: " accepted "s)};
	}
	return result;
}

std::map<std::string, std::set<std::string>> Grammar::get_closure(
		std::map<std::string, std::set<std::string>> mp) {

   std::map<std::string, std::set<std::string>> mp2 = mp;

   while (true) {
	   for (auto const& entity : mp) {
		   std::set<std::string> rhs_rules = entity.second;
		   for (auto const& rhs_rule_s : rhs_rules) {
			   std::vector<std::string> rhs = split(rhs_rule_s, ' ');
			   auto pos = std::find(rhs.begin(), rhs.end(), ".");
			   if (pos == rhs.end()) {
				   throw std::runtime_error("expected . in the production");
			   }
			   if (pos == std::prev(rhs.end())) {
				   continue;
			   }
			   if (!is_nonTerminal(*std::next(pos))) {
				   continue;
			   }
			   for (auto const& value : get_rules_for(*std::next(pos))) {
				   if (value.size() == 1 && value[0] == "eps") {
					   mp2[*std::next(pos)].insert(".");
				   } else {
					   mp2[*std::next(pos)].insert(". " + join(value, ' '));
				   }
			   }
		   }
	   }
	   if (mp == mp2) {
		   return mp;
	   }
	   mp = mp2;
   }
}

std::map<std::string, std::set<std::string>> Grammar::get_goto(std::map<std::string, std::set<std::string>> const& block, std::string const& symbol) {

	if (goto_table_cache.count({block, symbol})) {
		return goto_table_cache[{block, symbol}];
	}

	std::map<std::string, std::set<std::string>>& result = goto_table_cache[{block, symbol}];

	for (auto const& entity : block) {
		for (auto const& rhs_rule_s : entity.second) {
			auto rhs_rule = split(rhs_rule_s, ' ');
			auto dot_pos = std::find(rhs_rule.begin(), rhs_rule.end(), ".");
			if (dot_pos == rhs_rule.end()) {
				throw std::runtime_error("expected . in the production");
			}
			if (std::next(dot_pos) != rhs_rule.end() &&
					*std::next(dot_pos) == symbol) {
				std::iter_swap(dot_pos, std::next(dot_pos));

				auto block_closure = get_closure({{entity.first, {join(rhs_rule, ' ')}}});
				for (auto const& cl_entity : block_closure) {
					for (auto const& rhs_rule_s2 : cl_entity.second) {
						result[cl_entity.first].insert(rhs_rule_s2);
					}
				}
			}
		}
	}

	return result;
}

std::map<std::size_t, std::map<std::string, std::set<std::string> > > Grammar::build_goto_table() {
	std::size_t curidx {};
	goto_table[curidx++] = get_closure({{start, {". " + join(get_rules_for(start)[0], ' ')}}});
	goto_table_inv[goto_table[0]] = 0;
	while (true) {
		bool updated {false};
		for (auto const& entity : goto_table) {
			auto const& block = entity.second;
			for (auto const& term : get_terminals()) {
			   auto nxt = get_goto(block, term);
			   if (!nxt.empty() && !goto_table_inv.count(nxt)) {
					goto_table[curidx] = nxt;
					goto_table_inv[nxt] = curidx++;
					updated = true;
			   }
			}
			for (auto const& nterm : get_nonTerminals()) {
			   auto nxt = get_goto(block, nterm);
			   if (!nxt.empty() && !goto_table_inv.count(nxt)) {
					goto_table[curidx] = nxt;
					goto_table_inv[nxt] = curidx++;
					updated = true;
			   }
			}
		}
		if (!updated) {
			return goto_table;
		}
	}
}

void Grammar::prepare_parse_table() {
	auto aut = build_goto_table();
	for (auto const& entity : aut) {
		for (std::string const& value : get_terminals()) {
			get_action(entity.first, value);
		}
		for (std::string const& value : get_nonTerminals()) {
			get_action(entity.first, value);
		}
		get_action(entity.first, "$");
	}
}

/*
 * actions:
 * reduce - 1
 * shift  - 2
 * accept - 3
 */

std::pair<std::size_t, std::string> Grammar::get_action(std::size_t id, std::string symbol) {

//	if (parse_table.count({id, symbol})) {
//		return  parse_table[{id, symbol}];
//	}

	for (auto const& entity : goto_table[id]) {
		for (auto const& rhs_rule_s : entity.second) {
			auto rhs = split(rhs_rule_s, ' ');
			auto dot_pos = std::find(rhs.begin(), rhs.end(), ".");
			if (dot_pos == rhs.end()) {
				continue;
			}
			if (std::next(dot_pos) != rhs.end() &&
					*std::next(dot_pos) == symbol) {
				for (auto const& block : goto_table) {
					if (get_goto(goto_table[id], symbol) == block.second) {
						auto& action = parse_table[{id, symbol}];
						if (is_terminal(symbol)) {
							if (action.first == 1) {
								throw std::runtime_error("NOT SLR Grammar: Shift-Reduce Conflict {" + std::to_string(id) + ", " + symbol + "}");
							}
							action = {2, std::to_string(goto_table_inv[block.second])};
						} else if (is_nonTerminal(symbol)) {
							action = {2, std::to_string(goto_table_inv[block.second])};
						} else {
							throw std::runtime_error(symbol + " is not either terminal or nonTerminal");
						}
					}
				}
			}

		}
	}

	for (auto const& entity : goto_table[id]) {
		if (entity.first == start) {
			continue;
		}
		for (auto const& rhs_rule_s : entity.second) {
			auto dotted_rhs = split(rhs_rule_s, ' ');
			if (dotted_rhs.back() != ".") {
				continue;
			}
			for (auto const& nonTerminal : get_nonTerminals()) {
				for (auto const& rhs : get_rules_for(nonTerminal)) {
					if (entity.first == nonTerminal &&
							(std::equal(rhs.begin(), rhs.end(), dotted_rhs.begin()) ||
							 (rhs.size() == 1 && rhs[0] == "eps" && dotted_rhs.size() == 1 && dotted_rhs[0] == ".")) &&
							(symbol == "$" || is_terminal(symbol))) {
						std::string rule = nonTerminal + " -> " + join(rhs, ' ');
						for (auto const& term : get_follow(entity.first)) {
							auto& action = parse_table[{id, term}];
							if (action.first == 2) {
								throw std::runtime_error("NOT SLR Grammar: Shift-Reduce Conflict {" + std::to_string(id) + ", " + term + "}");
							}
							if (action.first != 0 &&
									(action.first != 1 || action.second != rule)) {
								throw std::runtime_error("NOT SLR Grammar: Reduce-Reduce Conflict in {" + std::to_string(id) + ", " + term + "}");
							}
							action = {1, rule};
						}
					}
				}
			}
		}
	}

	if (goto_table[id].count(start) && goto_table[id][start].count(join(get_rules_for(start)[0], ' ') + " .")) {
		parse_table[{id, "$"}] = {3, ""};
	}

	return parse_table[{id, symbol}];
}

