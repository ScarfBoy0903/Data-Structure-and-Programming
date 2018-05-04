/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr *cirMgr = 0;

enum CirParseError
{
  EXTRA_SPACE,
  MISSING_SPACE,
  ILLEGAL_WSPACE,
  ILLEGAL_NUM,
  ILLEGAL_IDENTIFIER,
  ILLEGAL_SYMBOL_TYPE,
  ILLEGAL_SYMBOL_NAME,
  MISSING_NUM,
  MISSING_IDENTIFIER,
  MISSING_NEWLINE,
  MISSING_DEF,
  CANNOT_INVERTED,
  MAX_LIT_ID,
  REDEF_GATE,
  REDEF_SYMBOLIC_NAME,
  REDEF_CONST,
  NUM_TOO_SMALL,
  NUM_TOO_BIG,

  DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0; // in printint, lineNo needs to ++
static unsigned colNo = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
  switch (err)
  {
  case EXTRA_SPACE:
    cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
         << ": Extra space character is detected!!" << endl;
    break;
  case MISSING_SPACE:
    cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
         << ": Missing space character!!" << endl;
    break;
  case ILLEGAL_WSPACE: // for non-space white space character
    cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
         << ": Illegal white space char(" << errInt
         << ") is detected!!" << endl;
    break;
  case ILLEGAL_NUM:
    cerr << "[ERROR] Line " << lineNo + 1 << ": Illegal "
         << errMsg << "!!" << endl;
    break;
  case ILLEGAL_IDENTIFIER:
    cerr << "[ERROR] Line " << lineNo + 1 << ": Illegal identifier \""
         << errMsg << "\"!!" << endl;
    break;
  case ILLEGAL_SYMBOL_TYPE:
    cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
         << ": Illegal symbol type (" << errMsg << ")!!" << endl;
    break;
  case ILLEGAL_SYMBOL_NAME:
    cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
         << ": Symbolic name contains un-printable char(" << errInt
         << ")!!" << endl;
    break;
  case MISSING_NUM:
    cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
         << ": Missing " << errMsg << "!!" << endl;
    break;
  case MISSING_IDENTIFIER:
    cerr << "[ERROR] Line " << lineNo + 1 << ": Missing \""
         << errMsg << "\"!!" << endl;
    break;
  case MISSING_NEWLINE:
    cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
         << ": A new line is expected here!!" << endl;
    break;
  case MISSING_DEF:
    cerr << "[ERROR] Line " << lineNo + 1 << ": Missing " << errMsg
         << " definition!!" << endl;
    break;
  case CANNOT_INVERTED:
    cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
         << ": " << errMsg << " " << errInt << "(" << errInt / 2
         << ") cannot be inverted!!" << endl;
    break;
  case MAX_LIT_ID:
    cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
         << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
         << endl;
    break;
  case REDEF_GATE:
    cerr << "[ERROR] Line " << lineNo + 1 << ": Literal \"" << errInt
         << "\" is redefined!!" << endl;
    break;
  case REDEF_SYMBOLIC_NAME:
    cerr << "[ERROR] Line " << lineNo + 1 << ": Symbolic name for \""
         << errMsg << errInt << "\" is redefined!!" << endl;
    break;
  case REDEF_CONST:
    cerr << "[ERROR] Line " << lineNo + 1 << ", Col " << colNo + 1
         << ": Cannot redefine constant (" << errInt << ")!!" << endl;
    break;
  case NUM_TOO_SMALL:
    cerr << "[ERROR] Line " << lineNo + 1 << ": " << errMsg
         << " is too small (" << errInt << ")!!" << endl;
    break;
  case NUM_TOO_BIG:
    cerr << "[ERROR] Line " << lineNo + 1 << ": " << errMsg
         << " is too big (" << errInt << ")!!" << endl;
    break;
  default:
    break;
  }
  return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool CirMgr::readCircuit(const string &fileName)
{
  /*----------------------clear----------------------------*/
  gate_limit = PI_num = PO_num = 0;
  lineNo = colNo = errInt = 0;
  bool is_write_c = false;
  errMsg.clear();
  list.clear();
  floating.clear();
  undefine.clear();
  defined_not_used.clear();
  write_PI_PO.clear();
  /*----------------------clear----------------------------*/

  /*----------------------read file----------------------------*/
  fstream file;
  file.open(fileName);
  if (!file)
  {
    cerr << "Cannot open design \"" + fileName + "\"!!" << endl;
    return false;
  }
  vector<string> content;
  content.reserve(1000);
  string str;

  while (getline(file, str))
  {
    if (str == "c")
    {
      is_write_c = true;
      break;
    }
    content.push_back(str);
  }
  file.close();
  /*----------------------read file----------------------------*/

  /*----------------------take PI/PO/AIG number----------------------------*/
  if (content[0].size() < 4)
  {
    errMsg = "\"aag\"";
    return parseError(MISSING_NUM);
  }
  if (!(content[0][0] == 'a' && content[0][1] == 'a' && content[0][2] == 'g' && content[0][3] == ' '))
  {
    return parseError(ILLEGAL_IDENTIFIER);
  }
  vector<string> list_num;
  split(content[0], " ", list_num);
  if (list_num.size() != 6)
  {
    errMsg = "number of variable(s)";
    return parseError(MISSING_NUM);
  }
  for (int i = 1; i < list_num.size(); i++)
  {
    for (int j = 0; j < list_num[i].size(); j++)
    {
      if (list_num[i][j] == char(9))
      {
        return parseError(MISSING_SPACE);
      }
      if (!isdigit(list_num[i][j]))
      {
        errMsg = "numbers : \"" + list_num[i] + "\"";
        return parseError(ILLEGAL_NUM);
      }
    }
  }
  gate_limit = stoi(list_num[1]);
  PI_num = stoi(list_num[2]);
  PO_num = stoi(list_num[4]);
  AIG_num = stoi(list_num[5]);
  if (stoi(list_num[3]))
  {
    errMsg = "latches";
    return parseError(ILLEGAL_NUM);
  }
  if (gate_limit < PI_num + AIG_num)
  {
    errMsg = "Number of variables";
    errInt = gate_limit;
    return parseError(NUM_TOO_SMALL);
  }
  /*----------------------take PI/PO/AIG number----------------------------*/

  /*----------------------detect space----------------------------*/
  if (content.size() <= PI_num + PO_num + AIG_num)
  {
    errMsg = "GATE";
    return parseError(MISSING_DEF);
  }
  for (int i = 0; i <= PI_num + PO_num + AIG_num; i++)
  {
    if (i >= 1 && i <= PI_num + PO_num)
    {
      if (!is_str_digit(content[i]))
      {
        errMsg = "(" + content[i] + ")";
        return parseError(ILLEGAL_NUM);
      }
    }
    else if (i > PI_num + PO_num && i <= PI_num + PO_num + AIG_num)
    {
      vector<string> token;
      split(content[i], " ", token);
      if (token.size() != 3)
      {
        lineNo = i;
        return parseError(MISSING_NEWLINE);
      }
      if (!is_str_digit(token[0]))
      {
        errMsg = "(" + token[0] + ")";
        return parseError(ILLEGAL_NUM);
      }
      if (!is_str_digit(token[1]))
      {
        errMsg = "(" + token[1] + ")";
        return parseError(ILLEGAL_NUM);
      }
      if (!is_str_digit(token[2]))
      {
        errMsg = "(" + token[2] + ")";
        return parseError(ILLEGAL_NUM);
      }
    }
    if (content[i][0] == ' ')
    {
      lineNo = i;
      return parseError(EXTRA_SPACE);
    }
    else if (content[i][0] == char(9))
    {
      lineNo = i;
      return parseError(MISSING_SPACE);
    }
    if (content[i][content[i].size() - 1] == ' ')
    {
      lineNo = i;
      colNo = content[i].size() - 1;
      return parseError(MISSING_NEWLINE);
    }
    else if (content[i][content[i].size() - 1] == char(9))
    {
      lineNo = i;
      colNo = content[i].size() - 1;
      return parseError(MISSING_SPACE);
    }
    for (int j = 0; j < content[i].size() - 1; j++)
    {
      if (content[i][j] == ' ' && content[i][j + 1] == ' ')
      {
        lineNo = i;
        colNo = j;
        return parseError(EXTRA_SPACE);
      }
      else if (content[i][j] == char(9) || content[i][j + 1] == char(9))
      {
        lineNo = i;
        colNo = j;
        return parseError(MISSING_SPACE);
      }
    }
  }
  if (!is_write_c)
  {
    string p;
    fstream fp;
    fp.open(fileName);
    getline(fp, p, '\0');
    fp.close();
    if (content.size() == AIG_num + PI_num + PO_num + 1 && p[p.size() - 1] != char(10))
    {
      lineNo = AIG_num + PI_num + PO_num;
      colNo = content[content.size() - 1].size() - 1;
      return parseError(MISSING_NEWLINE);
    }
    for (int i = 0; i < p.size() - 1; i++)
    {
      if (p[i] == '\n' && p[i + 1] == '\n')
      {
        return parseError(ILLEGAL_SYMBOL_TYPE);
      }
    }
  }
  /*----------------------detect format error----------------------------*/

  /*----------------------construct PI----------------------------*/
  list.push_back(new CirPI(0, 0, CONST_GATE));
  write_PI_PO.push_back(content[0]);
  list[0]->_line = 0;
  for (int i = 1; i <= PI_num; i++)
  {
    write_PI_PO.push_back(content[i]);
    int literal = stoi(content[i]);
    if (literal > gate_limit * 2)
    {
      lineNo = i;
      errInt = literal;
      return parseError(MAX_LIT_ID);
    }
    for (int j = 0; j < list.size(); j++)
    {
      if (literal == list[j]->_literal || literal == list[j]->_literal + 1)
      {
        lineNo = i;
        errInt = literal;
        return parseError(REDEF_GATE);
      }
    }
    CirGate *temp = new CirPI(literal, literal / 2);
    temp->_line = i + 1;
    list.push_back(temp);
  }
  /*----------------------construct PI----------------------------*/

  /*----------------------construct AIG/UNDEFINE----------------------------*/
  for (int i = 1; i <= AIG_num; i++)
  {
    vector<string> token;
    int literal = 0;
    split(content[PI_num + PO_num + i], " ", token);
    literal = stoi(token[0]);
    if (literal > gate_limit * 2)
    {
      lineNo = PI_num + PO_num + i;
      errInt = literal;
      return parseError(MAX_LIT_ID);
    }
    for (int j = 0; j < list.size(); j++)
    {
      if (literal == list[j]->_literal || literal == list[j]->_literal + 1)
      {
        lineNo = PI_num + PO_num + i;
        errInt = literal;
        return parseError(REDEF_GATE);
      }
    }
    CirGate *AIG = new CirAIG(literal, literal / 2);
    AIG->_line = PI_num + PO_num + i + 1;
    list.push_back(AIG);
  }
  for (int i = 1; i <= AIG_num; i++)
  {
    vector<string> token;
    int fanin1 = 0;
    int fanin2 = 0;
    bool fanin1_match = false;
    bool fanin2_match = false;
    split(content[PI_num + PO_num + i], " ", token);
    fanin1 = stoi(token[1]);
    fanin2 = stoi(token[2]);
    if (fanin1 > gate_limit * 2 + 1)
    {
      lineNo = PI_num + PO_num + i;
      errInt = fanin1;
      return parseError(MAX_LIT_ID);
    }
    if (fanin2 > gate_limit * 2 + 1)
    {
      lineNo = PI_num + PO_num + i;
      errInt = fanin2;
      return parseError(MAX_LIT_ID);
    }
    CirGate *AIG = list[PI_num + i];
    for (int j = 0; j < list.size(); j++)
    {
      if (fanin1 == list[j]->_literal)
      {
        connect(list[j], AIG);
        fanin1_match = true;
        break;
      }
      else if (fanin1 == list[j]->_literal + 1)
      {
        connect(list[j], AIG);
        AIG->fanin1_invert = true;
        fanin1_match = true;
        break;
      }
    }
    if (!fanin1_match)
    {
      if (!(fanin1 % 2))
      {
        CirGate *temp = new CirAIG(fanin1, fanin1 / 2, UNDEF_GATE);
        undefine.push_back(temp);
        connect(temp, AIG);
      }
      else
      {
        fanin1--;
        CirGate *temp = new CirAIG(fanin1, fanin1 / 2, UNDEF_GATE);
        undefine.push_back(temp);
        connect(temp, AIG);
        AIG->fanin1_invert = true;
      }
    }
    for (int j = 0; j < list.size(); j++)
    {
      if (fanin2 == list[j]->_literal)
      {
        connect(list[j], AIG);
        fanin2_match = true;
        break;
      }
      else if (fanin2 == list[j]->_literal + 1)
      {
        connect(list[j], AIG);
        AIG->fanin2_invert = true;
        fanin2_match = true;
        break;
      }
    }
    if (!fanin2_match)
    {
      if (!(fanin2 % 2))
      {
        CirGate *temp = new CirAIG(fanin2, fanin2 / 2, UNDEF_GATE);
        undefine.push_back(temp);
        connect(temp, AIG);
      }
      else
      {
        fanin2--;
        CirGate *temp = new CirAIG(fanin2, fanin2 / 2, UNDEF_GATE);
        undefine.push_back(temp);
        connect(temp, AIG);
        AIG->fanin2_invert = true;
      }
    }
  }
  /*----------------------construct AIG/UNDEFINE----------------------------*/

  /*----------------------construct PO----------------------------*/
  for (int i = 1; i <= PO_num; i++)
  {
    bool fanin_match = false;
    write_PI_PO.push_back(content[PI_num + i]);
    int ID = gate_limit + i;
    CirGate *PO = new CirPO(2 * ID, ID);
    int literal = stoi(content[PI_num + i]);
    if (literal > gate_limit * 2 + 1)
    {
      lineNo = i + PI_num;
      errInt = literal;
      return parseError(MAX_LIT_ID);
    }
    int _size = list.size();
    for (int j = 0; j < _size; j++)
    {
      if (literal == list[j]->_literal)
      {
        connect(list[j], PO);
        fanin_match = true;
        break;
      }
      else if (literal == list[j]->_literal + 1)
      {
        PO->fanin1_invert = true;
        connect(list[j], PO);
        fanin_match = true;
        break;
      }
    }
    if (!fanin_match)
    {
      if (!(literal % 2))
      {
        CirGate *temp = new CirAIG(literal, literal / 2, UNDEF_GATE);
        undefine.push_back(temp);
        connect(temp, PO);
      }
      else
      {
        literal--;
        CirGate *temp = new CirAIG(literal, literal / 2, UNDEF_GATE);
        undefine.push_back(temp);
        connect(temp, PO);
        PO->fanin1_invert = true;
      }
    }
    PO->_line = PI_num + i + 1;
    list.push_back(PO);
  }
  /*----------------------construct PO----------------------------*/

  /*----------------------detect DEFINED_NOT_USE/FLOATING----------------------------*/
  for (int i = 0; i < list.size(); i++)
  {
    if (list[i]->_gate_type == AIG_GATE)
    {
      if (list[i]->_fanin[0]->_gate_type == UNDEF_GATE && list[i]->_fanin[1]->_gate_type != UNDEF_GATE)
      {
        list[i]->_gatecase = FLOTING_ONE_GATE;
        floating.push_back(list[i]->_ID);
      }
      else if (list[i]->_fanin[0]->_gate_type != UNDEF_GATE && list[i]->_fanin[1]->_gate_type == UNDEF_GATE)
      {
        list[i]->_gatecase = FLOTING_ONE_GATE;
        floating.push_back(list[i]->_ID);
      }
      else if (list[i]->_fanin[0]->_gate_type == UNDEF_GATE && list[i]->_fanin[1]->_gate_type == UNDEF_GATE)
      {
        list[i]->_gatecase = FLOTING_TWO_GATE;
        floating.push_back(list[i]->_ID);
      }
    }
    else if (list[i]->_gate_type == PO_GATE && list[i]->_fanin[0]->_gate_type == UNDEF_GATE)
    {
      list[i]->_gatecase = FLOTING_ONE_GATE;
      floating.push_back(list[i]->_ID);
    }
    if ((list[i]->_gate_type == AIG_GATE || list[i]->_gate_type == PI_GATE) &&
        list[i]->_fanout.size() == 0)
    {
      defined_not_used.push_back(list[i]->_ID);
    }
  }
  /*----------------------detect DEFINED_NOT_USE/FLOATING----------------------------*/

  /*----------------------set _gatename----------------------------*/
  vector<int> _symbol_record_PI;
  vector<int> _symbol_record_PO;
  for (int i = PI_num + PO_num + AIG_num + 1; i < content.size(); i++)
  {
    vector<string> token;
    if (content[i][0] == ' ' || content[i][0] == char(9))
    {
      lineNo = i;
      return parseError(EXTRA_SPACE);
    }
    split(content[i], " ", token);
    if (token[0].size() == 1)
    {
      lineNo = i;
      colNo = 1;
      return parseError(EXTRA_SPACE);
    }
    if (token[0][0] == 'i')
    {
      if (!is_str_digit(&token[0][1]))
      {
        lineNo = i;
        errMsg = token[0];
        return parseError(ILLEGAL_SYMBOL_TYPE);
      }
      if (token.size() == 1 && (token[0].size() == content[i].size() || token[0].size() == content[i].size() - 1))
      {
        lineNo = i;
        errMsg = "symbolic name";
        return parseError(MISSING_NUM);
      }
      string temp = &content[i][token[0].size() + 1];
      for (int j = 0; j < temp.size(); j++)
      {
        if (int(temp[j]) <= 31)
        {
          lineNo = i;
          colNo = j;
          errInt = int(temp[j]);
          return parseError(ILLEGAL_SYMBOL_NAME);
        }
      }
      int set_ID = stoi(&token[0][1]);
      if (set_ID >= PI_num)
      {
        errMsg = "PI";
        lineNo = i;
        errInt = set_ID;
        return parseError(NUM_TOO_BIG);
      }
      for (int j = 0; j < _symbol_record_PI.size(); j++)
      {
        if (set_ID == _symbol_record_PI[j])
        {
          lineNo = i;
          errMsg = "i";
          errInt = set_ID;
          return parseError(REDEF_SYMBOLIC_NAME);
        }
      }
      _symbol_record_PI.push_back(set_ID);
      list[1 + set_ID]->_gatename = "\"" + temp + "\"";
      list[1 + set_ID]->_name = "(" + temp + ")";
      list[1 + set_ID]->_symbol = content[i];
    }
    else if (token[0][0] == 'o')
    {
      if (!is_str_digit(&token[0][1]))
      {
        lineNo = i;
        errMsg = token[0];
        return parseError(ILLEGAL_SYMBOL_TYPE);
      }
      if (token.size() == 1 && (token[0].size() == content[i].size() || token[0].size() == content[i].size() - 1))
      {
        lineNo = i;
        errMsg = "symbolic name";
        return parseError(MISSING_NUM);
      }
      string temp = &content[i][token[0].size() + 1];
      for (int j = 0; j < temp.size(); j++)
      {
        if (int(temp[j]) <= 31)
        {
          lineNo = i;
          colNo = j;
          errInt = int(temp[j]);
          return parseError(ILLEGAL_SYMBOL_NAME);
        }
      }
      int set_ID = stoi(&token[0][1]);
      if (set_ID >= PO_num)
      {
        errMsg = "PO";
        lineNo = i;
        errInt = set_ID;
        return parseError(NUM_TOO_BIG);
      }
      for (int j = 0; j < _symbol_record_PO.size(); j++)
      {
        if (set_ID == _symbol_record_PO[j])
        {
          lineNo = i;
          errMsg = "o";
          errInt = set_ID;
          return parseError(REDEF_SYMBOLIC_NAME);
        }
      }
      _symbol_record_PO.push_back(set_ID);
      list[1 + PI_num + AIG_num + set_ID]->_gatename = "\"" + temp + "\"";
      list[1 + PI_num + AIG_num + set_ID]->_name = "(" + temp + ")";
      list[1 + PI_num + AIG_num + set_ID]->_symbol = content[i];
    }
    else
    {
      lineNo = i;
      errMsg = token[0];
      return parseError(ILLEGAL_SYMBOL_TYPE);
    }
  }
  /*----------------------set _gatename----------------------------*/

  return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void CirMgr::printSummary() const
{
  cout << endl;
  cout << "Circuit Statistics\n==================\n"
       << "  PI" << setw(12) << right << PI_num << endl
       << "  PO" << setw(12) << right << PO_num << endl
       << "  AIG" << setw(11) << right << AIG_num << endl
       << "------------------\n"
       << "  Total" << setw(9) << right << PI_num + PO_num + AIG_num << endl;
}

void CirMgr::printNetlist()
{
  cout << endl;
  vector<CirGate *> _record;
  int idx = 0;
  for (int i = 1; i <= PO_num; i++)
  {
    _netlist_traversal(list[i + PI_num + AIG_num], idx, _record);
  }
  for (int i = 0; i < _record.size(); i++)
  {
    _record[i]->is_visited = false;
  }
}

void CirMgr::printPIs() const
{
  cout << "PIs of the circuit:";
  for (int i = 1; i <= PI_num; i++)
  {
    cout << " " << list[i]->_ID;
  }
  cout << endl;
}

void CirMgr::printPOs() const
{
  cout << "POs of the circuit:";
  for (int i = 1; i <= PO_num; i++)
  {
    cout << " " << list[i + PI_num + AIG_num]->_ID;
  }
  cout << endl;
}

void CirMgr::printFloatGates() const
{
  if (floating.size())
  {
    cout << "Gates with floating fanin(s):";
    for (int i = 0; i < floating.size(); i++)
    {
      cout << " " << floating[i];
    }
    cout << endl;
  }
  if (defined_not_used.size())
  {
    cout << "Gates defined but not used  :";
    for (int i = 0; i < defined_not_used.size(); i++)
    {
      cout << " " << defined_not_used[i];
    }
    cout << endl;
  }
}

void CirMgr::writeAag(ostream &outfile)
{
  int PI = 0;
  int PO = 0;
  int AIG = 0;
  vector<CirGate *> _record;
  for (int i = 1; i <= PO_num; i++)
  {
    _write_traversal(list[i + PI_num + AIG_num], _record);
  }
  for (int i = 0; i < _record.size(); i++)
  {
    if (_record[i]->_gate_type == AIG_GATE)
    {
      AIG++;
    }
  }
  outfile << "aag " << gate_limit << " " << PI_num << " " << '0' << " " << PO_num << " " << AIG << endl;
  for (int i = 1; i < write_PI_PO.size(); i++)
  {
    outfile << write_PI_PO[i] << endl;
  }
  for (int i = 0; i < _record.size(); i++)
  {
    if (_record[i]->_gate_type == AIG_GATE)
    {
      outfile << _record[i]->_literal << " ";
      if (_record[i]->fanin1_invert)
      {
        outfile << _record[i]->_fanin[0]->_literal + 1 << " ";
      }
      else
      {
        outfile << _record[i]->_fanin[0]->_literal << " ";
      }
      if (_record[i]->fanin2_invert)
      {
        outfile << _record[i]->_fanin[1]->_literal + 1 << endl;
      }
      else
      {
        outfile << _record[i]->_fanin[1]->_literal << endl;
      }
    }
    _record[i]->is_visited = false;
  }
  for (int i = 1; i < list.size(); i++)
  {
    if (list[i]->_symbol != "" && (list[i]->_gate_type == PI_GATE || list[i]->_gate_type == PO_GATE))
    {
      outfile << list[i]->_symbol << endl;
    }
  }

  outfile << "c\n";
  outfile << "AAG output by Chung-Yang (Ric) Huang\n";
}

CirGate *CirMgr::getGate(unsigned gid) const
{
  for (int i = 0; i < list.size(); i++)
  {
    if (list[i]->_ID == gid)
    {
      return list[i];
    }
  }
  for (int i = 0; i < undefine.size(); i++)
  {
    if (undefine[i]->_ID == gid)
    {
      return undefine[i];
    }
  }
  return 0;
}

void CirMgr::split(const string &s, const char *delim, vector<string> &v)
{
  char *dup = strdup(s.c_str());
  char *token = strtok(dup, delim);
  while (token != NULL)
  {
    v.push_back(string(token));
    token = strtok(NULL, delim);
  }
  free(dup);
}

void CirMgr::connect(CirGate *fan_in, CirGate *AIG)
{
  AIG->_fanin.push_back(fan_in);
  fan_in->_fanout.push_back(AIG);
}

void CirMgr::_netlist_traversal(CirGate *gate, int &idx, vector<CirGate *> &_record)
{
  if (gate->is_visited)
  {
    return;
  }
  if (gate->_gate_type == PI_GATE || gate->_gate_type == CONST_GATE)
  {
    printData(gate, idx);
    gate->is_visited = true;
    _record.push_back(gate);
    idx++;
  }
  else if (gate->_gate_type == PO_GATE)
  {
    if (!gate->_fanin[0]->is_visited)
    {
      _netlist_traversal(gate->_fanin[0], idx, _record);
    }
    printData(gate, idx);
    gate->is_visited = true;
    _record.push_back(gate);
    idx++;
  }
  else if (gate->_gate_type == AIG_GATE)
  {
    if (!gate->_fanin[0]->is_visited)
    {
      _netlist_traversal(gate->_fanin[0], idx, _record);
    }
    if (!gate->_fanin[1]->is_visited)
    {
      _netlist_traversal(gate->_fanin[1], idx, _record);
    }
    printData(gate, idx);
    gate->is_visited = true;
    _record.push_back(gate);
    idx++;
  }
  else
  {
    return;
  }
}

void CirMgr::_write_traversal(CirGate *gate, vector<CirGate *> &_record)
{
  if (gate->is_visited)
  {
    return;
  }
  if (gate->_gate_type == PI_GATE || gate->_gate_type == CONST_GATE)
  {
    gate->is_visited = true;
    _record.push_back(gate);
  }
  else if (gate->_gate_type == PO_GATE)
  {
    if (!gate->_fanin[0]->is_visited)
    {
      _write_traversal(gate->_fanin[0], _record);
    }
    gate->is_visited = true;
    _record.push_back(gate);
  }
  else if (gate->_gate_type == AIG_GATE)
  {
    if (!gate->_fanin[0]->is_visited)
    {
      _write_traversal(gate->_fanin[0], _record);
    }
    if (!gate->_fanin[1]->is_visited)
    {
      _write_traversal(gate->_fanin[1], _record);
    }
    gate->is_visited = true;
    _record.push_back(gate);
  }
  else
  {
    return;
  }
}

void CirMgr::printData(CirGate *gate, int idx)
{
  if (gate->_gate_type == PI_GATE)
  {
    if (gate->_name == "")
    {
      cout << "[" << idx << "] PI  " << gate->_ID << endl;
    }
    else
    {
      cout << "[" << idx << "] PI  " << gate->_ID << " " << gate->_name << endl;
    }
  }
  else if (gate->_gate_type == PO_GATE)
  {
    cout << "[" << idx << "] PO  " << gate->_ID << " ";
    if (gate->_fanin[0]->_gate_type == UNDEF_GATE)
    {
      cout << "*";
    }
    if (gate->fanin1_invert)
    {
      cout << "!";
    }
    if (gate->_name == "")
    {
      cout << gate->_fanin[0]->_ID << endl;
    }
    else
    {
      cout << gate->_fanin[0]->_ID << " " << gate->_name << endl;
    }
  }
  else if (gate->_gate_type == AIG_GATE)
  {
    cout << "[" << idx << "] AIG " << gate->_ID << " ";
    if (gate->_fanin[0]->_gate_type == UNDEF_GATE)
    {
      cout << "*";
    }
    if (gate->fanin1_invert)
    {
      cout << "!";
    }
    cout << gate->_fanin[0]->_ID << " ";
    if (gate->_fanin[1]->_gate_type == UNDEF_GATE)
    {
      cout << "*";
    }
    if (gate->fanin2_invert)
    {
      cout << "!";
    }
    cout << gate->_fanin[1]->_ID << endl;
  }
  else if (gate->_gate_type == CONST_GATE)
  {
    cout << "[" << idx << "] CONST0" << endl;
  }
}

bool CirMgr::is_str_digit(string str)
{
  for (int i = 0; i < str.size(); i++)
  {
    if (!isdigit(str[i]))
    {
      return false;
    }
  }
  return true;
}