#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

using namespace std;

class Trie {
private:
    Trie* alphachild[26];
    bool isEnd;

public:
    Trie() {
        isEnd = false;
        for (int i = 0; i < 26; i++)
            alphachild[i] = NULL;
    }

    void insert(const string& s) {
        Trie* current = this;
        for (int i = 0; i < s.length(); i++) {
            int idx = tolower(s[i]) - 'a';
            if (current -> alphachild[idx] == NULL)
                current -> alphachild[idx] = new Trie();
            current = current -> alphachild[idx];
        }
        current -> isEnd = true;
    }

    bool exactSearch(const string& s) {
        Trie* current = this;
        for (int i = 0; i < s.length(); i++) {
            int idx = tolower(s[i]) - 'a';
            if (current -> alphachild[idx] == NULL)
                return false;
            current = current -> alphachild[idx];
        }
        return current -> isEnd;
    }

    bool prefixSearch(const string& prefix) {
        Trie* current = this;
        for (int i = 0; i < prefix.length(); i++) {
            int idx = tolower(prefix[i]) - 'a';
            if (current -> alphachild[idx] == NULL)
                return false;
            current = current ->alphachild[idx];
        }
        return true;
    }

    bool wildcardSearch(const string& pattern) {
        return wildcardSearchHelper(this, pattern, 0);
    }

private:
    bool wildcardSearchHelper(Trie* node, const string& pattern, int index) {
        if (node == nullptr) return false;
        if (index == pattern.length()) return node -> isEnd;
        char ch = pattern[index];
        if (ch == '*') {
            for (int i = 0; i < 26; i++) {
                if (node -> alphachild[i] && wildcardSearchHelper(node -> alphachild[i], pattern, index + 1))
                    return true;
            }
            return wildcardSearchHelper(node, pattern, index + 1);
        } else {
            if (!isalpha(ch)) return false;
            int idx = tolower(ch) - 'a';
            return wildcardSearchHelper(node -> alphachild[idx], pattern, index + 1);
        }
    }
};

// Utility function to parse words from a string
vector<string> word_parse(const string& str) {
    vector<string> words;
    string word;
    for (char ch : str) {
        if (isalpha(ch)) {
            word.push_back(tolower(ch));
        } else if (!word.empty()) {
            words.push_back(word);
            word.clear();
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

vector<string> split(const string& str, const string& delim) {
    vector<string> res;
    if (str.empty()) return res;

    char* strs = new char[str.length() + 1];
    strcpy_s(strs, str.length() + 1, str.c_str());

    char* d = new char[delim.length() + 1];
    strcpy_s(d, delim.length() + 1, delim.c_str());

    char* context = nullptr;
    char* p = strtok_s(strs, d, &context);
    while (p) {
        string s = p;
        res.push_back(s);
        p = strtok_s(nullptr, d, &context);
    }

    delete[] strs;
    delete[] d;

    return res;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <data_dir> <query_file> <output_file>" << endl;
        return 1;
    }

    string data_dir = argv[1] + string("/");
    string query_file = string(argv[2]);
    string output_file = string(argv[3]);

    // Create a vector for titles names to print the matching result title into the output.txt
    vector<string> titles;

    // Create a vector of Trie objects for essays and suffixes
    vector<Trie> essayTrie;
    vector<Trie> suffixEssayTrie;

    // Read queries
    fstream queryFile(query_file, ios::in);
    string Query;

    vector<string> temp;
    // Create a 2D vector for each query to store the matching essay index
    vector<vector<int>> answerIndex;
    fstream of(output_file, ios::out);

    // Loop through each file
    for (int fileIdx = 0; ; ++fileIdx) {
        string filePath = data_dir + to_string(fileIdx) + ".txt";
        fstream fi;
        fi.open(filePath, ios::in);
        if (!fi.is_open()) {
            break; // Stop reading files if file not found
        }

        // Ensure the vectors are large enough
        if (essayTrie.size() <= fileIdx) {
            essayTrie.resize(fileIdx + 1);
            suffixEssayTrie.resize(fileIdx + 1);
            titles.resize(fileIdx + 1);
        }

        string line;
        // Read title
        getline(fi, line);
        titles[fileIdx] = line;
        temp = split(line, " ");
        string tempStr = accumulate(temp.begin(), temp.end(), string(" "));
        vector<string> titleWords = word_parse(tempStr);
        for (const string& word : titleWords) {
            essayTrie[fileIdx].insert(word);
            string suffix = word;
            reverse(suffix.begin(), suffix.end());
            suffixEssayTrie[fileIdx].insert(suffix);
            cout << "Inserted title word: " << word << " and its suffix: " << suffix << endl; // Debug print
        }

        // Read body
        while (getline(fi, line)) {
            temp = split(line, " ");
            string tempStr = accumulate(temp.begin(), temp.end(), string(" "));
            vector<string> bodyWords = word_parse(tempStr);
            for (const string& word : bodyWords) {
                essayTrie[fileIdx].insert(word);
                // reverse the word and store it in suffixEssayTrie
                string suffix = word;
                reverse(suffix.begin(), suffix.end());
                suffixEssayTrie[fileIdx].insert(suffix);
            }
        }

        int queryIdx = 0;
        while (getline(queryFile, Query)) {
            cout << "Reading query: " << Query << endl;
            vector<string> queryWords = split(Query, " ");
            vector<bool> found(queryWords.size(), false);
            for (int i = 0; i < queryWords.size(); i++) {
                cout << "Processing query word: " << queryWords[i] << endl; // Debug print
                if (queryWords[i][0] == '"') {
                    string temp = queryWords[i];
                    temp.pop_back();
                    temp.erase(temp.begin());
                    queryWords[i] = temp;
                    if (queryWords[i - 1][0] == '-') found[i] = !essayTrie[fileIdx].prefixSearch(queryWords[i]);
                    else if (queryWords[i - 1][0] == '+' && found[i - 2]) found[i] = essayTrie[fileIdx].prefixSearch(queryWords[i]);
                    else if (queryWords[i - 1][0] == '/') {
                        found[i] = essayTrie[fileIdx].prefixSearch(queryWords[i]);
                        if (!found[i] && !found[i - 2]) found[i] = found[i - 2] = false;
                        else found[i] = found[i - 2] = true;
                    }
                    else found[i] = essayTrie[fileIdx].prefixSearch(queryWords[i]);
                }
                else if (isalpha(queryWords[i][0])) {
                    if (queryWords[i - 1][0] == '-') found[i] = !essayTrie[fileIdx].prefixSearch(queryWords[i]);
                    else if (queryWords[i - 1][0] == '+' && found[i - 2]) found[i] = essayTrie[fileIdx].prefixSearch(queryWords[i]);
                    else if (queryWords[i - 1][0] == '/') {
                        found[i] = essayTrie[fileIdx].prefixSearch(queryWords[i]);
                        if (!found[i] && !found[i - 2]) found[i] = found[i - 2] = false;
                        else found[i] = found[i - 2] = true;
                    }
                    else found[i] = essayTrie[fileIdx].prefixSearch(queryWords[i]);
                }
                else if (queryWords[i][0] == '*') {
                    reverse(queryWords[i].begin(), queryWords[i].end());
                    string temp = queryWords[i];
                    temp.pop_back();
                    temp.erase(temp.begin());
                    queryWords[i] = temp;
                    if (queryWords[i - 1][0] == '-') found[i] = !suffixEssayTrie[fileIdx].prefixSearch(queryWords[i]);
                    else if (queryWords[i - 1][0] == '+' && found[i - 2]) found[i] = suffixEssayTrie[fileIdx].prefixSearch(queryWords[i]);
                    else if (queryWords[i - 1][0] == '/') {
                        found[i] = suffixEssayTrie[fileIdx].prefixSearch(queryWords[i]);
                        if (!found[i] && !found[i - 2]) found[i] = found[i - 2] = false;
                        else found[i] = found[i - 2] = true;
                    }
                    else found[i] = suffixEssayTrie[fileIdx].prefixSearch(queryWords[i]);
                }
                else if (queryWords[i][0] != '*' && queryWords[i].find('*')) {
                    if (queryWords[i - 1][0] == '-') found[i] = !suffixEssayTrie[fileIdx].wildcardSearch(queryWords[i]);
                    else if (queryWords[i - 1][0] == '+' && found[i - 2]) found[i] = suffixEssayTrie[fileIdx].wildcardSearch(queryWords[i]);
                    else if (queryWords[i - 1][0] == '/') {
                        found[i] = suffixEssayTrie[fileIdx].wildcardSearch(queryWords[i]);
                        if (!found[i] && !found[i - 2]) found[i] = found[i - 2] = false;
                        else found[i] = found[i - 2] = true;
                    }
                    else found[i] = suffixEssayTrie[fileIdx].wildcardSearch(queryWords[i]);
                }
                else if (queryWords[i][0] == '+' || queryWords[i][0] == '/' || queryWords[i][0] == '-') {
                    found[i] = true;
                }
            }
            // Check if every member of found is true, if it is, then add the fileIdx to answerIndex
            bool allTrue = true;
            for (int i = 0; i < queryWords.size(); i++) {
                if (!found[i]) {
                    allTrue = false;
                    break;
                }
            }
            if (allTrue) {
                answerIndex[queryIdx].push_back(fileIdx);
            }
        }

        fi.close();
    }
    queryFile.close();

    if (!of.is_open()) {
        cerr << "Error opening or creating output file" << endl;
        return 1;
    }
    for (int i = 0; i < answerIndex.size(); ++i) {
        for (int j = 0; j < answerIndex[i].size(); ++j) {
            of << titles[answerIndex[i][j]] << endl;
            of << "Writing title to output: " << titles[answerIndex[i][j]] << endl; // Debug print
        }
    }
    of.close();
    return 0;
}