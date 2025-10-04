#pragma once
#include "desc_parser.h"


template<class DescType>
std::optional<DescType> DescParser::Parse(const DescRegistry& registry, std::string_view data)
{
  nlohmann::json rootJson;
  try {
    rootJson = nlohmann::json::parse(data, nullptr, true, true);
  } catch (const nlohmann::json::parse_error& e) {
    registry._LogError(std::format("Failed to parse data: {}", e.what()));
    return std::nullopt;
  }

  std::string errorBuffer;
  std::optional<std::any> parsedDesc = RecursiveParse(registry, errorBuffer, std::move(rootJson));
  if (!parsedDesc) {
    registry._LogError(std::move(errorBuffer));
    return std::nullopt;
  }

  DescType* parsedTypedDesc = std::any_cast<DescType>(&parsedDesc.value());
  if (!parsedTypedDesc) {
    errorBuffer = std::format(
      "Mismatched type of parsed desc, expected \"{}\", but actual \"{}\"!",
      typeid(DescType).name(),
      parsedDesc->type().name());
    registry._LogError(std::move(errorBuffer));
    return std::nullopt;
  }

  return std::move(*parsedTypedDesc);
}