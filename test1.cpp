
#define FILE_EXTENSION ".txt"
#include<fstream>
#include<string>
#include<cstring>
#include <utility>
#include<vector>
#include<iostream>
#include <filesystem>
#include <regex>
#include <set>

using namespace std;

const int EXACT = 0;
const int PREFIX = 1;
const int SUFFIX = 2;
const int INFIX = 3;

const char OP_AND = '+';
const char OP_OR = '/';
const char OP_EXCLUDE = '-';

int extract_word(string &word) {
    if (word[0] == '"') {
        word.erase(std::remove(word.begin(), word.end(), '\"'), word.end());
        return EXACT;;
    }
    if (word[0] == '*' || word[word.length() - 1] == '*') {
        word.erase(std::remove(word.begin(), word.end(), '*'), word.end());
        return SUFFIX;;
    }
    if (word['<']) {
        word.erase(1, word.length() - 2);
        return INFIX;
    }
    return PREFIX;
}

struct Node {
    Node *child[26];
    char ch;
    bool isEndOfWord;
};

/**
 * Tri
 */
class TrieTree {
    Node *root;

public:
    TrieTree() {
        root = new Node();
    }

    ~TrieTree() {
        delete root;
    }

    void insert(const string &word) const {
        Node *current = root;
        const int len = word.length();
        for (int i = 0; i < len; i++) {
            const int c = tolower(word[i]);
            if (current->child[c - 'a'] == NULL) {
                Node *newNode = new Node();
                newNode->ch = c;
                newNode->isEndOfWord = i == len - 1 ? true : false;
                current->child[c - 'a'] = newNode;
                current = newNode;
            } else {
                current = current->child[c - 'a'];
            }
        }
    }

    void insert_reverse(const string &word) const {
        Node *current = root;
        const int index = word.length() - 1;
        for (int i = index; i >= 0; i--) {
            const int c = tolower(word[i]);
            if (current->child[c - 'a'] == nullptr) {
                Node *newNode = new Node();
                newNode->ch = c;
                newNode->isEndOfWord = i == 0 ? true : false;
                current->child[c - 'a'] = newNode;
                current = newNode;
            } else {
                current = current->child[c - 'a'];
            }
        }
    }

    bool search(string &word, bool exact_match) const {
        Node *current = root;
        int matchCount = 0;
        int len = word.length();
        for (int i = 0; i < len; i++) {
            const int c = tolower(word[i]);
            current = current->child[c - 'a'];
            if (current == nullptr) {
                return false;
            }
            matchCount++;
        }
        if (exact_match && current->isEndOfWord) {
            return true;
        }
        return matchCount == len;
    }
};

class Essay {
public:
    string name;
    TrieTree *prefix;
    TrieTree *suffix;

    Essay(string name, TrieTree *prefix, TrieTree *suffix): name(std::move(name)), prefix(prefix), suffix(suffix) {
    }

    ~Essay() {
        delete prefix;
        delete suffix;
    }
};


// Utility Func

// string parser : output vector of strings (words) after parsing
vector<string> word_parse(vector<string> tmp_string) {
    vector<string> parse_string;
    for (auto &word: tmp_string) {
        string new_str;
        for (auto &ch: word) {
            if (isalpha(ch))
                new_str.push_back(ch);
        }
        parse_string.emplace_back(new_str);
    }
    return parse_string;
}

vector<string> split(const string &str, const string &delim) {
    vector<string> res;
    if ("" == str) return res;

    char *strs = new char[str.length() + 1];
    strcpy(strs, str.c_str());

    char *d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char *p = strtok(strs, d);
    while (p) {
        string s = p;
        res.push_back(s);
        p = strtok(NULL, d);
    }

    return res;
}

vector<string> parse_query(const string &query_file) {
    vector<string> queries;
    string line;
    fstream fi;
    fi.open(query_file.c_str(), ios::in);
    while (getline(fi, line)) {
        queries.emplace_back(line);
    }
    fi.close();
    return queries;
}

void write_to_file(const string &file_name, const vector<string> &result) {
    fstream fi;
    fi.open(file_name.c_str(), ios::out);
    for (auto &title: result) {
        cout << title << endl;
        fi << title << endl;
    }
    fi.close();
}

vector<string> extract_operators(string &word, vector<char> &operators) {
    if (word.find(OP_AND) != string::npos) {
        operators.emplace_back(OP_AND);
    }
    if (word.find(OP_OR) != string::npos) {
        operators.emplace_back(OP_OR);
    }
    if (word.find(OP_EXCLUDE) != string::npos) {
        operators.emplace_back(OP_EXCLUDE);
    }
    if (operators.empty()) {
        return {word};
    }
    vector<string> tokens;
    std::sort(operators.begin(), operators.end());
    std::regex delimiter("\\+|-|/");
    std::copy(std::sregex_token_iterator(word.begin(), word.end(), delimiter, -1),
              std::sregex_token_iterator(),
              std::back_inserter(tokens));
    return tokens;
}

string trim(string &w) {
    return std::regex_replace(w, std::regex("^ +| +$|( ) +"), "$1");
}
vector<string> start_query(const vector<Essay *> &essays, vector<string> &query_strings) {
    vector<string> query_result;
    vector<string> keywords;
    vector<char> operations;
    int search_flag;
    vector<string> temp;
    for (auto &query: query_strings) {
        keywords = extract_operators(query, operations);
        if (operations.empty()) {
            search_flag = extract_word(query);
            for (auto &essay: essays) {
                if (search_flag == EXACT) {
                    if (essay->prefix->search(query, true)) {
                        query_result.emplace_back(essay->name);
                        continue;
                    }
                }
                if (search_flag == PREFIX) {
                    if (essay->prefix->search(query, false)) {
                        query_result.emplace_back(essay->name);
                        continue;
                    }
                }
                if (search_flag == SUFFIX) {
                    if (essay->suffix->search(query, false)) {
                        query_result.emplace_back(essay->name);
                        continue;
                    }
                }
                temp = split(query, " ");
                if (essay->prefix->search(temp[0], false) &&
                    essay->suffix->search(temp[1], false)) {
                    query_result.emplace_back(essay->name);
                }
            }
        } else {
            // Operator case
            set<int> tmp1;
            set<int> tmp2;
            set<int> diff;
            const int num_of_essays = essays.size();
            string w = trim(keywords.front());
            keywords.erase(keywords.begin());
            search_flag = extract_word(w);
            for (int i = 0; i < num_of_essays; i++) {
                if (search_flag == EXACT) {
                    if (essays[i]->prefix->search(w, true)) {
                        tmp1.insert(i);
                        continue;
                    }
                }
                if (search_flag == PREFIX) {
                    if (essays[i]->prefix->search(w, false)) {
                        tmp1.insert(i);
                        continue;
                    }
                }
                if (search_flag == SUFFIX) {
                    if (essays[i]->suffix->search(w, false)) {
                        tmp1.insert(i);
                        continue;
                    }
                }
                temp = split(query, " ");
                if (essays[i]->prefix->search(temp[0], false) &&
                    essays[i]->suffix->search(temp[1], false)) {
                    tmp1.insert(i);
                }
            }

            for (char &c: operations) {
                w = trim(keywords.front());
                keywords.erase(keywords.begin());
                search_flag = extract_word(w);
                if (c == OP_AND) {
                    for (int i = 0; i < num_of_essays; i++) {
                        if (search_flag == EXACT) {
                            if (essays[i]->prefix->search(w, true)) {
                                tmp1.insert(i);
                                continue;
                            }
                        }
                        if (search_flag == PREFIX) {
                            if (essays[i]->prefix->search(w, false)) {
                                tmp1.insert(i);
                                continue;
                            }
                        }
                        if (search_flag == SUFFIX) {
                            if (essays[i]->suffix->search(w, false)) {
                                tmp1.insert(i);
                                continue;
                            }
                        }
                        temp = split(query, " ");
                        if (essays[i]->prefix->search(temp[0], false) &&
                            essays[i]->suffix->search(temp[1], false)) {
                            tmp1.insert(i);
                        }
                    }
                    continue;
                }
                if (c == OP_OR) {
                    for (int i: tmp1) {
                        if (search_flag == EXACT) {
                            if (essays[i]->prefix->search(w, true)) {
                                tmp2.insert(i);
                                continue;
                            }
                        }
                        if (search_flag == PREFIX) {
                            if (essays[i]->prefix->search(w, false)) {
                                tmp2.insert(i);
                                continue;
                            }
                        }
                        if (search_flag == SUFFIX) {
                            if (essays[i]->suffix->search(w, false)) {
                                tmp2.insert(i);
                                continue;
                            }
                        }
                        temp = split(query, " ");
                        if (essays[i]->prefix->search(temp[0], false) &&
                            essays[i]->suffix->search(temp[1], false)) {
                            tmp2.insert(i);
                        }
                    }
                    tmp1.clear();
                    for (int i: tmp2) {
                        tmp1.insert(i);
                    }
                    tmp2.clear();
                    continue;
                }
                if (c == OP_EXCLUDE) {
                    // OP_EXCLUDE CASE
                    for (int i = 0; i < num_of_essays; i++) {
                        if (search_flag == EXACT) {
                            if (essays[i]->prefix->search(w, true)) {
                                tmp2.insert(i);
                                continue;
                            }
                        }
                        if (search_flag == PREFIX) {
                            if (essays[i]->prefix->search(w, false)) {
                                tmp2.insert(i);
                                continue;
                            }
                        }
                        if (search_flag == SUFFIX) {
                            if (essays[i]->suffix->search(w, false)) {
                                tmp2.insert(i);
                                continue;
                            }
                        }
                        temp = split(query, " ");
                        if (essays[i]->prefix->search(temp[0], false) &&
                            essays[i]->suffix->search(temp[1], false)) {
                            tmp2.insert(i);
                        }
                    }
                    set_difference(
                        tmp1.begin(), tmp1.end(), tmp2.begin(), tmp2.end(),
                        inserter(diff, diff.begin()));
                    tmp1.clear();
                    tmp2.clear();
                    for (int i: diff) {
                        tmp1.insert(i);
                    }
                    diff.clear();
                }
            }
            for (const int i: tmp1) {
                query_result.emplace_back(essays[i]->name);
            }
        }
    }
    return query_result;
}

vector<Essay *> parse_essays(const vector<string> &data_set) {
    vector<Essay *> essays;
    fstream fi;
    string title_name, tmp;
    vector<string> tmp_string;
    for (auto &essay: data_set) {
        fi.open(essay.c_str(), ios::in);
        getline(fi, title_name);
        tmp_string = split(title_name, " ");
        vector<string> title = word_parse(tmp_string);
        auto *new_essay = new Essay(title_name, new TrieTree(), new TrieTree());
        for (auto &entry: title) {
            new_essay->prefix->insert(entry);
            new_essay->suffix->insert_reverse(entry);
        }
        while (getline(fi, tmp)) {
            // GET CONTENT WORD VECTOR
            tmp_string = split(tmp, " ");

            // PARSE CONTENT
            vector<string> content = word_parse(tmp_string);
            for (auto &word: content) {
                new_essay->prefix->insert(word);
                new_essay->suffix->insert_reverse(word);
            }
        }
        essays.emplace_back(new_essay);
        fi.close();
    }
    return essays;
}

bool comparator(const string a, const string b) {
    string s1 = a.substr(5, a.length() - 9);
    string s2 = b.substr(5, b.length() - 9);
    return stoi(s1) < stoi(s2);
}

int main(int argc, char *argv[]) {
    // INPUT :
    // 1. data directory in data folder
    // 2. number of txt files
    // 3. output route

    string data_dir = argv[1] + string("/");
    string query = string(argv[2]);
    string output = string(argv[3]);


    // Read File & Parser Example
    vector<string> data_set;
    //for (const auto &entry: std::filesystem::directory_iterator(data_dir)) {
    //    //std::cout << entry.path() << std::endl;
    //    data_set.emplace_back(entry.path().string());
    //}
    data_set.emplace_back("data.txt");
    std::sort(data_set.begin(), data_set.end(), comparator);

    vector<string> queries = parse_query(query);
    vector<Essay *> essays = parse_essays(data_set);
    vector<string> result = start_query(essays, queries);

    write_to_file(output, result);

    // delete all essay
    for (const auto &essay: essays) {
        delete essay;
    }
}


// 1. UPPERCASE CHARACTER & LOWERCASE CHARACTER ARE SEEN AS SAME.
// 2. FOR SPECIAL CHARACTER OR DIGITS IN CONTENT OR TITLE -> PLEASE JUST IGNORE, YOU WONT NEED TO CONSIDER IT.
//    EG : "AB?AB" WILL BE SEEN AS "ABAB", "I AM SO SURPRISE!" WILL BE SEEN AS WORD ARRAY AS ["I", "AM", "SO", "SURPRISE"].
// 3. THE OPERATOR IN "QUERY.TXT" IS LEFT ASSOCIATIVE
//    EG : A + B / C == (A + B) / C

//

//////////////////////////////////////////////////////////
