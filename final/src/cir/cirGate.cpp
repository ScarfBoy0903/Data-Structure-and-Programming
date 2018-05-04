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
string
CirGate::getTypeStr() const
{
	if (_type == UNDEF_GATE)
		return "UNDEF";
	if (_type == PI_GATE)
		return "PI";
	if (_type == PO_GATE)
		return "PO";
	if (_type == AIG_GATE)
		return "AIG";
	if (_type == CONST_GATE)
		return "CONST";
	return "TOT_GATE";
}

void CirGate::reportGate()
{
	cout << "================================================================================" << endl;
	cout << "= " << getTypeStr() << "(" << this - cirMgr->getGate(0) << ")";
	if (symbol.size())
		cout << "\"" << symbol << "\"";
	cout << ", line " << line << endl;
	cout << "= FECs:";
	if ((_type == AIG_GATE || _type == CONST_GATE))
	{
		for (int i = 0; i < FECs.size(); i++)
			if (FECs[i] != _ID)
			{
				cout << " ";
				if (cirMgr->is_IFEC(_ID, FECs[i]))
					cout << "!";
				cout << FECs[i];
			}
	}
	cout << endl;
	cout << "= Value: " << sim_value() << endl;
	cout << "================================================================================" << endl;
}

void CirGate::reportFanin(int level) const
{
	assert(level >= 0);
	cirMgr->reset_visit();
	search_fanin(level, level, false);
	cout << endl;
}

void CirGate::reportFanout(int level) const
{
	assert(level >= 0);
	cirMgr->reset_visit();
	search_fanout(level, level, false);
	cout << endl;
}
void CirGate::search_fanin(int level, int unsearched_level, bool inverted) const
{
	int current_id = this - cirMgr->getGate(0);
	for (int i = 0; i < (level - unsearched_level) * 2; i++)
		cout << " ";
	if (inverted == true)
		cout << "!";
	cout << getTypeStr() << " " << current_id << " ";
	//when a gate is traverseled twice or more
	if (unsearched_level > 0 && cirMgr->gatelist[current_id].is_visit == true)
	{
		if (_type != PI_GATE)
			cout << "(*)";
		return;
	}
	//when searching to the innest gate and start returning back to the outside
	else if (unsearched_level == 0 || cirMgr->gatelist[current_id].is_visit == true)
		return;
	cirMgr->gatelist[current_id].is_visit = true;
	unsearched_level--;
	for (size_t i = 0; i < fanins.size(); i++)
	{
		cout << endl;
		//cirMgr -> is_visit[fanins[i]/2] = true;
		cirMgr->gatelist[fanins[i] / 2].search_fanin(level, unsearched_level, fanins[i] % 2);
	}
}
void CirGate::search_fanout(int level, int unsearched_level, bool inverted) const
{
	int current_id = this - cirMgr->getGate(0);
	for (int i = 0; i < (level - unsearched_level) * 2; i++)
		cout << " ";
	if (inverted == true)
		cout << "!";
	cout << getTypeStr() << " " << current_id << " ";
	//if unsearched_level > 0, and the gate has been visited, then its fanins are also visited!
	if (unsearched_level > 0 && cirMgr->gatelist[current_id].is_visit == true)
	{
		if (_type != PO_GATE)
			cout << "(*)";
		return;
	}
	else if (unsearched_level == 0 || cirMgr->gatelist[current_id].is_visit == true)
		return;
	cirMgr->gatelist[current_id].is_visit = true;
	unsearched_level--;
	for (size_t i = 0; i < fanouts.size(); i++)
	{
		cout << endl;
		cirMgr->gatelist[fanouts[i] / 2].search_fanout(level, unsearched_level, fanouts[i] % 2);
	}
}
