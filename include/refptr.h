#pragma once

#include <stdint.h>
#include <stddef.h>
#include <type_traits>


// This implementation forces the object to be aligned to the maximum alignment size
// which is really inefficient for small _Ts.  The max_align_t is currently 32 bytes.
// Fortunately, refobj has to know the full _T type, so we can actually figure out the
// alignment of it, and use that increment instead.  The issue is then, how do instances
// of refptr<_T> figure out how far back to look if they are using a _T2 that is a base
// class of _T, as _T2 might have weaker alignment requirements [1].
//
// [1]: By putting the padding between the beginning of the refobj and the object, it's
//      guaranteed that the refcount will always be the bytes immediately preceding the
//      object.  Any refptr can just subtract sizeof(refcount_t) from the object address.

using refcount_t = uint16_t;


template <typename _T> using AlignAs = std::conditional_t<(alignof(_T) > sizeof(refcount_t)), _T, refcount_t>;

// Align an instance of refobj by the greater of the alignment requirement of _T and the
// size of a refcount.  We need to make sure there's enough space in front of the type to
// fit an instance of refcount_t.
template <typename _T>
struct alignas(AlignAs<_T>) refobj {
  template <typename ..._Args> [[gnu::always_inline]] refobj(_Args &&...args) : refcount(0), object(args...) {}

  // Pad with the difference between the alignment
  char padding[alignof(AlignAs<_T>) - sizeof(refcount_t)];
  refcount_t refcount;
  _T object;

  [[gnu::always_inline]] inline _T       *operator->()       { return &object; }
  [[gnu::always_inline]] inline _T const *operator->() const { return &object; }
};


template <typename _T>
struct refptr {
  _T *referent;

  [[gnu::always_inline]]
  inline refcount_t &refcount() {
    return *reinterpret_cast<refcount_t *>(reinterpret_cast<char *>(referent) - sizeof(refcount_t));
  }

  refptr(               ): referent(   nullptr) {}
  refptr(refptr      &&r): referent(r.referent) { r.referent = nullptr; }
  refptr(refptr const &r): referent(r.referent) { ++refcount(); }
  refptr(_T *t): referent(t) { ++refcount(); }

  ~refptr() { if (referent) { --refcount(); } }

  refptr &operator=(refptr      &&r) { referent = r.referent; r.referent = nullptr; return *this; }
  refptr &operator=(refptr const &r) { referent = r.referent; ++refcount(); return *this; }

  [[gnu::always_inline]] inline _T       *operator->()       { return referent; }
  [[gnu::always_inline]] inline _T const *operator->() const { return referent; }
};
