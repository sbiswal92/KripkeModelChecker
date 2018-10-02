
/**
    Parser for the model checking tool.

    Abandon all hope, ye who enter here.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <cassert>

#include "model.h"

using namespace std;

// #define DEBUG
// #define DEBUG_FORMULA
// #define SHOW_FORMULA_LABELS


typedef enum {
  INIT=0, KRIPKE, STATES, INTEGER,
  ARCS, ARCS_S1, ARCS_ARROW, ARCS_S2,
  LABELS, LABELS_L, LABELS_COLON, LABELS_S, LABELS_COMMA,
  CTL, CTL_L, CTL_ASSIGN, CTL_FORMULA,
  CTL_S, CTL_MODELS, CTL_S_L,
  CTL_SET_OPEN, CTL_SET_L, CTL_SET_CLOSE,
  DONE
} fsm_state;

typedef enum {
  MODEL=0, LABEL, DISPLAY
} ctl_formula_type;

class ctl_formula {
  ctl_formula_type type;
  public:
  ctl_formula(ctl_formula_type f_type): type(f_type) {}
  ctl_formula_type getType() const { return type; }
  virtual void show() = 0;
};

map<string, state_set*> str2set;
state_set* getSet(string label) {
  return str2set[label];
}
void eraseSet(string label) {
  map<string, state_set*>::iterator iter = str2set.find(label);
  if (iter != str2set.end()) {
    str2set.erase(iter);
  }
}
void setSet(string label, state_set* sset) {
  // only way to over-write is to use a combination of
  // getSet() and eraseSet(), before using setSet().
  // also, remember to delete it from the model to avoid memory leaks.
  //
#ifdef DEBUG
  assert(str2set[label] == 0);
#endif
  str2set[label] = sset;
}

class ctl_formula_models : public ctl_formula {
  model* m;
  bool evaluated;
  bool result;
  state_id state;
  string label;
  public:
    ctl_formula_models()
    : ctl_formula(MODEL), m(0), evaluated(false), result(false) {}

    void show() {
      cout << "S" << state << " |= " << label << ": ";
      cout << (getResult() ? "Yes" : "No") << endl;
    }

    bool getResult() {
      if (!evaluated) {
        assert(m);
        assert(m->isValidState(state));
        state_set* sset = getSet(label);
        assert(sset);
        result = m->elementOf(state, sset);
        evaluated = true;
      }
      return result;
    }

    void setModel(model* a_model) {
      m = a_model;
      evaluated = false;
    }

    void setState(state_id s) {
      state = s;
      evaluated = false;
    }

    void setLabel(string l) {
      label = l;
      evaluated = false;
    }
};

class ctl_formula_displays : public ctl_formula {
  model* m;
  string label;
  public:
    ctl_formula_displays()
    : ctl_formula(DISPLAY), m(0) {}

    void show() {
      cout << "[[ " << label << " ]]: ";
      assert(m);
      state_set* sset = getSet(label);
      assert(sset);
      m->display(sset);
      cout << endl;
    }

    void setModel(model* a_model) { m = a_model; }

    void setLabel(string l) {
      label = l;
      assert(getSet(label));
    }
};

bool isUnaryOperator(string op);
bool isBinaryOperator(string op);

class ctl_formula_labels : public ctl_formula {
  model* m;
  string label;
  vector<string> formula;
  bool evaluated;
  state_set* result;
  public:
  ctl_formula_labels()
    : ctl_formula(LABEL), m(0), evaluated(false), result(0) {}

  void show() {
#ifdef SHOW_FORMULA_LABELS
    cout << label << " := ";
    for (int i = 0; i < formula.size(); i++) {
      cout << " " << formula[i];
    }
    cout << " (postfix notation) " << endl;
#endif
  }

  state_set* getResult() {
    if (!evaluated) {
      assert(m);
      // evaluate formula in postfix
      // TODO: finish evaluating this expression
      stack<state_set*> operands;
      for (int i = 0; i < formula.size(); i++) {
        string token = formula[i];
        if (isUnaryOperator(token)) {
          assert(!operands.empty());
          state_set* sset = operands.top(); operands.pop();
          // sset can be over-written with the result,
          // see operand case
          if (token == "!") {
            m->NOT(sset, sset);
          } else if (token == "EX") {
            m->EX(sset, sset);
          } else if (token == "EF") {
            m->EF(sset, sset);
          } else if (token == "EG") {
            m->EG(sset, sset);
          } else if (token == "AX") {
            m->AX(sset, sset);
          } else if (token == "AF") {
            m->AF(sset, sset);
          } else {
            assert(token == "AG");
            m->AG(sset, sset);
          }
          operands.push(sset);
        } else if (isBinaryOperator(token)) {
          assert(!operands.empty());
          state_set* sset2 = operands.top(); operands.pop();
          assert(!operands.empty());
          state_set* sset1 = operands.top(); operands.pop();
          // sset1 and sset2 can be over-written with the result,
          // see operand case
          if (token == "&") {
            m->AND(sset1, sset2, sset1);
          } else if (token == "|") {
            m->OR(sset1, sset2, sset1);
          } else if (token == "->") {
            m->IMPLIES(sset1, sset2, sset1);
          } else if (token == "EU") {
            m->EU(sset1, sset2, sset1);
          } else {
            assert(token == "AU");
            m->AU(sset1, sset2, sset1);
          }
          m->deleteSet(sset2);
          operands.push(sset1);
        } else {
          // operand, or label, put on top of stack
          state_set* sset = getSet(formula[i]);
          state_set* rset = m->makeEmptySet();
          m->copy(sset, rset);
          operands.push(rset);
        }
      }

      assert(!operands.empty());
      result = operands.top(); operands.pop();

      // write result to label (over-write if necessary)
      state_set* label_sset = getSet(label);
      if (label_sset != result) {
        eraseSet(label);
        if (label_sset != 0) m->deleteSet(label_sset);
        setSet(label, result);
      }
    
      evaluated = true;
    }
    return result;
  }

  void setModel(model* a_model) {
    if (result) { m->deleteSet(result); result = 0; }
    m = a_model;
    evaluated = false;
  }

  void setLabel(string l) {
    assert(m);
    if (result) { m->deleteSet(result); result = 0; }
    label = l;
    evaluated = false;
    if (0 == getSet(label)) {
      setSet(label, m->makeEmptySet());
    }
  }

  void setFormula(vector<string>& f) {
    if (result) { m->deleteSet(result); result = 0; }
    formula = f;
    evaluated = false;
  }
};


bool read_integer(string& line, int& i, int& integer) {
  if (i < line.size() && isdigit(line[i])) {
    string::size_type sz;
    integer = stoi(line.substr(i), &sz);
    i = (sz == string::npos)? line.size()-1: i+sz-1;
    return true;
  }
  return false;
}


bool read_state_id(string& line, int& i, int& state_id) {
  if (line[i] == 's' || line[i] == 'S') {
    ++i;
    return read_integer(line, i, state_id);
  }
  return false;
}


bool read_string(string& line, int& i, const string& match) {
  if (line.substr(i, match.size()) != match) return false;
  i += match.size() - 1;
  return true;
}


bool read_label(string& line, int& i, string& str) {
  if (i >= line.size()) return false;
  if (!isalpha(line[i])) return false;
  int start = i++;
  while (i < line.size() && (isalnum(line[i]) || line[i] == '_')) i++;
  str = line.substr(start, i-start);
  i--;    // i points to the last character of the string
  return true;
}


int opToVal(string op) {
  return
    op == "->"? 0
    : op == "|"? 1
    : op == "&"? 2
    : (op == "AU" || op == "EU")? 3
    : (op == "!" || op == "AX" || op == "AF" || op == "AG" || op == "EX" || op == "EF" || op == "EG")? 4
    : -1;
}


bool isUnaryOperator(string op) {
  return (op == "!"
      || op == "AX" || op == "AF" || op == "AG"
      || op == "EX" || op == "EF" || op == "EG");
}

bool isBinaryOperator(string op) {
  return (op == "&" || op == "|" || op == "->" || op == "EU" || op == "AU");
}


// op1 is on the operator stack and op2 is the newly discovered operator
bool isHigherPrecedence(string op1, string op2) {
  // Precedence: ascending order
  // ->
  // |
  // &
  // AU, EU
  // !, AX, AF, AG, EX, EF, EG
  // ()
  //
  // Note1: all unary ops have the same precedence,
  //        and from right to left in the CTL formula
  // Note2: the rest of the operations are sorted by precedence,
  //        and from left to right in the CTL formula
  //        (with the earlier operation having a higher precedence).
  return isUnaryOperator(op2)? false: opToVal(op1) >= opToVal(op2);
}


bool read_ctl_operator(string& line, int& i, string& op) {
  if (i >= line.size()) return false;
  if (line[i] == '!' || line[i] == '&' || line[i] == '|') {
    op = line[i]; return true;
  }
  // for the rest of the ctl operators the line must have atleast two characters
  if (i+1 < line.size()) {
    if (line[i] == 'A' || line[i] == 'E') {
      if (line[i+1] == 'X' || line[i+1] == 'F' || line[i+1] == 'G') {
        op = line.substr(i, 2); return true;
      }
      if (isspace(line[i+1]) || line[i+1] == '(') {
        op = line[i]; return true;
      }
    }
    if (line[i] == 'U' && (isspace(line[i+1]) || line[i+1] == '(')) {
      op = line[i]; return true;
    }
    if (line.substr(i, 2) == "->") {
      op = "->"; return true;
    }
  }
  return false;
}


bool read_formula(string& line, int& i, vector<string>& postfix) {
  if (i >= line.size()) return false;
  /*
   * Parsing a formula:
   *
   * Convert infix to postfix (Based on Knuth's algorithm):
   * . From left to right:
   *        . If "(", push to operator stack.
   *        . If ")", pop all operators till "(", and append to postfix vector.
   *                . If no matching "(" is found, signal syntax error.
   *        . If operator,
   *                . while stack.top() is a operator of equal or higher precedence,
   *                        . pop the operator and append to postfix vector.
   *                . push operator onto operator stack.
   *        . If operand, append to postfix vector.
   */

#ifdef DEBUG_FORMULA
  cout << "Reading new formula" << endl;
#endif
  stack<string> operators;
  while (i < line.size() && line[i] != ';') {
    if (isspace(line[i])) { i++; continue; }

    if (line[i] == '(') {
      operators.push("("); i++; continue;
    }
    if (line[i] == ')') {
      while (!operators.empty() && operators.top() != "(")  {
        postfix.push_back(operators.top());
        operators.pop();
      }
      if (operators.empty()) {
        // Did not find a matching "(" in the operators stack, signal error
        cout << "Syntax error: ) found without a matching ( " << endl;
        return false;
      }
      operators.pop(); i++; continue;
    }

    // could be a CTL operator
    string op;
    if (read_ctl_operator(line, i, op)) {
#ifdef DEBUG_FORMULA
      cout << endl << "Found CTL operator: " << op << endl;
#endif
      if (op == "A" || op == "E") {
        operators.push(op); i++; continue;
      } else if (op == "U") {
        // special case: A U, E U
        while (!operators.empty()
            && operators.top() != "("
            && operators.top() != "A"
            && operators.top() != "E") {
          postfix.push_back(operators.top());
          operators.pop();
        }
        // operators.top() must be either A or E
        if (operators.empty()
            || (operators.top() != "A" && operators.top() != "E")) {
          // Did not find a matching A or E in the operators stack, signal error
          cout << "Syntax error: U found without a matching A or E" << endl;
          return false;
        }
        op = operators.top() + op;
        operators.pop();
        operators.push(op);
        i++; continue;
      } else {
        while (!operators.empty()
            && operators.top() != "("
            && operators.top() != "A"
            && operators.top() != "E"
            && isHigherPrecedence(operators.top(), op)) {
          postfix.push_back(operators.top());
          operators.pop();
        }
        operators.push(op); i += op.size(); continue;
      }
    }

    // must be a label, i.e. operand
    string label;
    if (!read_label(line, i, label)) return false;

    if (getSet(label) == 0) {
      // error: unknown label
      cout << "Syntax error: previously undeclared label " << label << endl;
      i = i+1-label.size();
      return false;
    }

    postfix.push_back(label);
    i++;
    // i += label.size();
  }

  while(!operators.empty()) { postfix.push_back(operators.top()); operators.pop(); }

#ifdef DEBUG_FORMULA
  for (int j = 0; j < postfix.size(); j++) {
    cout << postfix[j] << " ";
  }
  cout << endl;
  cout << i << ": " << line[i] << endl;
#endif

  i--;    // set the pointer at the last character of the formula

  return true;
}


void syntax_error(ostream& out, const string& str, int line_number, int col_number,
  const string& line) {
  out << "Syntax error: expecting " << str
    << " at line: " << line_number
    << " and col: " << col_number << endl;
  out << line << endl;
  for (int i = 0; i < col_number; i++) out << " ";
  out << "^" << endl;
}


// parse the input source
model* parse_tokens(int debug_level, istream& source_stream) {
  model* m = 0;
  int num_states = 0;
  state_id s1, s2;
  string line;
  string label;
  vector<ctl_formula*> ctl_formulas;
  ctl_formula* current_formula = 0;
  state_set* sset = 0;
  vector<string> formula;
  fsm_state current_state = INIT;
  int line_number = 0;



  while (getline(source_stream, line)) {
    line_number++;
    for (int i=0; i<line.size(); i++) {
      if (line[i] == '#') break;        // comment; ignore rest of line
      if (isspace(line[i])) continue;   // whitespace; skip

      switch (current_state) {
        case INIT:
          // expecting "KRIPKE"
          if (!read_string(line, i, "KRIPKE")) {
            syntax_error(cout, "keyword KRIPKE", line_number, i, line);
            exit(1);
          }
#ifdef DEBUG
          cout << "KRIPKE" << endl;
#endif
          current_state = KRIPKE;
          m = makeEmptyModel(debug_level);
          if (0==m) return m;
          break;

        case KRIPKE:
          // expecting "STATES"
          if (!read_string(line, i, "STATES")) {
            syntax_error(cout, "keyword STATES", line_number, i, line);
            exit(1);
          }
#ifdef DEBUG
          cout << "STATES";
#endif
          current_state = STATES;
          break;

        case STATES:
          // expecting an integer
          if (!read_integer(line, i, num_states)) {
            syntax_error(cout, "an integer", line_number, i, line);
            exit(1);
          }
#ifdef DEBUG
          cout << " " << num_states;
#endif
          current_state = INTEGER;
          m->setNumStates(num_states);
          break;

        case INTEGER:
          // expecting ARCS
          if (!read_string(line, i, "ARCS")) {
            syntax_error(cout, "keyword ARCS", line_number, i, line);
            exit(1);
          }
#ifdef DEBUG
          cout << endl << "ARCS" << endl;
#endif
          current_state = ARCS;
          break;

        case ARCS:
          // expecting LABELS or a state (s1)
          if (read_string(line, i, "LABELS")) {
            current_state = LABELS;
#ifdef DEBUG
            cout << "LABELS" << endl;
#endif
          } else if (read_state_id(line, i, s1)) {
            current_state = ARCS_S1;
#ifdef DEBUG
            cout << "  " << s1;
#endif
          } else  {
            syntax_error(cout, "keyword LABELS or a state (S*)", line_number, i, line);
            exit(1);
          }
          break;

        case ARCS_S1:
          // expecting '->'
          if (!read_string(line, i, "->")) {
            syntax_error(cout, "->", line_number, i, line);
            exit(1);
          }
#ifdef DEBUG
          cout << " ->";
#endif
          current_state = ARCS_ARROW;
          break;

        case ARCS_ARROW:
          // expecting a state (s2)
          if (!read_state_id(line, i, s2)) {
            syntax_error(cout, "state (S*)", line_number, i, line);
            exit(1);
          }
#ifdef DEBUG
          cout << "  " << s2;
#endif
          current_state = ARCS_S2;
          break;

        case ARCS_S2:
          // expecting ';'
          if (!read_string(line, i, ";")) {
            syntax_error(cout, ";", line_number, i, line);
            exit(1);
          }
#ifdef DEBUG
          cout << ";" << endl;
#endif
          current_state = ARCS;
          m->addArc(s1, s2);
          break;

        case LABELS:
          // expecting CTL or a label
          if (read_string(line, i, "CTL")) {
            current_state = CTL;
#ifdef DEBUG
            cout << "CTL" << endl;
#endif
            if (!m->finish()) {
              cout << "Error: Kripke structure failed to finish\n";
              exit(1);
            }
          } else if (read_label(line, i, label)) {
            current_state = LABELS_L;
#ifdef DEBUG
            cout << "  " << label;
#endif
          } else  {
            syntax_error(cout, "keyword CTL or a label", line_number, i, line);
            exit(1);
          }
          break;

        case LABELS_L:
          // expecting ':'
          if (!read_string(line, i, ":")) {
            syntax_error(cout, ":", line_number, i, line);
            exit(1);
          }
#ifdef DEBUG
          cout << ":";
#endif
          current_state = LABELS_COLON;
          sset = getSet(label);
          eraseSet(label);
          if (sset == 0) {
            sset = m->makeEmptySet();
            setSet(label, sset);
          }
          break;

        case LABELS_COLON:
          // expecting a state or ';'
          if (read_state_id(line, i, s1)) {
            current_state = LABELS_S;
#ifdef DEBUG
            cout << " " << s1;
#endif
            m->addState(s1, sset);
          } else if (read_string(line, i, ";")) {
            current_state = LABELS;
#ifdef DEBUG
            cout << " ;" << endl;
#endif
            sset = 0;
          } else  {
            syntax_error(cout, "a state (S*) or ;", line_number, i, line);
            exit(1);
          }
          break;

        case LABELS_S:
          // expecting a ',' or ';'
          if (read_string(line, i, ",")) {
            current_state = LABELS_COMMA;
#ifdef DEBUG
            cout << " ,";
#endif
          } else if (read_string(line, i, ";")) {
            current_state = LABELS;
#ifdef DEBUG
            cout << " ;" << endl;
#endif
			setSet(label, sset); //
            sset = 0;
          } else  {
            syntax_error(cout, ", or ;", line_number, i, line);
            exit(1);
          }
          break;

        case LABELS_COMMA:
          // expecting a state
          if (!read_state_id(line, i, s1)) {
            syntax_error(cout, "state (S*)", line_number, i, line);
            exit(1);
          }
          current_state = LABELS_S;
#ifdef DEBUG
          cout << " " << s1;
#endif
          m->addState(s1, sset);
          break;

        case CTL:
          // expecting state, label or '[[' 
          formula.clear();

          if (read_state_id(line, i, s1)) {
            current_state = CTL_S;
#ifdef DEBUG
            cout << "  " << s1;
#endif
            if (!m->isValidState(s1)) {
              syntax_error(cout, "a valid state", line_number, i, line);
              exit(1);
            }
            ctl_formula_models* temp = new ctl_formula_models;
            temp->setModel(m);
            temp->setState(s1);
            current_formula = temp;
          } else if (read_label(line, i, label)) {
            current_state = CTL_L;
#ifdef DEBUG
            cout << "  " << label;
#endif
            ctl_formula_labels* temp = new ctl_formula_labels;
            temp->setModel(m);
            temp->setLabel(label);
            current_formula = temp;
          } else if (read_string(line, i, "[[")) {
            current_state = CTL_SET_OPEN;
#ifdef DEBUG
            cout << "  [[";
#endif
            ctl_formula_displays* temp = new ctl_formula_displays;
            temp->setModel(m);
            current_formula = temp;
          } else  {
            syntax_error(cout, "a state, a label, or [[", line_number, i, line);
            exit(1);
          }
          break;

        case CTL_S:
          // expecting '|='
          if (!read_string(line, i, "|=")) {
            syntax_error(cout, "|=", line_number, i, line);
            exit(1);
          }
          current_state = CTL_MODELS;
#ifdef DEBUG
          cout << " !=";
#endif
          break;

        case CTL_MODELS:
          // expecting label
          if (!read_label(line, i, label)) {
            syntax_error(cout, "a label", line_number, i, line);
            exit(1);
          }
          current_state = CTL_S_L;
#ifdef DEBUG
          cout << " " << label;
#endif
          if (getSet(label) == 0) {
            syntax_error(cout, "a previously declared label",
                line_number, i+1-label.size(), line);
            exit(1);
          }
          static_cast<ctl_formula_models*>(current_formula)->setLabel(label);
          break;

        case CTL_S_L:
          // expecting ';'
          if (!read_string(line, i, ";")) {
            syntax_error(cout, ";", line_number, i, line);
            exit(1);
          }
          current_state = CTL;
          ctl_formulas.push_back(current_formula);
          current_formula = 0;
#ifdef DEBUG
          cout << ";" << endl;
#endif
          break;

        case CTL_L:
          // expecting ':='
          if (!read_string(line, i, ":=")) {
            syntax_error(cout, ":=", line_number, i, line);
            exit(1);
          }
          current_state = CTL_ASSIGN;
#ifdef DEBUG
          cout << " :=";
#endif
          break;

        case CTL_ASSIGN:
          // expecting formula
          if (!read_formula(line, i, formula)) {
            syntax_error(cout, "CTL formula", line_number, i, line);
#ifdef DEBUG_FORMULA
            cout << endl;
            for (int j = 0; j < formula.size(); j++) cout << " " << formula[j];
            cout << endl;
#endif
            exit(1);
          }
#ifdef DEBUG_FORMULA
          cout << "Finished reading formula from: " << line << endl;
          cout << "Currently at:                  ";
          for (int j = 0; j < i; j++) cout << " ";
          cout << "^" << endl;
#endif
          current_state = CTL_FORMULA;
#ifdef DEBUG
          for (int j = 0; j < formula.size(); j++) cout << " " << formula[j];
#endif
          static_cast<ctl_formula_labels*>(current_formula)->setFormula(formula);
          break;

        case CTL_FORMULA:
          // expecting ';'
          if (!read_string(line, i, ";")) {
            syntax_error(cout, ";", line_number, i, line);
            exit(1);
          }
          current_state = CTL;
          ctl_formulas.push_back(current_formula);
          current_formula = 0;
#ifdef DEBUG
          cout << ";" << endl;
#endif
          break;

        case CTL_SET_OPEN:
          // expecting label
          if (!read_label(line, i, label)) {
            syntax_error(cout, "a label", line_number, i, line);
            exit(1);
          }
          current_state = CTL_SET_L;
#ifdef DEBUG
          cout << " " << label;
#endif
          if (getSet(label) == 0) {
            syntax_error(cout, "a previously declared label",
                line_number, i+1-label.size(), line);
            exit(1);
          }
          static_cast<ctl_formula_displays*>(current_formula)->setLabel(label);
          break;

        case CTL_SET_L:
          // expecting ']]'
          if (!read_string(line, i, "]]")) {
            syntax_error(cout, "]]", line_number, i, line);
            exit(1);
          }
          current_state = CTL_SET_CLOSE;
#ifdef DEBUG
          cout << "]]";
#endif
          break;

        case CTL_SET_CLOSE:
          // expecting ';'
          if (!read_string(line, i, ";")) {
            syntax_error(cout, ";", line_number, i, line);
            exit(1);
          }
          current_state = CTL;
          ctl_formulas.push_back(current_formula);
          current_formula = 0;
#ifdef DEBUG
          cout << ";" << endl;
#endif
          break;

        default:
          exit(1);
      }

    }
  }

  if (current_state == CTL) {
    current_state = DONE;
  } else {
#ifdef DEBUG
    cout << "Error: reached end of file in state #" << current_state << endl;
#endif
    exit(1);
  }


  // evaluate the CTL formulas
#if 1
  for (int i = 0; i < ctl_formulas.size(); i++) {
    if (ctl_formulas[i]->getType() == LABEL) {
        static_cast<ctl_formula_labels*>(ctl_formulas[i])->getResult();
    }
  }
  for (int i = 0; i < ctl_formulas.size(); i++) {
    switch (ctl_formulas[i]->getType()) {
      case MODEL:
        static_cast<ctl_formula_models*>(ctl_formulas[i])->show();
        break;
      case LABEL:
        static_cast<ctl_formula_labels*>(ctl_formulas[i])->show();
        break;
      case DISPLAY:
        static_cast<ctl_formula_displays*>(ctl_formulas[i])->show();
        break;
      default:
        exit(1);
    }
  }
#endif

  return m;
}

int usage(const char* who)
{
  cout << "\nUsage: " << who << " [-h] [-d debug_level] [input-file]\n\n";
  cout << "\t-h: display this help screen\n\n";
  cout << "\t-d: specify the debug level; a level of 0 (the default)\n";
  cout << "\t    should not display any debugging information\n\n";
  cout << "\tIf an input file is not specified, then the input file is\n";
  cout << "\tread from standard input.\n\n";
  return 1;
}

int main(int argc, const char *argv[])
{
  const char* fn = 0;
  model* m = 0;
  int debuglevel = 0;

  //
  // Process arguments, if any
  //
  for (int i=1; i<argc; i++) {

    if (strcmp("-h", argv[i]) == 0) {
      return usage(argv[0]);
    }

    if (strcmp("-d", argv[i]) == 0) {
      i++;
      if (i>=argc) return usage(argv[0]);
      debuglevel = atoi(argv[i]);
      continue;
    }

    if (fn) return usage(argv[0]);
    fn = argv[i];
  }
  
  if (debuglevel) {
    cout << "Using debug level " << debuglevel << endl;
  }

  //
  // Decide if input is file, or stdin
  //

  if (fn) {
    //
    // Filename specified
    //
    fstream source(fn);
    if ( source.fail() ) {
      cout << "An error has occurred whilst opening "<< fn << endl;
      exit(0);
    }
    m = parse_tokens(debuglevel, source);

  } else {
    //
    // Read from standard input
    //

    m = parse_tokens(debuglevel, cin);
  }

  if (m) {
    delete m;
  } else {
    cout << "Error, null model - did you rewrite function makeEmptyModel()?" << endl;
  }
}

