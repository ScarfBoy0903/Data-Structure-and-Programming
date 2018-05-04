/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void CirGate::reportGate() const
{
  if (_gate_type == AIG_GATE)
  {
    cout << "==================================================" << endl;
    string temp = "= AIG(" + to_string(_ID) + ")" + _gatename + ", line " + to_string(_line);
    cout << left << setw(49) << temp << "=" << endl;
    cout << "==================================================" << endl;
  }
  else if (_gate_type == PI_GATE)
  {
    cout << "==================================================" << endl;
    string temp = "= PI(" + to_string(_ID) + ")" + _gatename + ", line " + to_string(_line);
    cout << left << setw(49) << temp << "=" << endl;
    cout << "==================================================" << endl;
  }
  else if (_gate_type == PO_GATE)
  {
    cout << "==================================================" << endl;
    string temp = "= PO(" + to_string(_ID) + ")" + _gatename + ", line " + to_string(_line);
    cout << left << setw(49) << temp << "=" << endl;
    cout << "==================================================" << endl;
  }
  else if (_gate_type == CONST_GATE)
  {
    cout << "==================================================" << endl
         << "= CONST(0), line 0                               =" << endl
         << "==================================================" << endl;
  }
  else if (_gate_type == UNDEF_GATE)
  {
    cout << "==================================================" << endl;
    string temp = "= UNDEF(" + to_string(_ID) + "), line 0";
    cout << left << setw(49) << temp << "=" << endl;
    cout << "==================================================" << endl;
  }
}

void CirGate::reportFanin(int level)
{
  assert(level >= 0);
  vector<CirGate *> _record;
  _fanin_traversal(this, level, "  ", _record);
  for (int i = 0; i < _record.size(); i++)
  {
    _record[i]->is_visited = false;
  }
}

void CirGate::_fanin_traversal(CirGate *gate, int level, string str, vector<CirGate *> &_record)
{
  if (printGate(gate))
  {
    cout << gate->_ID;
    _record.push_back(gate);
    if (check_visited_fanin(gate, level))
    {
      return;
    }
  }
  else
  {
    return;
  }
  if (!level || gate->_gate_type == CONST_GATE ||
      gate->_gate_type == PI_GATE ||
      gate->_gate_type == UNDEF_GATE)
  {
    return;
  }
  cout << str;
  if (gate->fanin1_invert)
  {
    cout << "!";
  }
  _fanin_traversal(gate->_fanin[0], level - 1, str + "  ", _record);
  if (gate->_gate_type != PO_GATE)
  {
    cout << str;
    if (gate->fanin2_invert)
    {
      cout << "!";
    }
    _fanin_traversal(gate->_fanin[1], level - 1, str + "  ", _record);
  }
}

void CirGate::reportFanout(int level)
{
  assert(level >= 0);
  vector<CirGate *> _record;
  _fanout_traversal(this, level, "  ", _record);
  for (int i = 0; i < _record.size(); i++)
  {
    _record[i]->is_visited = false;
  }
}

void CirGate::_fanout_traversal(CirGate *gate, int level, string str, vector<CirGate *> &_record)
{
  if (printGate(gate))
  {
    cout << gate->_ID;
    _record.push_back(gate);
    if (check_visited_fanout(gate, level))
    {
      return;
    }
  }
  else
  {
    return;
  }
  if (!level || gate->_gate_type == PO_GATE || gate->_gate_type == UNDEF_GATE)
  {
    return;
  }
  for (int i = 0; i < gate->_fanout.size(); i++)
  {
    if (gate->_fanout[i]->_gate_type != PO_GATE &&
        (gate->_fanout[i]->_gate_type == PI_GATE ||
         gate->_fanout[i]->_gate_type == AIG_GATE ||
         gate->_fanout[i]->_gate_type == CONST_GATE ||
         gate->_fanout[i]->_gate_type == UNDEF_GATE))
    {
      cout << str;
      if (gate == gate->_fanout[i]->_fanin[0])
      {
        if (gate->_fanout[i]->fanin1_invert)
        {
          cout << "!";
        }
      }
      else if (gate == gate->_fanout[i]->_fanin[1])
      {
        if (gate->_fanout[i]->fanin2_invert)
        {
          cout << "!";
        }
      }
    }
    else if (gate->_fanout[i]->_gate_type == PO_GATE)
    {
      cout << str;
      if (gate->_fanout[i]->fanin1_invert)
      {
        cout << "!";
      }
    }
    _fanout_traversal(gate->_fanout[i], level - 1, str + "  ", _record);
  }
}

bool CirGate::printGate(CirGate *temp)
{
  if (temp->_gate_type == AIG_GATE)
  {
    if (!temp->_ID)
    {
      return false;
    }
    cout << "AIG ";
    return true;
  }
  else if (temp->_gate_type == PI_GATE)
  {
    if (!temp->_ID)
    {
      return false;
    }
    cout << "PI ";
    return true;
  }
  else if (temp->_gate_type == UNDEF_GATE)
  {
    if (!temp->_ID)
    {
      return false;
    }
    cout << "UNDEF ";
    return true;
  }
  else if (temp->_gate_type == PO_GATE)
  {
    if (!temp->_ID)
    {
      return false;
    }
    cout << "PO ";
    return true;
  }
  else if (temp->_gate_type == CONST_GATE)
  {
    cout << "CONST ";
    return true;
  }
  return false;
}

bool CirGate::check_visited_fanin(CirGate *gate, int level)
{
  if (gate->is_visited)
  {
    if (level == 0 || gate->_gate_type == PI_GATE || gate->_gate_type == CONST_GATE)
    {
      cout << endl;
    }
    else
    {
      cout << " (*)" << endl;
    }
    return true;
  }
  else
  {
    cout << "\n";
    if (gate->_gate_type != UNDEF_GATE)
    {
      gate->is_visited = true;
    }
    return false;
  }
}

bool CirGate::check_visited_fanout(CirGate *gate, int level)
{
  if (gate->is_visited)
  {
    if (level == 0 || gate->_gate_type == PO_GATE)
    {
      cout << endl;
    }
    else
    {
      cout << " (*)" << endl;
    }
    return true;
  }
  else
  {
    cout << "\n";
    if (gate->_gate_type != UNDEF_GATE)
    {
      gate->is_visited = true;
    }
    return false;
  }
}

void CirPI::printGate() const {}

void CirPO::printGate() const {}

void CirAIG::printGate() const {}