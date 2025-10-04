#include "std_headers.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "desc_builder.h"
#include "desc_registry.h"
#include "desc_parser.h"

namespace {

struct SimpleItem
{
  int x{ 0 };
  int y{ 0 };
};

struct ComplexItem
{
  SimpleItem a;
  SimpleItem b;
};


}

TEST(DescRegistryTest, ParseInt32)
{
  DescRegistry descRegistry;
  constexpr std::string_view SerializedData = R"(
{
  "__type": "int32",
  "__data": 100
})";

  const std::optional<int32_t> parsedItem = DescParser::Parse<int32_t>(descRegistry, SerializedData);
  ASSERT_TRUE(parsedItem.has_value());
  ASSERT_EQ(*parsedItem, 100);
}

TEST(DescRegistryTest, ParseString)
{
  DescRegistry descRegistry;
  constexpr std::string_view SerializedData = R"(
{
  "__type": "string",
  "__data": "data"
})";

  const std::optional<std::string> parsedItem = DescParser::Parse<std::string>(descRegistry, SerializedData);
  ASSERT_TRUE(parsedItem.has_value());
  ASSERT_EQ(*parsedItem, std::string_view{ "data" });
}

TEST(DescRegistryTest, ParseSimpleItem)
{
  DescRegistry descRegistry;
  DescBuilder<SimpleItem>(descRegistry, "simple_item")
    .AddField<&SimpleItem::x>("x")
    .AddField<&SimpleItem::y>("y")
    .Build();

  constexpr std::string_view SerializedData = R"(
{
  "__type": "simple_item",
  "__fields": {
    "x": {
      "__type": "int32",
      "__data": 10
    },
    "y": {
      "__type": "int32",
      "__data": 20
    }
  }
})";

  const std::optional<SimpleItem> parsedItem = DescParser::Parse<SimpleItem>(descRegistry, SerializedData);
  ASSERT_TRUE(parsedItem.has_value());
  ASSERT_EQ(parsedItem->x, 10);
  ASSERT_EQ(parsedItem->y, 20);
}

TEST(DescRegistryTest, ParseComplexItem)
{
  DescRegistry descRegistry;
  DescBuilder<SimpleItem>(descRegistry, "simple_item")
    .AddField<&SimpleItem::x>("x")
    .AddField<&SimpleItem::y>("y")
    .Build();

  DescBuilder<ComplexItem>(descRegistry, "complex_item")
    .AddField<&ComplexItem::a>("a")
    .AddField<&ComplexItem::b>("b")
    .Build();

  constexpr std::string_view SerializedData = R"(
{
  "__type": "complex_item",
  "__fields": {
    "a": {
      "__type": "simple_item",
      "__fields": {
        "x": {
          "__type": "int32",
          "__data": 1
        },
        "y": {
          "__type": "int32",
          "__data": 2
        }
      }
    },
    "b": {
      "__type": "simple_item",
      "__fields": {
        "x": {
          "__type": "int32",
          "__data": 3
        },
        "y": {
          "__type": "int32",
          "__data": 4
        }
      }
    }
  }
})";

  const std::optional<ComplexItem> parsedItem = DescParser::Parse<ComplexItem>(descRegistry, SerializedData);
  ASSERT_TRUE(parsedItem.has_value());
  ASSERT_EQ(parsedItem->a.x, 1);
  ASSERT_EQ(parsedItem->a.y, 2);
  ASSERT_EQ(parsedItem->b.x, 3);
  ASSERT_EQ(parsedItem->b.y, 4);
}

TEST(DescRegistryTest, ParseArrayOfInt)
{
  DescRegistry descRegistry;
  constexpr std::string_view SerializedArrayOfIntegers = R"(
{
  "__type": "array",
  "__subtype": "int32",
  "__values": [
    {
      "__type": "int32",
      "__data": 1
    },
    {
      "__type": "int32",
      "__data": 2
    },
    {
      "__type": "int32",
      "__data": 3
    }
  ]
})";

  const std::optional<std::vector<int32_t>> parsedItem = DescParser::Parse<std::vector<int32_t>>(descRegistry, SerializedArrayOfIntegers);
  ASSERT_TRUE(parsedItem.has_value());
  ASSERT_EQ(parsedItem->size(), 3);
  ASSERT_EQ(parsedItem->at(0), 1);
  ASSERT_EQ(parsedItem->at(1), 2);
  ASSERT_EQ(parsedItem->at(2), 3);
}

TEST(DescRegistryTest, ParseArrayOfComplexItem)
{
  DescRegistry descRegistry;
  DescBuilder<SimpleItem>(descRegistry, "simple_item")
    .AddField<&SimpleItem::x>("x")
    .AddField<&SimpleItem::y>("y")
    .Build();

  DescBuilder<ComplexItem>(descRegistry, "complex_item")
    .AddField<&ComplexItem::a>("a")
    .AddField<&ComplexItem::b>("b")
    .Build();
  constexpr std::string_view SerializedArrayOfComplexItems = R"(
{
  "__type": "array",
  "__subtype": "complex_item",
  "__values": [
    {
      "__type": "complex_item",
      "__fields": {
        "a": {
          "__type": "simple_item",
          "__fields": {
            "x": {
              "__type": "int32",
              "__data": 1
            },
            "y": {
              "__type": "int32",
              "__data": 2
            }
          }
        },
        "b": {
          "__type": "simple_item",
          "__fields": {
            "x": {
              "__type": "int32",
              "__data": 3
            },
            "y": {
              "__type": "int32",
              "__data": 4
            }
          }
        }
      }
    },
    {
      "__type": "complex_item",
      "__fields": {
        "a": {
          "__type": "simple_item",
          "__fields": {
            "x": {
              "__type": "int32",
              "__data": 10
            },
            "y": {
              "__type": "int32",
              "__data": 20
            }
          }
        },
        "b": {
          "__type": "simple_item",
          "__fields": {
            "x": {
              "__type": "int32",
              "__data": 30
            },
            "y": {
              "__type": "int32",
              "__data": 40
            }
          }
        }
      }
    }
  ]
})";

  const auto parsedItem = DescParser::Parse<std::vector<ComplexItem>>(descRegistry, SerializedArrayOfComplexItems);
  ASSERT_TRUE(parsedItem.has_value());
  ASSERT_EQ(parsedItem->size(), 2);
  ASSERT_EQ(parsedItem->at(0).a.x, 1);
  ASSERT_EQ(parsedItem->at(0).a.y, 2);
  ASSERT_EQ(parsedItem->at(0).b.x, 3);
  ASSERT_EQ(parsedItem->at(0).b.y, 4);
  ASSERT_EQ(parsedItem->at(1).a.x, 10);
  ASSERT_EQ(parsedItem->at(1).a.y, 20);
  ASSERT_EQ(parsedItem->at(1).b.x, 30);
  ASSERT_EQ(parsedItem->at(1).b.y, 40);
}