#pragma once

#include <string_view>
#include <string>

static_assert(__cpp_lib_generic_unordered_lookup, "Heterogeneous comparison lookup in unordered associative containers is required");

struct string_hash
{
  using hash_type = std::hash<std::string_view>;
  using is_transparent = void;

  std::size_t operator()(const char* str) const { return hash_type{}(str); }

  std::size_t operator()(std::string_view str) const { return hash_type{}(str); }

  std::size_t operator()(const std::string& str) const { return hash_type{}(str); }
};