/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
  HashSet(size_t b = 0) : _numBuckets(0), _buckets(0)
  {
    if (b != 0)
      init(b);
  }
  ~HashSet() { reset(); }

  // TODO: implement the HashSet<Data>::iterator
  // o An iterator should be able to go through all the valid Data
  //   in the Hash
  // o Functions to be implemented:
  //   - constructor(s), destructor
  //   - operator '*': return the HashNode
  //   - ++/--iterator, iterator++/--
  //   - operators '=', '==', !="
  //
  class iterator
  {
    friend class HashSet<Data>;

  public:
    iterator() {}
    iterator(Data d) : _node(d) {}
    iterator(vector<Data> *r_ptr, bool f, size_t n, Data d) : row_ptr(r_ptr), is_forward(f), num(n), _node(d), first_print(true) {}
    const Data &operator*()
    {
      check();
      return *col_ptr;
    }
    iterator &operator++()
    {
      check();
      col_ptr++;
      return (*this);
    }
    iterator &operator--()
    {
      check();
      col_ptr--;
      return (*this);
    }
    iterator operator++(int)
    {
      check();
      iterator temp = *(this);
      col_ptr++;
      return temp;
    }
    iterator operator--(int)
    {
      check();
      iterator temp = *(this);
      col_ptr--;
      return temp;
    }
    bool operator!=(const iterator &i)
    {
      check();
      if (*col_ptr == i._node)
        return false;
      return true;
    }
    bool operator==(const iterator &i)
    {
      check();
      if (*col_ptr == i._node)
        return true;
      return false;
    }

    iterator &operator=(const iterator &i)
    {
      iterator temp = i;
      return temp;
    }

    void check()
    {
      if (first_print)
      {
        for (int i = 0; i < num; i++)
        {
          if ((*row_ptr).size())
            for (int j = 0; j < (*row_ptr).size(); j++)
              vec.push_back((*row_ptr)[j]);
          row_ptr++;
        }
        vec.push_back(Data("the_end", -1));
        if (is_forward)
          col_ptr = &vec[0];
        else
          col_ptr = &vec[vec.size() - 1];
        first_print = false;
      }
    }

  private:
    Data _node;
    size_t num;
    vector<Data> *row_ptr;
    Data *col_ptr;
    vector<Data> vec;
    bool is_forward;
    bool first_print;
  };

  void init(size_t b)
  {
    _numBuckets = b;
    _buckets = new vector<Data>[b];
  }

  void reset()
  {
    _numBuckets = 0;
    if (_buckets)
    {
      delete[] _buckets;
      _buckets = 0;
    }
  }

  void clear()
  {
    for (size_t i = 0; i < _numBuckets; ++i)
      _buckets[i].clear();
  }

  size_t numBuckets() const { return _numBuckets; }

  vector<Data> &operator[](size_t i) { return _buckets[i]; }
  const vector<Data> &operator[](size_t i) const { return _buckets[i]; }

  // TODO: implement these functions
  //
  // Point to the first valid data
  iterator begin() const
  {
    for (int i = 0; i < _numBuckets; i++)
      if (_buckets[i].size())
        return iterator(_buckets, true, _numBuckets, _buckets[i][0]);
  }
  // Pass the end
  iterator end() const
  {
    return iterator(_buckets, false, _numBuckets, Data("the_end", -1));
  }
  // return true if no valid data
  bool empty() const
  {
    for (int i = 0; i < _numBuckets; i++)
      if (_buckets[i].size())
        return false;
    return true;
  }
  // number of valid data
  size_t size() const
  {
    size_t s = 0;
    for (int i = 0; i < _numBuckets; i++)
      s += _buckets[i].size();
    return s;
  }

  // check if d is in the hash...
  // if yes, return true;
  // else return false;
  bool check(const Data &d) const
  {
    size_t idx = bucketNum(d);
    for (int i = 0; i < _buckets[idx].size(); i++)
      if (_buckets[idx][i] == d)
        return true;
    return false;
  }

  bool check_get_index(const Data &d, size_t &idx, size_t &i) const
  {
    idx = bucketNum(d);
    for (i = 0; i < _buckets[idx].size(); i++)
      if (_buckets[idx][i] == d)
        return true;
    return false;
  }

  // query if d is in the hash...
  // if yes, replace d with the data in the hash and return true;
  // else return false;
  bool query(Data &d) const
  {
    size_t idx, col;
    if (check_get_index(d, idx, col))
    {
      d = _buckets[idx][col];
      return true;
    }
    return false;
  }

  // update the entry in hash that is equal to d (i.e. == return true)
  // if found, update that entry with d and return true;
  // else insert d into hash as a new entry and return false;
  bool update(const Data &d)
  {
    size_t idx, col;
    if (check_get_index(d, idx, col))
    {
      _buckets[idx][col] = d;
      return true;
    }
    _buckets[idx].push_back(d);
    return false;
  }

  // return true if inserted successfully (i.e. d is not in the hash)
  // return false is d is already in the hash ==> will not insert
  bool insert(const Data &d)
  {
    size_t idx, col;
    if (check_get_index(d, idx, col))
      return false;
    _buckets[idx].push_back(d);
    return true;
  }

  // return true if removed successfully (i.e. d is in the hash)
  // return fasle otherwise (i.e. nothing is removed)
  bool remove(const Data &d)
  {
    size_t idx, col;
    if (check_get_index(d, idx, col))
    {
      _buckets[idx].erase(_buckets[idx].begin() + col);
      return true;
    }
    return false;
  }

private:
  // Do not add any extra data member
  size_t _numBuckets;
  vector<Data> *_buckets;

  size_t bucketNum(const Data &d) const
  {
    return (d() % _numBuckets);
  }
};

#endif // MY_HASH_SET_H
