/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>
#include <bitset>

using namespace std;

template <class Data>
class HashSet
{
public:
  // HashSet() {}
  HashSet(size_t b = 0) : _numBuckets(2 * b + 2), _buckets(0)
  {
    if (b != 0)
      init(b);
  }
  ~HashSet() {}

  void init(size_t b)
  {
    b = 4 * b + 2;
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

  void debug()
  {
    for (int i = 0; i < _numBuckets; i++)
    {
      if (_buckets[i].size() > 1)
      {
        for (int j = 0; j < _buckets[i].size(); j++)
          cerr << _buckets[i][j]._ID << " ";
        cerr << endl;
      }
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

  bool insert(const Data &d, Data &cmp)
  {
    size_t idx = bucketNum(d);
    for (int i = 0; i < _buckets[idx].size(); i++)
    {
      cmp = _buckets[idx][i];
      if (check_in_same(d, cmp))
        return true;
    }
    _buckets[idx].push_back(d);
    return false;
  }

  bool check_in_same(Data d1, Data d2)
  {
    if (d1.fanins[0] == d2.fanins[0] && d1.fanins[1] == d2.fanins[1])
      return true;
    else if (d1.fanins[0] == d2.fanins[1] && d1.fanins[1] == d2.fanins[0])
      return true;
    return false;
  }

private:
  // Do not add any extra data member
  size_t _numBuckets;
  vector<Data> *_buckets;

  size_t bucketNum(const Data &d) const
  {
    return (d.fanins[0] + d.fanins[1]);
  }
};

template <class Data>
class FEC_HashSet
{
public:
  FEC_HashSet(size_t b = 0) : _numBuckets(2 * b + 2), _buckets(0)
  {
    if (b != 0)
      init(b);
  }
  ~FEC_HashSet() {}

  void init(size_t b)
  {
    b = 4 * b + 2;
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

  void debug()
  {
    for (int i = 0; i < _numBuckets; i++)
    {
      if (_buckets[i].size() > 1)
      {
        for (int j = 0; j < _buckets[i].size(); j++)
          cerr << _buckets[i][j]._ID << " ";
        cerr << endl;
      }
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

  void insert(const Data &d)
  {
    size_t idx = bucketNum(d);
    _buckets[idx].push_back(d);
  }

  bool get_same_pair_list(Data &d, vector<int> &l)
  {
    int idx = bucketNum(d);
    if (_buckets[idx].size() > 1)
    {
      for (int i = 0; i < _buckets[idx].size(); i++)
        if (check_in_same(d, _buckets[idx][i]))
          l.push_back(_buckets[idx][i]._ID);
    }
    if (l.size() > 1)
      return true;
    return false;
  }

  bool check_in_same(Data d1, Data d2)
  {
    size_t cmp_num = (d1.sim_bits ^ d2.sim_bits).count();
    if (cmp_num == 0 || cmp_num == 64)
      return true;
    return false;
  }

private:
  // Do not add any extra data member
  size_t _numBuckets;
  vector<Data> *_buckets;

  size_t bucketNum(const Data &d) const
  {
    bitset<64> temp = d.sim_bits;
    size_t n1 = temp.to_ullong();
    size_t n2 = (temp.flip()).to_ullong();
    if (n1 > n2)
      n1 = n2;
    return (n1 % _numBuckets);
  }
};

#endif // MY_HASH_H
