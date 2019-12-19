#include <fstream>
#include <gtest/gtest.h>
#include <poseidon/poseidon.h>

using namespace poseidon;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  int ret = RUN_ALL_TESTS();

  return ret;
}
