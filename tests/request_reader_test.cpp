#include <gtest/gtest.h>

#include "src/request_reader.h"
#include "test_utils.h"

#include <sstream>
#include <string>
#include <variant>
#include <vector>

TEST(TestParser, TestPostBusRequest) {
  using rm::PostBusRequest;

  struct TestCase {
    std::string name;
    std::string input;
    std::optional<PostBusRequest> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong format: empty BUS_NAME",
          .input = "  : stop1 -> stop2 -> stop1",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: one stop",
          .input = " Bus1: stop1 stop2 stop1",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: different delimiters - 1",
          .input = " bus1: stop1 - stop2 > stop1",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: different delimiters - 2",
          .input = " bus1: stop1 > stop2 - stop1",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: extra following delimiter(circular)",
          .input = " bus1: stop1 > stop2 > stop1 > ",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: extra following delimiter(non-circular)",
          .input = " bus1: stop1 - stop2 - stop3 - ",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format(circular): FIRST_STOP != LAST_STOP",
          .input = " bus1: stop1 > stop2 > stop3",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Short BUS_NAME(non-circular)",
          .input = " bus1: stop1 - stop2 - stop3",
          .want = PostBusRequest{
              .bus = "bus1",
              .stops = {"stop1", "stop2", "stop3", "stop2", "stop1"}},
      },
      TestCase{
          .name = "Short BUS_NAME(circular)",
          .input = " bus2: stop4 > stop5 > stop1 > stop4",
          .want = PostBusRequest{
              .bus = "bus2",
              .stops = {"stop4", "stop5", "stop1", "stop4"}},
      },
      TestCase{
          .name = "Long BUS_NAME(non-circular)",
          .input = " long bus name 1: stop2 - stop3 - stop4",
          .want = PostBusRequest{
              .bus = "long bus name 1",
              .stops = {"stop2", "stop3", "stop4", "stop3",
                        "stop2"}},
      },
      TestCase{
          .name = "Short BUS_NAME(circular)",
          .input = " long bus name 2: stop4 > stop3 > stop1 > stop4",
          .want = PostBusRequest{
              .bus = "long bus name 2",
              .stops = {"stop4", "stop3", "stop1", "stop4"}},
      },
      TestCase{
          .name = "Many stops(non-circular)",
          .input = " bus3: stop1 - stop1 - stop2 - stop1 - stop2 - stop3",
          .want = PostBusRequest{
              .bus = "bus3",
              .stops = {"stop1", "stop1", "stop2", "stop1",
                        "stop2",
                        "stop3", "stop2", "stop1", "stop2",
                        "stop1",
                        "stop1"}},
      },
      TestCase{
          .name = "Many stops(circular)",
          .input = " bus4: stop1 > stop1 > stop2 > stop1 > stop2 > stop3 > stop1",
          .want = PostBusRequest{
              .bus = "bus4",
              .stops = {"stop1", "stop1", "stop2", "stop1",
                        "stop2",
                        "stop3", "stop1"}},
      },
      TestCase{
          .name = "Long STOP_NAME(non-circular)",
          .input = " bus5: long stop 1 - long stop 2 - long stop 3",
          .want = PostBusRequest{
              .bus = "bus5",
              .stops = {"long stop 1", "long stop 2",
                        "long stop 3",
                        "long stop 2", "long stop 1"}},
      },
      TestCase{
          .name = "Long STOP_NAME(circular)",
          .input = " bus6: long stop 1 > long stop 2 > long stop 1",
          .want = PostBusRequest{
              .bus = "bus6",
              .stops = {"long stop 1", "long stop 2",
                        "long stop 1"}},
      },
      TestCase{
          .name = "Extra spaces",
          .input = "  b 1  :  long   stop   1  -  long stop 2   -   long stop 3  ",
          .want = PostBusRequest{
              .bus = "b 1",
              .stops = {"long   stop   1", "long stop 2",
                        "long stop 3",
                        "long stop 2", "long   stop   1"}},
      },
      TestCase{
          .name = "No spaces",
          .input = "b2:stop3-stop2-stop1",
          .want = PostBusRequest{
              .bus = "b2",
              .stops = {"stop3", "stop2", "stop1", "stop2",
                        "stop3"}},
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = rm::ParsePostBusRequest(input);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestParsr, TestPostStopRequest) {
  using rm::PostStopRequest;

  struct TestCase {
    std::string name;
    std::string input;
    std::optional<PostStopRequest> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong format: empty STOP_NAME",
          .input = " : 18.407908, 23.355151",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: unexpected symbols - 1",
          .input = "stop1: 18.407908an, 23.355151",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: unexpected symbols - 2",
          .input = "stop1: 18.407908, 23.355151e",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: no coords",
          .input = "stop1: latitude, longitude",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: no longitude",
          .input = "stop1: latitude, 23.355151",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: no coords delimiter",
          .input = "stop1: 18.407908 23.355151",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Wrong format: extra following symbols",
          .input = "stop1: 18.407908, 23.355151 123",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Positive coords",
          .input = "stop1: 18.407908, 23.355151",
          .want = PostStopRequest{
              .stop = "stop1",
              .coords = {18.407908, 23.355151}},
      },
      TestCase{
          .name = "Same coords",
          .input = "stop2: 18.407908, 18.407908",
          .want = PostStopRequest{
              .stop = "stop2",
              .coords = {18.407908, 18.407908}},
      },
      TestCase{
          .name = "Extra spaces",
          .input = "stooop stop3   :   57.407908  ,   108.407908  ",
          .want = PostStopRequest{
              .stop = "stooop stop3",
              .coords = {57.407908, 108.407908}},
      },
      TestCase{
          .name = "Negative lat",
          .input = "stop3: -57.407908, 23.355151",
          .want = PostStopRequest{
              .stop = "stop3",
              .coords = {-57.407908, 23.355151}},
      },
      TestCase{
          .name = "Negative long",
          .input = "stop4: 57.407908, -23.355151",
          .want = PostStopRequest{
              .stop = "stop4",
              .coords = {57.407908, -23.355151}},
      },
      TestCase{
          .name = "Negative coords",
          .input = "stop5: -57.407908, -23.355151",
          .want = PostStopRequest{
              .stop = "stop5",
              .coords = {-57.407908, -23.355151}},
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = rm::ParsePostStopRequest(input);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestParser, TestGetBusRequest) {
  using rm::GetBusRequest;

  struct TestCase {
    std::string name;
    std::string input;
    std::optional<GetBusRequest> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong format: empty BUS_NAME",
          .input = "",
          .want = std::nullopt,
      },
      TestCase{
          .name = "Short BUS_NAME",
          .input = "bus1",
          .want = GetBusRequest{.bus = "bus1"},
      },
      TestCase{
          .name = "Extra spaces",
          .input = "  123  ",
          .want = GetBusRequest{.bus = "123"},
      },
      TestCase{
          .name = "Long BUS_NAME",
          .input = "long bus name 1",
          .want = GetBusRequest{.bus = "long bus name 1"},
      },
      TestCase{
          .name = "Long BUS_NAME + extra spaces",
          .input = "  long bus name 2  ",
          .want = GetBusRequest{.bus = "long bus name 2"},
      },
  };

  for (auto &[name, input, want] : test_cases) {
    auto got = rm::ParseGetBusRequest(input);
    EXPECT_EQ(want, got) << name;
  }
}

TEST(TestReadInputRequests, TestRequests) {
  using rm::PostStopRequest;
  using rm::PostBusRequest;
  using rm::GetBusRequest;
  using rm::PostRequest;
  using rm::GetRequest;

  struct TestCase {
    std::string name;
    std::string input;
    std::vector<PostRequest> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "empty request",
          .input = "0\n",
          .want = {},
      },
      TestCase{
          .name = "Wrong format(request_count): non-numeric symbols",
          .input = "a3\n"
                   "Stop stop1: 18.407908, 23.355151\n"
                   "Bus 2: stop6 - stop7 - stop8\n"
                   "Bus 4: stop2 > stop4 > stop7 > stop12",
          .want = {},
      },
      TestCase{
          .name = "Wrong format(request_count): leading spaces",
          .input = " 3\n"
                   "Stop stop1: 18.407908, 23.355151\n"
                   "Bus 2: stop6 - stop7 - stop8\n"
                   "Bus 4: stop2 > stop4 > stop7 > stop12",
          .want = {},
      },
      TestCase{
          .name = "Wrong format(request_count): following spaces",
          .input = "3  \n"
                   "Stop stop1: 18.407908, 23.355151\n"
                   "Bus 2: stop6 - stop7 - stop8\n"
                   "Bus 4: stop2 > stop4 > stop7 > stop12",
          .want = {},
      },
      TestCase{
          .name = "Valid requests",
          .input = "6\n"
                   "Bus 1: stop1 > stop2 > stop3 > stop4 > "
                   "stop5 > stop1\n"
                   "Bus 2: stop6 - stop7 - stop8\n"
                   "Stop Stop1: 87.327412, 62.912265\n"
                   "Bus 3: first stop - second - third - forth - "
                   "fifth\n"
                   "Bus 4: stop2 > stop4 > stop7 > stop2\n"
                   "Stop Stop2: 35.395105, 82.385629\n",
          .want = {PostBusRequest{
              .bus = "1",
              .stops = {"stop1", "stop2", "stop3", "stop4",
                        "stop5", "stop1"}},
                   PostBusRequest{
                       .bus = "2",
                       .stops = {"stop6", "stop7", "stop8", "stop7", "stop6"}},
                   PostStopRequest{
                       .stop = "Stop1",
                       .coords = {87.327412, 62.912265}},
                   PostBusRequest{
                       .bus = "3",
                       .stops = {"first stop", "second", "third", "forth",
                                 "fifth", "forth", "third", "second",
                                 "first stop"}},
                   PostBusRequest{
                       .bus = "4",
                       .stops = {"stop2", "stop4", "stop7", "stop2"}},
                   PostStopRequest{
                       .stop = "Stop2",
                       .coords = {35.395105, 82.385629}},
          },
      }
  };

  for (auto &[name, input, want_input] : test_cases) {
    std::vector<PostRequest> got_input;

    std::istringstream ss_input(input);
    auto got = rm::ReadInputRequests(ss_input);
    for (auto &req : got)
      got_input.emplace_back(req);

    EXPECT_EQ(want_input, got_input) << name;
  }
}

TEST(TestReadOutputRequests, TestRequests) {
  using rm::GetBusRequest;
  using rm::GetRequest;

  struct TestCase {
    std::string name;
    std::string input;
    std::vector<GetRequest> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "empty request",
          .input = "0",
          .want = {}
      },
      TestCase{
          .name = "Wrong format(request_count): non-numeric symbols",
          .input = "at2\n"
                   "Bus bus1\n"
                   "Bus bus2",
          .want = {},
      },
      TestCase{
          .name = "Wrong format(request_count): leading spaces",
          .input = " at2\n"
                   "Bus bus1\n"
                   "Bus bus2",
          .want = {},
      },
      TestCase{
          .name = "Wrong format(request_count): following spaces",
          .input = "at2 \n"
                   "Bus bus1\n"
                   "Bus bus2",
          .want = {},
      },
      TestCase{
          .name = "Valid requests",
          .input = "4\n"
                   "  Bus 123  \n"
                   "Bus    432  \n"
                   "   Bus    234  \n"
                   "    Bus   543",
          .want = {GetBusRequest{.bus = "123"},
                   GetBusRequest{.bus = "432"},
                   GetBusRequest{.bus = "234"},
                   GetBusRequest{.bus = "543"}},
      },
  };

  for (auto &[name, input, want] : test_cases) {
    std::vector<GetRequest> got_output;

    std::istringstream ss_output(input);
    auto got = rm::ReadOutputRequests(ss_output);
    for (auto &req : got)
      got_output.emplace_back(req);

    EXPECT_EQ(want, got_output) << name;
  }
}
