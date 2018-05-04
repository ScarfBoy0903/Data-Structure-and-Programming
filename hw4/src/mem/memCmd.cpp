/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include <ctype.h>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"
#include <stdlib.h>
using namespace std;

extern MemTest mtest; // defined in memTest.cpp

bool initMemCmd()
{
	if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
		  cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
		  cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
		  cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)))
	{
		cerr << "Registering \"mem\" commands fails... exiting" << endl;
		return false;
	}
	return true;
}

//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string &option)
{
	// check option
	string token;
	if (!CmdExec::lexSingleOption(option, token))
	{
		return CMD_EXEC_ERROR;
	}
	if (token.size())
	{
		int b;
		if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj))))
		{
			cerr << "Illegal block size (" << token << ")!!" << endl;
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
		}
#ifdef MEM_MGR_H
		mtest.reset(toSizeT(b));
#else
		mtest.reset();
#endif // MEM_MGR_H
	}
	else
	{
		mtest.reset();
	}
	return CMD_EXEC_DONE;
}

void MTResetCmd::usage(ostream &os) const
{
	os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void MTResetCmd::help() const
{
	cout << setw(15) << left << "MTReset: "
		 << "(memory test) reset memory manager" << endl;
}

//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string &option)
{
	vector<string> token;

	CmdExec::lexOptions(option, token);
	// cout << token.size();
	if (token.size() == 0)
	{
		return CmdExec::errorOption(CMD_OPT_MISSING, "");
	}
	else if (token.size() == 1)
	{
		int c;
		if (!myStr2Int(token[0], c) || c <= 0)
		{
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[0]);
		}
		mtest.newObjs(c);
	}

	else if (token.size() == 3)
	{
		int c1, c2;
		if (!myStr2Int(token[0], c1) || c1 <= 0)
		{
			if (!myStrNCmp("-Array", token[0], 2))
			{
				if (myStr2Int(token[2], c1) && c1 > 0)
				{
					if (myStr2Int(token[1], c2) && c2 > 0)
					{
						mtest.newArrs(c1, c2);
						return CMD_EXEC_DONE;
					}
					return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[1]);
				}
				return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[2]);
			}
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[0]);
		}
		else if (!myStrNCmp("-Array", token[1], 2) == 0)
		{
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[1]);
		}
		if (!myStr2Int(token[2], c2) || c2 <= 0)
		{
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[2]);
		}
		mtest.newArrs(c1, c2);
	}
	else
	{
		// cout << token.size();
		return CmdExec::errorOption(CMD_OPT_EXTRA, "");
	}

	// TODO
	return CMD_EXEC_DONE;
}

void MTNewCmd::usage(ostream &os) const
{
	os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void MTNewCmd::help() const
{
	cout << setw(15) << left << "MTNew: "
		 << "(memory test) new objects" << endl;
}

//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string &option)
{
	size_t numOlist = mtest.getObjListSize();
	size_t numAlist = mtest.getArrListSize();
	vector<string> token;
	CmdExec::lexOptions(option, token);
	if (token.size() == 0)
	{
		return CmdExec::errorOption(CMD_OPT_MISSING, "");
	}
	else if (token.size() == 2)
	{
		int c;
		if (!(myStrNCmp("-Index", token[0], 2) == 0 || myStrNCmp("-Random", token[0], 2) == 0))
		{
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[0]);
		}
		else if (!myStr2Int(token[1], c) || c < 0)
		{
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[1]);
		}
		if (!myStrNCmp("-Index", token[0], 2))
		{
			if (mtest.getObjListSize() <= c)
			{
				char str[1000];
				sprintf(str, "%d", c);
				cerr << "Size of object list (" << mtest.getObjListSize() << ") is <= " << c << "!!" << endl;
				return CmdExec::errorOption(CMD_OPT_ILLEGAL, str);
			}
			mtest.deleteObj(c);
		}
		if (!myStrNCmp("-Random", token[0], 2))
		{
			if (numOlist != 0)
			{
				for (int i = 0; i < c; i++)
				{
					mtest.deleteObj(rnGen(numOlist));
				}
			}
		}
	}
	else if (token.size() == 3)
	{
		int c;
		if (!(myStrNCmp("-Index", token[0], 2) == 0 || myStrNCmp("-Random", token[0], 2) == 0))
		{
			if (myStrNCmp("-Array", token[0], 2) == 0)
			{
				if (!myStrNCmp("-Index", token[1], 2))
				{
					if (myStr2Int(token[2], c) && c >= 0)
					{
						if (mtest.getArrListSize() <= c)
						{
							char str[1000];
							sprintf(str, "%d", c);
							cerr << "Size of object list (" << mtest.getArrListSize() << ") is <= " << c << "!!" << endl;
							return CmdExec::errorOption(CMD_OPT_ILLEGAL, str);
						}
						mtest.deleteArr(c);
						return CMD_EXEC_DONE;
					}
					return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[2]);
				}
				else if (!myStrNCmp("-Random", token[1], 2))
				{
					if (myStr2Int(token[2], c) && c >= 0)
					{
						if (numAlist != 0)
						{
							for (int i = 0; i < c; i++)
							{
								mtest.deleteArr(rnGen(numAlist));
							}
						}
						return CMD_EXEC_DONE;
					}
					return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[2]);
				}
				return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[1]);
			}

			return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[0]);
		}
		else if (!myStr2Int(token[1], c) || c < 0)
		{
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[1]);
		}
		else if (!myStrNCmp("-Array", token[2], 2) == 0)
		{
			return CmdExec::errorOption(CMD_OPT_ILLEGAL, token[2]);
		}
		if (!myStrNCmp("-Index", token[0], 2))
		{
			if (mtest.getArrListSize() <= c)
			{
				char str[1000];
				sprintf(str, "%d", c);
				cerr << "Size of object list (" << mtest.getArrListSize() << ") is <= " << c << "!!" << endl;
				return CmdExec::errorOption(CMD_OPT_ILLEGAL, str);
			}
			mtest.deleteArr(c);
		}
		if (!myStrNCmp("-Random", token[0], 2))
		{
			if (numAlist != 0)
			{
				for (int i = 0; i < c; i++)
				{
					mtest.deleteArr(rnGen(numAlist));
				}
			}
		}
	}
	else
	{
		return CmdExec::errorOption(CMD_OPT_EXTRA, "");
	}
	// TODO

	return CMD_EXEC_DONE;
}

void MTDeleteCmd::usage(ostream &os) const
{
	os << "Usage: MTDelete <-Index (size_t objId) | "
	   << "-Random (size_t numRandId)> [-Array]" << endl;
}

void MTDeleteCmd::help() const
{
	cout << setw(15) << left << "MTDelete: "
		 << "(memory test) delete objects" << endl;
}

//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string &option)
{
	// check option
	if (option.size())
		return CmdExec::errorOption(CMD_OPT_EXTRA, option);
	mtest.print();

	return CMD_EXEC_DONE;
}

void MTPrintCmd::usage(ostream &os) const
{
	os << "Usage: MTPrint" << endl;
}

void MTPrintCmd::help() const
{
	cout << setw(15) << left << "MTPrint: "
		 << "(memory test) print memory manager info" << endl;
}