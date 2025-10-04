#pragma once

#include <any>
#include <optional>

#include <nlohmann/json_fwd.hpp>

#include "desc_registry.h"

class DescParser
{
public:
  template<class DescType>
  static std::optional<DescType> Parse(const DescRegistry& registry, std::string_view data);

private:
  static std::optional<std::any> AbstractParse(const DescRegistry& registry, std::string& errorBuffer, std::string_view data);
  static std::optional<std::any> RecursiveParse(const DescRegistry& registry, std::string& errorBuffer, nlohmann::json dataJson);
  static std::optional<std::any> RecursiveObjectParse(const DescRegistry& registry, std::string& errorBuffer, DescTypeId descTypeId, nlohmann::json rootJson);
  static std::optional<std::any> RecursiveParseArray(const DescRegistry& registry, std::string& errorBuffer, nlohmann::json dataJson);
  static std::optional<std::any> RecursiveParsePrimitive(const DescRegistry& registry, std::string& errorBuffer, DescTypeId descTypeId, nlohmann::json dataJson);
};

#include "desc_parser.hpp"