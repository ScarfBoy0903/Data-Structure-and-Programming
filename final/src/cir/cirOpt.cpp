/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void CirMgr::sweep()
{
  CirGate *temp = new CirGate[gatenum];
  for (int i = 0; i < DFS_list.size(); i++)
    temp[DFS_list[i]] = gatelist[DFS_list[i]];

  for (int i = 0; i < gatenum; i++)
  {
    if (gatelist[i]._type != temp[i]._type)
    {
      if (gatelist[i]._type == AIG_GATE)
      {
        cout << "Sweeping: " << gatelist[i].getTypeStr() << "(" << i << ") removed..." << endl;
        a--;
        erase_gate(gatelist[i]);
      }
      else if (gatelist[i]._type == UNDEF_GATE)
      {
        cout << "Sweeping: " << gatelist[i].getTypeStr() << "(" << i << ") removed..." << endl;
        gatelist[i] = CirGate();
      }
    }
  }
  delete[] temp;
  temp = 0;
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void CirMgr::optimize()
{
  for (int i = 0; i < DFS_list.size(); i++)
  {
    int idx = DFS_list[i];
    /*----------------------------merge same fanins----------------------------*/
    if (gatelist[idx]._type == AIG_GATE)
    {
      if (gatelist[idx].fanins[0] / 2 == gatelist[idx].fanins[1] / 2)
      {
        if (gatelist[idx].fanins[0] == gatelist[idx].fanins[1])
        {
          cross_connect(gatelist[idx].fanins[0], idx);
        }
        else
        {
          cross_connect(0, idx);
        }
      }
    }
    /*----------------------------merge same fanins----------------------------*/

    /*----------------------------process const fanins----------------------------*/
    if (gatelist[idx]._type == AIG_GATE)
    {
      if (gatelist[idx].fanins[0] / 2 == 0)
      {
        if (gatelist[idx].fanins[0] % 2) //if invert
          cross_connect(gatelist[idx].fanins[1], idx);
        else
          cross_connect(0, idx);
      }
      else if (gatelist[idx].fanins[1] / 2 == 0)
      {
        if (gatelist[idx].fanins[1] % 2)
          cross_connect(gatelist[idx].fanins[0], idx);
        else
          cross_connect(0, idx);
      }
    }
    /*----------------------------process const fanins----------------------------*/
  }
  for (int i = 0; i < DFS_list.size(); i++)
  {
    int index = DFS_list[i];
    if (gatelist[index]._type != TOT_GATE && gatelist[index]._type != PO_GATE)
    {
      vector<unsigned> temp;
      for (int j = 0; j < gatelist[index].fanouts.size(); j++)
      {
        int idx = gatelist[index].fanouts[j] / 2;
        if (gatelist[idx]._type != TOT_GATE)
        {
          temp.push_back(gatelist[index].fanouts[j]);
        }
      }
      gatelist[index].fanouts = temp;
    }
  }
  setDFS();
}

void CirMgr::cross_connect(int f_in, int idx)
{
  cout << "Simplifying: " << f_in / 2 << " merging ";
  if (f_in % 2)
    cout << "!";
  cout << idx << "..." << endl;
  a--;
  vector<unsigned> &g_out = gatelist[idx].fanouts;
  vector<unsigned> &i_out = gatelist[f_in / 2].fanouts;
  for (int index = 0; index < g_out.size(); index++)
  {
    if (g_out[index] % 2 != 0 && f_in % 2 != 0)
    {
      g_out[index]--;
      i_out.push_back(g_out[index]);
      if (gatelist[g_out[index] / 2].fanins[0] / 2 == idx)
      {
        gatelist[g_out[index] / 2].fanins[0] = f_in - 1;
      }
      else if (gatelist[g_out[index] / 2].fanins[1] / 2 == idx)
      {
        gatelist[g_out[index] / 2].fanins[1] = f_in - 1;
      }
    }
    else if (g_out[index] % 2 != 0 && f_in % 2 == 0)
    {
      i_out.push_back(g_out[index]);
      if (gatelist[g_out[index] / 2].fanins[0] / 2 == idx)
      {
        gatelist[g_out[index] / 2].fanins[0] = f_in + 1;
      }
      else if (gatelist[g_out[index] / 2].fanins[1] / 2 == idx)
      {
        gatelist[g_out[index] / 2].fanins[1] = f_in + 1;
      }
    }
    else if (g_out[index] % 2 == 0 && f_in % 2 != 0)
    {
      g_out[index]++;
      i_out.push_back(g_out[index]);
      if (gatelist[g_out[index] / 2].fanins[0] / 2 == idx)
      {
        gatelist[g_out[index] / 2].fanins[0] = f_in;
      }
      else if (gatelist[g_out[index] / 2].fanins[1] / 2 == idx)
      {
        gatelist[g_out[index] / 2].fanins[1] = f_in;
      }
    }
    else
    {
      i_out.push_back(g_out[index]);
      if (gatelist[g_out[index] / 2].fanins[0] / 2 == idx)
      {
        gatelist[g_out[index] / 2].fanins[0] = f_in;
      }
      else if (gatelist[g_out[index] / 2].fanins[1] / 2 == idx)
      {
        gatelist[g_out[index] / 2].fanins[1] = f_in;
      }
    }
  }
  gatelist[idx] = CirGate();
}

void CirMgr::erase_gate(CirGate &g)
{
  int fanins1_idx = g.fanins[0] / 2;
  int fanins2_idx = g.fanins[1] / 2;

  if (gatelist[fanins1_idx]._type == PI_GATE || gatelist[fanins1_idx]._type == AIG_GATE)
  {
    for (int j = 0; j < gatelist[fanins1_idx].fanouts.size(); j++)
    {
      if (g._ID == gatelist[fanins1_idx].fanouts[j] / 2)
      {
        gatelist[fanins1_idx].fanouts.erase(gatelist[fanins1_idx].fanouts.begin() + j);
        break;
      }
    }
  }

  if (gatelist[fanins2_idx]._type == PI_GATE || gatelist[fanins2_idx]._type == AIG_GATE)
  {
    for (int j = 0; j < gatelist[fanins2_idx].fanouts.size(); j++)
    {
      if (g._ID == gatelist[fanins2_idx].fanouts[j] / 2)
      {
        gatelist[fanins2_idx].fanouts.erase(gatelist[fanins2_idx].fanouts.begin() + j);
        break;
      }
    }
  }
  g = CirGate();
}
/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
