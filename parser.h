#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "lexer.h"
#include "grammar.h"
#include "utility"
#include <iostream>
#include <iomanip>
#include <stack>


struct parser_exception : public std::exception {
public:
    explicit parser_exception(const char* message);
    explicit parser_exception(const std::string& message);
    ~parser_exception() override;
    const char* what() const noexcept override;
protected:
    std::string msg_;
};

struct Tree {

    template<typename... Trees,
             typename = std::enable_if_t<(!std::is_lvalue_reference_v<Trees> && ...)>>
    Tree(std::string vl, Trees&&... cdren)
        : root (std::make_unique<node>(vl)) {
        (root->push_node(std::move(cdren.root)),...);
    }

	Tree(Tree&& other);

    Tree(std::string value);

    template<typename... Trees,
             typename = std::enable_if_t<(!std::is_lvalue_reference_v<Trees> && ...)>>
    void add_children(Trees&&... child) {
        (root->push_node(std::move(child.root)),...);
    }

    std::vector<std::string> data();

private:

    struct node {

        node(std::string const& value);

        void push_node(std::unique_ptr<node>&& pn);
        void emplace_node(std::string value, std::vector<std::unique_ptr<node>>&& children);

        void add_child(std::unique_ptr<node>&& pn);

        std::string value;
        std::vector<std::unique_ptr<node>> children;

    private:
    };


    void dfs(std::unique_ptr<node> const& cur, std::vector<std::string>& res);

    std::unique_ptr<node> root {};
    std::vector<std::string> ws;
};

struct Parser {

	Parser(Grammar const& g);
	Tree parseLL1(std::string const& str);
	Tree parseSLR(std::string const& str);

private:

    std::map<std::string, std::function<Tree(void)>> parse_;
	Grammar grammar;

	/// building actiong and goto tables ///

};

#endif // PARSER_H
