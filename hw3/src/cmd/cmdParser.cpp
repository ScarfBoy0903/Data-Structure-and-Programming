/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "util.h"
#include "cmdParser.h"
#include <string.h>
#include <algorithm>
#include <string>
// #include <dirent.h>

using namespace std;

//----------------------------------------------------------------------
//    External funcitons
//----------------------------------------------------------------------
void mybeep();
bool have_matched = false;
bool first_match = true;
bool once = false;
bool have_print_usage = false;
vector<string> filelist;
//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool CmdParser::openDofile(const string &dof)
{
  _dofileStack.push(_dofile);
  _dofile = new ifstream(dof.c_str());
  if (_dofile->fail())
  {
    delete _dofile;
    _dofile = NULL;
    _dofile = _dofileStack.top();
    _dofileStack.pop();
    return false;
  }
  return _dofile;
}

// Must make sure _dofile != 0
void CmdParser::closeDofile()
{
  if (_dofile != NULL)
  {
    if (_dofile->is_open())
    {
      _dofile->close();
      delete _dofile;
      _dofile = NULL;
      _dofile = _dofileStack.top();
      _dofileStack.pop();
    }
  }
}

// Return false if registration fails
bool CmdParser::regCmd(const string &cmd, unsigned nCmp, CmdExec *e)
{
  // Make sure cmd hasn't been registered and won't cause ambiguity
  string str = cmd;
  unsigned s = str.size();
  if (s < nCmp)
    return false;
  while (true)
  {
    if (getCmd(str))
      return false;
    if (s == nCmp)
      break;
    str.resize(--s);
  }

  // Change the first nCmp characters to upper case to facilitate
  //    case-insensitive comparison later.
  // The strings stored in _cmdMap are all upper case
  //
  assert(str.size() == nCmp); // str is now mandCmd
  string &mandCmd = str;
  for (unsigned i = 0; i < nCmp; ++i)
    mandCmd[i] = toupper(mandCmd[i]);
  string optCmd = cmd.substr(nCmp);
  // cout << optCmd;
  assert(e != 0);
  e->setOptCmd(optCmd);

  // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
  return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
  bool newCmd = false;
  if (_dofile != 0)
    newCmd = readCmd(*_dofile);
  else
    newCmd = readCmd(cin);

  // execute the command
  if (newCmd) //->after pressing enter key
  {
    string option;
    CmdExec *e = parseCmd(option);
    if (e != 0)
      return e->exec(option);
  }
  return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void CmdParser::printHelps() const
{
  cerr << setw(15) << left << "DBAPpend: "
       << "append a row or column of data to the table" << endl
       << setw(15) << left << "DBAVerage: "
       << "compute the average of a column" << endl
       << setw(15) << left << "DBCount: "
       << "report the distinct count of data in a column" << endl
       << setw(15) << left << "DBDelete: "
       << "delete a row or column from the table" << endl
       << setw(15) << left << "DBMAx: "
       << "report the maximum number of a column" << endl
       << setw(15) << left << "DBMIn: "
       << "report the minimum number of a column" << endl
       << setw(15) << left << "DBPrint: "
       << "print the data in the table" << endl
       << setw(15) << left << "DBRead: "
       << "read data from .csv file" << endl
       << setw(15) << left << "DBSort: "
       << "sort the data in the table" << endl
       << setw(15) << left << "DBSUm: "
       << "compute the summation of a column" << endl
       << setw(15) << left << "DOfile: "
       << "execute the commands in the dofile" << endl
       << setw(15) << left << "HELp: "
       << "print this help message" << endl
       << setw(15) << left << "HIStory: "
       << "print command history" << endl
       << setw(15) << left << "Quit: "
       << "quit the execution" << endl;
}

void CmdParser::printHistory(int nPrint) const
{
  assert(_tempCmdStored == false);
  if (_history.empty())
  {
    cout << "Empty command history!!" << endl;
    return;
  }
  int s = _history.size();
  if ((nPrint < 0) || (nPrint > s))
    nPrint = s;
  for (int i = s - nPrint; i < s; ++i)
    cout << "   " << i << ": " << _history[i] << endl;
}

//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
CmdExec *
CmdParser::parseCmd(string &option)
{
  have_matched = true;
  first_match = true;
  have_print_usage = false;
  assert(_tempCmdStored == false);
  assert(!_history.empty());
  string str = _history.back();

  char *temp = new char[str.size()];
  char *temp_original = new char[str.size()];
  vector<string> cmd_temp;
  strcpy(temp, str.c_str());
  char *pch;
  char *delim = " ";
  pch = strtok(temp, delim);
  while (pch != NULL)
  {
    cmd_temp.push_back(pch);
    pch = strtok(NULL, delim);
  }
  delete[] temp;
  temp = 0;
  str = cmd_temp[0];
  char *cmd = new char[cmd_temp[0].size()];
  strcpy(cmd, cmd_temp[0].c_str());
  for (int i = 0; i < cmd_temp[0].size(); i++)
  {
    cmd[i] = toupper(cmd[i]);
  }
  cmd_temp[0].assign(cmd);

  for (int i = 1; i < cmd_temp.size(); i++)
  {
    if (i == cmd_temp.size() - 1)
    {
      option = option + cmd_temp[i];
    }
    else
    {
      option = option + cmd_temp[i] + " ";
    }
  }
  // cout << "test" << endl;
  if ((cmd_temp[0] == "HIS") || (cmd_temp[0] == "HIST") ||
      (cmd_temp[0] == "HISTO") || (cmd_temp[0] == "HISTOR") ||
      (cmd_temp[0] == "HISTORY"))
  {
    return getCmd("HIS");
  }

  else if (cmd_temp[0] == "Q" || cmd_temp[0] == "QU" ||
           cmd_temp[0] == "QUI" || cmd_temp[0] == "QUIT")
  {
    return getCmd("Q");
  }

  else if ((cmd_temp[0] == "HEL") || (cmd_temp[0] == "HELP"))
  {
    return getCmd("HEL");
  }

  else if ((cmd_temp[0] == "DO") || (cmd_temp[0] == "DOF") ||
           (cmd_temp[0] == "DOFI") || (cmd_temp[0] == "DOFIL") ||
           (cmd_temp[0] == "DOFILE"))
  {
    return getCmd("DO");
  }

  else if ((cmd_temp[0] == "DBAP") || (cmd_temp[0] == "DBAPP") ||
           (cmd_temp[0] == "DBAPPE") || (cmd_temp[0] == "DBAPPEN") ||
           (cmd_temp[0] == "DBAPPEND"))
  {
    // cout << "test" << endl;
    return getCmd("DBAP");
  }

  else if ((cmd_temp[0] == "DBAV") || (cmd_temp[0] == "DBAVE") ||
           (cmd_temp[0] == "DBAVER") || (cmd_temp[0] == "DBAVERA") ||
           (cmd_temp[0] == "DBAVERAG") || (cmd_temp[0] == "DBAVERAGE"))
  {
    return getCmd("DBAV");
  }

  else if ((cmd_temp[0] == "DBC") || (cmd_temp[0] == "DBCO") ||
           (cmd_temp[0] == "DBCOU") || (cmd_temp[0] == "DBCOUN") ||
           (cmd_temp[0] == "DBCOUNT"))
  {
    return getCmd("DBC");
  }

  else if ((cmd_temp[0] == "DBD") || (cmd_temp[0] == "DBDE") ||
           (cmd_temp[0] == "DBDEL") || (cmd_temp[0] == "DBDELE") ||
           (cmd_temp[0] == "DBDELET") || (cmd_temp[0] == "DBDELETE"))
  {
    return getCmd("DBD");
  }

  else if ((cmd_temp[0] == "DBMA") || (cmd_temp[0] == "DBMAX"))
  {
    return getCmd("DBMA");
  }

  else if ((cmd_temp[0] == "DBMI") || (cmd_temp[0] == "DBMIN"))
  {
    return getCmd("DBMI");
  }

  else if ((cmd_temp[0] == "DBP") || (cmd_temp[0] == "DBPR") ||
           (cmd_temp[0] == "DBPRI") || (cmd_temp[0] == "DBPRIN") ||
           (cmd_temp[0] == "DBPRINT"))
  {
    return getCmd("DBP");
  }

  else if ((cmd_temp[0] == "DBR") || (cmd_temp[0] == "DBRE") ||
           (cmd_temp[0] == "DBREA") || (cmd_temp[0] == "DBREAD"))
  {
    return getCmd("DBR");
  }

  else if ((cmd_temp[0] == "DBSO") || (cmd_temp[0] == "DBSOR") ||
           (cmd_temp[0] == "DBSORT"))
  {
    return getCmd("DBSO");
  }

  else if ((cmd_temp[0] == "DBSU") || (cmd_temp[0] == "DBSUM"))
  {
    return getCmd("DBSU");
  }

  assert(str[0] != 0 && str[0] != ' ');

  cerr << "Illegal command!! (" << str << ")" << endl;

  return NULL;
}

// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. LIST ALL COMMANDS
//    --- 1.1 ---
//    [Before] Null cmd
//    cmd> $
//    --- 1.2 ---
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. LIST ALL PARTIALLY MATCHED COMMANDS
//    --- 2.1 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    --- 2.2 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$llo                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$llo                // and then re-print the partial command
//
// 3. LIST THE SINGLY MATCHED COMMAND
//    ==> In either of the following cases, print out cmd + ' '
//    ==> and reset _tabPressCount to 0
//    --- 3.1 ---
//    [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $               // auto completed with a space inserted
//    --- 3.2 ---
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$ahah
//    [After Tab]
//    cmd> heLp $ahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//    --- 3.3 ---
//    [Before] fully matched (cursor right behind cmd)
//    cmd> hElP$sdf
//    [After Tab]
//    cmd> hElP $sdf            // a space character is inserted
//
// 4. NO MATCH IN FITST WORD
//    --- 4.1 ---
//    [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
//    --- 5.1 ---
//    [Before] Already matched on first tab pressing
//    cmd> help asd$gh
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$gh
//
// 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
//    ==> Note: command usage has been printed under first tab press
//    ==> Check the word the cursor is at; get the prefix before the cursor
//    ==> So, this is to list the file names under current directory that
//        match the prefix
//    ==> List all the matched file names alphabetically by:
//           cout << setw(16) << left << fileName;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location
//    --- 6.1 ---
//    [Before] if prefix is empty, print all the file names
//    cmd> help $sdfgh
//    [After]
//    .               ..              Homework_3.docx Homework_3.pdf  Makefile
//    MustExist.txt   MustRemove.txt  bin             dofiles         include
//    lib             mydb            ref             src             testdb
//    cmd> help $sdfgh
//    --- 6.2 ---
//    [Before] with a prefix and with mutiple matched files
//    cmd> help M$Donald
//    [After]
//    Makefile        MustExist.txt   MustRemove.txt
//    cmd> help M$Donald
//    --- 6.3 ---
//    [Before] with a prefix and with mutiple matched files,
//             and these matched files have a common prefix
//    cmd> help Mu$k
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help Must$k
//    --- 6.4 ---
//    [Before] with a prefix and with a singly matched file
//    cmd> help MustE$aa
//    [After] insert the remaining of the matched file name followed by a ' '
//    cmd> help MustExist.txt $aa
//    --- 6.5 ---
//    [Before] with a prefix and NO matched file
//    cmd> help Ye$kk
//    [After] beep and stay in the same location
//    cmd> help Ye$kk
//
//    [Note] The counting of tab press is reset after "newline" is entered.
//
// 7. FIRST WORD NO MATCH
//    --- 7.1 ---
//    [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location

void CmdParser::listCmd(const string &str)
{
  char *keyword;
  char *head;
  char *end;
  char *temp = new char[str.size()];
  // char *temp_original = new char[str.size()];
  strcpy(temp, str.c_str());
  // strcpy(temp_original, str.c_str());
  vector<string> cmd;
  vector<string> recorder;
  string original;
  cmd.push_back("DBAPpend");
  cmd.push_back("DBAVerage");
  cmd.push_back("DBCount");
  cmd.push_back("DBDelete");
  cmd.push_back("DBMAx");
  cmd.push_back("DBMIn");
  cmd.push_back("DBPrint");
  cmd.push_back("DBRead");
  cmd.push_back("DBSOrt");
  cmd.push_back("DBSUm");
  cmd.push_back("DOfile");
  cmd.push_back("HELp");
  cmd.push_back("HIStory");
  cmd.push_back("Quit");
  bool print_all = true;
  bool print_more_than_one = false;
  bool print_one = false;

  for (int i = 0; i < str.size(); i++)
  {
    if (temp[i] != ' ')
    {
      head = &temp[i];
      end = &temp[str.size() - 1];
      print_all = false;
      break;
    }
  }
  if (print_all)
  {
    cout << endl;
    for (int i = 0; i < cmd.size(); i++)
    {
      cout << setw(12) << left << cmd[i];
      if ((i + 1) % 5 == 0 || i == cmd.size() - 1)
      {
        cout << endl;
      }
    }
    cout << "mydb> " << _readBuf;
    for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
    {
      cout << '\b';
    }
  }

  else if (!print_all)
  {
    // cout << endl << *end << endl << *head << endl << end - head + 1 << endl;
    keyword = new char[end - head + 1];
    strncpy(keyword, head, end - head + 1);
    string key;
    original.clear();
    for (int i = 0; i < end - head + 1; i++)
    {
      original = original + keyword[i];
      keyword[i] = toupper(keyword[i]);
      key = key + keyword[i];
    }
    if (first_match == true)
    {
      char *pch;
      char *delim = " ";
      string match_cmd;
      pch = strtok(temp, delim);
      while (pch != NULL)
      {
        match_cmd.assign(pch);
        break;
        pch = strtok(NULL, delim);
      }
      for (int i = 0; i < cmd.size(); i++)
      {
        string str1 = cmd[i];
        string str2 = match_cmd;
        transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
        transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
        if (str1 == str2)
        {
          have_matched = true;
          first_match = false;
          break;
        }
      }
    }
    // cout << endl << match_cmd << endl;
    for (int i = 0; i < cmd.size(); i++)
    {
      string str = cmd[i];
      transform(str.begin(), str.end(), str.begin(), ::toupper);
      if (strstr(str.c_str(), key.c_str()) && (str[0] == key[0]))
      {
        recorder.push_back(cmd[i]);
      }
    }
    // cout << recorder.size();
    if (recorder.size() == 1)
    {
      print_one = true;
    }
    else if (recorder.size() > 1)
    {
      print_more_than_one = true;
    }
    else if (recorder.size() == 0)
    {
      bool error = true;
      char *pch;
      char *delim = " ";
      string match_cmd[2];
      pch = strtok(temp, delim);
      int a = 0;
      while (pch != NULL)
      {
        match_cmd[a].assign(pch);
        a++;
        if (a >= 2)
        {
          break;
        }
        pch = strtok(NULL, delim);
      }
      for (int i = 0; i < cmd.size(); i++)
      {
        string str1 = cmd[i];
        string str2 = match_cmd[0];
        transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
        transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
        if (str1 == str2)
        {
          error = false;
          break;
        }
      }
      listDir(filelist, match_cmd[1], ".");
      if (error == false && filelist.size() == 0 && have_print_usage == true)
      {
        error = true;
      }

      if (filelist.size() != 0 && have_print_usage == true)
      {
        bool the_same_name = true;
        int same_size = filelist[0].size();
        if (filelist.size() > 1)
        {
          for (int i = 0; i < filelist.size(); i++)
          {
            for (int j = i + 1; j < filelist.size(); j++)
            {
              int min_size;
              if (filelist[i].size() > filelist[j].size())
              {
                min_size = filelist[j].size();
              }
              else
              {
                min_size = filelist[i].size();
              }
              // same_size = counters;
              int counters = 0;
              for (int k = 0; k < min_size; k++)
              {
                if (filelist[i][k] != filelist[j][k])
                {
                  break;
                }
                counters++;
              }
              if (counters == 0)
              {
                the_same_name = false;
                break;
              }
              else
              {
                if (same_size > counters)
                {
                  same_size = counters;
                }
              }
            }
            if (the_same_name == false)
            {
              break;
            }
          }
        }
        if (the_same_name == false)
        {
          cout << endl;
          for (int i = 0; i < filelist.size(); i++)
          {
            cout << setw(16) << left << filelist[i];
            if ((i + 1) % 5 == 0 || i == filelist.size() - 1)
            {
              cout << endl;
            }
          }
          cout << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (the_same_name == true)
        {
          if (same_size - match_cmd[1].size() == 0)
          {
            if (filelist.size() != 1)
            {
              cout << endl;
              for (int i = 0; i < filelist.size(); i++)
              {
                cout << setw(16) << left << filelist[i];
                if ((i + 1) % 5 == 0 || i == filelist.size() - 1)
                {
                  cout << endl;
                }
              }
            }
            else
            {
              cout << endl;
            }
            cout << "mydb> " << _readBuf;
            for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
            {
              cout << '\b';
            }
          }
          else
          {
            cout << filelist[0].substr(match_cmd[1].size(), same_size - match_cmd[1].size()) << _readBufPtr;

            string buf_str;
            string insert_str;
            insert_str.assign(filelist[0].substr(match_cmd[1].size(), same_size - match_cmd[1].size()));
            // insert_str = insert_str + " ";
            buf_str.assign(_readBuf);
            buf_str.insert(_readBufPtr - _readBuf, insert_str);
            strcpy(_readBuf, buf_str.c_str());
            _readBufEnd = _readBufEnd + insert_str.size();
            _readBufPtr = _readBufPtr + insert_str.size();
            cout << endl
                 << "mydb> " << _readBuf;
            for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
            {
              cout << '\b';
            }
          }
        }
      }

      filelist.clear();
      //six write here to determine error
      if (error)
      {
        mybeep();
      }
    }
  }

  if (print_more_than_one)
  {
    cout << endl;
    for (int i = 0; i < recorder.size(); i++)
    {
      cout << setw(12) << left << recorder[i];
      if ((i + 1) % 5 == 0 || i == recorder.size() - 1)
      {
        cout << endl;
      }
    }
    cout << "mydb> " << _readBuf;
    for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
    {
      cout << '\b';
    }
  }
  else if (print_one)
  {
    cout << &recorder[0][end - head + 1] << " " << _readBufPtr;
    for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
    {
      cout << '\b';
    }
    string buf_str;
    string insert_str;
    insert_str.assign(&recorder[0][end - head + 1]);
    insert_str = insert_str + " ";
    buf_str.assign(_readBuf);
    buf_str.insert(_readBufPtr - _readBuf, insert_str);
    strcpy(_readBuf, buf_str.c_str());
    _readBufEnd = _readBufEnd + insert_str.size();
    _readBufPtr = _readBufPtr + insert_str.size();
    have_matched = true;
    have_print_usage = false;
  }
  else if (have_matched)
  {
    char *pch;
    char *delim = " ";
    string match_cmd;
    pch = strtok(temp, delim);

    while (pch != NULL)
    {
      match_cmd.assign(pch);
      break;
      pch = strtok(NULL, delim);
    }
    // cout << endl << match_cmd << endl;
    for (int i = 0; i < cmd.size(); i++)
    {
      string str1 = cmd[i];
      string str2 = match_cmd;
      transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
      transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
      if (str1 == str2)
      {
        if (cmd[i] == "DBAPpend")
        {
          cout << endl
               << "Usage: DBAPpend <-Row | -Column> <(int data)...>" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "DBAVerage")
        {
          cout << endl
               << "Usage: DBAVerage <(int colIdx)>" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "DBCount")
        {
          cout << endl
               << "Usage: DBCount <(int colIdx)>" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "DBDelete")
        {
          cout << endl
               << "Usage: DBDelete <-Row | -Column> <(int index)>" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "DBMAx")
        {
          cout << endl
               << "Usage: DBMAx <(int colIdx)>" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "DBMIn")
        {
          cout << endl
               << "Usage: DBMIn <(int colIdx)>" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "DBPrint")
        {
          cout << endl
               << "DBPrint < (int rowIdx) (int colIdx)\n"
               << "        | -Row (int rowIdx) | -Column (colIdx) | -Table | -Summary>" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "DBRead")
        {
          cout << endl
               << "Usage: DBRead <(string csvFile)> [-Replace]" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        // cout << endl << cmd[i] << endl;
        else if (cmd[i] == "DBSOrt")
        {
          // cout << "test" << endl;S
          cout << endl
               << "Usage: DBSOrt <(int colIdx)>..." << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "DBSUm")
        {
          cout << endl
               << "Usage: DBSUm <(int colIdx)>" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "DOfile")
        {
          cout << endl
               << "Usage: DOfile <(string file)>" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "HELp")
        {
          cout << endl
               << "Usage: HELp [(string cmd)]" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "HIStory")
        {
          cout << endl
               << "Usage: HIStory [(int nPrint)]" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
        else if (cmd[i] == "Quit")
        {
          cout << endl
               << "Usage: Quit [-Force]" << endl
               << endl
               << "mydb> " << _readBuf;
          for (int i = 0; i < _readBufEnd - _readBufPtr; i++)
          {
            cout << '\b';
          }
        }
      }
    }
    have_matched = false;
    have_print_usage = true;
  }

  delete[] temp;
  temp = 0;
  // TODO...
}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//

CmdExec *
CmdParser::getCmd(string cmd)
{
  char *temp = new char[cmd.size()];
  strcpy(temp, cmd.c_str());
  for (int i = 0; i < cmd.size(); i++)
  {
    temp[i] = toupper(temp[i]);
  }
  cmd.clear();
  cmd.assign(temp);
  delete[] temp;
  temp = 0;
  // cout << cmd << endl;
  if ((cmd == "HIS") || (cmd == "HIST") ||
      (cmd == "HISTO") || (cmd == "HISTOR") ||
      (cmd == "HISTORY"))
  {
    cmd = "HIS";
  }

  else if (cmd == "Q" || cmd == "QU" ||
           cmd == "QUI" || cmd == "QUIT")
  {
    cmd = "Q";
  }

  else if ((cmd == "HEL") || (cmd == "HELP"))
  {
    cmd = "HEL";
  }

  else if ((cmd == "DO") || (cmd == "DOF") ||
           (cmd == "DOFI") || (cmd == "DOFIL") ||
           (cmd == "DOFILE"))
  {
    cmd = "DO";
  }

  else if ((cmd == "DBAP") || (cmd == "DBAPP") ||
           (cmd == "DBAPPE") || (cmd == "DBAPPEN") ||
           (cmd == "DBAPPEND"))
  {
    cmd = "DBAP";
  }

  else if ((cmd == "DBAV") || (cmd == "DPAVE") ||
           (cmd == "DBAVER") || (cmd == "DBAVERA") ||
           (cmd == "DBAVERAG") || (cmd == "DBAVERAGE"))
  {
    cmd = "DBAV";
  }

  else if ((cmd == "DBC") || (cmd == "DBCO") ||
           (cmd == "DBCOU") || (cmd == "DBCOUN") ||
           (cmd == "DBCOUNT"))
  {
    cmd = "DBC";
  }

  else if ((cmd == "DBD") || (cmd == "DBDE") ||
           (cmd == "DBDEL") || (cmd == "DBDELE") ||
           (cmd == "DBDELET") || (cmd == "DBDELETE"))
  {
    cmd = "DBD";
  }

  else if ((cmd == "DBMA") || (cmd == "DBMAX"))
  {
    cmd = "DBMA";
  }

  else if ((cmd == "DBMI") || (cmd == "DBMIN"))
  {
    cmd = "DBMI";
  }

  else if ((cmd == "DBP") || (cmd == "DBPR") ||
           (cmd == "DBPRI") || (cmd == "DBPRIN") ||
           (cmd == "DBPRINT"))
  {
    cmd = "DBP";
  }

  else if ((cmd == "DBR") || (cmd == "DBRE") ||
           (cmd == "DBREA") || (cmd == "DBREAD"))
  {
    cmd = "DBR";
  }

  else if ((cmd == "DBSO") || (cmd == "DBSOR") ||
           (cmd == "DBSORT"))
  {
    cmd = "DBSO";
  }

  else if ((cmd == "DBSU") || (cmd == "DBSUM"))
  {
    cmd = "DBSU";
  }
  CmdExec *e = 0;
  for (map<const string, CmdExec *>::iterator i = _cmdMap.begin(); i != _cmdMap.end(); i++)
  {
    if (cmd == (*i).first)
    {
      return (*i).second;
    }
  }
  return e;
}

//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool CmdExec::lexSingleOption(const string &option, string &token, bool optional) const
{
  size_t n = myStrGetTok(option, token);
  if (!optional)
  {
    if (token.size() == 0)
    {
      errorOption(CMD_OPT_MISSING, "");
      return false;
    }
  }
  if (n != string::npos)
  {
    errorOption(CMD_OPT_EXTRA, option.substr(n));
    return false;
  }
  return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool CmdExec::lexOptions(const string &option, vector<string> &tokens, size_t nOpts) const
{
  //help -a -r -> vector : -a | -r
  string token;
  size_t n = myStrGetTok(option, token);
  // cout << token << endl;
  while (token.size())
  {
    tokens.push_back(token);
    n = myStrGetTok(option, token, n);
  }

  if (nOpts != 0)
  {
    if (tokens.size() < nOpts)
    {
      errorOption(CMD_OPT_MISSING, "");
      return false;
    }
    if (tokens.size() > nOpts)
    {
      errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
      return false;
    }
  }
  return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string &opt) const
{
  switch (err)
  {
  case CMD_OPT_MISSING:
    cerr << "Error: Missing option";
    if (opt.size())
      cerr << " after (" << opt << ")";
    cerr << "!!" << endl;
    break;
  case CMD_OPT_EXTRA:
    cerr << "Error: Extra option!! (" << opt << ")" << endl;
    break;
  case CMD_OPT_ILLEGAL:
    cerr << "Error: Illegal option!! (" << opt << ")" << endl;
    break;
  case CMD_OPT_FOPEN_FAIL:
    cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
    break;
  default:
    cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
    exit(-1);
  }
  return CMD_EXEC_ERROR;
}
