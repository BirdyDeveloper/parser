#include "lexer.h"

Lexer::Lexer(std::string const& str) : iss(str + "$") {
    tokens_trie.add_string("and", Token::And);
    tokens_trie.add_string("or", Token::Or);
    tokens_trie.add_string("xor", Token::Xor);
    tokens_trie.add_string("not", Token::Not);
    tokens_trie.add_string("(", Token::LB);
    tokens_trie.add_string(")", Token::RB);
    tokens_trie.add_string("$", Token::End);
    tokens_trie.add_string("a", Token::Var);
    tokens_trie.add_string("b", Token::Var);
    tokens_trie.add_string("c", Token::Var);
}

Lexer::Lexer(std::string const& str, std::vector<std::string> const& v) : Lexer(str) {
    for (auto const& s : v) {
        tokens_trie.add_string(s, Token::Smth);
    }
}

Token Lexer::next_token() {
    char c;
    std::string cur_string_token;
    while (isspace(iss.peek())) {
        iss.get();
    }
    tokens_trie.reset_state();
    while (!iss.eof() && (c = static_cast<char>(iss.get()))) {
        cur_string_token += c;
        if (tokens_trie.move_state(c)) {
            if (tokens_trie.has_key()
                    && !tokens_trie.has_next(static_cast<char>(iss.peek()))) {
                _current_string_token = cur_string_token;
                return _current_token = tokens_trie.get_key();
            }
        } else {
            _current_string_token = cur_string_token;
            return _current_token = Token::None;
        }
    }
    _current_string_token = cur_string_token;
    return _current_token = Token::None;
}

Token Lexer::current_token() const {
    return _current_token;
}

std::string const& Lexer::current_string_token() const {
    return _current_string_token;
}

Lexer::trie::trie() {
    t.emplace_back();
}

void Lexer::trie::add_string(std::string const& s, Token key) {
    unsigned long cur = 0;
    for (char c : s) {
        if (!t[cur].to.count(c)) {
            t[cur].to[c] = t.size();
            t.emplace_back();
        }
        cur = t[cur].to[c];
    }
    t[cur].key = key;
}

bool Lexer::trie::move_state(char c) {
    if (t[state].to.count(c)) {
        state = t[state].to[c];
        return true;
    } else {
        state = 0;
        return false;
    }
}

void Lexer::trie::reset_state() {
    state = 0;
}

bool Lexer::trie::has_key() const {
    return t[state].key != Token::None;
}

bool Lexer::trie::has_next(char c) const {
    return t[state].to.count(c);
}

Token Lexer::trie::get_key() const {
    return t[state].key;
}

Token Lexer::token_by_string(std::string const& s) {
    for (char c : s) {
        tokens_trie.move_state(c);
    }
    return tokens_trie.get_key();
}
