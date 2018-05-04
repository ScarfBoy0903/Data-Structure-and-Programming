/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>
#include <algorithm>

template <class T>
class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
  friend class DList<T>;
  friend class DList<T>::iterator;

  DListNode(const T &d, DListNode<T> *p = 0, DListNode<T> *n = 0) : _data(d), _prev(p), _next(n) {}

  // [NOTE] DO NOT ADD or REMOVE any data member
  T _data;
  DListNode<T> *_prev;
  DListNode<T> *_next;
};

template <class T>
class DList
{
public:
  // TODO: decide the initial value for _isSorted
  DList()
  {
    _isSorted = false;
    _head = new DListNode<T>(T());
    _head->_prev = _head->_next = _head; // _head is a dummy node
  }
  ~DList()
  {
    clear();
    delete _head;
  }

  // DO NOT add any more data member or function for class iterator
  class iterator
  {
    friend class DList;

  public:
    iterator(DListNode<T> *n = 0) : _node(n) {}
    iterator(const iterator &i) : _node(i._node) {}
    ~iterator() {} // Should NOT delete _node

    // TODO: implement these overloaded operators
    const T &operator*() const { return *(this); }
    T &operator*() { return _node->_data; }
    iterator &operator++()
    {
      _node = _node->_next;
      return *(this);
    }
    iterator operator++(int)
    {
      iterator temp = *(this);
      _node = _node->_next;
      return temp;
    }
    iterator &operator--()
    {
      _node = _node->_prev;
      return *(this);
    }
    iterator operator--(int)
    {
      iterator temp = *(this);
      _node = _node->_prev;
      return temp;
    }
    iterator &operator=(const iterator &i)
    {
      _node = i._node;
      return *(this);
    }

    bool operator!=(const iterator &i) const { return _node != i._node; }
    bool operator==(const iterator &i) const { return _node == i._node; }

  private:
    DListNode<T> *_node;
  };

  // TODO: implement these functions
  iterator begin() const { return iterator(_head); }
  iterator end() const
  {
    DListNode<T> *temp = _head;
    while (temp->_next != 0 && temp->_next != _head)
    {
      temp = temp->_next;
    }
    return iterator(temp);
  }
  bool empty() const
  {
    if (begin() == end())
      return true;
    return false;
  }
  size_t size() const
  {
    size_t size = 0;
    DListNode<T> *temp = _head;
    while (temp->_next != 0 && temp->_next != _head)
    {
      ++size;
      temp = temp->_next;
    }
    return size;
  }

  void push_back(const T &x)
  {
    DListNode<T> *temp = end()._node;
    *temp = DListNode<T>(x, temp->_prev, new DListNode<T>(T(), temp, 0));
    _isSorted = false;
  }

  void pop_front()
  {
    iterator ITend = end();
    --ITend;
    for (iterator i = begin(); i != ITend; ++i)
    {
      i._node->_data = i._node->_next->_data;
    }
    DList::pop_back();
  }

  void
  pop_back()
  {
    DListNode<T> *temp = end()._node->_prev;
    DListNode<T> *node_end = temp->_next;
    delete node_end;
    node_end = 0;
    *temp = DListNode<T>(T(), temp->_prev, 0);
  }

  // return false if nothing to erase
  bool erase(iterator pos)
  {
    _isSorted = false;
    if (_head->_next == _head)
    {
      return false;
    }
    iterator ITend = end();
    --ITend;
    for (iterator i = pos; i != ITend; ++i)
    {
      i._node->_data = i._node->_next->_data;
    }
    DList::pop_back();
    return true;
  }

  bool erase(const T &x)
  {
    _isSorted = false;
    for (iterator i = begin(); i != end(); ++i)
    {
      if (i._node->_data == x)
      {
        erase(i);
        return true;
      }
    }
    return false;
  }

  void clear()
  {
    DListNode<T> *temp = end()._node;
    while (temp != _head)
    {
      temp = temp->_prev;
      DListNode<T> *node_end = temp->_next;
      delete node_end;
      node_end = 0;
    }
    *_head = DListNode<T>(T(), _head, _head);
    _isSorted = false;
  } // delete all nodes except for the dummy node

  void sort() const
  {
    if (!_isSorted)
    {
      iterator start = begin();
      start++;
      for (iterator i = start; i != end(); ++i)
      {
        for (iterator j = i; j != begin(); --j)
        {
          iterator cmp = j;
          cmp--;
          if (cmp._node->_data > j._node->_data)
          {
            T temp = cmp._node->_data;
            cmp._node->_data = j._node->_data;
            j._node->_data = temp;
          }
        }
      }
      _isSorted = true;
    }
  }

private:
  // [NOTE] DO NOT ADD or REMOVE any data member
  DListNode<T> *_head;    // = dummy node if list is empty
  mutable bool _isSorted; // (optionally) to indicate the array is sorted

  // [OPTIONAL TODO] helper functions; called by public member functions
};

#endif // DLIST_H
