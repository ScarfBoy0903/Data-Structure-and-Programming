/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "myHashMap.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
  CirMgr() {}
  ~CirMgr() {}
  friend class CirGate;
  friend class HashSet<CirGate>;
  // Access functions
  // return '0' if "gid" corresponds to an undefined gate.
  CirGate *getGate(unsigned gid) const;
  // Member functions about circuit construction
  bool readCircuit(const string &);

  // Member functions about circuit optimization
  void sweep();
  void erase_gate(CirGate &);

  void optimize();
  void cross_connect(int, int);

  // Member functions about simulation
  void randomSim();
  void fileSim(ifstream &);
  void setSimLog(ofstream *logFile) { _simLog = logFile; }
  void sim_gate(int idx);
  void operate_sim();
  bool is_IFEC(int idx1, int idx2);
  void write_simlog(int);
  bool check_FEC_pair(vector<int> FEC_list, vector<vector<int>> &FEC_s);
  size_t randomSize_t();

  // Member functions about fraig
  void strash();
  void printFEC() const;
  void merge_gate(CirGate &, CirGate &);
  void fraig();

  // Member functions about circuit reporting
  void printSummary() const;
  void printNetlist() const;
  void printPIs() const;
  void printPOs() const;
  void printFloatGates() const;
  void printFECPairs();
  void writeAag(ostream &) const;
  void writeGate(ostream &, CirGate *);
  void connect(int, int); //connect the current gate with previous one
  void DFS(unsigned);
  void DFS_write(unsigned, vector<unsigned> &);
  void setDFS();
  void reset_visit() const;
  void split(const string &s, const char *delim, vector<string> &v);
 
private:
  ofstream *_simLog;
  int m, i, l, o, a;
  int gatenum;                    // maximum number of propable gates
  std::vector<unsigned> PI_list;  //gate IDs of PIs
  std::vector<unsigned> PO_list;  //gate IDs of POs
  std::vector<unsigned> DFS_list; //gate IDs through DFS traversal
  std::vector<vector<int>> FEC_pair;
  CirGate *gatelist;
  HashSet<CirGate> _cirHash;
  FEC_HashSet<CirGate> _FECHash;
  bool ever_sim;
};

struct FirstColumnOnlyCmp
{
  bool operator()(const vector<int> &v1, const vector<int> &v2)
  {
    return v1[0] < v2[0];
  }
};

#endif // CIR_MGR_H
