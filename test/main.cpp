#include <glog/logging.h>
#include <gtest/gtest.h>

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);

  FLAGS_log_dir = "../../log";
  return RUN_ALL_TESTS();
}
