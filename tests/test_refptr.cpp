#include "refptr.h"

#include <catch2/catch_test_macros.hpp>


struct Foo {
  static int instances;

  Foo() { ++instances; }
  ~Foo() { --instances; }

  int x;
  char y;
};

int Foo::instances = 0;


TEST_CASE("Reference counting, no delete") {
  CHECK(sizeof(Foo) == 8);
  CHECK(sizeof(refobj<Foo>) == 12);
  CHECK(sizeof(refptr<Foo>) == sizeof(void *));
  CHECK(alignof(Foo) >= alignof(refcount_t));
  CHECK(sizeof(refobj<Foo>::padding) == 2);

  // No instances of Foo exist.
  CHECK(Foo::instances == 0);
  {
    auto r1 = refptr<Foo>(New<Foo>());
    // One instance of a Foo.
    CHECK(Foo::instances == 1);
    r1->x = 12345;
    r1->y =  'a';
    {
      auto r = refptr<Foo>(r1);
      // Still only one instance of a Foo.
      CHECK(Foo::instances == 1);
      // Refcount went up
      CHECK(r1.refcount().count == 2);
      CHECK(1 << r1.refcount().alignment == alignof(refobj<Foo>));
      // Values are the same
      CHECK( r1->x ==  r->x);
      CHECK( r1->y ==  r->y);
      // Addresses are the same
      CHECK(&r1->x == &r->x);
      CHECK(&r1->y == &r->y);
    }
    CHECK(r1.refcount().count == 1);
    // Still only one instance of a Foo.
    CHECK(Foo::instances == 1);
  }
  // Finally, no instances of a Foo exist.
  CHECK(Foo::instances == 0);
}
