#include "std_headers.h"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

enum class DescTypeId : uint32_t
{
  Int32,
  String,
  Array,
  Object,

  Invalid = std::numeric_limits<uint32_t>::max()
};

enum class DescFieldId : uint32_t
{
  Invalid = std::numeric_limits<uint32_t>::max()
};


//
// template<class Func, class ResourceType>
// concept ResourceSerializable = std::is_invocable_r_v<ResourceStorage, Func, ResourceType>;
//
// template<class Func, class ResourceType>
// concept ResourceDeserializable = std::is_invocable_r_v<std::optional<ResourceType>, Func, ResourceStorage>;

class DescRegistry;

struct DescField
{
  DescFieldId id;
  DescTypeId type;
};

struct ParsedDescField
{
  DescField field;
  std::any value;
};

struct Desc
{
  using Constructor = std::move_only_function<std::optional<std::any>(std::string&, std::vector<ParsedDescField>)>;

  std::string name;
  std::vector<DescField> fields;
  mutable Constructor constructor;
};

template<class DescType>
class DescBuilder
{
public:
  DescBuilder(DescRegistry& registry, std::string_view name);

  template<class FieldType>
  DescBuilder& AddField(std::string_view name, FieldType DescType::* fieldPtr);

  DescTypeId Build();

private:
  using DescConstructor = std::move_only_function<std::optional<std::any>(std::string& errors, std::vector<ParsedDescField> parsedFields)>;

  struct FieldConstructor
  {
    using Constructor = std::move_only_function<bool(std::string& /*errors*/, std::any /*parsedField*/, std::any& /*object*/)>;

    DescFieldId fieldId{ DescFieldId::Invalid };
    Constructor constructor;
  };

  Ref<DescRegistry> _registry;
  std::vector<DescField> _fields;
  std::vector<FieldConstructor> _fieldConstructors;
  std::string _error;
  std::string _name;
};

class DescRegistry
{
public:
  DescRegistry();

  template<class DescType>
  DescTypeId FindDescTypeId() const;
  DescTypeId FindDescTypeId(std::string_view name) const;
  std::string_view GetDescTypeName(DescTypeId typeId) const;
  const Desc* FindDesc(DescTypeId typeId) const;

  DescFieldId FindDescFieldId(std::string_view name) const;
  std::string_view GetDescFieldName(DescFieldId fieldId) const;

  template<class DescType>
  DescBuilder<DescType> Register(std::string_view name);

private:
  friend class DescParser;
  template<class DescType>
  friend class DescBuilder;

  template<class DescType>
  DescTypeId _RegisterDesc(Desc desc);

  DescFieldId _GetOrRegisterField(std::string_view field);
  void _LogError(std::string error) const;

private:
  std::vector<std::string> _fieldNames;
  std::unordered_map<std::string_view, DescFieldId> _fieldIndexByName;

  std::vector<Desc> _descList;
  std::unordered_map<std::type_index, DescTypeId> _descIndexByGenericType;
  std::unordered_map<std::string_view, DescTypeId> _descIndexByName;
};

class DescParser
{
public:
  template<class DescType>
  static std::optional<DescType> Parse(const DescRegistry& registry, std::string_view data);

private:
  static std::optional<std::any> RecursiveParse(const DescRegistry& registry, std::string& errorBuffer, DescTypeId descTypeId, nlohmann::json dataJson);
  static std::optional<std::any> RecursiveObjectParse(const DescRegistry& registry, std::string& errorBuffer, DescTypeId descTypeId, nlohmann::json rootJson);
  static std::optional<std::any> RecursiveParseArray(const DescRegistry& registry, std::string& errorBuffer, nlohmann::json dataJson);
  static std::optional<std::any> RecursiveParsePrimitive(const DescRegistry& registry, std::string& errorBuffer, DescTypeId descTypeId, nlohmann::json dataJson);
};

template<class DescType>
template<class FieldType>
DescBuilder<DescType>& DescBuilder<DescType>::AddField(std::string_view name, FieldType DescType::* fieldPtr)
{
  const DescFieldId fieldId = _registry->_GetOrRegisterField(name);
  const DescTypeId fieldTypeId = _registry->FindDescTypeId<FieldType>();
  _fields.emplace_back(fieldId, fieldTypeId);
  _fieldConstructors.emplace_back(fieldId, [fieldPtr](std::string& errors, std::any fieldObject, std::any& object) {
    FieldType* field = std::any_cast<FieldType>(&fieldObject);
    if (!field) {
      errors += std::format("Internal error: mismatched type of parsed field (expected \"{}\", actual \"{}\")!\n", typeid(FieldType).name(), fieldObject.type().name());
      return false;
    }
    DescType* desc = std::any_cast<DescType>(&object);
    if (!desc) {
      errors += std::format("Internal error: mismatched type of target object (expected \"{}\", actual \"{}\")!\n", typeid(DescType).name(), object.type().name());
      return false;
    }
    desc->*fieldPtr = std::move(*field);
    return true;
  });
  return *this;
}

template<class DescType>
DescBuilder<DescType> DescRegistry::Register(std::string_view name)
{
  return DescBuilder<DescType>{ *this, name };
}

template<class DescType>
DescTypeId DescRegistry::_RegisterDesc(Desc desc)
{
  const std::type_index genericDescTypeIndex{ typeid(DescType) };
  if (_descIndexByGenericType.contains(genericDescTypeIndex)) {
    return DescTypeId::Invalid;
  }

  const DescTypeId id{ static_cast<std::underlying_type_t<DescTypeId>>(_descList.size()) };
  const std::string_view nameView = desc.name; //< store name before desc moving
  _descList.push_back(std::move(desc));
  _descIndexByGenericType[genericDescTypeIndex] = id;
  _descIndexByName[nameView] = id;
  return id;
}

template<class DescType>
std::optional<DescType> DescParser::Parse(const DescRegistry& registry, std::string_view data)
{
  nlohmann::json rootJson = nlohmann::json::parse(data, nullptr, false, true);
  const DescTypeId descTypeId = registry.FindDescTypeId<DescType>();
  const Desc* desc = registry.FindDesc(descTypeId);
  if (!desc) {
    registry._LogError(std::format("desc \"{}\" not found", typeid(DescType).name()));
    return std::nullopt;
  }

  std::string errorBuffer;
  std::optional<std::any> parsedDesc = RecursiveParse(registry, errorBuffer, descTypeId, std::move(rootJson));
  if (!parsedDesc) {
    registry._LogError(std::move(errorBuffer));
    return std::nullopt;
  }

  DescType* parsedTypedDesc = std::any_cast<DescType>(&parsedDesc.value());
  if (!parsedTypedDesc) {
    errorBuffer = std::format("Internal error: mismatched type of parsed desc \"{}\"", registry.GetDescTypeName(descTypeId));
    registry._LogError(std::move(errorBuffer));
    return std::nullopt;
  }

  return std::move(*parsedTypedDesc);
}

template<class DescType>
DescTypeId DescBuilder<DescType>::Build()
{
  Desc::Constructor descInstanceConstructor =
    [fieldConstructors = std::move(_fieldConstructors), registry = _registry.get()](std::string& errors, std::vector<ParsedDescField> parsedFields) mutable -> std::optional<std::any> {
    const DescTypeId descTypeId = registry->FindDescTypeId<DescType>();
    std::any descStorage = std::make_any<DescType>();

    if (fieldConstructors.size() != parsedFields.size()) {
      errors += std::format(
        "Mismatched fields to construct desc {}: expected %d, but actual %d!",
        registry->GetDescTypeName(descTypeId),
        fieldConstructors.size(),
        parsedFields.size());
      return std::nullopt;
    }

    for (auto& [field, value] : parsedFields) {
      auto it = std::find_if(fieldConstructors.begin(), fieldConstructors.end(), [fieldId = field.id](const FieldConstructor& constructor) {
        return constructor.fieldId == fieldId;
      });
      if (it == fieldConstructors.end()) {
        errors += std::format("Constructor for field {} was not found!", registry->GetDescFieldName(field.id));
        return std::nullopt;
      }

      FieldConstructor& fieldConstructor = *it;
      const bool fieldConstructed = fieldConstructor.constructor(errors, std::move(value), descStorage);
      if (!fieldConstructed) {
        errors += std::format("Failed to construct field {}!", registry->GetDescFieldName(field.id));
        return std::nullopt;
      }
    }

    return std::any_cast<DescType>(std::move(descStorage));
  };

  Desc desc{
    std::move(_name),
    std::move(_fields),
    std::move(descInstanceConstructor)
  };
  const DescTypeId typeId = _registry->_RegisterDesc<DescType>(std::move(desc));
  if (typeId == DescTypeId::Invalid) {
    _registry->_LogError(std::format("DescType {} has been already registered!", typeid(DescType).name()));
  }
  return typeId;
}

template<class DescType>
DescBuilder<DescType>::DescBuilder(DescRegistry& registry, std::string_view name)
  : _registry(&registry)
  , _name(name)
{
}

template<class DescType>
DescTypeId DescRegistry::FindDescTypeId() const
{
  const auto it = _descIndexByGenericType.find(std::type_index(typeid(DescType)));
  if (it == _descIndexByGenericType.end()) {
    return DescTypeId::Invalid;
  }
  return it->second;
}

DescRegistry::DescRegistry()
{
  _RegisterDesc<int32_t>(Desc{"int32"});
  _RegisterDesc<std::string>(Desc{"string"});

  struct ArrayTag {};
  _RegisterDesc<ArrayTag>(Desc{"array"});

  struct ObjectTag {};
  _RegisterDesc<ObjectTag>(Desc{"object"});
}

DescTypeId DescRegistry::FindDescTypeId(std::string_view name) const
{
  const auto it = _descIndexByName.find(name);
  if (it == _descIndexByName.end()) {
    return DescTypeId::Invalid;
  }
  return it->second;
}

std::string_view DescRegistry::GetDescTypeName(DescTypeId typeId) const
{
  const Desc* desc = FindDesc(typeId);
  if (desc == nullptr) {
    return "Invalid";
  }
  return desc->name;
}

const Desc* DescRegistry::FindDesc(DescTypeId typeId) const
{
  if (typeId == DescTypeId::Invalid) {
    return nullptr;
  }
  const auto index = std::to_underlying(typeId);
  if (index >= _descList.size()) {
    return nullptr;
  }
  return &_descList[index];
}

DescFieldId DescRegistry::FindDescFieldId(std::string_view name) const
{
  const auto it = _fieldIndexByName.find(name);
  if (it == _fieldIndexByName.end()) {
    return DescFieldId::Invalid;
  }
  return it->second;
}

std::string_view DescRegistry::GetDescFieldName(DescFieldId fieldId) const
{
  if (fieldId == DescFieldId::Invalid) {
    return "Invalid";
  }
  const auto index = std::to_underlying(fieldId);
  if (index >= _fieldNames.size()) {
    return GetDescFieldName(DescFieldId::Invalid);
  }
  return _fieldNames[index];
}

DescFieldId DescRegistry::_GetOrRegisterField(std::string_view field)
{
  const auto it = std::ranges::find(_fieldNames, field);
  if (it != _fieldNames.end()) {
    const size_t fieldNameIndex = std::distance(_fieldNames.begin(), it);
    return static_cast<DescFieldId>(fieldNameIndex);
  }
  const DescFieldId fieldId = static_cast<DescFieldId>(_fieldNames.size());
  _fieldNames.emplace_back(field);
  _fieldIndexByName[field] = fieldId;
  return fieldId;
}

void DescRegistry::_LogError(std::string error) const
{
  std::println("DescRegistry > {}", std::move(error));
}

std::optional<std::any> DescParser::RecursiveParse(const DescRegistry& registry, std::string& errorBuffer, DescTypeId descTypeId, nlohmann::json rootJson)
{
  const nlohmann::json typeJson = rootJson.at("__type");
  if (!typeJson.is_string()) {
    errorBuffer += std::format("Internal error: field \"__type\" is reserved keyword and must be a string!\n");
    return std::nullopt;
  }

  const std::string_view parsedType = typeJson.get<std::string_view>();
  const std::string_view descName = registry.GetDescTypeName(descTypeId);
  if (parsedType != descName) {
    errorBuffer += std::format("Mismatched type of desc: expected \"{}\", but actual \"{}\"\n", descName, parsedType);
    return std::nullopt;
  }

  if (descTypeId == DescTypeId::Array) {
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

  std::vector<ParsedDescField> parsedFields;
  parsedFields.reserve(fieldsJson.size());
  for (const DescField& field : desc->fields) {
    const std::string_view fieldName = registry.GetDescFieldName(field.id);
    if (!fieldsJson.contains(fieldName)) {
      errorBuffer += std::format("Desc {} requires field {}!\n", descName, fieldName);
      continue;
    }

    std::optional<std::any> parsedField = RecursiveParse(registry, errorBuffer, field.type, std::move(fieldsJson.at(fieldName)));
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

  std::optional<std::any> parsedObject = desc->constructor(errorBuffer, std::move(parsedFields));
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
    std::optional<std::any> parsedValue = RecursiveParse(registry, errorBuffer, subtypeId, std::move(valueJson));
    if (!parsedValue) {
      errorBuffer += std::format("Failed to parse values[{}] of array!\n", values.size());
    }

    values.emplace_back(std::move(parsedValue).value_or(std::any()));
  }

  return std::move(values);
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

namespace {

constexpr std::string_view SerializedArrayOfSimpleItems = R"(
{
  "__type": "array",
  "__subtype": "simple_item",
  "__values": [
    {
      "__type": "simple_item",
      "__fields": {
        "x": {
          "__type": "int32",
          "__data": 20
        },
        "y": {
          "__type": "int32",
          "__data": 10
        }
      }
    },
    {
      "__type": "simple_item",
      "__fields": {
        "x": {
          "__type": "int32",
          "__data": 200
        },
        "y": {
          "__type": "int32",
          "__data": 100
        }
      }
    },
  ]
})";

constexpr std::string_view SerializedArrayOfIntegers = R"(
{
  "__type": "array",
  "__subtype": "int",
  "__values": [
    {
      "__type": "int",
      "__data": 1
    },
    {
      "__type": "int",
      "__data": 2
    },
    {
      "__type": "int",
      "__data": 3
    },
  ]
})";

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
  ASSERT_EQ(*parsedItem, std::string_view{"data"});
}

TEST(DescRegistryTest, ParseSimpleItem)
{
  DescRegistry descRegistry;
  descRegistry
    .Register<SimpleItem>("simple_item")
    .AddField("x", &SimpleItem::x)
    .AddField("y", &SimpleItem::y)
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
  ASSERT_EQ(parsedItem->x, 10);
  ASSERT_EQ(parsedItem->y, 20);
}

TEST(DescRegistryTest, ParseComplexItem)
{
  DescRegistry descRegistry;
  descRegistry
    .Register<SimpleItem>("simple_item")
    .AddField("x", &SimpleItem::x)
    .AddField("y", &SimpleItem::y)
    .Build();

  descRegistry
    .Register<ComplexItem>("complex_item")
    .AddField("a", &ComplexItem::a)
    .AddField("b", &ComplexItem::b)
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
  ASSERT_EQ(parsedItem->a.x, 1);
  ASSERT_EQ(parsedItem->a.y, 2);
  ASSERT_EQ(parsedItem->b.x, 3);
  ASSERT_EQ(parsedItem->b.y, 4);
}