/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void CirMgr::strash()
{
  _cirHash = HashSet<CirGate>(gatenum);
  vector<unsigned> _route_list;
  for (int i = 0; i < DFS_list.size(); i++)
  {
    int idx = DFS_list[i];
    if (gatelist[idx]._type == AIG_GATE)
    {
      CirGate cmp;
      if (_cirHash.insert(gatelist[idx], cmp))
      {
        int idx_mer = cmp._ID;
        cout << "Strashing: " << idx_mer << " merging " << idx << "..." << endl;
        merge_gate(gatelist[idx_mer], gatelist[idx]);
      }
    }
  }
  _cirHash.reset();
  setDFS();
}

void CirMgr::fraig()
{
}

void CirMgr::merge_gate(CirGate &g1, CirGate &g2)
{
  for (int i = 0; i < g2.fanouts.size(); i++)
  {
    bool invert = g2.fanouts[i] % 2;
    int idx_o = g2.fanouts[i] / 2;
    int new_f_in = 2 * g1._ID;
    if (invert)
      new_f_in++;
    if (gatelist[idx_o].fanins[0] / 2 == g2._ID)
    {
      gatelist[idx_o].fanins[0] = new_f_in;
    }
    else if (gatelist[idx_o].fanins[1] / 2 == g2._ID)
    {
      gatelist[idx_o].fanins[1] = new_f_in;
    }
    g1.fanouts.push_back(g2.fanouts[i]);
  }
  a--;
  g2 = CirGate();
}
/********************************************/
/*   Private member functions about fraig   */
/********************************************/
