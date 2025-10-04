#include "desc_registry.h"


template<class DescType>
DescTypeId DescRegistry::FindDescTypeId() const
{
  const auto it = _descIndexByGenericType.find(std::type_index(typeid(DescType)));
  if (it == _descIndexByGenericType.end()) {
    return DescTypeId::Invalid;
  }
  return it->second;
}

template<class DescType>
DescTypeId DescRegistry::_RegisterDesc(Desc desc)
{
  return RegisterDesc(typeid(DescType), std::move(desc));
}