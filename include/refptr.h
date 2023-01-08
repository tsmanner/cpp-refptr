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

using refcount_t = uint16_t;

template <typename _T, typename _AlignAsT = typename std::conditional<(alignof(_T) > alignof(refcount_t)), _T, refcount_t>::type>
struct alignas(_AlignAsT) obj {
  static constexpr auto padding = alignof(_AlignAsT) - sizeof(refcount_t);

  template <typename ..._Args> obj(_Args &&...args) : refcount(0), object(args...) {}

  // Put the padding first so we know the refcount is _always_ bytes immediately preceding
  // the object.  That way, references can always just subtract sizeof(refcount_t) from
  // the object address and get the count.
  char _[padding];
  refcount_t refcount;
  _T object;

  inline _T       *operator->()       { return &object; }
  inline _T const *operator->() const { return &object; }
};



template <typename _T>
struct ref {
  _T *referent;

  refcount_t &refcount() {
    return *reinterpret_cast<refcount_t *>(reinterpret_cast<char *>(referent) - sizeof(refcount_t));
  }

  ref(ref      &&r): referent(r.referent) { r.referent = nullptr; }
  ref(ref const &r): referent(r.referent) { ++refcount(); }

  ref(_T &t): referent(&t) {
    ++refcount();
  }

  ref(_T *t): referent(t) {
    ++refcount();
  }

  ~ref() { if (referent) { --refcount(); } }

  inline _T       *operator->()       { return referent; }
  inline _T const *operator->() const { return referent; }
};
