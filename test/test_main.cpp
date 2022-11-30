#include "gtest/gtest.h"

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  int val = RUN_ALL_TESTS();
  return val;
}