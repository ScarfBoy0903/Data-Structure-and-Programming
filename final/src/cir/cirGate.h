/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"
#include <bitset>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
  CirGate()
  {
    _type = TOT_GATE;
    line = -1;
    _ID = -1;
    symbol = "";
  }
  ~CirGate() {}
  friend class CirMgr;
  friend class HashSet<CirGate>;
  friend class FEC_HashSet<CirGate>;

  // Basic access methods
  bool isAig() const
  {
    if (_type == AIG_GATE)
      return true;
    return false;
  }
  string getTypeStr() const;
  unsigned getLineNo() const { return line; }

  // Printing functions
  void printGate() const {}
  void reportGate();
  void reportFanin(int level) const;
  void reportFanout(int level) const;
  string sim_value()
  {
    string str = sim_bits.to_string();
    vector<string> part;
    for (int index = 0; index < 64; index += 8)
    {
      part.push_back(str.substr(index, 8));
    }
    return (part[0] + "_" + part[1] + "_" + part[2] + "_" + part[3] + "_" + part[4] + "_" + part[5] + "_" + part[6] + "_" + part[7]);
  }

private:
  GateType _type; //the type of the gate
  unsigned line;
  int _ID;
  std::vector<unsigned> fanins;
  std::vector<unsigned> fanouts;
  std::vector<int> FECs;
  string symbol;
  bitset<64> sim_bits;
  void search_fanin(int, int, bool) const;
  void search_fanout(int, int, bool) const;
  void reset_visit() { is_visit = false; }
  mutable bool is_visit;
  // bool is_FEC_gate;
};

#endif // CIR_GATE_H
