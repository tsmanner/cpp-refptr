#include "refptr.h"

#include <catch2/catch_test_macros.hpp>


struct Foo {
  int x;
  char y;
};

#include <iostream>

TEST_CASE("") {
  // The object is one max-alignment increment larger than the object
  // CHECK(sizeof(Foo) == 8);
  // CHECK(sizeof(obj<Foo>) == 32);
  // CHECK(sizeof(obj<Foo>) == sizeof(max_align_t) + sizeof(Foo));
  // CHECK(alignof(Foo) == 4);
  // CHECK(alignof(obj<Foo>) == alignof(max_align_t));
  // CHECK(sizeof(ref<Foo>) == sizeof(void *));


  CHECK(sizeof(Foo) == 8);
  CHECK(sizeof(obj<Foo>) == 12);
  CHECK(sizeof(ref<Foo>) == sizeof(void *));

  auto f = obj<Foo>{};
  f->x = 12345;
  f->y =  'a';
  {
    auto r = ref<Foo>(f.object);
    // Refcount went up
    CHECK(f.refcount == 1);
    // Values are the same
    CHECK( f->x ==  r->x);
    CHECK( f->y ==  r->y);
    // Addresses are the same
    CHECK(&f->x == &r->x);
    CHECK(&f->y == &r->y);
  }
  CHECK(f.refcount == 0);
}
