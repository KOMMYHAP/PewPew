#pragma once
#include "desc_builder.h"


template<class DescType>
DescBuilder<DescType>::DescBuilder(DescRegistry& registry, std::string_view name)
  : _registry(&registry)
  , _name(name)
{
}


template<class DescType>
template<auto FieldPtr>
DescBuilder<DescType>& DescBuilder<DescType>::AddField(std::string_view name)
{
  static_assert(!std::is_fundamental_v<DescType>);
  using FieldType = std::decay_t<decltype(std::declval<DescType>().*FieldPtr)>;

  const DescFieldId fieldId = _registry->_GetOrRegisterField(name);
  const DescTypeId fieldTypeId = _registry->FindDescTypeId<FieldType>();
  _fields.emplace_back(fieldId, fieldTypeId);
  _fieldConstructors.emplace_back(fieldId, [](std::string& errors, std::any fieldObject, std::any& object) {
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
    desc->*FieldPtr = std::move(*field);
    return true;
  });
  return *this;
}


template<class DescType>
DescTypeId DescBuilder<DescType>::Build()
{
  Desc::Constructor descInstanceConstructor;
  if (!_fieldConstructors.empty()) {
    descInstanceConstructor = [fieldConstructors = std::move(_fieldConstructors), registry = _registry.get()](std::string& errors, std::vector<ParsedDescField> parsedFields) mutable -> std::optional<std::any> {
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
  }

  Desc::Unpack unpack = [](std::string& errors, std::vector<std::any> values) {
    std::vector<DescType> unpackedValues;
    unpackedValues.reserve(values.size());
    for (std::any& value : values) {
      DescType* unpackedValue = std::any_cast<DescType>(&value);
      if (!unpackedValue) {
        errors += std::format("Mismatched type of array[{}]: expected {}, but actual {}!", unpackedValues.size(), typeid(DescType).name(), value.type().name());
        unpackedValues.emplace_back();
        continue;
      }

      unpackedValues.push_back(std::move(*unpackedValue));
    }
    return std::any(std::move(unpackedValues));
  };


  Desc desc{
    std::move(_name),
    std::move(_fields),
    std::move(descInstanceConstructor),
    std::move(unpack)
  };
  const DescTypeId typeId = _registry->_RegisterDesc<DescType>(std::move(desc));
  if (typeId == DescTypeId::Invalid) {
    _registry->_LogError(std::format("DescType {} has been already registered!", typeid(DescType).name()));
  }
  return typeId;
}
