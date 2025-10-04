#pragma once
#include "desc_parser.h"

template<class DescType>
std::optional<DescType> DescParser::Parse(const DescRegistry& registry, std::string_view data)
{
  std::string errorBuffer;
  std::optional<std::any> parsedDesc = AbstractParse(registry, errorBuffer, data);
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