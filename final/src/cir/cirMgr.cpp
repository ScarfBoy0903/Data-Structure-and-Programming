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
#include <algorithm>
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
         << "\" is redefined, previously defined as "
         << errGate->getTypeStr() << " in line " << errGate->getLineNo()
         << "!!" << endl;
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
  /* error handle and return false*/
  lineNo = 0;
  colNo = 0;
  DFS_list.clear();
  ifstream ifs(fileName.c_str());
  if (!ifs.is_open())
  {
    cout << "Can't open design \"" << fileName << "\"!" << endl;
    return false;
  }
  string str;
  char ch;
  int literal, fanin;
  //checking aag
  ifs.get(ch);

  //lineNo = 0:
  //empty file:
  if (ifs.eof())
  {
    errMsg = "aag";
    return parseError(MISSING_IDENTIFIER);
  }
  //1st char == space/tab
  if (ch == ' ')
  {
    colNo = 0;
    return parseError(EXTRA_SPACE);
  }
  if (int(ch) == 9)
  {
    colNo = 0;
    errInt = 9;
    return parseError(ILLEGAL_WSPACE);
  }
  //1st char != a
  if (ch != 'a')
  {
    ifs.unget();
    ifs >> str;
    errMsg = str;
    return parseError(ILLEGAL_IDENTIFIER);
  }
  if (ch == 'a')
  {
    ifs.unget();
    ifs >> str;
    //aa
    if (str.size() < 3)
    {
      errMsg = str;
      return parseError(ILLEGAL_IDENTIFIER);
    }
    string test = str.substr(0, 3);
    if (test != "aag")
    {
      errMsg = str;
      return parseError(ILLEGAL_IDENTIFIER);
    }
    if (str.size() > 3)
    {
      //aag3
      if (isdigit(str[3]))
      {
        colNo = 3;
        return parseError(MISSING_SPACE);
      }
      //aagasd
      /*colNo = 3; */ errMsg = str;
      return parseError(ILLEGAL_IDENTIFIER);
    }
  }
  ifs.get(ch);
  if (ch == '\n')
  {
    errMsg = "numbers of variables";
    return parseError(MISSING_NUM);
  }
  //end parsing "aag"
  //parsing M I L O A
  colNo = 4;
  for (int index = 0; index < 5; ++index)
  {
    ifs.get(ch);
    if (ch == ' ')
    {
      return parseError(EXTRA_SPACE);
    }
    if (int(ch) == 9)
    {
      return parseError(ILLEGAL_WSPACE);
    }
    if (index == 0)
    {
      //aag + space + '\n'
      if (ch == '\n')
      {
        errMsg = "number of variables";
        return parseError(MISSING_NUM);
      }
      ifs.unget();
      ifs >> str;
      if (!myStr2Int(str, m) || m < 0)
      {
        errMsg = "numbers of variables (" + str + ")";
        return parseError(ILLEGAL_NUM);
      }
    }
    else if (index == 1)
    {
      //aag m + space + '\n'
      if (ch == '\n')
      {
        errMsg = "number of PIs";
        return parseError(MISSING_NUM);
      }
      ifs.unget();
      ifs >> str;
      if (!myStr2Int(str, i) || i < 0)
      {
        errMsg = "numbers of PIs (" + str + ")";
        return parseError(ILLEGAL_NUM);
      }
    }
    else if (index == 2)
    {
      //aag m i + space + '\n'
      if (ch == '\n')
      {
        errMsg = "number of POs";
        return parseError(MISSING_NUM);
      }
      ifs.unget();
      ifs >> str;
      if (!myStr2Int(str, l) || l < 0)
      {
        errMsg = "numbers of latches (" + str + ")";
        return parseError(ILLEGAL_NUM);
      }
    }
    else if (index == 3)
    {
      //aag m i l + space + '\n'
      if (ch == '\n')
      {
        errMsg = "number of latches";
        return parseError(MISSING_NUM);
      }
      ifs.unget();
      ifs >> str;
      if (!myStr2Int(str, o) || o < 0)
      {
        errMsg = "numbers of POs (" + str + ")";
        return parseError(ILLEGAL_NUM);
      }
    }
    else if (index == 4)
    {
      //aag m i l o + space + '\n'
      if (ch == '\n')
      {
        errMsg = "number of AIGs";
        return parseError(MISSING_NUM);
      }
      ifs.unget();
      ifs >> str;
      if (!myStr2Int(str, a) || a < 0)
      {
        errMsg = "numbers of AIG (" + str + ")";
        return parseError(ILLEGAL_NUM);
      }
    }
    colNo = colNo + str.size();
    ifs.get(ch);
    //contains the situation of "EOF"
    if (index == 4 && ch != '\n')
    {
      return parseError(MISSING_NEWLINE);
    }
    //num + tab + num
    if (index == 4 && ch == '\n')
      break; //skip space-checking
    else if (ch != ' ')
    {
      return parseError(MISSING_SPACE);
    }
    ++colNo;
  }
  if (l != 0)
  {
    errMsg = "latches";
    return parseError(ILLEGAL_NUM);
  }
  if (m < l + a + i)
  {
    errMsg = "Number of variables";
    errInt = m;
    return parseError(NUM_TOO_SMALL);
  }
  gatenum = m + o + 1;
  gatelist = new CirGate[gatenum];
  gatelist[0]._type = CONST_GATE;
  gatelist[0].line = 0;
  ++lineNo;
  //deal with PI:
  for (int index = 0; index < i; index++)
  {
    // 1st char for PI def.
    colNo = 0;
    ifs.get(ch);
    if (ch == '\n')
    {
      errMsg = "PI";
      return parseError(MISSING_DEF);
    }
    if (ch == ' ')
    {
      return parseError(EXTRA_SPACE);
    }
    if (int(ch) == 9)
    {
      return parseError(ILLEGAL_WSPACE);
    }
    ifs.unget();
    ifs >> str;
    colNo = colNo + str.size();
    if (!myStr2Int(str, literal) || literal < 0)
    {
      errMsg = "Illegal PI literal ID (" + str + ")";
      return parseError(ILLEGAL_NUM);
    }
    if (literal / 2 == 0)
    {
      colNo = 0;
      errInt = literal;
      return parseError(REDEF_CONST);
    }
    if (literal / 2 > m)
    {
      colNo = 0;
      errInt = literal;
      return parseError(MAX_LIT_ID);
    }
    if (literal % 2 == 1)
    {
      colNo = 0;
      errInt = literal;
      errMsg = "PI";
      return parseError(CANNOT_INVERTED);
    }
    std::vector<unsigned>::iterator it;
    it = find(PI_list.begin(), PI_list.end(), unsigned(literal / 2));
    if (it != PI_list.end())
    {
      errInt = literal;
      errGate = getGate(*it);
      return parseError(REDEF_GATE);
    }
    PI_list.push_back(literal / 2);
    gatelist[literal / 2]._type = PI_GATE;
    gatelist[literal / 2].line = lineNo + 1;
    ifs.get(ch);
    //contains the situation of "EOF"
    if (ch != '\n')
    {
      return parseError(MISSING_NEWLINE);
    }
    colNo = 0;
    ++lineNo;
  }

  //deal with PO:
  for (int index = 0; index < o; index++)
  {
    literal = (m + 1 + index) * 2; //literal of (index)th output
    ifs.get(ch);
    if (ch == '\n')
    {
      errMsg = "PO";
      return parseError(MISSING_DEF);
    }
    if (ch == ' ')
    {
      return parseError(EXTRA_SPACE);
    }
    if (int(ch) == 9)
    {
      errInt = 9;
      return parseError(ILLEGAL_WSPACE);
    }
    ifs.unget();
    ifs >> str;
    if (!myStr2Int(str, fanin))
    {
      errMsg = "Illegal PO literal ID (" + str + ")";
      return parseError(ILLEGAL_NUM);
    }
    if (fanin / 2 > m)
    {
      errInt = fanin;
      return parseError(MAX_LIT_ID);
    }
    PO_list.push_back(literal / 2);
    gatelist[literal / 2]._type = PO_GATE;
    gatelist[literal / 2].line = lineNo + 1;
    connect(fanin, literal);
    ifs.get(ch);
    if (ch != '\n')
    {
      return parseError(MISSING_NEWLINE);
    }
    colNo = 0;
    ++lineNo;
  }
  //deal with AIG:
  for (int index = 0; index < a; index++)
  {
    //lineNo++;
    ifs.get(ch);
    if (ch == '\n')
    {
      errMsg = "AIG";
      return parseError(MISSING_DEF);
    }
    if (ch == ' ')
    {
      return parseError(EXTRA_SPACE);
    }
    if (int(ch) == 9)
    {
      errInt = 9;
      return parseError(ILLEGAL_WSPACE);
    }
    ifs.unget();
    ifs >> str;
    colNo = colNo + str.size();
    if (!myStr2Int(str, literal) || literal < 0)
    {
      errMsg = "Illegal AIG gate literal ID (" + str + ")";
      return parseError(ILLEGAL_NUM);
    }
    if (literal % 2 == 1)
    {
      colNo = 0;
      errInt = literal;
      errMsg = "AIG";
      return parseError(CANNOT_INVERTED);
    }
    if (literal / 2 > m)
    {
      colNo = 0;
      errInt = literal;
      return parseError(MAX_LIT_ID);
    }
    if (literal / 2 == 0)
    {
      colNo = 0;
      errInt = literal;
      return parseError(REDEF_CONST);
    }
    if (gatelist[literal / 2]._type == AIG_GATE || gatelist[literal / 2]._type == PI_GATE)
    {
      errInt = literal;
      errGate = &gatelist[literal / 2];
      return parseError(REDEF_GATE);
    }
    gatelist[literal / 2]._type = AIG_GATE;
    gatelist[literal / 2].line = lineNo + 1;
    //fanin 1 for AIG
    ifs.get(ch);
    ++colNo;
    if (ch != ' ')
    {
      return parseError(MISSING_SPACE);
    }
    ifs.get(ch);
    if (ch == ' ')
    {
      return parseError(EXTRA_SPACE);
    }
    if (int(ch) == 9)
    {
      errInt = 9;
      return parseError(ILLEGAL_WSPACE);
    }
    ifs.unget();
    ifs >> str;
    colNo = colNo + str.size();
    if (!myStr2Int(str, fanin) || fanin < 0)
    {
      return parseError(ILLEGAL_NUM);
    }
    if (fanin / 2 > m)
    {
      errInt = fanin;
      return parseError(MAX_LIT_ID);
    }
    connect(fanin, literal);
    ifs.get(ch);
    ++colNo;
    if (ch != ' ')
    {
      return parseError(MISSING_SPACE);
    }
    ifs.get(ch);
    if (ch == ' ')
    {
      return parseError(EXTRA_SPACE);
    }
    if (int(ch) == 9)
    {
      return parseError(ILLEGAL_WSPACE);
    }
    ifs.unget();
    ifs >> str;
    colNo = colNo + str.size();
    if (!myStr2Int(str, fanin) || fanin < 0)
    {
      errMsg = "Illegal AIG input literal ID (" + str + ")";
      return parseError(ILLEGAL_NUM);
    }
    if (fanin / 2 > m)
    {
      errInt = fanin;
      return parseError(MAX_LIT_ID);
    }
    connect(fanin, literal);
    ifs.get(ch);
    if (ch != '\n')
    {
      return parseError(MISSING_NEWLINE);
    }
    colNo = 0;
    ++lineNo;
  }

  ifs.get(ch);
  if (!ifs.eof())
  {
    while (ch) //hwhile(1)??
    {
      if (ch == ' ')
      {
        return parseError(EXTRA_SPACE);
      }
      else if (int(ch) == 9)
      {
        errInt = 9;
        return parseError(ILLEGAL_WSPACE);
      }

      else if (ch == 'i')
      {
        str = "";
        ifs.get(ch);
        ++colNo;
        if (ch == ' ')
        {
          return parseError(EXTRA_SPACE);
        }
        if (int(ch) == 9)
        {
          errInt = 9;
          return parseError(ILLEGAL_WSPACE);
        }
        ifs.unget();
        ifs >> str;
        colNo = colNo + str.size();
        if (!myStr2Int(str, literal) || literal < 0)
        {
          errMsg = "symbol index (" + str + ")";
          return parseError(ILLEGAL_NUM);
        }
        if (literal > i)
        {
          errMsg = "PI index";
          errInt = literal;
          return parseError(NUM_TOO_BIG);
        }
        if (gatelist[PI_list[literal]].symbol.size() != 0)
        {
          errMsg = "i";
          errInt = literal;
          return parseError(REDEF_SYMBOLIC_NAME);
        }
        ifs.get(ch);
        ++colNo;
        if (ch == '\n')
        {
          errMsg = "symbolic name";
          return parseError(MISSING_IDENTIFIER);
        } //missing symbol name
        if (ch != ' ')
        {
          return parseError(MISSING_SPACE);
        }
        ifs.get(ch);
        ++colNo;
        if (ch == ' ')
        {
          return parseError(EXTRA_SPACE);
        }
        if (ch == '\n')
        {
          errMsg = "symbolic name";
          return parseError(MISSING_IDENTIFIER);
        } // missing symbol name
        ifs.unget();
        getline(ifs, str);
        for (size_t i = 0; i < str.size(); i++)
        {
          if ((str[i] < 31 && str[i] > 0 && str[i] != 9) || str[i] == 127)
          {
            errInt = int(str[i]);
            return parseError(ILLEGAL_SYMBOL_NAME);
          }
        }
        gatelist[PI_list[literal]].symbol = str;
      }
      else if (ch == 'o')
      {
        ifs.get(ch);
        ++colNo;
        if (ch == ' ')
        {
          return parseError(EXTRA_SPACE);
        }
        if (int(ch) == 9)
        {
          errInt = 9;
          return parseError(ILLEGAL_WSPACE);
        }
        ifs.unget();
        ifs >> str; //colNo = colNo + str.size();
        if (!myStr2Int(str, literal) || literal < 0)
        {
          errMsg = "symbol index (" + str + ")";
          return parseError(ILLEGAL_NUM);
        }
        if (literal > o)
        {
          errMsg = "PO index";
          errInt = literal;
          return parseError(NUM_TOO_BIG);
        }
        ifs.get(ch);
        ++colNo;
        if (ch == '\n')
        {
          errMsg = "symbolic name";
          return parseError(MISSING_IDENTIFIER);
        } //missing symbol name
        if (ch != ' ')
        {
          return parseError(MISSING_SPACE);
        }
        ifs.get(ch);
        ++colNo;
        if (ch == ' ')
        {
          return parseError(EXTRA_SPACE);
        }
        if (int(ch) == 9)
        {
          return parseError(ILLEGAL_WSPACE);
        }
        if (ch == '\n')
        {
          errMsg = "symbolic name";
          return parseError(MISSING_IDENTIFIER);
        }
        ifs.unget();
        getline(ifs, str);
        for (size_t i = 0; i < str.size(); i++)
        {
          if (int(str[i]) > 0 && int(str[i]) < 32 && int(str[i]) != 9 || int(str[i]) == 127)
          {
            errInt = int(str[i]);
            return parseError(ILLEGAL_SYMBOL_NAME);
          }
        }
        if (gatelist[PO_list[literal]].symbol.size() != 0)
        {
          errMsg = "o";
          errInt = literal;
          return parseError(REDEF_SYMBOLIC_NAME);
        }
        gatelist[PO_list[literal]].symbol = str;
      }
      else if (ch == 'c')
      {
        ifs.get(ch);
        ++colNo;
        if (ch != '\n')
        {
          return parseError(MISSING_NEWLINE);
        }
        break; //skip parsing
      }
      else
      {
        if (ch == '\n')
        {
          errMsg = "";
          return parseError(ILLEGAL_SYMBOL_TYPE);
        }
        errMsg = ch;
        return parseError(ILLEGAL_SYMBOL_TYPE);
      }
      ifs.get(ch);
      if (ifs.eof())
        break; //??
      ++lineNo;
      colNo = 0;
    }
  }
  gatelist[0]._ID = 0;
  for (int k = 0; k < gatenum; k++)
  {
    if (gatelist[k]._type == AIG_GATE)
    {
      gatelist[k]._ID = k;
    }
  }
  setDFS();
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
  cout << "Circuit Statistics" << endl;
  cout << "==================" << endl;
  cout << "  PI" << setw(12) << right << i << endl;
  cout << "  PO" << setw(12) << right << o << endl;
  cout << "  AIG" << setw(11) << right << a << endl;
  cout << "------------------" << endl;
  cout << "  Total" << setw(9) << right << i + o + a << endl;
}
void CirMgr::printNetlist() const
{
  cout << endl;
  int counter = 0;
  for (size_t i = 0; i < DFS_list.size(); i++)
  {
    if (gatelist[DFS_list[i]]._type == UNDEF_GATE)
      continue;
    cout << "[" << counter << "] ";
    cout << gatelist[DFS_list[i]].getTypeStr();
    if (gatelist[DFS_list[i]].getTypeStr().size() < 4)
      for (size_t k = 0; k < 4 - gatelist[DFS_list[i]].getTypeStr().size(); k++)
        cout << " ";
    cout << DFS_list[i];

    for (size_t j = 0; j < gatelist[DFS_list[i]].fanins.size(); j++)
    {
      cout << " ";
      if (gatelist[gatelist[DFS_list[i]].fanins[j] / 2]._type == UNDEF_GATE)
        cout << "*";
      if (gatelist[DFS_list[i]].fanins[j] % 2 == 1)
        cout << "!";
      cout << gatelist[DFS_list[i]].fanins[j] / 2;
    }
    if (gatelist[DFS_list[i]].symbol.size())
      cout << " (" << gatelist[DFS_list[i]].symbol << ")";
    cout << endl;
    counter++;
  }
}

void CirMgr::printPIs() const
{
  cout << "PIs of the circuit: ";
  for (size_t index = 0; index < PI_list.size(); index++)
  {
    cout << PI_list[index] << " ";
  }
  cout << endl;
}

void CirMgr::printPOs() const
{
  cout << "POs of the circuit: ";
  for (size_t index = 0; index < PO_list.size(); index++)
  {
    cout << PO_list[index] << " ";
  }
  cout << endl;
}
void CirMgr::printFloatGates() const
{
  bool doprint;
  for (int index = 0; index < gatenum; index++)
  {
    if (gatelist[index]._type == AIG_GATE || gatelist[index]._type == PO_GATE)
    {
      for (size_t j = 0; j < gatelist[index].fanins.size(); j++)
      {
        if (gatelist[(gatelist[index].fanins[j]) / 2]._type == UNDEF_GATE)
        {
          if (!doprint)
            cout << "Gates with floating fanin(s): ";
          cout << index << " ";
          doprint = true;
          break;
        }
      }
    }
  }
  doprint = false;
  for (int index = 0; index < gatenum; index++)
  {
    if (gatelist[index]._type == AIG_GATE || gatelist[index]._type == PI_GATE)
      if (!gatelist[index].fanouts.size())
      {
        if (!doprint)
          cout << endl
               << "Gates defined but not used  : ";
        cout << index << " ";
        doprint = true;
      }
  }
  if (doprint)
    cout << endl;
}

void CirMgr::writeAag(ostream &outfile) const
{
  int AIG_num = 0;
  // eliminating floating and undefined ones:
  for (size_t index = 0; index < DFS_list.size(); index++)
  {
    if (gatelist[DFS_list[index]]._type == AIG_GATE)
      AIG_num++;
  }
  // 1st line:
  outfile << "aag " << m << " " << i << " " << l << " " << o << " " << AIG_num << endl;
  // deal with PI:
  for (size_t index = 0; index < PI_list.size(); index++)
  {
    outfile << PI_list[index] * 2 << endl;
  }
  // deal with PO:
  for (size_t index = 0; index < PO_list.size(); index++)
  {
    outfile << gatelist[PO_list[index]].fanins[0] << endl;
  }
  // deal with AIG:
  for (size_t index = 0; index < DFS_list.size(); index++)
  {
    if (gatelist[DFS_list[index]]._type == AIG_GATE)
    {
      outfile << DFS_list[index] * 2 << " " << gatelist[DFS_list[index]].fanins[0];
      outfile << " " << gatelist[DFS_list[index]].fanins[1] << endl;
    }
  }
  //deal with symbols:
  //input:
  for (size_t index = 0; index < PI_list.size(); index++)
  {
    if (gatelist[PI_list[index]].symbol.size())
      outfile << "i" << index << " " << gatelist[PI_list[index]].symbol << endl;
  }
  //output:
  for (size_t index = 0; index < PO_list.size(); index++)
  {
    if (gatelist[PO_list[index]].symbol.size())
      outfile << "o" << index << " " << gatelist[PO_list[index]].symbol << endl;
  }
}
void CirMgr::writeGate(std::ostream &outfile, CirGate *g)
{
  vector<unsigned> list;
  vector<unsigned> PI;
  vector<unsigned> AIG;
  vector<string> symbol_list;
  unsigned id = g - gatelist;
  unsigned max = id;
  reset_visit();
  DFS_write(id, list);
  for (int i = 0; i < list.size(); i++)
  {
    int idx = list[i];
    if (gatelist[idx]._type == AIG_GATE)
    {
      if (list[i] > max)
        max = list[i];
      AIG.push_back(list[i]);
    }
  }
  for (int i = 0; i < PI_list.size(); i++)
    for (int j = 0; j < list.size(); j++)
      if (list[j] == PI_list[i])
      {
        PI.push_back(PI_list[i]);
        if (gatelist[PI_list[i]].symbol.size())
        {
          string str = "i" + std::to_string(i) + " " + gatelist[PI_list[i]].symbol;
          symbol_list.push_back(str);
        }
        break;
      }
  outfile << "aag " << max << " " << PI.size() << " 0 1 " << AIG.size() << endl;
  for (int i = 0; i < PI.size(); i++)
    outfile << PI[i] * 2 << endl;
  outfile << id * 2 << endl;
  for (int i = 0; i < AIG.size(); i++)
  {
    outfile << AIG[i] * 2 << " " << gatelist[AIG[i]].fanins[0];
    outfile << " " << gatelist[AIG[i]].fanins[1] << endl;
  }
  for (int i = 0; i < symbol_list.size(); i++)
    outfile << symbol_list[i] << endl;

  outfile << "o0 " << id << endl
          << "c\n"
          << "Write gate (" << id << ") by Chung-Yang (Ric) Huang" << endl;
}

CirGate *
CirMgr::getGate(unsigned gid) const
{
  //invalid gate ID:
  if (gid >= gatenum || gid < 0)
    return 0;
  //variable not used:
  if (gatelist[gid]._type == TOT_GATE)
    return 0;
  //gatelist[gid]:
  return (gatelist + gid);
  //return 0;
}
void CirMgr::connect(int fanin, int literal)
{
  bool inverted = fanin % 2;
  gatelist[literal / 2].fanins.push_back(fanin); //10 (4) 7
  gatelist[fanin / 2].fanouts.push_back(literal + inverted);
  //if a gate is used in AIG but neither is definded in PI nor PO, falls into UNDEF_GATE
  if (gatelist[fanin / 2]._type == TOT_GATE)
  {
    gatelist[fanin / 2]._type = UNDEF_GATE;
    gatelist[fanin / 2].line = 0;
  }
}
void CirMgr::DFS(unsigned gid)
{
  gatelist[gid].is_visit = true;
  for (size_t j = 0; j < gatelist[gid].fanins.size(); j++)
  {
    unsigned tempid = (gatelist[gid].fanins[j]) / 2;
    if (!gatelist[tempid].is_visit)
    {
      DFS(tempid);
    }
  }
  DFS_list.push_back(gid);
}
void CirMgr::DFS_write(unsigned gid, vector<unsigned> &list)
{
  gatelist[gid].is_visit = true;
  for (size_t j = 0; j < gatelist[gid].fanins.size(); j++)
  {
    unsigned tempid = (gatelist[gid].fanins[j]) / 2;
    if (!gatelist[tempid].is_visit)
    {
      DFS_write(tempid, list);
    }
  }
  list.push_back(gid);
}

void CirMgr::reset_visit() const
{
  for (int i = 0; i < gatenum; i++)
    gatelist[i].reset_visit();
}

void CirMgr::setDFS()
{
  DFS_list.clear();
  reset_visit();
  for (int i = 0; i < PO_list.size(); i++)
    DFS(PO_list[i]);
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

void CirMgr::printFECPairs()
{
  for (int i = 0; i < FEC_pair.size(); i++)
  {
    cout << "[" << i << "] " << FEC_pair[i][0];
    for (int j = 1; j < FEC_pair[i].size(); j++)
    {
      cout << " ";
      if (is_IFEC(FEC_pair[i][0], FEC_pair[i][j]))
        cout << "!";
      cout << FEC_pair[i][j];
    }
    cout << endl;
  }
}

bool CirMgr::is_IFEC(int idx1, int idx2)
{
  size_t cmp_num = (gatelist[idx1].sim_bits ^ gatelist[idx2].sim_bits).count();
  if (cmp_num == 64)
    return true;
  return false;
}