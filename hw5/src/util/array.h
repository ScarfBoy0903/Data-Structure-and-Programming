/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <algorithm>

using namespace std;

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
  // TODO: decide the initial value for _isSorted
  Array() : _data(0), _size(0), _capacity(0), _isSorted(false) {}
  ~Array() { delete[] _data; }

  // DO NOT add any more data member or function for class iterator
  class iterator
  {
    friend class Array;

  public:
    iterator(T *n = 0) : _node(n) {}
    iterator(const iterator &i) : _node(i._node) {}
    ~iterator() {} // Should NOT delete _node

    // TODO: implement these overloaded operators
    const T &operator*() const { return (*this); }
    T &operator*() { return (*_node); }
    iterator &operator++()
    {
      _node++;
      return *(this);
    }
    iterator operator++(int)
    {
      iterator temp = *(this);
      _node++;
      return temp;
    }
    iterator &operator--()
    {
      _node--;
      return *(this);
    }
    iterator operator--(int)
    {
      iterator temp = *(this);
      _node--;
      return temp;
    }
    iterator operator+(int i) const
    {
      return iterator(_node + i);
    }
    iterator &operator+=(int i)
    {
      _node += i;
      return (*this);
    }

    iterator &operator=(const iterator &i)
    {
      _node = i._node;
      return (*this);
    }

    bool operator!=(const iterator &i) const { return _node != i._node; }
    bool operator==(const iterator &i) const { return _node == i._node; }

  private:
    T *_node;
  };

  // TODO: implement these functions
  iterator begin() const
  {
    return iterator(_data);
  }
  iterator end() const
  {
    return iterator(_data + _size);
  }
  bool empty() const
  {
    if (_size == 0)
      return true;
    return false;
  }
  size_t size() const { return _size; }

  T &operator[](size_t i) { return _data[i]; }
  const T &operator[](size_t i) const { return _data[i]; }

  void push_back(const T &x)
  {
    if (_size == 0 && _capacity == 0)
    {
      _capacity = 1;
      _data = new T[_capacity];
    }
    if (_size >= _capacity)
    {
      _capacity *= 2;
      T *temp = new T[_capacity];
      for (int i = 0; i < _size; i++)
      {
        temp[i] = _data[i];
      }
      delete[] _data;
      _data = temp;
    }
    _size++;
    _data[_size - 1] = x;
    _isSorted = false;
  }
  void pop_front()
  {
    if (_size != 0)
    {
      _data[0] = _data[_size - 1];
      _size--;
    }
  }
  void pop_back()
  {
    if (_size != 0)
    {
      _size--;
    }
  }
  bool erase(iterator pos)
  {
    _isSorted = false;
    if (pos == end() || (!_size))
    {
      return false;
    }
    *(pos._node) = _data[_size - 1];
    _size--;
    return true;
  }
  bool erase(const T &x)
  {
    _isSorted = false;
    for (int i = 0; i < _size; i++)
    {
      if (x == _data[i])
      {
        _data[i] = _data[_size - 1];
        _size--;
        return true;
      }
    }
    return false;
  }

  void clear()
  {
    _size = 0;
    delete[] _data;
    _data = 0;
    _isSorted = false;
    _capacity = 0;
  }

  void sort() const
  {
    if (_isSorted == false)
    {
      if (!empty())
        ::sort(_data, _data + _size);
      _isSorted = true;
    }
  }
  // Nice to have, but not required in this homework...
  // void reserve(size_t n) { ... }
  // void resize(size_t n) { ... }

private:
  T *_data;
  size_t _size;           // number of valid elements
  size_t _capacity;       // max number of elements
  mutable bool _isSorted; // (optionally) to indicate the array is sorted

  // [OPTIONAL TODO] Helper functions; called by public member functions
};

#endif // ARRAY_H
