#include "utility.h"
#include <iostream>

std::string build_json_tree(std::vector<std::string> const& tree) {
//    using std::string_literals::operator""s;
    using namespace std; // to avoid annoying ""s warning
    std::string res;
    std::map<size_t, std::string> lst;
    int node_id {};
    std::string indent("            ");
    res += "var config = {\n"s;
    res += indent + "container: \""s + "#my_tree"s + "\",\n"s;
    res += indent + "connectors: { type: 'step'},\n"s;
    res += indent + "node: { HTMLclass: 'tree-style'},\n"s;
    res += "        },\n";

    for (auto const& branch : tree) {
        size_t w = branch.find_first_not_of(" ");
        std::string current_node = "node" + std::to_string(node_id);
        node_id++;
        res += "        "s + current_node + " = {\n"s;
        if (lst.count(w - 2)) {
            res += indent + "parent: " + lst[w - 2] + ",\n";
        }
        res += indent + "text: {name: \""s + branch.substr(w) + "\"}\n        },\n"s;
        lst[w] = current_node;
    }

    res += "        chart_config = [\n";
    res += indent + "config,\n";
    for (int i = 0; i < node_id; ++i) {
        res += indent + "node"s + std::to_string(i) + ",\n";
    }
    res += "        ];\n";
    return res;
}

std::string build_json_map(std::map<std::string, std::set<std::string>> const& mp, std::string where) {
//    using std::string_literals::operator""s;
    using namespace std; // to avoid annoying ""s warning
    std::string res;
    res += "function generate_" + where + "_table() {\n"s;
    res += "    var div = document.getElementById('" + where + "_place');\n"s;
    res += "    var tbl = document.createElement('" + where + "_table');\n"s;
    res += "    var tblBody = document.createElement('tbody');\n"s;
    res += "    var row = document.createElement('tr');\n"s;
    res += "    var cell_caption = document.createElement('td');\n"s;
    res += "    var cell_caption_text = document.createTextNode('"s + where + "_set');\n"s;
    res += "    cell_caption.appendChild(cell_caption_text);\n"s;
    res += "    row.appendChild(cell_caption);\n"s;
    res += "        tblBody.appendChild(row);\n"s;
    res += "    var nonTerms = [\n";
    for (auto const& assoc : mp) {
        res += "            \""s +assoc.first + "\",\n"s;
    }
    res += "    ];\n"s;
    res += "    var term_sets = [\n";
    for (auto const& assoc : mp) {

        std::string term_set = "'{";
        bool comma = {false};
        for_each(assoc.second.begin(), assoc.second.end(), [&comma, &term_set](auto const& term) {
            if (comma) {
                term_set += ", ";
            }
            comma = true;
            term_set += term;
        });
        term_set += "}'";

        res += "            "s +term_set + ",\n"s;
    }
    res += "    ];\n";

    res += "    for (var i = 0; i < "s + to_string(mp.size()) + "; i++){\n"s;
    res += "        var row = document.createElement('tr');\n"s;
    res += "        var cell_nonTerm = document.createElement('td');\n"s;
    res += "        var cell_term_set = document.createElement('td');\n"s;
    res += "        var cell_nonTerm_text = document.createTextNode(nonTerms[i] + \": \");\n"s;
    res += "        var cell_term_set_text = document.createTextNode(term_sets[i]);\n"s;
    res += "        cell_nonTerm.appendChild(cell_nonTerm_text);\n"s;
    res += "        row.appendChild(cell_nonTerm);\n"s;
    res += "        cell_term_set.appendChild(cell_term_set_text);\n"s;
    res += "        row.appendChild(cell_term_set);\n"s;
    res += "        tblBody.appendChild(row);\n"s;
    res += "    };\n";

    res += "    tbl.appendChild(tblBody);\n";
    res += "    div.appendChild(tbl);\n";
    res += "    tbl.setAttribute('border', '2');\n";
    res += "};\n";
    res += "generate_" + where + "_table();\n";

    return res;
}


std::string build_json_map(std::map<std::string, std::map<std::string, std::vector<std::string>>> const& mp, std::string where) {
//    using std::string_literals::operator""s;
    using namespace std; // to avoid annoying ""s warning
    std::string res;
    res += "function createCell(cell, text) {\n";
    res += "    var div = document.createElement('div'),\n";
    res += "        txt = document.createTextNode(text === undefined ? \"\" : text);\n";
    res += "    div.appendChild(txt);\n";
    res += "    div.setAttribute('class', \"td-move_table\");\n";
    res += "    cell.appendChild(div);\n";
    res += "}\n\n";

    // append row to the HTML table
    res += "function appendRow(text) {\n";
    res += "    var tbl = document.getElementById('move_table_table'),\n";
    res += "        row = tbl.insertRow(tbl.rows.length),\n";
    res += "        i;\n";
    res += "    for (i = 0; i < tbl.rows[0].cells.length; i++) {\n";
    res += "        createCell(row.insertCell(i), text === undefined ? \"\" : text);\n";
    res += "    }\n";
    res += "}\n\n";

    res += "function appendColumn() {\n"s;

    res += "    var mp = {};\n"s;
    std::set<std::string> nt_s;
    std::set<std::string> t_s;

    for (auto const& rule_set : mp) {
        auto const& nt = rule_set.first;
        nt_s.insert(nt);
        for (auto const& rule : rule_set.second) {
            auto const& t = rule.first;
            t_s.insert(t);
            auto const& rl = rule.second;
            std::string sr;
            for (auto const& x : rl) {
                sr += x + " "s;
            }
            sr.pop_back();
            res += "    mp[["s + "\""s + nt + "\""s + ","s + "\""s + t + "\""s + "]] "s + "= \""s + sr + "\";\n"s;
        }
    }

    res += "    var nt = [";
    bool comma {false};
    for (auto const& nt : nt_s) {
        if (comma) {
            res += ", ";
        }
        comma = true;
        res += "\""s + nt + "\"";
    }
    res += "];\n";
    res += "    var tt = [";
    comma = false;
    for (auto const& t : t_s) {
        if (t == "eps") {
            continue;
        }
        if (comma) {
            res += ", ";
        }
        comma = true;
        res += "\""s + t + "\"";
    }
    res += "];\n";
    res += "    var tbl = document.getElementById('"s + where + "_table');\n"s;

    res += "    for (var i = 0; i < tt.length; ++i) {\n";
    res += "        appendRow(tt[i]);\n";
    res += "    }\n";

    res += "    for (var j = 0; j < nt.length; ++j) {\n";
    res += "          createCell(tbl.rows[0].insertCell(tbl.rows[0].cells.length), nt[j]);\n";
    res += "          for (var i = 0; i < tt.length; i++) {\n";
    res += "              createCell(tbl.rows[i + 1].insertCell(tbl.rows[i + 1].cells.length), mp[[nt[j], tt[i]]]);\n";
    res += "          }\n";
    res += "    }\n";

    res += "}\n";

    res += "appendColumn();\n"s;

    return res;
}

std::ostream& operator << (std::ostream& out, Token token) {
    out << token_names[static_cast<std::underlying_type<Token>::type>(token)];
    return out;
}

std::string trim(std::string str) {
    str.erase(0, str.find_first_not_of(" \n\t\r"));
    str.erase(str.find_last_not_of(" \n\t\r") + 1);
    return str;
}

std::pair<std::string, std::string> get_browser_arguments() {
    char cCurrentPath[FILENAME_MAX];
    if (!getcwd(cCurrentPath, sizeof(cCurrentPath))) {
        throw std::exception();
    }
    std::string f(cCurrentPath);
    f += "/index.html";
    std::string ff("firefox");
    return {ff, f};
}

std::vector<std::string> split(std::string s, char c) {
    std::vector<std::string> res;
    std::istringstream iss(s);
    std::string current_term;
    while (std::getline(iss, current_term, c)) {
        res.emplace_back(std::move(current_term));
    }
    return res;
}


std::string join(std::vector<std::string> values, char separator) {
	std::string result = *(values.begin());
	for (auto it = std::next(values.begin()); it != values.end(); ++it) {
		result += separator;
		result += *it;
	}
	return result;
}

std::string build_json_header(const std::string &header) {
	using namespace std;
	return "function parser_string() { document.getElementById('parser_string_place').innerHTML = '"s + header + "';}\nparser_string();";
}
