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

using namespace std;

class CirGate;

enum GateCase
{
  DEF_NOT_USE_GATE = 0,
  FLOTING_ONE_GATE = 1,
  FLOTING_TWO_GATE = 2,
  NORMAL_GATE = 3,
};

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
public:
  friend class CirMgr;
  friend class CirAIG;
  CirGate(int a, int b) : _literal(a), _ID(b), _gatecase(NORMAL_GATE)
  {
    is_visited = false;
    fanin1_invert = false;
    fanin2_invert = false;
    _gatename.clear();
    _name.clear();
    _symbol.clear();
  }
  CirGate(int a, int b, GateType c) : _literal(a), _ID(b), _gate_type(c)
  {
    is_visited = false;
    fanin1_invert = false;
    fanin2_invert = false;
    _gatename.clear();
    _name.clear();
    _symbol.clear();
  }
  // Basic access methods
  string getTypeStr() const { return ""; }
  unsigned getLineNo() const { return 0; }

  // Printing functions
  virtual void printGate() const = 0;
  void reportGate() const;
  void reportFanin(int level);
  void reportFanout(int level);
  bool check_visited_fanin(CirGate *gate, int level);
  bool check_visited_fanout(CirGate *gate, int level);
  bool printGate(CirGate *gate);
  void _fanin_traversal(CirGate *gate, int level, string str, vector<CirGate *> &_record);
  void _fanout_traversal(CirGate *gate, int level, string str, vector<CirGate *> &_record);

private:
  vector<CirGate *> _fanin;
  vector<CirGate *> _fanout;
  vector<CirGate *> _record;
  int _literal;
  int _ID;
  int _line;
  bool is_visited;
  bool fanin1_invert;
  bool fanin2_invert;
  GateType _gate_type;
  GateCase _gatecase;
  string _gatename;
  string _name;
  string _symbol;

protected:
};
#endif // CIR_GATE_H

class CirPI : public CirGate
{
public:
  CirPI(int a, int b) : CirGate(a, b, PI_GATE) {}
  CirPI(int a, int b, GateType c) : CirGate(a, b, c) {}
  void printGate() const;
};

class CirPO : public CirGate
{
public:
  CirPO(int a, int b) : CirGate(a, b, PO_GATE) {}
  CirPO(int a, int b, GateType c) : CirGate(a, b, c) {}
  void printGate() const;
};

class CirAIG : public CirGate
{
  friend class CirMgr;

public:
  CirAIG(int a, int b) : CirGate(a, b, AIG_GATE) {}
  CirAIG(int a, int b, GateType c) : CirGate(a, b, c) {}
  CirAIG(int a, int b, GateCase c) : CirGate(a, b, AIG_GATE) {}
  void printGate() const;
};
