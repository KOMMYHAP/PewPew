#include "desc_registry.h"
#include "desc_builder.h"

namespace {

struct DescRegistryTagArray
{};

struct DescRegistryTagObject
{};

}

Desc::Desc(std::string name, std::vector<DescField> fields, Constructor constructor, Unpacker unpack)
  : _name(std::move(name))
  , _fields(std::move(fields))
  , _constructor(std::move(constructor))
  , _unpack(std::move(unpack))
{
}

std::optional<std::any> Desc::Construct(std::string& errors, std::vector<DescParsedField> parsedFields) const
{
  if (!_constructor) {
    errors += std::format("Constructor of desc {} is undefined!", _name);
    return std::nullopt;
  }

  return _constructor(errors, std::move(parsedFields));
}

std::optional<std::any> Desc::UnpackValuesArray(std::string& errors, std::vector<std::any> packedValues) const
{
  if (!_unpack) {
    errors += std::format("Unpacker of desc {} is undefined!", _name);
    return std::nullopt;
  }

  return _unpack(errors, std::move(packedValues));
}

DescRegistry::DescRegistry()
{
  DescBuilder<int32_t>(*this, "int32").Build();
  DescBuilder<std::string>(*this, "string").Build();
  DescBuilder<DescRegistryTagArray>(*this, "array").Build();
  DescBuilder<DescRegistryTagObject>(*this, "object").Build();
}

DescTypeId DescRegistry::RegisterDesc(std::type_index descType, Desc desc)
{
  if (_descIndexByGenericType.contains(descType)) {
    return DescTypeId::Invalid;
  }

  const DescTypeId id{ static_cast<std::underlying_type_t<DescTypeId>>(_descList.size()) };
  const std::string nameView{desc.GetName()}; //< store name before desc moving
  _descList.push_back(std::move(desc));
  _descIndexByGenericType[descType] = id;
  _descIndexByName[nameView] = id;
  return id;
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
  return desc->GetName();
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
  std::fflush(stdout);
}
