/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>

using namespace std;

template <class T>
class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
  friend class BSTree<T>;
  friend class BSTree<T>::iterator;

  BSTreeNode(const T &d, BSTreeNode<T> *k = 0, BSTreeNode<T> *p = 0, BSTreeNode<T> *n = 0) : _data(d), _parent(k), _leftchild(p), _rightchild(n) {}

  T _data;
  BSTreeNode<T> *_leftchild;
  BSTreeNode<T> *_rightchild;
  BSTreeNode<T> *_parent;
  // TODO: design your own class!!
};

template <class T>
class BSTree
{
  // TODO: design your own class!!
  friend class iterator;

public:
  BSTree()
  {
    _tail = new BSTreeNode<T>(T(""), 0, 0, 0);
    _size = 0;
  }

  ~BSTree()
  {
    clear();
    delete _root;
  }

  class iterator
  {
    friend class BSTree;

  public:
    iterator(BSTreeNode<T> *n = 0) : _node(n) { count = 0; }
    iterator(const iterator &i) : _node(i._node) { count = 0; }
    iterator(BSTreeNode<T> *n, vector<BSTreeNode<T> *> p, bool cre) : _node(n), _trace(p)
    {
      if (cre)
      {
        count = 0;
      }
      else
      {
        count = _trace.size() - 1;
      }
    }
    ~iterator() {} // Should NOT delete _node

    // TODO: implement these overloaded operators

    const T &operator*() const { return (*this); }
    T &operator*()
    {
      return (_node->_data);
    }

    iterator &operator++()
    {
      count++;
      _node = _trace[count];
      return (*this);
    }
    iterator operator++(int)
    {
      iterator temp = *(this);
      count++;
      _node = _trace[count];
      return temp;
    }

    iterator &operator--()
    {
      count--;
      _node = _trace[count];
      return (*this);
    }

    iterator operator--(int)
    {
      iterator temp = *(this);
      count--;
      _node = _trace[count];
      return temp;
    }

    iterator &operator=(const iterator &i)
    {
      return i;
    }

    bool operator!=(const iterator &i) const { return _node != i._node; }
    bool operator==(const iterator &i) const { return _node == i._node; }

  private:
    BSTreeNode<T> *_node = 0;
    size_t count;
    vector<BSTreeNode<T> *> _trace;
  };

  iterator begin() const
  {
    if (_size == 0)
    {
      return iterator(_root);
    }
    else
    {
      return iterator(_trace[0], _trace, true);
    }
  }
  iterator end() const
  {
    if (_size == 0)
    {
      return iterator(_root);
    }
    else
    {
      return iterator(_trace[_size], _trace, false);
    }
  }
  bool empty() const
  {
    if (_size == 0)
      return true;
    return false;
  }
  size_t size() const { return _size; }

  void insert(const T x)
  {
    if (_size != 0)
    {
      _trace.vector<BSTreeNode<T> *>::pop_back();
      BSTreeNode<T> *temp = _root;
      while (true)
      {
        if (x < temp->_data)
        {
          if (temp->_leftchild == 0)
          {
            temp->_leftchild = new BSTreeNode<T>(x, temp, 0, 0);
            for (int i = 0; i < _trace.size(); i++)
            {
              if (temp == _trace[i])
              {
                _trace.vector<BSTreeNode<T> *>::insert(_trace.begin() + i, temp->_leftchild);
                break;
              }
            }
            temp = temp->_leftchild;
            break;
          }
          temp = temp->_leftchild;
        }
        else
        {
          if (temp->_rightchild == 0)
          {
            temp->_rightchild = new BSTreeNode<T>(x, temp, 0, 0);
            for (int i = 0; i < _trace.size(); i++)
            {
              if (temp == _trace[i])
              {
                _trace.vector<BSTreeNode<T> *>::insert(_trace.begin() + i + 1, temp->_rightchild);
                break;
              }
            }
            temp = temp->_rightchild;
            break;
          }
          temp = temp->_rightchild;
        }
      }
      _trace.push_back(_tail);
    }
    else
    {
      _root = new BSTreeNode<T>(x, _root, 0, 0);
      _trace.push_back(_root);
      _trace.push_back(_tail);
    }
    _size++;
  }

  void pop_front()
  {
    if (_size > 1)
    {
      BSTreeNode<T> *pop_node = _trace[0];
      _trace.erase(_trace.begin());
      if (pop_node != _root)
      {
        if (pop_node->_rightchild == 0)
        {
          pop_node->_parent->_leftchild = 0;
          delete pop_node;
          pop_node = 0;
        }
        else
        {
          BSTreeNode<T> *match_node = pop_node->_rightchild;
          pop_node->_parent->_leftchild = match_node;
          match_node->_parent = pop_node->_parent;
          delete pop_node;
          pop_node = 0;
        }
      }
      else
      {
        _root = pop_node->_rightchild;
        delete pop_node;
        pop_node = 0;
        _root->_parent = _root;
      }
      _size--;
    }
    else if (_size == 1)
    {
      _trace.pop_back();
      _trace.pop_back();
      delete _root;
      _root->_parent = _root = 0;
      _size--;
    }
  }

  void pop_back()
  {
    if (_size > 1)
    {
      _trace.pop_back();
      BSTreeNode<T> *pop_node = _trace[_size - 1];
      _trace.pop_back();
      if (pop_node != _root)
      {
        if (pop_node->_leftchild == 0)
        {
          pop_node->_parent->_rightchild = 0;
          delete pop_node;
          pop_node = 0;
        }
        else
        {
          BSTreeNode<T> *match_node = pop_node->_leftchild;
          pop_node->_parent->_rightchild = match_node;
          match_node->_parent = pop_node->_parent;
          delete pop_node;
          pop_node = 0;
        }
      }
      else
      {
        _root = pop_node->_leftchild;
        delete pop_node;
        pop_node = 0;
        _root->_parent = _root;
      }
      _trace.push_back(_tail);
      _size--;
    }
    else if (_size == 1)
    {
      _trace.pop_back();
      _trace.pop_back();
      delete _root;
      _root = 0;
      _size--;
    }
  }
  void print() const
  {
    traversal(_root);
  }
  // return false if nothing to erase
  bool erase(iterator pos)
  {
    if (_size == 0)
    {
      return false;
    }
    size_t idx = 0;
    BSTreeNode<T> *pop_node = pos._node;
    for (int i = 0; i < _size; ++i)
    {
      if (pos._node == _trace[i])
      {
        idx = i;
        _trace.erase(_trace.begin() + i);
        break;
      }
    }
    if (pop_node == _root)
    {
      if (pop_node->_leftchild != 0 && pop_node->_rightchild != 0)
      {
        BSTreeNode<T> *match_tail = _trace[idx];
        BSTreeNode<T> *match_head = pop_node->_leftchild;
        match_head->_parent = match_tail;
        match_tail->_leftchild = match_head;
        _root = pop_node->_rightchild;
        delete pop_node;
        pop_node = 0;
        _root->_parent = _root;
      }
      else if (pop_node->_leftchild == 0 && pop_node->_rightchild != 0)
      {
        _root = pop_node->_rightchild;
        delete pop_node;
        pop_node = 0;
        _root->_parent = _root;
      }
      else if (pop_node->_leftchild != 0 && pop_node->_rightchild == 0)
      {
        _root = pop_node->_leftchild;
        delete pop_node;
        pop_node = 0;
        _root->_parent = _root;
      }
      else
      {
        _root->_leftchild = _root->_rightchild = 0;
        delete _root;
        _root = _root->_parent = 0;
        _trace.pop_back();
      }
    }
    else if (_isLeftchild(pop_node))
    {
      if (pop_node->_leftchild != 0 && pop_node->_rightchild != 0)
      {
        BSTreeNode<T> *match_node = _trace[idx];
        pop_node->_leftchild->_parent = match_node;
        match_node->_leftchild = pop_node->_leftchild;
        pop_node->_rightchild->_parent = pop_node->_parent;
        pop_node->_parent->_leftchild = pop_node->_rightchild;
        delete pop_node;
        pop_node = 0;
      }
      else if (pop_node->_leftchild == 0 && pop_node->_rightchild != 0)
      {
        pop_node->_parent->_leftchild = pop_node->_rightchild;
        pop_node->_rightchild->_parent = pop_node->_parent;
        delete pop_node;
        pop_node = 0;
      }
      else if (pop_node->_leftchild != 0 && pop_node->_rightchild == 0)
      {
        pop_node->_parent->_leftchild = pop_node->_leftchild;
        pop_node->_leftchild->_parent = pop_node->_parent;
        delete pop_node;
        pop_node = 0;
      }
      else
      {
        pop_node->_parent->_leftchild = 0;
        delete pop_node;
        pop_node = 0;
      }
    }
    else
    {
      if (pop_node->_leftchild != 0 && pop_node->_rightchild != 0)
      {
        BSTreeNode<T> *match_node = _trace[idx];
        pop_node->_leftchild->_parent = match_node;
        match_node->_leftchild = pop_node->_leftchild;
        pop_node->_rightchild->_parent = pop_node->_parent;
        pop_node->_parent->_rightchild = pop_node->_rightchild;
        delete pop_node;
        pop_node = 0;
      }
      else if (pop_node->_leftchild == 0 && pop_node->_rightchild != 0)
      {
        pop_node->_parent->_rightchild = pop_node->_rightchild;
        pop_node->_rightchild->_parent = pop_node->_parent;
        delete pop_node;
        pop_node = 0;
      }
      else if (pop_node->_leftchild != 0 && pop_node->_rightchild == 0)
      {
        pop_node->_parent->_rightchild = pop_node->_leftchild;
        pop_node->_leftchild->_parent = pop_node->_parent;
        delete pop_node;
        pop_node = 0;
      }
      else
      {
        pop_node->_parent->_rightchild = 0;
        delete pop_node;
        pop_node = 0;
      }
    }
    _size--;
    return true;
  }
  bool erase(const T &x)
  {
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
    while (_size > 0)
    {
      BSTree::pop_back();
    }
    _size = 0;
  }

  void sort() const {}

  bool _isLeftchild(BSTreeNode<T> *cmp) const
  {
    if (cmp == cmp->_parent->_leftchild)
    {
      return true;
    }
    return false;
  }

  void traversal(BSTreeNode<T> *p) const
  {
    if (!p)
      return;
    traversal(p->_leftchild);
    cout << p->_data << endl; // 挪到中間，改變輸出順序。
    traversal(p->_rightchild);
  }

private:
  vector<BSTreeNode<T> *> _trace;
  BSTreeNode<T> *_root;
  BSTreeNode<T> *_tail;
  size_t _size;
};

#endif // BST_H
