#include "desc_parser.h"

#include <nlohmann/json.hpp>

std::optional<std::any> DescParser::AbstractParse(const DescRegistry& registry, std::string& errorBuffer, std::string_view data)
{
  nlohmann::json rootJson;
  try {
    rootJson = nlohmann::json::parse(data, nullptr, true, true);
  } catch (const nlohmann::json::parse_error& e) {
    registry._LogError(std::format("Failed to parse data: {}", e.what()));
    return std::nullopt;
  }

  return RecursiveParse(registry, errorBuffer, std::move(rootJson));
}

std::optional<std::any> DescParser::RecursiveParse(const DescRegistry& registry, std::string& errorBuffer, nlohmann::json rootJson)
{
  const nlohmann::json typeJson = rootJson.at("__type");
  if (!typeJson.is_string()) {
    errorBuffer += std::format("Internal error: field \"__type\" is reserved keyword and must be a string!\n");
    return std::nullopt;
  }

  const std::string_view parsedType = typeJson.get<std::string_view>();
  const DescTypeId descTypeId = registry.FindDescTypeId(parsedType);
  if (descTypeId == DescTypeId::Invalid) {
    errorBuffer += std::format("Desc type \"{}\" was not registered!\n", parsedType);
    return std::nullopt;
  }

  if (descTypeId == DescTypeId::Array) {
    // parse & unpack
    return RecursiveParseArray(registry, errorBuffer, std::move(rootJson));
  }
  if (rootJson.contains("__data")) {
    return RecursiveParsePrimitive(registry, errorBuffer, descTypeId, std::move(rootJson));
  }
  if (rootJson.contains("__fields")) {
    return RecursiveObjectParse(registry, errorBuffer, descTypeId, std::move(rootJson));
  }

  errorBuffer += std::format("Failed to recognize type of data. It must be one of supported: primitive, array or object!\n");
  return std::nullopt;
}

std::optional<std::any> DescParser::RecursiveObjectParse(const DescRegistry& registry, std::string& errorBuffer, DescTypeId descTypeId, nlohmann::json rootJson)
{
  const Desc* desc = registry.FindDesc(descTypeId);
  if (!desc) {
    errorBuffer += std::format("Internal error: cannot find a desc by id {}\n", std::to_underlying(descTypeId));
    return std::nullopt;
  }
  if (!rootJson.contains("__fields")) {
    errorBuffer += std::format("Internal error: array requires a \"__fields\" !\n");
    return std::nullopt;
  }
  nlohmann::json fieldsJson = std::move(rootJson.at("__fields"));
  const std::string_view descName = registry.GetDescTypeName(descTypeId);

  std::vector<DescParsedField> parsedFields;
  parsedFields.reserve(fieldsJson.size());
  for (const DescField& field : desc->GetFields()) {
    const std::string_view fieldName = registry.GetDescFieldName(field.id);
    if (!fieldsJson.contains(fieldName)) {
      errorBuffer += std::format("Desc {} requires field {}!\n", descName, fieldName);
      continue;
    }

    std::optional<std::any> parsedField = RecursiveParse(registry, errorBuffer, std::move(fieldsJson.at(fieldName)));
    if (!parsedField) {
      errorBuffer += std::format("Failed to parse field \"{}\" of desc {}!\n", fieldName, descName);
      break;
    }

    parsedFields.emplace_back(field, std::move(parsedField).value_or(std::any()));
  }

  if (parsedFields.size() != parsedFields.capacity()) {
    // Parsing errors were already logged.
    return std::nullopt;
  }

  std::optional<std::any> parsedObject = desc->Construct(errorBuffer, std::move(parsedFields));
  if (!parsedObject) {
    errorBuffer += std::format("Failed to construct object by its desc {}!\n", descName);
  }
  return parsedObject;
}

std::optional<std::any> DescParser::RecursiveParseArray(const DescRegistry& registry, std::string& errorBuffer, nlohmann::json dataJson)
{
  if (!dataJson.contains("__subtype")) {
    errorBuffer += std::format("Internal error: array requires a \"__subtype\" !\n");
    return std::nullopt;
  }

  const std::string_view subtype = dataJson.at("__subtype").get<std::string_view>();
  const DescTypeId subtypeId = registry.FindDescTypeId(subtype);
  if (subtypeId == DescTypeId::Invalid) {
    errorBuffer += std::format("Internal error: subtype of array \"{}\" is unknown!\n", subtype);
    return std::nullopt;
  }

  const Desc* desc = registry.FindDesc(subtypeId);
  if (!desc) {
    errorBuffer += std::format("Internal error: failed to find desc for type \"{}\"!\n", subtype);
    return std::nullopt;
  }

  if (!dataJson.contains("__values")) {
    errorBuffer += std::format("Internal error: array requires a \"__values\" field !\n");
    return std::nullopt;
  }

  nlohmann::json valuesJson = dataJson.at("__values");
  if (!valuesJson.is_array()) {
    errorBuffer += std::format("Internal error: array requires an array type of field \"__values\"!\n");
    return std::nullopt;
  }

  std::vector<std::any> values;
  values.reserve(valuesJson.size());
  for (nlohmann::json& valueJson : valuesJson) {
    std::optional<std::any> parsedValue = RecursiveParse(registry, errorBuffer, std::move(valueJson));
    if (!parsedValue) {
      errorBuffer += std::format("Failed to parse values[{}] of array!\n", values.size());
    }

    values.emplace_back(std::move(parsedValue).value_or(std::any()));
  }

  return desc->UnpackValuesArray(errorBuffer, std::move(values));
}

std::optional<std::any> DescParser::RecursiveParsePrimitive(const DescRegistry& registry, std::string& errorBuffer, DescTypeId descTypeId, nlohmann::json rootJson)
{
  nlohmann::json dataJson = rootJson.at("__data");
  if (descTypeId == DescTypeId::String) {
    std::string data = std::move(dataJson.get_ref<std::string&>());
    return std::move(data);
  }
  if (descTypeId == DescTypeId::Int32) {
    return dataJson.get<int32_t>();
  }

  errorBuffer += std::format("Internal error: parser for native type {} is unimplemented!!\n", registry.GetDescTypeName(descTypeId));
  return std::nullopt;
}