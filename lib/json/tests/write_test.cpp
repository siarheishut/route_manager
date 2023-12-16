#include "json.h"

#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace {
struct TestCase {
  std::string name;
  json::Node input;
  std::string want;
};

void Compare(const std::vector<TestCase> &test_cases) {
  for (auto &[name, input, want] : test_cases) {
    std::ostringstream ss_got;
    ss_got << input;
    std::string got = ss_got.str();
    EXPECT_EQ(want, got);
  }
}
}

TEST(TestWriteFucntions, TestArray) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Empty array",
          .input = List{},
          .want = "[]",
      },
      TestCase{
          .name = "Non-empty array",
          .input = List{12, List{41.7, 12}, "what", std::monostate{},
                        Dict{{"key", List{"value", 4}}}},
          .want = "[12,[41.7,12],\"what\",null,{\"key\":[\"value\",4]}]",
      },
  };

  Compare(test_cases);
}

TEST(TestWriteFucntions, TestMap) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Empty map",
          .input = Dict{},
          .want = "{}",
      },
      TestCase{
          .name = "Non-empty map",
          .input = Dict{{"nullkey", std::monostate{}}, {"key 1", 3},
                        {"key 2", 21.0}, {"key 3", "value"},
                        {"key 4", Dict{{"subkey", List{3, 1.2, "kk"}}}}},
          .want = "{\"key 1\":3,\"key 2\":21,\"key 3\":\"value\","
                  "\"key 4\":{\"subkey\":[3,1.2,\"kk\"]},\"nullkey\":null}",
      },
  };

  Compare(test_cases);
}

TEST(TestWriteFucntions, TestString) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Empty string",
          .input = "",
          .want = "\"\"",
      },
      TestCase{
          .name = "String of spaces",
          .input = "   ",
          .want = "\"   \"",
      },
      TestCase{
          .name = "Non-empty string",
          .input = "Some string",
          .want = "\"Some string\"",
      },
  };

  Compare(test_cases);
}

TEST(TestWriteFucntions, TestBool) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "True",
          .input = true,
          .want = "true",
      },
      TestCase{
          .name = "False",
          .input = false,
          .want = "false",
      },
  };

  Compare(test_cases);
}

TEST(TestWriteFucntions, TestInt) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Int - positive",
          .input = 1512,
          .want = "1512",
      },
      TestCase{
          .name = "Int - negative",
          .input = -4812,
          .want = "-4812",
      },
      TestCase{
          .name = "Int - zero",
          .input = 0,
          .want = "0",
      },
  };

  Compare(test_cases);
}

TEST(TestWriteFucntions, TestDouble) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Double - 1",
          .input = 15.123551362,
          .want = "15.1236",
      },
      TestCase{
          .name = "Double - 2",
          .input = 42135.91246,
          .want = "42135.9",
      },
      TestCase{
          .name = "Double - (> 0 && < 1)",
          .input = 0.1532235,
          .want = "0.153224",
      },
      TestCase{
          .name = "Double - negative",
          .input = -21.32719,
          .want = "-21.3272",
      },
      TestCase{
          .name = "Double - too long",
          .input = 20'000'000.0,
          .want = "2e+07",
      },
  };

  Compare(test_cases);
}

TEST(TestWriteFucntions, TestMonostate) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Monostate",
          .input = std::monostate{},
          .want = "null",
      },
  };

  Compare(test_cases);
}
