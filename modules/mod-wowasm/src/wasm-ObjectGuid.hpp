#pragma once

#include "ObjectGuid.h"
#include "wasm-host.hpp"

namespace wasm {
template <>
struct from_abi<ObjectGuid> {
  using R = ObjectGuid;

  static inline R from_i64(int64_t ptr) {
    return from_i64_owned(ptr);
  }

  static inline R from_i64_owned(int64_t ptr) {
    return R{from_abi<uint64_t>::from_i64_owned(ptr)};
  }
};

template <>
struct into_abi<ObjectGuid> {
  using A = ObjectGuid;

  static inline int64_t into_i64(A self, std::vector<buffer_t>& buffers) {
    return into_i64_unown(self);
  }

  static inline int64_t into_i64_unown(A self) {
    return into_abi<uint64_t>::into_i64_unown(self.GetRawValue());
  }
};
}
