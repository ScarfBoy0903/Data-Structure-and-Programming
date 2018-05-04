/****************************************************************************
  FileName     [ dbTable.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Table member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <string>
#include <string.h>
#include <vector>
#include <cctype>
#include <cassert>
#include <set>
#include <algorithm>
#include "dbTable.h"
#include "util.h"

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream &operator<<(ostream &os, const DBRow &r)
{
  for (int i = 0; i < r._data.size(); i++)
  {
    if (r._data[i] == INT_MAX)
    {
      os << ".";
    }
    else
    {
      os << r._data[i];
    }
    if (i != r._data.size() - 1)
    {
      os << " ";
    }
  }
  // TODO: to print out a row.
  // - Data are seperated by a space. No trailing space at the end.
  // - Null cells are printed as '.'
  return os;
}

ostream &operator<<(ostream &os, const DBTable &t)
{
  for (int i = 0; i < t.nRows(); i++)
  {
    for (int j = 0; j < t.nCols(); j++)
    {
      if (t._table[i][j] != INT_MAX)
      {
        os << right << setw(6) << t._table[i][j];
      }
      else
      {
        os << right << setw(6) << ".";
      }
    }
    os << endl;
  }
  return os;
}

ifstream &operator>>(ifstream &ifs, DBTable &t)
{
  int row_number = 0;
  int column_number = 0;
  /*--------------calculate row_numbers------------------*/
  string *line = new string[1000000];
  while (getline(ifs, line[row_number], '\n'))
  {
    // cout << line[row_number] << endl;
    row_number = row_number + 1;
  }
  t._table.resize(row_number);
  char *temp = new char[20000000];
  strcpy(temp, line[0].c_str());

  /*--------------calculate row_numbers------------------*/

  /*--------------calculate nCols()s------------------*/
  int i = 0;
  while (true)
  {
    if (temp[i] == ',')
    {
      column_number++;
    }
    i = i + 1;
    if (temp[i] == 0)
    {
      column_number++;
      break;
    }
  }
  /*--------------calculate nCols()s------------------*/

  /*--------------input data to _table[][]------------------*/
  for (int i = 0; i < 20000000; i++)
  {
    temp[i] = 0;
  }
  for (int i = 0; i < row_number; i++)
  {
    t._table[i].set_column(column_number);
    strcpy(temp, line[i].c_str());
    int a = 0;
    int b = 0;
    string str_temp;
    while (temp[a] != 0)
    {
      if (temp[a] == '0' || temp[a] == '1' || temp[a] == '2' || temp[a] == '3' || temp[a] == '4' || temp[a] == '5' || temp[a] == '6' || temp[a] == '7' || temp[a] == '8' || temp[a] == '9')
      {

        if (temp[a + 1] == '0' || temp[a + 1] == '1' || temp[a + 1] == '2' || temp[a + 1] == '3' || temp[a + 1] == '4' || temp[a + 1] == '5' || temp[a + 1] == '6' || temp[a + 1] == '7' || temp[a + 1] == '8' || temp[a + 1] == '9')
        {
          str_temp = str_temp + temp[a];
        }
        else if (temp[a + 1] == ',' || (!(temp[a + 1] == '0' || temp[a + 1] == '1' || temp[a + 1] == '2' || temp[a + 1] == '3' || temp[a + 1] == '4' || temp[a + 1] == '5' || temp[a + 1] == '6' || temp[a + 1] == '7' || temp[a + 1] == '8' || temp[a + 1] == '9') && !(temp[a + 1] == ',')))
        {
          str_temp = str_temp + temp[a];
          t._table[i][b] = atoi(str_temp.c_str());
          str_temp.clear();
          b = b + 1;
        }
      }
      else if (temp[a] == ',')
      {
        if (a == 0)
        {
          t._table[i][b] = INT_MAX;
          b = b + 1;
        }
        if (temp[a + 1] == ',' || (!(temp[a + 1] == '0' || temp[a + 1] == '1' || temp[a + 1] == '2' || temp[a + 1] == '3' || temp[a + 1] == '4' || temp[a + 1] == '5' || temp[a + 1] == '6' || temp[a + 1] == '7' || temp[a + 1] == '8' || temp[a + 1] == '9') && !(temp[a + 1] == ',')))
        {
          t._table[i][b] = INT_MAX;
          b = b + 1;
        }
      }
      if (b > column_number)
      {
        break;
      }
      a++;
    }
  }
  /*--------------input data to _table[][]------------------*/
  delete[] temp;
  delete[] line;
  return ifs;
}

/*****************************************/
/*   Member Functions for class DBRow    */
/*****************************************/
void DBRow::removeCell(size_t c)
{
  _data.erase(_data.begin() + c);
  // TODO
}

/*****************************************/
/*   Member Functions for struct DBSort  */
/*****************************************/
bool DBSort::operator()(const DBRow &r1, const DBRow &r2) const
{
  for (int i = 0; i < _sortOrder.size(); i++)
  {
    if (r1[_sortOrder[i]] < r2[_sortOrder[i]])
    {
      return true;
    }
    else if (r1[_sortOrder[i]] > r2[_sortOrder[i]])
    {
      return false;
    }
  }

  // TODO: called as a functional object that compares the data in r1 and r2
  //       based on the order defined in _sortOrder
  // return false;
}

/*****************************************/
/*   Member Functions for class DBTable  */
/*****************************************/
void DBTable::reset()
{
  int i = 0;
  while (_table.size() != 0)
  {
    _table[_table.size() - 1].reset();
    // row_number--;
    _table.pop_back();
  }
  // nCols() = row_number = 0;
}

void DBTable::addCol(const vector<int> &d)
{
  for (int i = 0; i < nRows(); i++)
  {
    if (i < d.size())
    {
      _table[i].addData(d[i]);
    }
    else
    {
      _table[i].addData(INT_MAX);
    }
  }
  // nCols()++;
  // TODO: add a nCols() to the right of the table. Data are in 'd'.
}

void DBTable::delRow(int c)
{
  if (c >= nRows() || c < 0)
  {
    cout << "Error: Row index " << c << " is out of range!!" << endl;
  }
  else
  {
    _table[c].reset();
    _table.erase(_table.begin() + c);
  }
}

void DBTable::delCol(int c)
{
  int input_clo = c;
  if (input_clo >= nCols() || input_clo < 0)
  {
    cout << "Error: Column index " << input_clo << " is out of range!!" << endl;
  }
  else
  {
    for (size_t i = 0, n = _table.size(); i < n; ++i)
    {
      _table[i].removeCell(c);
    }
  }
  // cout << "Row " << c << " is deleted.";
}

// For the following getXXX() functions...  (except for getCount())
// - Ignore null cells
// - If all the cells in nCols() #c are null, return NAN
// - Return "float" because NAN is a float.
float DBTable::getMax(size_t c) const
{
  int input_clo = c;
  if (input_clo >= nCols() || input_clo < 0)
  {
    cout << "Error: Column index " << input_clo << " is out of range!!" << endl;
    return NAN;
  }
  else
  {
    int answer = _table[0][input_clo];
    for (int i = 0; i < nRows(); i++)
    {
      if (_table[i][input_clo] != INT_MAX)
      {
        if (answer < _table[i][input_clo])
        {
          answer = _table[i][input_clo];
        }
      }
    }
    if (answer == INT_MAX)
    {
      return NAN;
    }
    return answer;
  }
}

float DBTable::getMin(size_t c) const
{
  int input_clo = c;
  if (input_clo >= nCols() || input_clo < 0)
  {
    cout << "Error: Column index " << input_clo << " is out of range!!" << endl;
    return NAN;
  }
  else
  {
    int answer = _table[0][input_clo];
    for (int i = 0; i < nRows(); i++)
    {
      if (_table[i][input_clo] != INT_MAX)
      {
        if (answer > _table[i][input_clo])
        {
          answer = _table[i][input_clo];
        }
      }
    }
    if (answer == INT_MAX)
    {
      return NAN;
    }
    return answer;
  }
}

float DBTable::getSum(size_t c) const
{
  bool donan = true;
  int input_clo = c;
  if (input_clo >= nCols() || input_clo < 0)
  {
    cout << "Error: Column index " << input_clo << " is out of range!!" << endl;
    return NAN;
  }
  else
  {
    int answer = 0;
    for (int i = 0; i < nRows(); i++)
    {
      if (_table[i][input_clo] != INT_MAX)
      {
        donan = false;
        answer = answer + _table[i][input_clo];
      }
    }
    
    if (donan = true)
    {
      return NAN;
    }
    return answer;
  }
}
int DBTable::getCount(size_t c) const
{
  int input_clo = c;
  if (input_clo >= nCols() || input_clo < 0)
  {
    cout << "Error: Column index " << input_clo << " is out of range!!" << endl;
    return NAN;
  }
  else
  {
    int answer = nRows();
    int *same_temp = new int[nRows()];
    for (int i = 0; i < nRows(); i++)
    {
      same_temp[i] = _table[i][input_clo];
    }
    for (int i = 0; i < nRows(); i++)
    {
      for (int j = i + 1; j < nRows(); j++)
      {
        if (_table[i][input_clo] == same_temp[j])
        {
          same_temp[j] = INT_MAX;
        }
      }
    }
    for (int i = 0; i < nRows(); i++)
    {
      if (same_temp[i] == INT_MAX)
      {
        answer--;
      }
    }
    delete[] same_temp;
    return answer;
  }
}

float DBTable::getAve(size_t c) const
{
  bool donan = true;
  int input_clo = c;
  if (input_clo >= nCols() || input_clo < 0)
  {
    cout << "Error: Column index " << input_clo << " is out of range!!" << endl;
    // return NAN;
  }
  else
  {
    double answer = 0;
    double number = 0;
    for (int i = 0; i < nRows(); i++)
    {
      if (_table[i][input_clo] != INT_MAX)
      {
        donan = false;
        answer = answer + _table[i][input_clo];
        number++;
      }
    }
    if (donan = true)
    {
      return NAN;
    }
    answer = answer / number;
    return answer;
  }
}

void DBTable::sort(const struct DBSort &s)
{
  ::sort(_table.begin(), _table.end(), s);
  // TODO: sort the data according to the order of nCols()s in 's'
}

void DBTable::printCol(size_t c) const
{
  for (int i = 0; i < nRows(); i++)
  {
    if (_table[i][c] == INT_MAX)
    {
      cout << ".";
    }
    else
    {
      cout << _table[i][c];
    }
    if (i != nRows() - 1)
    {
      cout << " ";
    }
  }
  // TODO: to print out a nCols().
  // - Data are seperated by a space. No trailing space at the end.
  // - Null cells are printed as '.'
}

void DBTable::printSummary() const
{
  size_t nr = nRows(), nc = nCols(), nv = 0;
  for (size_t i = 0; i < nr; ++i)
    for (size_t j = 0; j < nc; ++j)
      if (_table[i][j] != INT_MAX)
        ++nv;
  cout << "(#rows, #cols, #data) = (" << nr << ", " << nc << ", "
       << nv << ")" << endl;
}
