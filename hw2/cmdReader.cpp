
/****************************************************************************
  FileName     [ cmdReader.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command line reader member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <cstring>
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    Extrenal funcitons
//----------------------------------------------------------------------
void mybeep();
char mygetc(istream&);
ParseChar getChar(istream&);
int his_row_num = 0;
string history_temp;
int last_add_difference;

//----------------------------------------------------------------------
//    Member Function for class Parser
//----------------------------------------------------------------------
void
CmdParser::readCmd()
{
  _history.resize(100000);
  _historyIdx = 0; 
  if (_dofile.is_open()) {
      readCmdInt(_dofile);
      _dofile.close();
   }
   else{
      readCmdInt(cin);
   }
}

void
CmdParser::readCmdInt(istream& istr)
{
   resetBufAndPrintPrompt();
   
   while (1) {
      ParseChar pch = getChar(istr);

      for (int i = 0; i < &_readBuf[READ_BUF_SIZE - 1] - _readBufEnd; i ++){
        *(_readBufEnd + i) = 0;
      }

      if (pch == INPUT_END_KEY) break;
      switch (pch) {
         case LINE_BEGIN_KEY : 
         case HOME_KEY       : moveBufPtr(_readBuf); break;
         case LINE_END_KEY   :
         case END_KEY        : moveBufPtr(_readBufEnd); break;
         case BACK_SPACE_KEY : 
         {
          if (_readBufPtr != _readBuf){
            *(_readBufPtr - 1) = 0;
            strcpy (_readBufPtr - 1,_readBufPtr);
            * _readBufEnd = 0;
            _readBufEnd --;
            _readBufPtr --;
            string temp;
            for (int i = 0; i < _readBufEnd - _readBufPtr + 1; i ++){
              temp = temp + "\b";
            }
            if (_historyIdx == his_row_num){
              history_temp.assign(_readBuf);
            }    
            cout << '\b' << _readBufPtr << " " << temp;
          }
          else if (_readBufPtr == _readBuf){
            mybeep();
          }
         }
         break;
         case DELETE_KEY     : deleteChar(); break;
         case NEWLINE_KEY    : addHistory();
                               cout << char(NEWLINE_KEY);
                               resetBufAndPrintPrompt(); break;
         case ARROW_UP_KEY   : moveToHistory(_historyIdx - 1); break;
         case ARROW_DOWN_KEY : moveToHistory(_historyIdx + 1); break;
         case ARROW_RIGHT_KEY: 
         {
          if (_readBufPtr != _readBufEnd){
            cout << *_readBufPtr;
            _readBufPtr ++;
          }
          else if (_readBufPtr == _readBufEnd){
            mybeep();
          }
         }
         break;

         case ARROW_LEFT_KEY : 
         {
          if (_readBufPtr != _readBuf){
            cout << '\b';
            _readBufPtr --;
          }
          else if (_readBufPtr == _readBuf){
            mybeep();
          }
         }
         break;
         case PG_UP_KEY      : moveToHistory(_historyIdx - PG_OFFSET); break;
         case PG_DOWN_KEY    : moveToHistory(_historyIdx + PG_OFFSET); break;
         case TAB_KEY        : 
         {
          int start = _readBufPtr - _readBuf + 1;
          int multiple_index = start;
          while (true){
            if (multiple_index % 8 == 0){
              break;
            }
            multiple_index ++;
          }
          if (_readBufPtr == _readBufEnd){
            _readBufEnd = &(_readBuf[multiple_index]);
            string temp;
            for (int i = 0;i < _readBufEnd - _readBufPtr; i ++){
              * (_readBufPtr + i) = 32;
              temp = temp + " ";
            }
            if (_historyIdx == his_row_num){
              history_temp.assign(_readBuf);
            }    
            _readBufPtr = _readBufEnd;
            cout << temp;
          }
          else if (_readBufPtr != _readBufEnd){
            int ptr_difference = _readBufEnd - _readBufPtr;
            strcpy(&_readBuf[multiple_index],_readBufPtr);
            *(_readBufPtr) = 32;
            _readBufPtr = &(_readBuf[multiple_index]);
            string temp = " ";
            for (int i = 0; i < multiple_index - start; i++){
              temp = temp + " ";
              _readBuf[start + i] = 32;
            }
            string back;
            for (int i = 0; i < ptr_difference; i++){
              back = back + "\b";
            }
            if (_historyIdx == his_row_num){
              history_temp.assign(_readBuf);
            }    
            _readBufEnd = _readBufPtr + ptr_difference;
            cout << temp << _readBufPtr << back;
          }
          last_add_difference = _readBufEnd - _readBuf;
         }
         break;
         
         case INSERT_KEY     : // not yet supported; fall through to UNDEFINE
         case UNDEFINED_KEY:   mybeep(); break;
         default:  // printable character
          insertChar(char(pch)); break;
      }
      #ifdef TA_KB_SETTING
      taTestOnly();
      #endif
   }
}

bool
CmdParser::moveBufPtr(char* const ptr)
{
  if (ptr == _readBuf){
    string temp;
    for (int i = 0; i < _readBufPtr - ptr; i ++){
      temp = temp + "\b";
    }
    cout << temp;
    _readBufPtr = ptr;
  }
  else if (ptr == _readBufEnd){
    cout << _readBufPtr;
    _readBufPtr = ptr;
  }
  return true;
}

bool
CmdParser::deleteChar()
{
  if (_readBufPtr != _readBufEnd){
    *_readBufPtr = 0;
    strcpy (_readBufPtr,_readBufPtr + 1);
    * _readBufEnd = 0;
    _readBufEnd --;
    string temp;
    for (int i = 0; i < _readBufEnd - _readBufPtr + 1; i ++){
      temp = temp + "\b";
    }
    if (_historyIdx == his_row_num){
      history_temp.assign(_readBuf);
    }    
    cout << _readBufPtr << " " << temp;
  }
  else if (_readBufPtr == _readBufEnd){
    mybeep();
  }
  return true;
}

void
CmdParser::insertChar(char ch, int repeat)
{
  assert(repeat >= 1);
  if (_readBufPtr == _readBufEnd){
    _readBufPtr ++;
    _readBufEnd ++; 
    *(_readBufPtr - 1) = ch;
    cout << ch; 
    if (_historyIdx == his_row_num){
      history_temp.assign(_readBuf);
    }       
  }
  else if (_readBufPtr != _readBufEnd){
    if (_readBufPtr != _readBuf){
      strcpy (_readBufPtr,_readBufPtr - 1);
    }
    else if (_readBufPtr == _readBuf){
      strcpy (_readBufPtr + 1,_readBufPtr);
    }
    *(_readBufPtr) = ch;
    string temp;
    for (int i = 0; i < _readBufEnd - _readBufPtr; i ++){
      temp = temp + "\b";
    }
    _readBufPtr ++;
    _readBufEnd ++;
    cout << ch << _readBufPtr << temp;
    if (_historyIdx == his_row_num){
      history_temp.assign(_readBuf);
    }  
  }
  last_add_difference = _readBufEnd - _readBuf;
}

void
CmdParser::deleteLine()
{
  for (int i = 0; i < sizeof(_readBuf); i ++){
    _readBuf[i] = 0;
  }
}

void
CmdParser::moveToHistory(int index)
{ 
  int index_difference;
  if (index > _historyIdx){
    index_difference = index - _historyIdx;
  }
  else{
    index_difference = _historyIdx - index;
  }
  _historyIdx = index;
  if (_historyIdx < 0){
    mybeep();
    _historyIdx = _historyIdx + index_difference;
  }
  else if (_historyIdx > his_row_num){
    mybeep();
    _historyIdx = _historyIdx - index_difference;
  }
  else{
    for (int i = 0; i < _readBufPtr - _readBuf; i ++){
      cout << "\b \b";
    }
    
    int down_difference;
    retrieveHistory();
    if (_historyIdx != his_row_num){
      int up_difference;
      if (_history[_historyIdx + 1].size() > _history[_historyIdx].size()){
        up_difference = _history[_historyIdx + 1].size() - _history[_historyIdx].size();
      }
      else{
        up_difference = _history[_historyIdx].size() - _history[_historyIdx + 1].size();
      }
      for (int i = 0; i < up_difference + last_add_difference; i ++){
        cout << " ";
      }
      for (int i = 0; i < up_difference + last_add_difference; i ++){
        cout << "\b";
      }  
    }
    if (_historyIdx != 0 && _historyIdx != his_row_num){
      int down_difference;
      if (_history[_historyIdx - 1].size() > _history[_historyIdx].size()){
        down_difference = _history[_historyIdx - 1].size() - _history[_historyIdx].size();
      }
      else {
        down_difference = _history[_historyIdx].size() - _history[_historyIdx - 1].size();
      }
      for (int i = 0; i < down_difference + last_add_difference; i ++){
        cout << " ";
      }
      for (int i = 0; i < down_difference + last_add_difference; i ++){
        cout << "\b";
      }  
    }
    else if (_historyIdx == his_row_num){
      int down_difference;
      if (_history[_historyIdx - 1].size() > history_temp.size()){
        down_difference = _history[_historyIdx - 1].size() - history_temp.size();
      }
      else{
        down_difference = history_temp.size() -_history[_historyIdx - 1].size();
      }
      for (int i = 0; i < down_difference + last_add_difference; i ++){
        cout << " ";
      }
      for (int i = 0; i < down_difference + last_add_difference; i ++){
        cout << "\b";
      }  
    }
    if (_historyIdx == his_row_num - 1){
      int up_difference;
      if (_history[_historyIdx].size() > history_temp.size()){
        up_difference = _history[_historyIdx].size() - history_temp.size();
      }
      else {
        up_difference = history_temp.size() - _history[_historyIdx].size();
      }
      for (int i = 0; i < up_difference + last_add_difference; i ++){
        cout << " ";
      }
      for (int i = 0; i < up_difference + last_add_difference; i ++){
        cout << "\b";
      }  
    }
  }
}

void
CmdParser::addHistory()
{ 
  for (int i = 1; i <= _readBufEnd - _readBuf; i ++){
    if (*(_readBufEnd - i) == 32){
      if (*(_readBufEnd - i - 1) != 32){
        *(_readBufEnd - i) = 0;
        break;
      }
      *(_readBufEnd - i) = 0;
    }
    else if (*(_readBufEnd - i) != 32){
      break;
    }
  }
  if (_readBuf[0] != 0){
    _history [his_row_num].assign(_readBuf);
    his_row_num ++;
    _historyIdx = his_row_num;
  }
  history_temp.clear();
  deleteLine();
}

void
CmdParser::retrieveHistory()
{
   deleteLine();
   if (_historyIdx == his_row_num){
    strcpy(_readBuf, history_temp.c_str());
    _readBufPtr = _readBufEnd = _readBuf + history_temp.size();
   } 
   else{
    strcpy(_readBuf, _history[_historyIdx].c_str());
    _readBufPtr = _readBufEnd = _readBuf + _history[_historyIdx].size();
   }
   cout << _readBuf;
}