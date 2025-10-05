#pragma once
#include "desc_parser.h"

template<class DescType>
std::optional<DescType> DescParser::TypedParse(const DescRegistry& registry, std::string_view data)
{
  std::string errors;
  std::optional<std::any> parsedData = StartParse(registry, errors, data);
  if (!parsedData) {
    registry._LogError(std::move(errors));
    return std::nullopt;
  }

  const std::type_index parsedDataTypeIndex = std::type_index{ parsedData->type() };
  std::optional<DescType> parsedDesc = EndParse<DescType>(std::move(parsedData).value());
  if (!parsedDesc) {
    errors = std::format(
      "Mismatched type of parsed desc, expected \"{}\", but actual \"{}\"!",
      typeid(DescType).name(),
      parsedDataTypeIndex.name());
    registry._LogError(std::move(errors));
    return std::nullopt;
  }

  return parsedDesc;
}

template<class DescType>
std::optional<DescType> DescParser::EndParse(std::any parsedData)
{
  DescType* parsedTypedDesc = std::any_cast<DescType>(&parsedData);
  if (!parsedTypedDesc) {
    return std::nullopt;
  }

  return std::move(*parsedTypedDesc);
}