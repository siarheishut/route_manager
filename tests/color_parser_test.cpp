#include <optional>
#include <string>
#include <vector>

#include "json.h"
#include "gtest/gtest.h"
#include "svg/common.h"

#include "src/color_parser.h"
#include "test_utils.h"

TEST(TestColor, TestColor) {
  using namespace svg;

  struct TestCase {
    std::string name;
    json::Node input;
    std::optional<Color> want;
  };

  std::vector<TestCase> test_cases{
      TestCase{
          .name = "Wrong format: input is neither string nor array",
          .input = 123,
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong format: input array size is less than 3",
          .input = json::List{1, 3},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong format: input array size is more than 4",
          .input = json::List{1, 3, 5, 6.4, 3},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong Rgb format: <red> isn't int",
          .input = json::List{194.1, 41, 5},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong Rgb format: <green> isn't int",
          .input = json::List{19, "41", 5},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong Rgb format: <blue> isn't int",
          .input = json::List{194, 12, 51.1},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong Rgba format: <red> isn't int",
          .input = json::List{194.1, 41, 5, 0.3},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong Rgba format: <green> isn't int",
          .input = json::List{19, "41", 5, 0.3},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong Rgba format: <blue> isn't int",
          .input = json::List{194, 12, 51.1, 0.4},
          .want = std::nullopt
      },
      TestCase{
          .name = "Wrong Rgba format: <alpha> isn't double",
          .input = json::List{194, 12, 51, "0.412"},
          .want = std::nullopt
      },
      TestCase{
          .name = "Valid color: string type",
          .input = "brown",
          .want = "brown"
      },
      TestCase{
          .name = "Valid color: Rgb type",
          .input = json::List{15, 14, 17},
          .want = Color{Rgb{.red = 15, .green = 14, .blue = 17}}
      },
      TestCase{
          .name = "Valid color: Rgba type",
          .input = json::List{111, 144, 177, 0.555},
          .want = Color{Rgba{
              .red = 111, .green = 144, .blue = 177, .alpha = 0.555}}
      }
  };

  for (auto &[name, input, want] : test_cases) {
    if (!want) {
      EXPECT_FALSE(rm::IsColor(input)) << name;
      continue;
    }

    auto got = rm::AsColor(std::move(input));
    EXPECT_EQ(*want, got) << name;
  }
}
