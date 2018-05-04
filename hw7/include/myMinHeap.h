/****************************************************************************
  FileName     [ myMinHeap.h ]
  PackageName  [ util ]
  Synopsis     [ Define MinHeap ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>

template <class Data>
class MinHeap
{
public:
  MinHeap(size_t s = 0)
  {
    if (s != 0)
      _data.reserve(s);
  }
  ~MinHeap() {}

  void clear() { _data.clear(); }

  // For the following member functions,
  // We don't respond for the case vector "_data" is empty!
  const Data &operator[](size_t i) const { return _data[i]; }
  Data &operator[](size_t i) { return _data[i]; }

  size_t size() const { return _data.size(); }

  // TODO
  const Data &min() const { return _data[0]; }
  void insert(const Data &d)
  {
    _data.push_back(d);
    int i = _data.size() - 1;
    float_up(i);
  }

  void delMin()
  {
    _data[0] = _data[_data.size() - 1];
    _data.pop_back();
    sink(0);
  }

  void delData(size_t i)
  {
    _data[i] = _data[_data.size() - 1];
    _data.pop_back();
    if (!float_up(i))
    {
      sink(i);
    }
  }

  bool float_up(size_t i)
  {
    if (i <= 0 || _data[(i - 1) / 2] < _data[i])
    {
      return false;
    }
    while (i > 0 && _data[i] < _data[(i - 1) / 2])
    {
      exchange_node(_data[i], _data[(i - 1) / 2]);
      i = (i - 1) / 2;
    }
    return true;
  }

  void sink(size_t i)
  {
    int j = 0;
    while (true)
    {
      if (2 * i + 2 < _data.size())
      {
        if (_data[i].getLoad() <= _data[2 * i + 1].getLoad() && _data[i].getLoad() <= _data[2 * i + 2].getLoad())
          break;
        if (_data[2 * i + 2] < _data[2 * i + 1])
          j = 2 * i + 2;
        else
          j = 2 * i + 1;
      }
      else if (2 * i + 2 == _data.size())
      {
        if (_data[i].getLoad() <= _data[2 * i + 1].getLoad())
          break;
        j = 2 * i + 1;
      }
      else
        break;
      exchange_node(_data[i], _data[j]);
      i = j;
    }
  }

  void exchange_node(Data &a, Data &b)
  {
    Data temp = a;
    a = b;
    b = temp;
  }

private:
  // DO NOT add or change data members
  vector<Data> _data;
};

#endif // MY_MIN_HEAP_H
