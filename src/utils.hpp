/*!
   \file utils.hpp
   \author Ha NGUYEN
   \since 06 Oct 2014
   \date 10 Feb 2015


   \brief Some utils for Xios
 */

#ifndef __XIOS_UTILS_HPP__
#define __XIOS_UTILS_HPP__

#include <vector>
#include "array_new.hpp"
#include "exception.hpp"

namespace xios
{
  template<typename K>
  struct CArrayTraits {
    typedef K ArrayType;
  };

  template<typename K>
  struct CArrayBoolTraits : public CArrayTraits<K> {
    typedef bool Type;
  };

  template<>
  struct CArrayBoolTraits<CArray<bool,1> >
  {
    static inline void resizeArray(CArray<bool,1>& boolArray, const std::vector<int>& dimensionSize)
    {
      if (1 != dimensionSize.size())
        ERROR("utils::CArrayBoolTraits",
                <<"Dimension of resized array mismatch"<<endl
                <<"Dimension of resized is 1 "<<endl
                <<"Dimension of vetor resizing is "<< dimensionSize.size());
      boolArray.resize(dimensionSize[0]);
    }
  };

  template<>
  struct CArrayBoolTraits<CArray<bool,2> >
  {
    static inline void resizeArray(CArray<bool,2>& boolArray, const std::vector<int>& dimensionSize)
    {
      if (2 != dimensionSize.size())
        ERROR("utils::CArrayBoolTraits",
                <<"Dimension of resized array mismatch"<<endl
                <<"Dimension of resized is 2 "<<endl
                <<"Dimension of vetor resizing is "<< dimensionSize.size());
      boolArray.resize(dimensionSize[0], dimensionSize[1]);
    }
  };

  template<>
  struct CArrayBoolTraits<CArray<bool,3> >
  {
    static inline void resizeArray(CArray<bool,3>& boolArray, const std::vector<int>& dimensionSize)
    {
      if (3 != dimensionSize.size())
        ERROR("utils::CArrayBoolTraits",
                <<"Dimension of resized array mismatch"<<endl
                <<"Dimension of resized is 3 "<<endl
                <<"Dimension of vetor resizing is "<< dimensionSize.size());
      boolArray.resize(dimensionSize[0], dimensionSize[1], dimensionSize[2]);
    }
  };

  template<>
  struct CArrayBoolTraits<CArray<bool,4> >
  {
    static inline void resizeArray(CArray<bool,4>& boolArray, const std::vector<int>& dimensionSize)
    {
      if (4 != dimensionSize.size())
        ERROR("utils::CArrayBoolTraits",
                <<"Dimension of resized array mismatch"<<endl
                <<"Dimension of resized is 4 "<<endl
                <<"Dimension of vetor resizing is "<< dimensionSize.size());
      boolArray.resize(dimensionSize[0], dimensionSize[1], dimensionSize[2], dimensionSize[3]);
    }
  };

  template<>
  struct CArrayBoolTraits<CArray<bool,5> >
  {
    static inline void resizeArray(CArray<bool,5>& boolArray, const std::vector<int>& dimensionSize)
    {
      if (5 != dimensionSize.size())
        ERROR("utils::CArrayBoolTraits",
                <<"Dimension of resized array mismatch"<<endl
                <<"Dimension of resized is 5 "<<endl
                <<"Dimension of vetor resizing is "<< dimensionSize.size());
      boolArray.resize(dimensionSize[0], dimensionSize[1],
                       dimensionSize[2], dimensionSize[3], dimensionSize[4]);
    }
  };

  template<>
  struct CArrayBoolTraits<CArray<bool,6> >
  {
    static inline void resizeArray(CArray<bool,6>& boolArray, const std::vector<int>& dimensionSize)
    {
      if (6 != dimensionSize.size())
        ERROR("utils::CArrayBoolTraits",
                <<"Dimension of resized array mismatch"<<endl
                <<"Dimension of resized is 6 "<<endl
                <<"Dimension of vetor resizing is "<< dimensionSize.size());
      boolArray.resize(dimensionSize[0], dimensionSize[1],
                       dimensionSize[2], dimensionSize[3],
                       dimensionSize[4], dimensionSize[5]);
    }
  };

  template<>
  struct CArrayBoolTraits<CArray<bool,7> >
  {
    static inline void resizeArray(CArray<bool,7>& boolArray, const std::vector<int>& dimensionSize)
    {
      if (7 != dimensionSize.size())
        ERROR("utils::CArrayBoolTraits",
                <<"Dimension of resized array mismatch"<<endl
                <<"Dimension of resized is 7 "<<endl
                <<"Dimension of vetor resizing is "<< dimensionSize.size());
      boolArray.resize(dimensionSize[0], dimensionSize[1],
                       dimensionSize[2], dimensionSize[3],
                       dimensionSize[4], dimensionSize[5], dimensionSize[6]);
    }
  };

  template <int v>
  struct Int2Type
  {
  enum { value = v };
  };

  template<typename T>
  union TypeToBytes {
    T value;
    unsigned char bytes[sizeof(value)];
  };

  template<typename T>
  struct HashAlgorithm
  {
    /*!
      Adapted version of one-at-a-time hash by Bob Jenkins,
      which is an expanded version of his Dr. Dobbs article.
    */
    static inline size_t jenkins_hash(const T& value)
    {
      TypeToBytes<T> u;
      (u.value) = value;

      size_t hash = 0;
      size_t length = sizeof(value);

      for (size_t i = 0; i < length; ++i)
      {
          hash += u.bytes[i];
          hash += (hash << 10);
          hash ^= (hash >> 6);
      }
      hash += (hash << 3);
      hash ^= (hash >> 11);
      hash += (hash << 15);

      return hash;
    }
  };

  template<typename T, typename Algo = Int2Type<0> >
  struct HashXIOS
  {
    std::size_t operator()(const T& val)
    {
      Algo al;
      return hash_value(val, al);
    }

  private:
    size_t hash_value(const T& val, Int2Type<0>)
    {
      return HashAlgorithm<T>::jenkins_hash(val);
    }
  };
}

#endif // __UTILS_HPP__