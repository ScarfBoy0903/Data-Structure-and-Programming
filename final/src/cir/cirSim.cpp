/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <bitset>
using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void CirMgr::randomSim()
{
  /*-------------------------------generate bit sequence-------------------------------*/
  vector<bitset<64>> bits_data;
  bits_data.resize(i);
  /*-------------------------------generate bit sequence-------------------------------*/
  cout << "320 patterns simulated." << endl;
  for (int num = 0; num < 5; num++)
  {
    /*-------------------------------simulate PI-------------------------------*/
    for (int i = 0; i < PI_list.size(); i++)
    {
      int idx = PI_list[i];
      bitset<64> temp(randomSize_t());
      gatelist[idx].sim_bits = temp;
    }
    /*-------------------------------simulate PI-------------------------------*/

    /*-------------------------------operate simulation-------------------------------*/
    operate_sim();
    /*-------------------------------operate simulation-------------------------------*/

    /*-------------------------------output simlogfile-------------------------------*/
    if (_simLog)
      write_simlog(64);
    /*-------------------------------output simlogfile-------------------------------*/
  }
  ::sort(FEC_pair.begin(), FEC_pair.end(), FirstColumnOnlyCmp());
  
  /*-------------------------------copy FECpair-------------------------------*/
  for (int i = 0; i < FEC_pair.size(); i++)
    for (int j = 0; j < FEC_pair[i].size(); j++)
    {
      int idx = FEC_pair[i][j];
      gatelist[idx].FECs = FEC_pair[i];
    }
  
}

void CirMgr::fileSim(ifstream &patternFile)
{
  /*-------------------------------read stream-------------------------------*/
  vector<bitset<64>> bits_data;
  vector<string> bits_str;
  bits_str.resize(i);
  bits_data.resize(i);
  string inputStr;
  int count = 0;
  int sim_num = 0;
  bool do_error = false;
  while (getline(patternFile, inputStr))
  {
    vector<string> token;
    split(inputStr, " ", token);
    inputStr = token[0];
    if (inputStr.size() != i)
    {
      cerr << "\nError: Pattern(" << inputStr << ") length(" << inputStr.size() << ") does not match the number of inputs(" << i << ") in a circuit !!" << endl;
      do_error = true;
      break;
    }
    if (token.size() != 1)
    {
      inputStr = token[1];
      cerr << "\nError: Pattern(" << inputStr << ") length(" << inputStr.size() << ") does not match the number of inputs(" << i << ") in a circuit !!" << endl;
      do_error = true;
      break;
    }
    for (int i = 0; i < inputStr.size(); i++)
    {
      if (inputStr[i] != '0' && inputStr[i] != '1')
      {
        cerr << "\nError: Pattern(" << inputStr << ") contains a non-0/1 character('" << inputStr[i] << "')." << endl;
        do_error = true;
        break;
      }
      bits_str[i] = inputStr[i] + bits_str[i];
    }
    if (do_error)
      break;
    count++;
    sim_num++;
    if (count >= 64)
    {
      int bits_long = 64;
      /*-------------------------------simulate PI-------------------------------*/
      for (int i = 0; i < PI_list.size(); i++)
      {
        int idx = PI_list[i];
        bitset<64> temp(bits_str[i]);
        gatelist[idx].sim_bits = temp;
      }
      /*-------------------------------simulate PI-------------------------------*/

      /*-------------------------------operate simulation-------------------------------*/
      operate_sim();
      /*-------------------------------operate simulation-------------------------------*/

      /*-------------------------------output simlogfile-------------------------------*/
      if (_simLog)
        write_simlog(bits_long);
      /*-------------------------------output simlogfile-------------------------------*/
      count = 0;
      for (int i = 0; i < PI_list.size(); i++)
      {
        bits_str[i].clear();
      }
    }
  }

  /*---------------------do error-------------------*/
  cout << endl;
  if (do_error)
  {
    if (sim_num < 64)
    {
      cout << "0 patterns simulated." << endl;
      // return;
    }
    else
    {
      cout << sim_num - (sim_num % 64) << " patterns simulated." << endl;
      // return;
    }
  }
  else
  {
    cout << sim_num << " patterns simulated." << endl;
    if (bits_str[0].size())
    {
      int bits_long = bits_str[0].size();
      /*-------------------------------simulate PI-------------------------------*/
      for (int i = 0; i < PI_list.size(); i++)
      {
        int idx = PI_list[i];
        bitset<64> temp(bits_str[i]);
        gatelist[idx].sim_bits = temp;
      }
      /*-------------------------------simulate PI-------------------------------*/

      /*-------------------------------operate simulation-------------------------------*/
      operate_sim();
      /*-------------------------------operate simulation-------------------------------*/

      /*-------------------------------output simlogfile-------------------------------*/
      if (_simLog)
        write_simlog(bits_long);
    }
  }
  /*---------------------do error-------------------*/
  /*-------------------------------read stream-------------------------------*/

  ::sort(FEC_pair.begin(), FEC_pair.end(), FirstColumnOnlyCmp());
  /*-------------------------------copy FECpair-------------------------------*/
  for (int i = 0; i < FEC_pair.size(); i++)
    for (int j = 0; j < FEC_pair[i].size(); j++)
    {
      int idx = FEC_pair[i][j];
      gatelist[idx].FECs = FEC_pair[i];
    }
  /*-------------------------------copy FECpair-------------------------------*/
}

void CirMgr::sim_gate(int idx)
{
  if (gatelist[idx]._type == AIG_GATE)
  {
    int f_in1 = gatelist[idx].fanins[0] / 2;
    int f_in2 = gatelist[idx].fanins[1] / 2;
    bitset<64> f_in1_bits = gatelist[f_in1].sim_bits;
    bitset<64> f_in2_bits = gatelist[f_in2].sim_bits;
    if (gatelist[idx].fanins[0] % 2)
    {
      f_in1_bits.flip();
    }
    if (gatelist[idx].fanins[1] % 2)
    {
      f_in2_bits.flip();
    }
    gatelist[idx].sim_bits = f_in1_bits & f_in2_bits;
  }
  else if (gatelist[idx]._type == PO_GATE)
  {
    int f_in = gatelist[idx].fanins[0] / 2;
    bitset<64> f_in_bits = gatelist[f_in].sim_bits;
    if (gatelist[idx].fanins[0] % 2)
    {
      f_in_bits.flip();
    }
    gatelist[idx].sim_bits = f_in_bits;
  }
}

void CirMgr::operate_sim()
{
  /*-------------------------------set simulation list-------------------------------*/
  vector<unsigned> sim_list;
  vector<int> FEC_list;
  FEC_list.push_back(0);
  for (int i = 0; i < DFS_list.size(); i++)
  {
    int idx = DFS_list[i];
    if (gatelist[idx]._type == AIG_GATE || gatelist[idx]._type == PO_GATE)
    {
      sim_list.push_back(idx);
      if (gatelist[idx]._type == AIG_GATE)
        FEC_list.push_back(idx);
    }
  }
  /*-------------------------------set simulation list-------------------------------*/

  /*-------------------------------simulation-------------------------------*/
  for (int i = 0; i < sim_list.size(); i++)
    sim_gate(sim_list[i]);
  /*-------------------------------simulation-------------------------------*/

  /*-------------------------------set temp FECpair-------------------------------*/
  if (!ever_sim)
  {
    check_FEC_pair(FEC_list, FEC_pair);
    ever_sim = true;
    return;
  }
  /*-------------------------------set temp FECpair-------------------------------*/

  /*-------------------------------detect non-FECpair-------------------------------*/
  vector<vector<int>> temp;
  for (int i = 0; i < FEC_pair.size(); i++)
  {
    vector<vector<int>> inserted_sub_FEC;
    if (check_FEC_pair(FEC_pair[i], inserted_sub_FEC))
    {
      if (inserted_sub_FEC.size())
      {
        for (int j = 0; j < inserted_sub_FEC.size(); j++)
        {
          temp.push_back(inserted_sub_FEC[j]);
        }
      }
    }
    else
    {
      temp.push_back(FEC_pair[i]);
    }
  }
  FEC_pair = temp;
  /*-------------------------------detect non-FECpair-------------------------------*/
}

void CirMgr::write_simlog(int size)
{
  vector<string> bit_pi;
  vector<string> bit_po;
  int print_num = 64 - size;
  for (int i = 0; i < PI_list.size(); i++)
  {
    int idx = PI_list[i];
    bit_pi.push_back(gatelist[idx].sim_bits.to_string());
  }

  for (int i = 0; i < PO_list.size(); i++)
  {
    int idx = PO_list[i];
    bit_po.push_back(gatelist[idx].sim_bits.to_string());
  }

  for (int i = 63; i >= print_num; i--)
  {
    for (int j = 0; j < bit_pi.size(); j++)
    {
      *_simLog << bit_pi[j][i];
    }
    *_simLog << " ";
    for (int j = 0; j < bit_po.size(); j++)
    {
      *_simLog << bit_po[j][i];
    }
    *_simLog << endl;
  }
}

bool CirMgr::check_FEC_pair(vector<int> FEC_list, vector<vector<int>> &FEC_s)
{
  if (!ever_sim)
    sort(FEC_list.begin(), FEC_list.end());

  _FECHash = FEC_HashSet<CirGate>(gatenum);
  for (int i = 0; i < FEC_list.size(); i++)
  {
    int idx = FEC_list[i];
    _FECHash.insert(gatelist[idx]);
    gatelist[idx].reset_visit();
  }

  for (int i = 0; i < FEC_list.size(); i++)
  {
    int idx = FEC_list[i];
    if (!gatelist[idx].is_visit)
    {
      vector<int> same_list;
      if (_FECHash.get_same_pair_list(gatelist[idx], same_list))
      {
        if (same_list.size() == FEC_list.size() && i == 0)
        {
          _FECHash.reset();
          return false;
        }
        FEC_s.push_back(same_list);
        for (int j = 0; j < same_list.size(); j++)
        {
          int idx_v = same_list[j];
          gatelist[idx_v].is_visit = true;
        }
      }
    }
  }
  _FECHash.reset();
  return true;
}

size_t CirMgr::randomSize_t()
{
  size_t temp = (size_t)rand();
  return (temp << 32) + ((size_t)rand());
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
