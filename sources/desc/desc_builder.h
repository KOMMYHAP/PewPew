#pragma once

#include "desc_registry.h"

template<class DescType>
class DescBuilder
{
public:
  DescBuilder(DescRegistry& registry, std::string_view name);

  template<auto FieldPtr = nullptr>
  DescBuilder& AddField(std::string_view name);

  DescTypeId Build();

private:
  using DescConstructor = std::move_only_function<std::optional<std::any>(std::string& errors, std::vector<DescParsedField> parsedFields)>;

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

#include "desc_builder.hpp"