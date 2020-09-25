#include <catch2/catch.hpp>

#include <ponos/ponos.h>

using namespace ponos;

TEST_CASE("Array1", "[storage][array]") {
  SECTION("Constructors") {
    {
      Array1<vec2> a(10);
      REQUIRE(a.size() == 10u);
      REQUIRE(a.memorySize() == 10 * sizeof(vec2));
      for (u64 i = 0; i < a.size(); ++i)
        a[i] = vec2(i, i * 2);
      Array1<vec2> b = a;
      for (u64 i = 0; i < a.size(); ++i)
        REQUIRE(a[i] == b[i]);
    } //
    {
      std::vector<Array1<int>> v;
      v.emplace_back(10);
      v.emplace_back(10);
      v.emplace_back(10);
      for (int i = 0; i < 3; i++)
        for (u64 j = 0; j < v[i].size(); ++j)
          v[i][j] = j * 10;
      std::vector<Array1<int>> vv = v;
      for (int i = 0; i < 3; i++)
        for (u64 j = 0; j < v[i].size(); ++j)
          REQUIRE(vv[i][j] == j * 10);
    } //
    {
      Array1<int> a = std::move(Array1<int>(10));
      auto b(Array1<int>(10));
    } //
    {
      std::vector<int> data = {1, 2, 3, 4, 5, 6};
      Array1<int> a = data;
      REQUIRE(a.size() == 6);
      for (u64 i = 0; i < a.size(); ++i)
        REQUIRE(a[i] == data[i]);
    } //
    {
      Array1<int> a = {1, 2, 3};
      REQUIRE(a.size() == 3);
      for (u64 i = 0; i < a.size(); ++i)
        REQUIRE(a[i] == i + 1);
    }
  }//
  SECTION("Operators") {
      Array1<int> a(10);
      a = 3;
      int count = 0;
      for (u64 i = 0; i < a.size(); ++i)
        REQUIRE(a[i] == 3);
      for (const auto &e : a) {
        REQUIRE(e.value == 3);
        count++;
      }
      std::cerr << a << std::endl;
      REQUIRE(count == 10);
  }//
  SECTION("Array1-iterator") {
    Array1<vec2> a( 10);
    for (auto e : a)
      e.value = vec2(1, 2);
    int count = 0;
    for (auto e : a) {
      count++;
      REQUIRE(e.value == vec2(1, 2));
    }
    REQUIRE(count == 10);
  }//
  SECTION("Const Array1-iterator") {
    Array1<vec2> a( 10);
    a = vec2(1, 2);
    auto f = [](const Array1<vec2> &array) {
      for (const auto &d : array)
        REQUIRE(d.value == vec2(1, 2));
    };
    f(a);
  }//
}

TEST_CASE("Array2", "[storage][array]") {
  SECTION("Constructors") {
    {
      Array2<vec2> a(size2(10, 10));
      REQUIRE(a.pitch() == 10 * sizeof(vec2));
      REQUIRE(a.size() == size2(10, 10));
      REQUIRE(a.memorySize() == 10 * 10 * sizeof(vec2));
      for (index2 ij : Index2Range<i32>(a.size()))
        a[ij] = vec2(ij.i, ij.j);
      Array2<vec2> b = a;
      for (index2 ij : Index2Range<i32>(a.size()))
        REQUIRE(a[ij] == b[ij]);
    }
    {
      std::vector<Array2<int>> v;
      v.emplace_back(size2(10, 10));
      v.emplace_back(size2(10, 10));
      v.emplace_back(size2(10, 10));
      for (int i = 0; i < 3; i++)
        for (index2 ij : Index2Range<i32>(v[i].size()))
          v[i][ij] = ij.i * 10 + ij.j;
      std::vector<Array2<int>> vv = v;
      for (int i = 0; i < 3; i++)
        for (index2 ij : Index2Range<i32>(v[i].size()))
          REQUIRE(vv[i][ij] == ij.i * 10 + ij.j);
    }
    {
      Array2<int> a = std::move(Array2<int>(size2(10, 10)));
      Array2<int> b(Array2<int>(size2(10, 10)));
    }
    {
      std::vector<std::vector<int>> data = {{1, 2, 3}, {4, 5, 6}};
      Array2<int> a = data;
      REQUIRE(a.size() == size2(3, 2));
      for (index2 ij : Index2Range<i32>(a.size()))
        REQUIRE(a[ij] == data[ij.j][ij.i]);
    }
    {
      Array2<int> a = {{1, 2, 3}, {11, 12, 13}};
      REQUIRE(a.size() == size2(3, 2));
      for (index2 ij : Index2Range<i32>(a.size()))
        REQUIRE(a[ij] == ij.j * 10 + ij.i + 1);
    }
  }//
  SECTION("Operators") {
    {
      Array2<int> a(size2(10, 10));
      a = 3;
      int count = 0;
      for (index2 ij : Index2Range<i32>(a.size()))
        REQUIRE(a[ij] == 3);
      for (const auto &e : a) {
        REQUIRE(e.value == 3);
        count++;
      }
      std::cerr << a << std::endl;
      REQUIRE(count == 10 * 10);
    }
  }//
  SECTION("Array2-iterator") {
    Array2<vec2> a(size2(10, 10));
    for (auto e : a)
      e.value = vec2(1, 2);
    int count = 0;
    for (auto e : a) {
      count++;
      REQUIRE(e.value == vec2(1, 2));
    }
    REQUIRE(count == 100);
  }//
  SECTION("Const Array2-iterator") {
    Array2<vec2> a(size2(10, 10));
    a = vec2(1, 2);
    auto f = [](const Array2<vec2> &array) {
      for (const auto &d : array)
        REQUIRE(d.value == vec2(1, 2));
    };
    f(a);
  }//
}
