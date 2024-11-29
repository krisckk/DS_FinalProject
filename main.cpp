#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <queue>
#include <regex>

using namespace std;

// Normalize a string: convert to lowercase, remove no-alphabetic characters.
string normalize(const string &text){
    string result;
    for(char c: text){
        if(isalpha(c){
            result += tolower(c);
        })
    }
    return result;
}
// Split a string by spaces.
vector<string> split(const string &line) {
    vector<string> words;
    string word;
    for (char c : line) {
        if (isspace(c)) {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}
struct TrieNode {
    unordered_map<char, TrieNode *> children;
    unordered_set<int> essay_ids; // Stores essay IDs that match.
    bool is_end_of_word = false;
};

class Trie {
private:
    TrieNode *root;

public:
    Trie() {
        root = new TrieNode();
    }

    // Insert a word into the Trie.
    void insert(const string &word, int essay_id) {
        TrieNode *node = root;
        for (char c : word) {
            if (!node->children.count(c)) {
                node->children[c] = new TrieNode();
            }
            node = node->children[c];
            node->essay_ids.insert(essay_id);
        }
        node->is_end_of_word = true;
    }

    // Search for words with a given prefix.
    unordered_set<int> search_prefix(const string &prefix) {
        TrieNode *node = root;
        for (char c : prefix) {
            if (!node->children.count(c)) {
                return {}; // Prefix not found.
            }
            node = node->children[c];
        }
        return node->essay_ids;
    }

    // Search for wildcard pattern using BFS.
    unordered_set<int> search_pattern(const string &pattern) {
        queue<pair<TrieNode *, int>> q;
        q.push({root, 0});
        unordered_set<int> result;

        while (!q.empty()) {
            auto [node, i] = q.front();
            q.pop();

            if (i == pattern.size()) {
                result.insert(node->essay_ids.begin(), node->essay_ids.end());
                continue;
            }

            char c = pattern[i];
            if (c == '*') {
                // Match 0 or more characters.
                q.push({node, i + 1}); // Skip '*'.
                for (auto &[child_char, child_node] : node->children) {
                    q.push({child_node, i}); // Include child nodes.
                }
            } else {
                if (node->children.count(c)) {
                    q.push({node->children[c], i + 1});
                }
            }
        }

        return result;
    }
};
void parse_essays(const string &folder_path, Trie &trie, vector<string> &titles) {
    for (int i = 0; ; ++i) {
        string file_path = folder_path + "/" + to_string(i) + ".txt";
        ifstream file(file_path);
        if (!file.is_open()) break;

        string title, abstract, line;
        getline(file, title);
        titles.push_back(title);

        // Process title and abstract.
        int essay_id = i;
        vector<string> words = split(normalize(title));
        while (getline(file, line)) {
            vector<string> abstract_words = split(normalize(line));
            words.insert(words.end(), abstract_words.begin(), abstract_words.end());
        }

        // Insert words into the Trie.
        for (const string &word : words) {
            trie.insert(word, essay_id);
        }

        file.close();
    }
}
unordered_set<int> process_query(const string &query, Trie &trie) {
    if (query[0] == '"') { // Exact search.
        string word = query.substr(1, query.size() - 2);
        return trie.search_prefix(normalize(word));
    } else if (query[0] == '*') { // Suffix search.
        string word = query.substr(1);
        // Reverse Trie can be used for optimization (not implemented here).
        return trie.search_prefix(normalize(word));
    } else if (query[0] == '<') { // Wildcard search.
        string pattern = query.substr(1, query.size() - 2);
        return trie.search_pattern(normalize(pattern));
    } else { // Prefix search.
        return trie.search_prefix(normalize(query));
    }
}
unordered_set<int> combine_sets(const unordered_set<int> &set_a, const unordered_set<int> &set_b, char op) {
    unordered_set<int> result;

    if (op == '+') { // AND
        for (int id : set_a) {
            if (set_b.count(id)) {
                result.insert(id);
            }
        }
    } else if (op == '/') { // OR
        result = set_a;
        result.insert(set_b.begin(), set_b.end());
    } else if (op == '-') { // EXCLUDE
        result = set_a;
        for (int id : set_b) {
            result.erase(id);
        }
    }

    return result;
}
int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "Usage: ./essay_search <input_folder> <query_file> <output_file>" << endl;
        return 1;
    }

    string folder_path = argv[1];
    string query_file = argv[2];
    string output_file = argv[3];

    Trie trie;
    vector<string> titles;

    // Parse essays and build the Trie.
    parse_essays(folder_path, trie, titles);

    // Process queries.
    ifstream queries(query_file);
    ofstream output(output_file);
    string query;

    while (getline(queries, query)) {
        unordered_set<int> result_set = process_query(query, trie);
        if (result_set.empty()) {
            output << "Not Found!" << endl;
        } else {
            for (int id : result_set) {
                output << titles[id] << endl;
            }
        }
    }

    queries.close();
    output.close();
    return 0;
}

