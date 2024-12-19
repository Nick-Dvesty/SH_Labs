#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <queue>
#include <limits>
#include <stack>

using namespace std;


template<typename T>
class PostfixConverter {
public:
    explicit PostfixConverter(vector<tuple<char, int, T(*)(T&, T&)>> &&priorsBi, vector<tuple<char, int, T(*)(T&)>> &&priorsUn, T(*converter)(char));
    string ToPostfix(string &infixExpr);
    T calc(string postfixExpr);
    T executeUn(char op, T& first);
    T executeBi(char op, T& first, T& second);
private:
    T(*converter)(char);
    map<char, pair<int, T(*)(T&)>> operationPriorityUn;
    map<char, pair<int, T(*)(T&, T&)>> operationPriorityBi;
    string GetStringNumber(string expr, int &pos);
};

class DFA {
public:
    explicit DFA(int st);
    void setStartState(int st);
    void addAcceptState(int state);
    void addTransition(int from, int to, char symbol);
    bool accepts(const std::string &word);
    void print();
private:
    int start_state;
    set<int> accept_states;
    map<int, map<char, int>> transition;
};

class NFA {
public:
    explicit NFA(int st);
    void addAcceptState(int state);
    void addTransition(int from, int to, char symbol);
    DFA convertToDFA();
    void print();
private:
    int start_state;
    set<int> accept_states;
    map<int, map<char, set<int>>> transitions;
    map<set<int>, map<char, set<int>>> convertMapToDFA();
};

class NFAL{
public:
    explicit NFAL(int st);
    NFAL(NFAL && other) noexcept;
    void addAcceptState(int state);
    void addTransition(int from, int to, char symbol);
    NFA convertToNFA();
    static NFAL redexToNfal(string redex);
    static NFAL clini(NFAL &automat);
    static NFAL unification(NFAL &automatLeft, NFAL &automatRight);
    static NFAL concatenation(NFAL &automat_from, NFAL &automat_to);
    void print();
private:
    int* start_state;
    vector<int*> states;
    vector<int*> accept_states;
    map<int*, map<char, set<int*>>> transitions;

    int* retAddressState(int state);
    set<int*> findAchievable(int* state);
private:
    void findAchievableRec(int* state, set<int*> &ans);
    void mergeConflict(NFAL &other);
    void mergeNfal(NFAL &other);
    void clear();
};

DFA::DFA(int st) : start_state(st) {}
void DFA::setStartState(int st){
    start_state = st;
}
void DFA::addAcceptState(int state) {
    accept_states.insert(state);
}
void DFA::addTransition(int from, int to, char symbol) {
    transition[from][symbol] = to;
}
bool DFA::accepts(const std::string &word) {
    int current_state = start_state;

    for (char symbol : word) {
        if (transition[current_state].find(symbol) != transition[current_state].end())
            current_state = transition[current_state][symbol];
        else
            return false;
    }

    return accept_states.find(current_state) != accept_states.end();
}
void DFA::print() {
    cout<<"Start state: "<<(start_state)<<endl;
    cout<<"Accept States: ";
    for (auto & st : accept_states) {
        cout<<st<<" ";
    }
    cout<<endl;
    cout<<"Transitions: "<<endl;
    for (auto & st : transition) {
        for(auto & way : st.second){
            cout<<(st.first)<<" "<<way.first<<" "<<way.second<<endl;
        }
    }
}

NFA::NFA(int st) : start_state(st) {}
void NFA::addAcceptState(int state) {
    accept_states.insert(state);
}
void NFA::addTransition(int from, int to, char symbol) {
    transitions[from][symbol].insert(to);
}
void NFA::print() {
    cout<<"Start state: "<<(start_state)<<endl;
    cout<<"Accept States: ";
    for (auto & st : accept_states) {
        cout<<st<<" ";
    }
    cout<<endl;
    cout<<"Transitions: "<<endl;
    for (auto & st : transitions) {
        for(auto & way : st.second){
            cout<<(st.first)<<" "<<way.first<<"{ ";
            for(auto & ch : way.second) {
                cout<<ch<<" ";
            }
            cout<<"}"<<endl;
        }
    }
}
DFA NFA::convertToDFA(){
    DFA dfa(start_state);
    auto dfa_transitions = convertMapToDFA();
    int state_id = 0;

    for (const auto& entry : dfa_transitions) {
        const set<int>& state_set = entry.first;

        for (int state : state_set) {
            if (accept_states.find(state) != accept_states.end()) {
                dfa.addAcceptState(state_id);
                break;
            }
        }

        for (int state : state_set) {
            if (state == start_state) {
                dfa.setStartState(state_id);
                break;
            }
        }

        for (const auto& transition : entry.second) {
            char symbol = transition.first;
            const set<int>& next_states = transition.second;

            int next_state_id = distance(dfa_transitions.begin(),
                                         find_if(dfa_transitions.begin(), dfa_transitions.end(),
                                                 [&next_states](const auto& t) { return t.first == next_states; }));

            dfa.addTransition(state_id, next_state_id, symbol);
        }

        state_id++;
    }
    return dfa;
}
map<set<int>, map<char, set<int>>> NFA::convertMapToDFA() {
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

NFAL::NFAL(int st) {
    states.push_back(new int(st));
    start_state = states.back();
}
NFAL::NFAL(NFAL &&other) noexcept {
    start_state = other.start_state;
    states = other.states;
    accept_states = other.accept_states;
    transitions = other.transitions;
    other.clear();
}
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
NFA NFAL::convertToNFA() {
    NFA answer{*start_state};
    vector<set<int*>> E(states.size());
    int i = 0;
    for (auto & st : states) {
        E[i++] = findAchievable(st);
    }
    for (int j = 0; j < E.size(); j++) { // бежим по всем элементам
        int* ptr_start = states[j];
        for (auto & st : E[j]) { // бежим по всем достижимым из ptr_start элементам
            for (auto & te : transitions[st]) { // бежим по всем буквам
                if (te.first != -1) {
                    for (auto & ptr_end : te.second) {
                        answer.addTransition(*ptr_start, *ptr_end, te.first);
                    }
                }
            }
        }
    }
    for (auto & st : accept_states) {
        answer.addAcceptState(*st);
    }
    return answer;
}//
NFAL NFAL::redexToNfal(std::string redex) {
    PostfixConverter<NFAL> converter ({{',',2, unification}, {'+',1, concatenation}},{{'*',3, clini}}, [](char ch){
        NFAL nfal(0);
        nfal.addTransition(0,1,ch);
        nfal.addAcceptState(1);
        return nfal;
    });
    string ans = converter.ToPostfix(redex);
    return converter.calc(ans);
}
NFAL NFAL::clini(NFAL &automat) {
    NFAL cliniNfal = std::move(automat);
    int max = 0;
    for (auto & elem : cliniNfal.states) {
        if (*elem > max) max = *elem;
    }
    for (auto & elem : cliniNfal.accept_states) {
        cliniNfal.addTransition(*elem, *(cliniNfal.start_state),-1);
    }
    cliniNfal.addTransition(max + 1, *(cliniNfal.start_state), -1);
    cliniNfal.addAcceptState(max + 1);
    cliniNfal.start_state = cliniNfal.retAddressState(max + 1);
    return cliniNfal;
}
NFAL NFAL::unification(NFAL &automatLeft, NFAL &automatRight) {
    NFAL unNfal = std::move(automatLeft);
    unNfal.mergeNfal(automatRight);
    unNfal.accept_states.insert(unNfal.accept_states.end(), automatRight.accept_states.begin(), automatRight.accept_states.end());
    int max = 0;
    for (auto & elem : unNfal.states) {
        if (*elem > max) max = *elem;
    }
    unNfal.addTransition(max + 1, *(unNfal.start_state), -1);
    unNfal.addTransition(max + 1, *(automatRight.start_state), -1);
    unNfal.start_state = unNfal.retAddressState(max + 1);
    automatRight.clear();
    return unNfal;
}
NFAL NFAL::concatenation(NFAL &automat_from, NFAL &automat_to) {
    auto conNfal = std::move(automat_from);
    conNfal.mergeNfal(automat_to);
    for ( auto & elem : conNfal.accept_states){
        conNfal.addTransition(*elem, *automat_to.start_state, -1);
    }
    conNfal.accept_states = automat_to.accept_states;
    automat_to.clear();
    return conNfal;
}
void NFAL::print() {
    cout<<"Start state: "<<(*start_state)<<endl;
    cout<<"Accept States: ";
    for (auto & st : accept_states) {
        cout<<*st<<" ";
    }
    cout<<endl;
    cout<<"Transitions: "<<endl;
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
void NFAL::mergeConflict(NFAL &other) {
    vector<int*> conflicts;
    for (auto & st_old : states){
        for (auto & st : other.states) {
            if (*st == *st_old){
                conflicts.push_back(st);
            }
        }
    }
    for (int i = 0, j = 0; i < std::numeric_limits<int>::max() && j != conflicts.size(); i++) {
        bool used = false;
        for ( auto & elem : states){
            if (*elem == i) {
                used = true;
                break;
            }
        }
        for ( auto & elem : other.states){
            if (*elem == i) {
                used = true;
                break;
            }
        }
        if (!used) {
            *(conflicts[j++]) = i;

        }
    }
}
set<int *> NFAL::findAchievable(int *state) {
    set<int *> ans;
    findAchievableRec(state, ans);
    return ans;
}
void NFAL::findAchievableRec(int *state, set<int* > &ans) {
    ans.insert(state);
    for (auto & st : transitions[state][-1]) {
        if (ans.find(st) == ans.end()) findAchievableRec(st, ans);
    }
}
void NFAL::mergeNfal(NFAL &other) {
    mergeConflict(other);
    states.insert(states.end(), other.states.begin(), other.states.end());
    for (auto & elem : other.transitions) {
        transitions[elem.first] = elem.second;
    }
}
void NFAL::clear() {
    start_state = nullptr;
    states.clear();
    accept_states.clear();
    transitions.clear();
}

template<typename T>
PostfixConverter<T>::PostfixConverter(vector<tuple<char, int, T(*)(T&, T&)>> &&priorsBi, vector<tuple<char, int, T(*)(T&)>> &&priorsUn, T(*converter)(char)) :
    converter{converter},operationPriorityUn{}, operationPriorityBi{}  {
    operationPriorityUn['('] = pair<int, T(*)(T&)>(0, nullptr);
    for (auto & prior : priorsUn) {
        operationPriorityUn[get<0>(prior)] = pair<int, T(*)(T&)>(get<1>(prior), get<2>(prior));
    }
    for (auto & prior : priorsBi) {
        operationPriorityBi[get<0>(prior)] = pair<int, T(*)(T&, T&)>(get<1>(prior), get<2>(prior));
    }

}
template<typename T>
string PostfixConverter<T>::GetStringNumber(std::string expr, int &pos) {
    string strNumber;
    for (; pos < expr.length(); pos++) {
        char num = expr[pos];
        if (std::isalpha(num)) {
            strNumber += num;
        } else {
            pos--;
            break;
        }
    }
    return strNumber;
}
template<typename T>
string PostfixConverter<T>::ToPostfix(string &infixExpr) {
    string postfixExpr;
    stack<char> stackEl;
    for (int i = 0; i < infixExpr.length(); i++) {
        char c = infixExpr[i];
        if (std::isalpha(c) || std::isdigit(c)) {
            postfixExpr += GetStringNumber(infixExpr, i) + " ";
        }
        else if (c == '(') {
            stackEl.push(c);
        } else if (c == ')') {
            while (!stackEl.empty() && stackEl.top() != '(') {
                postfixExpr += stackEl.top();
                postfixExpr += ' ';
                stackEl.pop();
            }
            stackEl.pop();
        }
        else if (operationPriorityUn.find(c) != operationPriorityUn.end() || operationPriorityBi.find(c) != operationPriorityBi.end()) {
            char op = c;
            while (!stackEl.empty()) {
                int stackTopPrior = 0;
                if (operationPriorityUn.find(stackEl.top()) != operationPriorityUn.end()){
                    stackTopPrior = operationPriorityUn[stackEl.top()].first;
                } else if (operationPriorityBi.find(stackEl.top()) != operationPriorityBi.end()) {
                    stackTopPrior = operationPriorityBi[stackEl.top()].first;
                }
                int opPrior = 0;
                if (operationPriorityUn.find(op) != operationPriorityUn.end()){
                    opPrior = operationPriorityUn[op].first;
                } else if (operationPriorityBi.find(op) != operationPriorityBi.end()) {
                    opPrior = operationPriorityBi[op].first;
                }
                if (stackTopPrior < opPrior) break;
                postfixExpr += stackEl.top();
                postfixExpr += ' ';
                stackEl.pop();
            }
            stackEl.push(op);
        }
    }
    while (!stackEl.empty()) {
        postfixExpr += stackEl.top();
        stackEl.pop();
    }
    return postfixExpr;
}
template<typename T>
T  PostfixConverter<T>::calc(string postfixExpr) {
    stack<T> locals;
    int counter = 0;
    //	Проходим по строке
    for (int i = 0; i < postfixExpr.length(); i++) {
        char c = postfixExpr[i];
        if (std::isdigit(c) || std::isalpha(c)) {
            T elem = converter(c);
            locals.push(std::move(elem));
        } else if (operationPriorityUn.find(c) != operationPriorityUn.end() ||
                   operationPriorityBi.find(c) != operationPriorityBi.end()) {
            counter += 1;
            if (operationPriorityUn.find(c) != operationPriorityUn.end()) {
                //	Проверяем, пуст ли стек: если да - задаём нулевое значение,
                //	еси нет - выталкиваем из стека значение
                T last = std::move(locals.top());
                locals.pop();
                //	Получаем результат операции и заносим в стек
                locals.push(executeUn(c,last));
                //	Отчитываемся пользователю о проделанной работе
                //Console.WriteLine($"{counter}) {c}{last} = {locals.Peek()}");
                //	Указываем, что нужно перейти к следующей итерации цикла
                //	 для того, чтобы пропустить остальной код
                continue;
            }

            //	Получаем значения из стека в обратном порядке
            T second = std::move(locals.top());
            locals.pop();
            T first = std::move(locals.top());
            locals.pop();
            //	Получаем результат операции и заносим в стек
            locals.push(executeBi(c, first, second));
            //	Отчитываемся пользователю о проделанной работе
            //Console.WriteLine($"{counter}) {first} {c} {second} = {locals.Peek()}");
        }
    }
    return std::move(locals.top());
}
template<typename T>
T PostfixConverter<T>::executeUn(char op, T &first) {
    return operationPriorityUn[op].second(first);
}
template<typename T>
T PostfixConverter<T>::executeBi(char op, T &first, T &second) {
    return operationPriorityBi[op].second(first, second);
}

int main() {
    NFAL nf1 = std::move(NFAL::redexToNfal("((a+b),(c+d))*"));
    nf1.print();
    NFA nf2 = nf1.convertToNFA();
    nf2.print();
    DFA nf3 = nf2.convertToDFA();
    nf3.print();
    auto t6 = nf3.accepts("abcdabcdcd");
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

    auto dfa = nfa.convertToDFA();

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