#pragma once

#include <stdint.h>
#include <stddef.h>
#include <type_traits>


// This implementation forces the object to be aligned to the maximum alignment size
// which is really inefficient for small _Ts.  The max_align_t is currently 32 bytes.
// Fortunately, obj has to know the full _T type, so we can actually figure out the
// alignment of it, and use that increment instead.  The issue is then, how do instances
// of ref<_T> figure out how far back to look if they are using a _T2 that is a base
// class of _T, as _T2 might have weaker alignment requirements.

// template <typename _T, typename _MaxAlignT = max_align_t>
// struct alignas(_MaxAlignT) obj {
//   static constexpr auto alignment = sizeof(_MaxAlignT);

template <typename _T, typename _AlignAsT = typename std::conditional<(alignof(_T) > alignof(uint16_t)), _T, uint16_t>::type>
struct alignas(_AlignAsT) obj {
  static constexpr auto alignment = alignof(_AlignAsT);
  static constexpr auto padding = alignment - sizeof(uint16_t);

  template <typename ..._Args> obj(_Args &&...args) : refcount(0), object(args...) {}

  uint16_t refcount;
  char _[padding];
  _T object;

  inline _T       *operator->()       { return &object; }
  inline _T const *operator->() const { return &object; }
};



template <typename _T>
struct ref {
  obj<_T> *referent;

  ref(ref      &&r): referent(r.referent) { r.referent = nullptr; }
  ref(ref const &r): referent(r.referent) { ++referent->refcount; }

  ref(_T &t):
    referent(reinterpret_cast<obj<_T> *>(reinterpret_cast<char *>(&t) - obj<_T>::alignment))
  {
    ++referent->refcount;
  }

  ref(_T *t):
    referent(reinterpret_cast<obj<_T> *>(reinterpret_cast<char *>( t) - obj<_T>::alignment))
  {
    ++referent->refcount;
  }

  ~ref() { if (referent) { --referent->refcount; } }

  inline _T       *operator->()       { return &referent->object; }
  inline _T const *operator->() const { return &referent->object; }
};
