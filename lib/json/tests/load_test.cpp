#include "json.h"

#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace {

struct TestCase {
  std::string name;
  std::string input;
  std::optional<json::Node> want;
};

std::string ToString(const json::Node &node) {
  std::stringstream ss;
  ss << node;
  return ss.str();
}

void Compare(const std::vector<TestCase> &test_cases) {

  for (auto &[name, input, want_] : test_cases) {
    std::istringstream in(input);
    json::Node got_;
    in >> got_;
    if (!in) {
      EXPECT_FALSE(want_) << name;
      continue;
    }
    auto got = ToString(got_);
    auto want = ToString(*want_);
    EXPECT_EQ(want, got) << name;
  }
}
}

TEST(TestLoadFunctions, TestArray) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Extra comma",
          .input = "\t[\n"
                   "\t\"stop1\",\n"
                   "\t\"stop 2\",,\n"
                   "\t\"long stop name 1\",\n"
                   "\t\"long  stop    namestop 4\",\n"
                   "\t\"stop1\"\n"
                   "\t]",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Fixed \"Extra comma\"",
          .input = "\t[\n"
                   "\t\"stop1\",\n"
                   "\t\"stop 2\",\n"
                   "\t\"long stop name 1\",\n"
                   "\t\"long  stop    namestop 4\",\n"
                   "\t\"stop1\"\n"
                   "\t]",
          .want = Node{
              List{Node{"stop1"}, Node{"stop 2"}, Node{"long stop name 1"},
                   Node{"long  stop    namestop 4"}, Node{"stop1"}}},
      },
      TestCase{
          .name = "Array without end sign",
          .input = "\t[\n"
                   "\t\"stop1\",\n"
                   "\t\"stop 2\",,\n"
                   "\t\"long stop name 1\",\n"
                   "\t\"long  stop    namestop 4\",\n"
                   "\t\"stop1\"\n",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Fixed \"Array without end sign\"",
          .input = "\t[\n"
                   "\t\"stop1\",\n"
                   "\t\"stop 2\",\n"
                   "\t\"long stop name 1\",\n"
                   "\t\"long  stop    namestop 4\",\n"
                   "\t\"stop1\"\n"
                   "\t]",
          .want = Node{
              List{Node{"stop1"}, Node{"stop 2"}, Node{"long stop name 1"},
                   Node{"long  stop    namestop 4"}, Node{"stop1"}}},
      },
      TestCase{
          .name = "Redefinition in road_distances",
          .input = "{\n"
                   "\"type\": \"Stop\",\n"
                   "\"road_distances\": {\n"
                   "\"stop 2\": 750,\n"
                   "\"stop 2\": 1000\n"
                   "},\n"
                   "\"longitude\": 37.64839,\n"
                   "\"name\": \"stop 1\",\n"
                   "\"latitude\": 55.581065\n"
                   "},",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Fixed \"Redefinition in road_distances\"",
          .input = "{\n"
                   "\"type\": \"Stop\",\n"
                   "\"road_distances\": {\n"
                   "\"stop 2\": 750,\n"
                   "\"stop 3\": 1000\n"
                   "},\n"
                   "\"longitude\": 37.64839,\n"
                   "\"name\": \"stop 1\",\n"
                   "\"latitude\": 55.581065\n"
                   "},",
          .want = Node{
              Dict{{"type", "Stop"},
                   {"road_distances", Dict{{"stop 2", 750}, {"stop 3", 1000}}},
                   {"latitude", 55.581065},
                   {"longitude", 37.64839},
                   {"name", "stop 1"}}},
      },
      TestCase{
          .name = "Array of strings",
          .input = "\t[\n"
                   "\t\"stop1\",\n"
                   "\t\"stop 2\",\n"
                   "\t\"long stop name 1\",\n"
                   "\t\"long  stop    namestop 4\",\n"
                   "\t\"stop1\"\n"
                   "\t]",
          .want = Node{
              List{Node{"stop1"}, Node{"stop 2"}, Node{"long stop name 1"},
                   Node{"long  stop    namestop 4"}, Node{"stop1"}}},
      },
      TestCase{
          .name = "Array of maps",
          .input = "[\n"
                   "{\n"
                   "\"type\": \"Bus\",\n"
                   "\"name\": \"256\",\n"
                   "\"id\": 1965312327\n"
                   "},\n"
                   "{\n"
                   "\"type\": \"Bus\",\n"
                   "\"name\": \"750\",\n"
                   "\"id\": 519139350\n"
                   "}\n"
                   "]",
          .want = Node{
              List{Dict{{"type", "Bus"}, {"name", "256"},
                        {"id", 1965312327}},
                   Dict{{"type", "Bus"}, {"name", "750"},
                        {"id", 519139350}}}},

      },
      TestCase{
          .name = "Empty array",
          .input = "[]",
          .want = Node{List{}},
      },
      TestCase{
          .name = "Array with comma before ']'",
          .input = "[\"stop1\", \"stop2\", \"stop3\",]",
          .want = Node{List{"stop1", "stop2", "stop3"}},
      },
      TestCase{
          .name = "Array of numbers",
          .input = "[5362, 632356.32, 52.352323, 10]",
          .want = Node{
              List{5362, 632356.32, 52.352323, 10}},
      },
      TestCase{
          .name = "Array of arrays",
          .input = "[\n"
                   "\t[5362, 632356.32, 52.352323, 10],\n"
                   "\t[\"stop1\", \"stop2\", \"stop3\"]\n"
                   "]",
          .want = Node{
              List{List{5362, 632356.32, 52.352323, 10},
                   List{"stop1", "stop2", "stop3"}}},
      },
      TestCase{
          .name = "Mixed array",
          .input = "[\n"
                   "21415, \"stop1\", 12.1511, true"
                   "]",
          .want = Node{
              List{21415, "stop1", 12.1511, true}},
      },
  };

  Compare(test_cases);
}

TEST(TestLoadFunctions, TestMap) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Extra comma",
          .input = "{\n"
                   "   \"Stop 1\": 341225,,\n"
                   "   \"Long stop name 2\": 1204\n"
                   "}",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Fixed \"Extra comma\"",
          .input = "{\n"
                   "   \"Stop 1\": 341225,\n"
                   "   \"Long stop name 2\": 1204\n"
                   "}",
          .want = Node{
              Dict{{"Stop 1", 341225}, {"Long stop name 2", 1204}}},
      },
      TestCase{
          .name = "Map without end sign",
          .input = "{\n"
                   "   \"Stop 1\": 341225,,\n"
                   "   \"Long stop name 2\": 1204\n",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Fixed \"Map without end sign\"",
          .input = "{\n"
                   "   \"Stop 1\": 341225,\n"
                   "   \"Long stop name 2\": 1204\n"
                   "}",
          .want = Node{
              Dict{{"Stop 1", 341225}, {"Long stop name 2", 1204}}},
      },
      TestCase{
          .name = "Map with non-string key",
          .input = "{123: \"some stop\","
                   "24.214: 14}",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Empty map",
          .input = "{}",
          .want = Node{Dict{}},
      },
      TestCase{
          .name = "Map with comma before }",
          .input = "{\"stop1\":1000, \"stop2\":2000, \"stop3\":3000,}",
          .want = Node{Dict{{"stop1", 1000}, {"stop2", 2000}, {"stop3", 3000}}},
      },
      TestCase{
          .name = "Map {string, Dict}",
          .input = "{\t\"stat_requests\": [\n"
                   "    {\n"
                   "      \"type\": \"Bus\",\n"
                   "      \"name\": \"Bus 1\",\n"
                   "      \"id\": 1965312327\n"
                   "    },\n"
                   "    {\n"
                   "      \"type\": \"Stop\",\n"
                   "      \"name\": \"Stop 1\",\n"
                   "      \"id\": 65100610\n"
                   "    },\n"
                   "    {\n"
                   "      \"type\": \"Stop\",\n"
                   "      \"name\": \"Stop 2\",\n"
                   "      \"id\": 1042838872\n"
                   "    }\n"
                   "  ]\n"
                   "\t}",
          .want = Node{
              Dict{{"stat_requests",
                    List{Dict{{"type", "Bus"},
                              {"name", "Bus 1"},
                              {"id", 1965312327}},
                         Dict{{"type", "Stop"},
                              {"name", "Stop 1"},
                              {"id", 65100610}},
                         Dict{{"type", "Stop"},
                              {"name", "Stop 2"},
                              {"id", 1042838872}}}}}},
      },
      TestCase{
          .name = "Map {string, int}",
          .input = "{\n"
                   "   \"Stop 1\": 341225,\n"
                   "   \"Long stop name 2\": 1204\n"
                   "}",
          .want = Node{
              Dict{{"Stop 1", 341225}, {"Long stop name 2", 1204}}},
      },
  };

  Compare(test_cases);
}

TEST(TestLoadFunctions, TestString) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Empty string",
          .input = "\"\"",
          .want = Node{""},
      },
      TestCase{
          .name = "String – short",
          .input = "\"Name\"",
          .want = Node{"Name"},
      },
      TestCase{
          .name = "String – long",
          .input = "\"Some long    string nama\"",
          .want = Node{"Some long    string nama"},
      },
  };

  Compare(test_cases);
}

TEST(TestLoadFunctions, TestBool) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Bool – true",
          .input = "true",
          .want = Node{true},
      },
      TestCase{
          .name = "Bool - false",
          .input = "false",
          .want = Node{false},
      },
  };

  Compare(test_cases);
}

TEST(TestLoadFunctions, TestInt) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Empty int",
          .input = "",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Non-digit non-a-f symbol",
          .input = "876h9",
          .want = Node{876},
      },
      TestCase{
          .name = "Non-digit symbol",
          .input = "876a9",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Int",
          .input = "123",
          .want = Node{123},
      },
      TestCase{
          .name = "Int – negative",
          .input = "-1234",
          .want = Node{-1234},
      },
      TestCase{
          .name = "Int - INT_MAX",
          .input = "2147483647",
          .want = Node{std::numeric_limits<int>::max()},
      },
      TestCase{
          .name = "Int - INT_MIN",
          .input = "-2147483648",
          .want = Node{std::numeric_limits<int>::min()},
      },
      TestCase{
          .name = "Int - INT_MAX+1",
          .input = "2147483648",
          .want = Node{2147483648.0},
      },
      TestCase{
          .name = "Int - INT_MIN-1",
          .input = "-2147483649",
          .want = Node{-2147483649.0},
      },
  };

  Compare(test_cases);
}

TEST(TestLoadFunctions, TestDouble) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Empty double",
          .input = "",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Double – extra a-f character",
          .input = "21a4.43e2",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Double – extra ] character",
          .input = "214.43]",
          .want = Node{214.43},
      },
      TestCase{
          .name = "Double – extra , character",
          .input = "214.43,",
          .want = Node{214.43},
      },
      TestCase{
          .name = "Double – extra } character",
          .input = "214.43}",
          .want = Node{214.43},
      },
      TestCase{
          .name = "Double – extra non-digit non-a-f character",
          .input = "21k4.43e2",
          .want = Node{21},
      },
      TestCase{
          .name = "Double – exponential form",
          .input = "214.43e2",
          .want = Node{21443},
      },
      TestCase{
          .name = "Negative double",
          .input = "-32.314515",
          .want = Node{-32.314515},
      },
      TestCase{
          .name = "Double – small",
          .input = "412.153",
          .want = Node{412.153},
      },
      TestCase{
          .name = "Double - huge",
          .input = "3429874.234214",
          .want = Node{3429874.234214},
      },
  };

  Compare(test_cases);
}

TEST(TestLoadFunctions, TestMonostate) {
  using namespace json;

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Monostate",
          .input = "null",
          .want = std::monostate{},
      },
  };

  Compare(test_cases);
}
