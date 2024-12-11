#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <queue>

using namespace std;


class NFA {
private:
    int start_state;
    map<int, map<char, set<int>>> transitions;
public:
    set<int> accept_states;

    explicit NFA(int st) : start_state(st) {};

    void addAcceptState(int state) {
        accept_states.insert(state);
    }

    void addTransition(int from, int to, char symbol) {
        transitions[from][symbol].insert(to);
    }

    map<set<int>, map<char, set<int>>> toDFA() {
        map<set<int>, map<char, set<int>>> dfa;
        set<set<int>> dfa_states;

        set<int> start_set = {start_state};
        dfa_states.insert(start_set);
        queue<set<int>> state_queue;
        state_queue.push(start_set);

        while (!state_queue.empty()) {
            set<int> current_set = state_queue.front();
            state_queue.pop();

            for (int state: current_set) {
                for (const auto &trans: transitions[state]) {
                    char symbol = trans.first;
                    const set<int> &next_states = trans.second;

                    dfa[current_set][symbol].insert(next_states.begin(), next_states.end());
                }
            }

            for (const auto &transition: dfa[current_set]) {
                const set<int> &next_dfa_states = transition.second;

                if (dfa_states.find(next_dfa_states) == dfa_states.end()) {
                    dfa_states.insert(next_dfa_states);
                    state_queue.push(next_dfa_states);
                }
            }
        }
        return dfa;
    }
};

class DFA {
private:
    int start_state;
    set<int> accept_states;
    map<int, map<char, int>> transition;
public:

    explicit DFA(int st) : start_state(st) {};


    void addAcceptState(int state) {
        accept_states.insert(state);
    }

    void addTransition(int from, int to, char symbol) {
        transition[from][symbol] = to;
    }

    bool accepts(const std::string &word) {
        int current_state = start_state;

        for (char symbol : word) {
            if (transition[current_state].find(symbol) != transition[current_state].end())
                current_state = transition[current_state][symbol];
            else
                return false;
        }

        return accept_states.find(current_state) != accept_states.end();
    }
};


class NFAL{
public:
    explicit NFAL(int st);
    void addAcceptState(int state);
    void addTransition(int from, int to, char symbol);
    static NFAL clini(NFAL &automat);
    static NFAL unification(NFAL &automat_left, NFAL &automat_right);
    static NFAL concatenation(NFAL &automat_from, NFAL &automat_to);
    void replace_name(int old_name, int newName);
    void print();
private:
    int* start_state;
    vector<int*> states;
    vector<int*> accept_states;
    map<int*, map<char, set<int*>>> transitions;

    int* retAddressState(int state);
    void merge_conflict(NFAL &other);
};

NFAL::NFAL(int st) {
    states.push_back(new int(st));
    start_state = states.back();
};
void NFAL::addAcceptState(int state) {
    int *ptr_st = retAddressState(state);
    for (auto & ac_st : accept_states) {
        if (ac_st == ptr_st) throw invalid_argument("Duplicate accept state.");
    }
    accept_states.push_back(ptr_st);
}
void NFAL::addTransition(int from, int to, char symbol) {
    int* ptr_from = retAddressState(from);
    int* ptr_to = retAddressState(to);
    transitions[ptr_from][symbol].insert(ptr_to);
}
NFAL NFAL::clini(NFAL &automat) {

}
NFAL unification(NFAL &automat_left, NFAL &automat_right);
NFAL concatenation(NFAL &automat_from, NFAL &automat_to);
void NFAL::replace_name(int old_name, int newName) {
    int* ptr_st = retAddressState(old_name);
    *ptr_st = newName;
}
void NFAL::print() {
    for (auto & st : transitions) {
        for(auto & way : st.second){
            cout<<*(st.first)<<" "<<way.first<<"{ ";
            for(auto & ch : way.second) {
                cout<<*ch<<" ";
            }
            cout<<"}"<<endl;
        }
    }
}

int *NFAL::retAddressState(int state) {
    for(auto & st : states) {
        if (*st == state) return st;
    }
    states.push_back(new int(state));
    return states.back();
}

void NFAL::merge_conflict(NFAL &other) {
    set<int*> conflicts;
    for (auto & st_old : states){
        for (auto & st : other.states) {
            if (*st == *st_old){
                conflicts.insert(st);
            }
        }
    }

}

int main() {

    vector<int> f1{1,2,4,5};
    int* test = &f1[0];
    NFAL nfal(1);
    nfal.addTransition(1,2,'a');
    nfal.addTransition(1,3,'b');
    nfal.addTransition(2,1,'x');
    nfal.addTransition(1,2,0);
    nfal.replace_name(1,5);
    nfal.addTransition(5,4,'x');
    nfal.addTransition(5,2,'x');
    nfal.print();


    return 0;
    int k,st,f,n,t,tmp,a1,a2;
    char symbol;

    cin >> k >> st >> f;
    NFA nfa(st);
    for(int i = 0; i < f;i++) {
        cin >> tmp;
        nfa.addAcceptState(tmp);
    }
    cin >> n;
    for(int i = 0; i < n; i++) {
        cin >> a1 >> a2 >> symbol;
        nfa.addTransition(a1,a2,symbol);
    }

    DFA dfa(st);
    auto dfa_transitions = nfa.toDFA();
    int state_id = 0;

    for (const auto& entry : dfa_transitions) {
        const set<int>& state_set = entry.first;

        for (int state : state_set) {
            if (nfa.accept_states.find(state) != nfa.accept_states.end()) {
                dfa.addAcceptState(state_id);
                break;
            }
        }

        for (const auto& transition : entry.second) {
            symbol = transition.first;
            const set<int>& next_states = transition.second;

            int next_state_id = distance(dfa_transitions.begin(),
                                         find_if(dfa_transitions.begin(), dfa_transitions.end(),
                                                 [&next_states](const auto& t) { return t.first == next_states; }));

            dfa.addTransition(state_id, next_state_id, symbol);
        }

        state_id++;
    }
    cin >> t;
    vector<string> words(t);
    for(int i = 0; i < t; i++)
        cin >> words[i];
    for(int i = 0; i < t; i++) {
        if (dfa.accepts(words[i]))
            cout << "YES\n";
        else
            cout << "NO\n";
    }
    return 0;
}