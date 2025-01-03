#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <sstream>

using namespace std;

class TrieNode {
public:
    unordered_map<char, shared_ptr<TrieNode>> children;
    bool is_end_of_word;

    TrieNode() : is_end_of_word(false) {}
};

class Trie {
private:
    shared_ptr<TrieNode> root;

    string normalize(const string &word) const {
        string normalized;
        for (char ch : word) {
            if (isalpha(ch)) {
                normalized.push_back(tolower(ch));
            }
        }
        return normalized;
    }

public:
    Trie() { root = make_shared<TrieNode>(); }

    void insert(const string &word) {
        string normalized = normalize(word);
        shared_ptr<TrieNode> node = root;
        for (char ch : normalized) {
            if (!node->children[ch]) {
                node->children[ch] = make_shared<TrieNode>();
            }
            node = node->children[ch];
        }
        node->is_end_of_word = true;
    }

    bool search(const string &word) const {
        string normalized = normalize(word);
        shared_ptr<TrieNode> node = root;
        for (char ch : normalized) {
            if (!node->children[ch]) {
                return false;
            }
            node = node->children[ch];
        }
        return node->is_end_of_word;
    }
};

vector<string> split(const string &str, const string &delim) {
    vector<string> res;
    size_t start = 0, end;
    while ((end = str.find(delim, start)) != string::npos) {
        res.emplace_back(str.substr(start, end - start));
        start = end + delim.length();
    }
    res.emplace_back(str.substr(start));
    return res;
}

vector<string> word_parse(const vector<string> &words) {
    vector<string> parsed;
    for (const string &word : words) {
        string clean_word;
        for (char ch : word) {
            if (isalpha(ch)) {
                clean_word.push_back(tolower(ch));
            }
        }
        if (!clean_word.empty()) {
            parsed.push_back(clean_word);
        }
    }
    return parsed;
}

vector<string> parse_query(const string &query) {
    vector<string> tokens;
    string token;
    istringstream stream(query);
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool evaluate_query(const Trie &trie, const vector<string> &tokens) {
    vector<bool> results;
    bool expect_result = true;

    for (const string &token : tokens) {
        if (token == "+") {
            expect_result = true;
        } else if (token == "-") {
            expect_result = false;
        } else if (token == "/") {
            // OR operator, do nothing
        } else {
            bool found = trie.search(token);
            if (expect_result) {
                results.push_back(found);
            } else {
                results.push_back(!found);
            }
        }
    }

    // Evaluate results for OR (if any are true, return true)
    return any_of(results.begin(), results.end(), [](bool result) { return result; });
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <data_dir> <query_file> <output_file>" << endl;
        return 1;
    }

    string data_dir = argv[1];
    string query_file = argv[2];
    string output_file = argv[3];

    Trie trie;

    // Reading all files in the data directory (assuming file names are 0.txt, 1.txt, ...)
    for (int i = 0; i < 10; ++i) { // Adjust range based on actual file count
        string file_path = data_dir + "/" + to_string(i) + ".txt";
        ifstream file(file_path);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << file_path << endl;
            continue;
        }

        string line;

        // Parse title
        if (getline(file, line)) {
            vector<string> words = split(line, " ");
            vector<string> parsed_words = word_parse(words);
            for (const string &word : parsed_words) {
                trie.insert(word);
            }
        }

        // Parse content
        while (getline(file, line)) {
            vector<string> words = split(line, " ");
            vector<string> parsed_words = word_parse(words);
            for (const string &word : parsed_words) {
                trie.insert(word);
            }
        }

        file.close();
    }

    // Process queries
    ifstream query_input(query_file);
    ofstream output(output_file);
    if (!query_input.is_open() || !output.is_open()) {
        cerr << "Failed to open query or output file." << endl;
        return 1;
    }

    string query;
    while (getline(query_input, query)) {
        vector<string> tokens = parse_query(query);
        if (evaluate_query(trie, tokens)) {
            output << query << ": Found" << endl;
        } else {
            output << query << ": Not Found" << endl;
        }
    }

    query_input.close();
    output.close();

    return 0;
}
