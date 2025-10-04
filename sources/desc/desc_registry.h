#pragma once

#include "desc.h"

#include "string_hash.h"

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
  using Unpack = std::move_only_function<std::optional<std::any>(std::string&, std::vector<std::any>)>;

  std::string name;
  std::vector<DescField> fields;
  mutable Constructor constructor;
  mutable Unpack unpack;
};

class DescRegistry
{
public:
  DescRegistry();

  template<class DescType>
  DescTypeId _RegisterDesc(Desc desc);

  DescTypeId RegisterDesc(std::type_index descType, Desc desc);

  template<class DescType>
  DescTypeId FindDescTypeId() const;
  DescTypeId FindDescTypeId(std::string_view name) const;
  std::string_view GetDescTypeName(DescTypeId typeId) const;
  const Desc* FindDesc(DescTypeId typeId) const;

  DescFieldId FindDescFieldId(std::string_view name) const;
  std::string_view GetDescFieldName(DescFieldId fieldId) const;

private:
  friend class DescParser;
  template<class DescType>
  friend class DescBuilder;


  DescFieldId _GetOrRegisterField(std::string_view field);
  void _LogError(std::string error) const;

private:
  std::vector<std::string> _fieldNames;
  std::unordered_map<std::string_view, DescFieldId> _fieldIndexByName;

  std::vector<Desc> _descList;
  std::unordered_map<std::type_index, DescTypeId> _descIndexByGenericType;
  std::unordered_map<std::string, DescTypeId, string_hash, std::equal_to<>> _descIndexByName;
};

#include "desc_registry.hpp"