#ifndef SORTEDVEC_H
#define SORTEDVEC_H

#include <functional>
#include <algorithm>
#include <vector>

template< class T, class Less=std::less<T> >
class SortedVec {
  public:
    typedef typename std::vector<T>::iterator iterator;

    iterator find(const T& t) {
      iterator i = std::lower_bound(data.begin(), data.end(), t, less);
      if(i!=data.end() && *i==t)
        return i;
      return data.end();
      }

    void insert( const T& t ){
      auto l = std::lower_bound(data.begin(), data.end(), t, less);
      data.insert(l, t);
      }

    iterator end(){
      return data.end();
      }

    size_t size() const{
      return data.size();
      }

    T& operator [] (size_t id){
      return data[id];
      }

    const T& operator [] (size_t id) const {
      return data[id];
      }

    std::vector<T> data;
  private:
    Less less;
  };

#endif // SORTEDVEC_H
