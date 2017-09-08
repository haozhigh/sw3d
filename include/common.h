#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;


void ExitAsssert(bool b, const string& s, int e = -1) {
    if (! b) {
        cout << s << endl;
        exit(e);
    }
}

bool StartsWith(const string& s, const string& sub) {
    for (unsigned i = 0; i < sub.size(); i++)
        if (s[i] != sub[i])
            return false;
    return true;
}

vector<string> Split(const string& s, char c) {
    vector<string> r;
    istringstream ss {s};
    string s_tmp;
    while (getline(ss, s_tmp, c))
        r.push_back(move(s_tmp));
    return r;
}
