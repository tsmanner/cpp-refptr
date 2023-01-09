#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limits>
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

struct refcount_t {
  // A 3-bit code that represents the alignment requirement of the refobj.
  uint16_t alignment :  3;
  // The remaining 13 bits are the reference count, max 8192.
  uint16_t count     : 13;

  // Pre-increment
  refcount_t &operator++() { ++count; return *this; }
  // Pre-deccrement
  refcount_t &operator--() { --count; return *this; }
  // Post-increment
  refcount_t  operator++(int) { refcount_t temp {alignment, count++}; return temp; }
  // Post-deccrement
  refcount_t  operator--(int) { refcount_t temp {alignment, count--}; return temp; }
};


// Compile-time mapping from alignof to 3-bit alignment code point.
template <unsigned _I> struct _AlignmentCode;
template <> struct _AlignmentCode<  1> { static constexpr unsigned value = 0; };
template <> struct _AlignmentCode<  2> { static constexpr unsigned value = 1; };
template <> struct _AlignmentCode<  4> { static constexpr unsigned value = 2; };
template <> struct _AlignmentCode<  8> { static constexpr unsigned value = 3; };
template <> struct _AlignmentCode< 16> { static constexpr unsigned value = 4; };
template <> struct _AlignmentCode< 32> { static constexpr unsigned value = 5; };
template <> struct _AlignmentCode< 64> { static constexpr unsigned value = 6; };
template <> struct _AlignmentCode<128> { static constexpr unsigned value = 7; };


// Align an instance of refobj by the greater of the alignment requirement of _T and the
// size of a refcount.  We need to make sure there's enough space in front of the type to
// fit an instance of refcount_t.
template <typename _T> using AlignAs = std::conditional_t<(alignof(_T) > sizeof(refcount_t)), _T, refcount_t>;


template <typename _T, bool _AutoDelete = false>
struct alignas(AlignAs<_T>) refobj {
  template <typename ..._Args>
  [[gnu::always_inline]]
  refobj(_Args &&...args
  ) : object(args...) {}

  // Pad with the difference between the alignment
  char padding[alignof(AlignAs<_T>) - sizeof(refcount_t)];
  refcount_t refcount {_AlignmentCode<alignof(AlignAs<_T>)>::value, 0};
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
  [[gnu::always_inline]]
  inline unsigned alignment() {
    return 1 << refcount().alignment;
  }

  [[gnu::always_inline]]
  inline refobj<_T> *refobj_p() {
    return reinterpret_cast<refobj<_T> *>(reinterpret_cast<char *>(referent) - alignment());
  }

  refptr(               ): referent(   nullptr) {}
  refptr(        _T   *t): referent(t) { ++refcount(); }
  refptr(refobj<_T>   *o): referent(&o->object) { ++o->refcount; }
  refptr(refptr      &&r): referent(r.referent) { r.referent = nullptr; }
  refptr(refptr const &r): referent(r.referent) { ++refcount(); }

  ~refptr() {
    if (referent) {
        if (refcount().count == 1) {
          delete refobj_p();
          referent = nullptr;
        }
        else {
          --refcount();
        }
    }
  }

  refptr &operator=(refptr      &&r) { referent = r.referent; r.referent = nullptr; return *this; }
  refptr &operator=(refptr const &r) { referent = r.referent; ++refcount(); return *this; }

  [[gnu::always_inline]] inline _T       *operator->()       { return referent; }
  [[gnu::always_inline]] inline _T const *operator->() const { return referent; }
};


template <typename _T, typename ..._Args>
inline refobj<_T> *New(_Args &&...args) {
  return new refobj<_T>(args...);
}
