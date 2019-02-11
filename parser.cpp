#include "parser.h"

parser_exception::parser_exception(const char* message) : msg_(message) {}

parser_exception::parser_exception(const std::string& message) : msg_(message) {}

parser_exception::~parser_exception() {}

const char* parser_exception::what() const noexcept{
   return msg_.c_str();
}

Tree::Tree(Tree&& other) {
    root = (std::move(other.root));
}

Tree::Tree(std::string value) : root (std::make_unique<node>(std::move(value))) {}

Tree::node::node(std::string const& vl) : value(vl) {}

void Tree::node::push_node(std::unique_ptr<node>&& pn) {
	children.emplace_back(std::move(pn));
}

void Tree::node::emplace_node(std::string vl, std::vector<std::unique_ptr<node>>&& ch) {
    children.emplace_back(std::make_unique<node>(vl));
    std::move(ch.begin(), ch.end(), children.begin());
}

void Tree::node::add_child(std::unique_ptr<node>&& pn) {
    children.emplace_back(std::move(pn));
}

std::vector<std::string> Tree::data() {
    std::vector<std::string> res;
    dfs(root, res);
    return res;
}

void Tree::dfs(std::unique_ptr<node> const& cur, std::vector<std::string>& res) {
    static std::string w = "";
    if (!cur) return ;
    res.push_back(w + cur->value);
    for (auto const& to : cur->children) {
        w += "  ";
        dfs(to, res);
        w.pop_back();
        w.pop_back();
    }
}

Parser::Parser(Grammar const& g) : grammar(g) {}

Tree Parser::parseLL1(std::string const& str) {
	Lexer lexer{str, grammar.get_terminals()};
    grammar.build_first_set();
	grammar.build_ll1_table();
	for (auto const& nt : grammar.get_nonTerminals()) {
		parse_[nt] = [this, &lexer, nt]() mutable {
            bool has_eps {false};
            Tree res(nt);
            for (auto const& tl : grammar.get_rules_for(nt)) {
                has_eps |= grammar.get_first(tl).count("eps");
                if (grammar.get_first(tl).count(lexer.current_string_token())) {
                    for (auto const& v : tl) {
                        if (grammar.is_nonTerminal(v)) {
                            res.add_children(parse_[v]());
                        } else if (grammar.is_terminal(v)) {
                            if (lexer.current_string_token() != v) {
                                throw parser_exception("Expected \')\', but found:" + lexer.current_string_token());
                            }
                            lexer.next_token();
                            res.add_children(Tree(v));
                        } else {
                            throw parser_exception(lexer.current_string_token() + " is not a token");
                        }
                    }
                    return res;
                }
            }
            if (has_eps) {
                if (grammar.get_follow(nt).count(lexer.current_string_token())) {
                    res.add_children(Tree("eps"));
                } else {
                    throw parser_exception("Unexpected symbol: " + lexer.current_string_token());
                }
                return  res;
            } else {
                throw parser_exception("Unexpected symbol: " + lexer.current_string_token());
            }
        };
    }
    lexer.next_token();
    return parse_[grammar.get_start()]();
}

Tree Parser::parseSLR(std::string const& str) {
	grammar.prepare_parse_table();
	Lexer lexer{str, grammar.get_terminals()};
	try {
		std::stack<std::string> stack;
		std::stack<Tree> tree_stack;
		stack.push("0");
		tree_stack.push(Tree(grammar.get_start()));
		lexer.next_token();
		while (true) {
			std::size_t current_symbol;
			std::stringstream(stack.top()) >> current_symbol;
			auto action = grammar.get_action(current_symbol, lexer.current_string_token());
			switch (action.first) {
				case 1 : {
							size_t pos = action.second.find("->");
							auto nonTerminal = trim(action.second.substr(0, pos));
							auto rhs_string = trim(action.second.substr(pos + 2));
							std::vector<std::string> rhs = split(rhs_string, ' ');
							Tree tree = Tree(nonTerminal);
							if (!(rhs.size() == 1 && rhs[0] == "eps")) {
							   for (std::size_t i = 0; i < rhs.size() * 2; ++i) {
								   stack.pop();
							   }
							   std::vector<Tree> temp;
							   for (std::size_t i = 0; i < rhs.size(); ++i) {
									temp.emplace_back(std::move(tree_stack.top()));
									tree_stack.pop();
							   }
							   for (std::size_t i = rhs.size(); i > 0; --i) {
								   tree.add_children(std::move(temp[i - 1]));
							   }
							}
							std::size_t state;
							std::stringstream(stack.top()) >> state;
							stack.push(nonTerminal);
							stack.push(grammar.get_action(state, nonTerminal).second);
							tree_stack.push(std::move(tree));
							break;
						 }
				case 2 : {
							stack.push(lexer.current_string_token());
							stack.push(action.second);
							tree_stack.push(Tree(lexer.current_string_token()));
							lexer.next_token();
							break;
						 }
				case 3 : goto done;
				default: throw parser_exception("invalid input :" + lexer.current_string_token());
			}
		}
		done:
		return std::move(tree_stack.top());
	} catch (std::runtime_error& e) {
		throw parser_exception(e.what());
	}
}


