#pragma once

#include "wasm_export.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <exception>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <array>
#include <random>
#include <iostream>
#include <functional>
#ifdef __cpp_lib_expected
#include <expected>
#endif

#define KILOBYTE(n) ((n) * 1024)
#define MEGABYTE(n) (KILOBYTE(n) * 1024)

namespace wasm {
using module_t = std::shared_ptr<WASMModuleCommon>;
using module_inst_t = std::shared_ptr<WASMModuleInstanceCommon>;
using exec_env_t = std::shared_ptr<WASMExecEnv>;

class heap : public std::vector<uint8_t> {
public:
  explicit heap(size_t heap_size) : vector(heap_size, 0) {}

  inline void reset() {
    std::fill(begin(), end(), 0);
  }
};

class native_module {
public:
  std::string name;
  std::vector<NativeSymbol> symbols;
};

inline auto runtime_init() {
  auto result = wasm_runtime_init();
  return result;
}

inline auto runtime_destroy() {
  wasm_runtime_destroy();
}

inline auto runtime_init_with_heap(const uint8_t* heap, uint32_t heap_size) {
  RuntimeInitArgs init_args;
  std::memset(&init_args, 0, sizeof(RuntimeInitArgs));
  init_args.mem_alloc_type = Alloc_With_Pool;
  init_args.mem_alloc_option.pool.heap_buf = (void*)heap;
  init_args.mem_alloc_option.pool.heap_size = heap_size;

  auto result = wasm_runtime_full_init(&init_args);
  return result;
}

inline auto runtime_init_with_heap(std::span<uint8_t> heap) {
  auto result = runtime_init_with_heap(heap.data(), heap.size());
  return result;
}

inline bool runtime_register_natives(const char* module_name, std::span<NativeSymbol> native_symbols) {
  auto result = wasm_runtime_register_natives_raw(module_name, native_symbols.data(), native_symbols.size());
  return result;
}

inline bool runtime_register_natives(native_module& module) {
  auto result = runtime_register_natives(module.name.data(), module.symbols);
  return result;
}

inline auto runtime_load(const uint8_t* buf, uint32_t buf_size) {
  char error_buf[256];

  auto pointer = wasm_runtime_load((uint8_t*)buf, buf_size, error_buf, sizeof(error_buf));
  if (!pointer) {
    throw std::runtime_error{error_buf};
  }

  auto result = module_t{pointer, &wasm_runtime_unload};
  return result;
}

inline auto runtime_load(std::span<uint8_t> buf) {
  auto result = runtime_load(buf.data(), buf.size());
  return result;
}

inline auto runtime_create_exec_env(module_inst_t instance, uint32_t stack_size = MEGABYTE(1)) {
  auto pointer = wasm_runtime_create_exec_env(instance.get(), stack_size);

  auto result = exec_env_t{pointer, &wasm_runtime_destroy_exec_env};
  return result;
}

namespace _detail {
struct module_inst_data_t {
private:
  wasm_module_inst_t _module_inst;
  std::unordered_map<std::thread::id, exec_env_t> _exec_env;

public:
  explicit inline module_inst_data_t(wasm_module_inst_t module_inst) {
    _module_inst = module_inst;
  }

  static inline auto set_into(wasm_module_inst_t module_inst) {
    wasm_runtime_set_custom_data(module_inst, (void*)new module_inst_data_t(module_inst));
  }

  static inline auto get_from(wasm_module_inst_t module_inst) {
    return (module_inst_data_t*)wasm_runtime_get_custom_data(module_inst);
  }

  static void destructor(wasm_module_inst_t module_inst) {
    delete get_from(module_inst);

    wasm_runtime_deinstantiate(module_inst);
  }

  inline exec_env_t exec_env() {
    auto& result = _exec_env[std::this_thread::get_id()];
    if (result == nullptr) {
      result = runtime_create_exec_env({_module_inst, [](auto) {}});
      wasm_runtime_init_thread_env();
    }

    return result;
  }
};

inline exec_env_t get_exec_env(wasm_module_inst_t module_inst) {
  return module_inst_data_t::get_from(module_inst)->exec_env();
}
}

inline auto runtime_instantiate(module_t module, uint32_t stack_size = MEGABYTE(1), uint32_t heap_size = MEGABYTE(16)) {
  char error_buf[256];

  auto pointer = wasm_runtime_instantiate(module.get(), stack_size, heap_size, error_buf, sizeof(error_buf));
  if (!pointer) {
    throw std::runtime_error{error_buf};
  }

  _detail::module_inst_data_t::set_into(pointer);

  auto result = module_inst_t{pointer, &_detail::module_inst_data_t::destructor};
  return result;
}

inline std::string runtime_get_exception(module_inst_t module) {
  return wasm_runtime_get_exception(module.get());
}

inline uint32_t runtime_get_callstack_size(exec_env_t exec_env) {
  return wasm_runtime_get_call_stack_buf_size(exec_env.get());
}

inline uint32_t runtime_get_callstack(exec_env_t exec_env, char* buf, uint32_t buf_len) {
  return wasm_runtime_dump_call_stack_to_buf(exec_env.get(), buf, buf_len);
}

inline std::string runtime_get_callstack(exec_env_t exec_env) {
  size_t str_size = runtime_get_callstack_size(exec_env) - 1;
  std::string str;
  str.resize(str_size);
  runtime_get_callstack(exec_env, str.data(), (uint32_t)str.size());
  return str;
}

struct unit {
public:
  std::vector<uint8_t> binary;
  module_t module;
  module_inst_t instance;

public:
  inline unit(std::vector<uint8_t>&& _binary, uint32_t heap_size = MEGABYTE(16)) : binary{std::move(_binary)}, module{runtime_load(binary)}, instance{runtime_instantiate(module, heap_size)} {
    // fdm
  }
};

namespace _detail {
inline uint32_t malloc(wasm_module_inst_t module_inst, uint32_t size, void** native_ptr) {
  auto addr = (uint32_t)wasm_runtime_module_malloc(module_inst, size, native_ptr);

  // auto nptr = wasm_runtime_addr_app_to_native(module_inst, addr);
  // printf("wasm_malloc(_, %ld as %d) // size:%d\n", (uint64_t)nptr, addr, size);

  return addr;
}

inline void free(wasm_module_inst_t module_inst, uint32_t addr) {
  // auto nptr = wasm_runtime_addr_app_to_native(module_inst, addr);
  // printf("wasm_free(_, %ld as %d) // str:%s\n", (uint64_t)nptr, addr, (char*)nptr);

  wasm_runtime_module_free(module_inst, (uint64_t)addr);
}
}

struct buffer_t {
private:
  wasm_module_inst_t _instance;

  uint8_t* _pointer_native;

  int64_t _pointer_wasm = 0;

  bool _owned = false;

  inline uint32_t& _addr() const {
    return *&((uint32_t*)&_pointer_wasm)[0];
  }

  inline uint32_t& _size() const {
    return *&((uint32_t*)&_pointer_wasm)[1];
  }

  inline buffer_t(wasm_module_inst_t instance, uint32_t size) : _instance{instance}, _owned{true} {
    _size() = size;
    _addr() = _detail::malloc(instance, size, (void**)&_pointer_native);

    if (!_addr()) {
      throw std::runtime_error{"wasm_runtime_module_malloc(...) failed"};
    }
  }

  inline buffer_t(wasm_module_inst_t instance, int64_t pointer_wasm) : _instance{instance}, _owned{false} {
    _pointer_wasm = pointer_wasm;

    auto valid = wasm_runtime_validate_app_addr(instance, (uint64_t)_addr(), (uint64_t)_size());
    if (valid) {
      _pointer_native = (uint8_t*)wasm_runtime_addr_app_to_native(instance, (uint64_t)_addr());
    } else {
      _pointer_wasm = 0;
    }
  }

public:
  inline buffer_t() {
    // null
  }

  inline buffer_t(const buffer_t&) = default;
  inline buffer_t(buffer_t&& o) {
    *this = std::move(o);
  }
  inline buffer_t& operator=(const buffer_t&) = default;
  inline buffer_t& operator=(buffer_t&& o) {
    _instance = o._instance;
    _pointer_native = o._pointer_native;
    _pointer_wasm = o._pointer_wasm;
    _owned = o._owned;
    o._instance = nullptr;
    o._pointer_native = nullptr;
    o._pointer_wasm = 0;
    o._owned = false;
    return *this;
  }

  static inline auto malloc(wasm_module_inst_t instance, uint32_t size) {
    return buffer_t{instance, size};
  }

  static inline auto malloc(module_inst_t instance, uint32_t size) {
    return buffer_t{instance.get(), size};
  }

  static inline auto malloc(exec_env_t exec_env, uint32_t size) {
    return buffer_t{wasm_runtime_get_module_inst(exec_env.get()), size};
  }

  static inline auto from(wasm_module_inst_t instance, int64_t pointer_wasm) {
    return buffer_t{instance, pointer_wasm};
  }

  inline ~buffer_t() {
    if (_owned && _instance && _pointer_wasm) {
      _detail::free(_instance, _addr());
    }
  }

  inline auto data() {
    return _pointer_native;
  }

  inline const auto data() const {
    return _pointer_native;
  }

  inline auto size() const {
    return _size();
  }

  inline auto wasm() const {
    return _pointer_wasm;
  }

  inline auto wasm_addr() const {
    return _addr();
  }

  inline operator bool() const {
    return _addr();
  }

  inline void set_owned(bool owned) {
    _owned = owned;
  }

  static inline auto malloc_from_string(wasm_module_inst_t instance, std::string_view string) {
    auto buffer = malloc(instance, string.size());
    std::memcpy(buffer.data(), reinterpret_cast<const uint8_t*>(string.data()), string.size());
    return buffer;
  }

  inline auto to_string() const {
    auto string_size = (std::string::size_type)size();
    std::string_view string{reinterpret_cast<const char*>(data()), string_size};
    return string;
  }

  static inline auto malloc_from_span(wasm_module_inst_t instance, std::span<int64_t> span) {
    auto buffer = malloc(instance, span.size());
    std::memcpy(buffer.data(), reinterpret_cast<const uint8_t*>(span.data()), span.size());
    return buffer;
  }

  inline auto to_span() const {
    auto span_size = (std::span<int64_t>::size_type)size() / sizeof(int64_t);
    std::span<int64_t> span{reinterpret_cast<int64_t*>(data()), span_size};
    return span;
  }
};

thread_local static wasm_module_inst_t current_module_inst = nullptr;

namespace _detail {
struct set_global_exec_env {
  wasm_module_inst_t prev_module_inst;

  explicit inline set_global_exec_env(wasm_module_inst_t module_inst) {
    prev_module_inst = current_module_inst;
    current_module_inst = module_inst;
  }

  explicit inline set_global_exec_env(wasm_exec_env_t exec_env) : set_global_exec_env(wasm_runtime_get_module_inst(exec_env)) {}

  inline ~set_global_exec_env() {
    current_module_inst = prev_module_inst;
  }
};
}

template <typename _T, typename = void>
struct from_abi {
  // using R = _T;

  // static inline R from_i64(int64_t ptr) {
  //   return {};
  // }

  // static inline R from_i64_owned(int64_t ptr) {
  //   return {};
  // }
};

template <typename _T, typename = void>
struct into_abi {
  // using A = _T;

  // static inline int64_t into_i64(const A& self, std::vector<buffer_t>& buffers) {
  //   return {};
  // }

  // static inline int64_t into_i64_unown(A& self) {
  //   return {};
  // }
};

template <typename T>
using into_abi_arg_t = typename into_abi<std::remove_reference_t<T>>::A;

template <typename T>
using from_abi_res_t = typename from_abi<std::remove_reference_t<T>>::R;

// template <typename T>
// constexpr static bool is_into_abi_v = typename into_abi<T>::A;

// template <typename T>
// constexpr static bool is_from_abi_v = typename from_abi<T>::R;

template <>
struct from_abi<void> {
  using R = void;
};

template <>
struct into_abi<void> {
  using A = void;
};

template <typename _T>
struct from_abi<_T*> {
  using R = _T*;

  static inline R from_i64(int64_t ptr) {
    return reinterpret_cast<R>(ptr);
  }

  static inline R from_i64_owned(int64_t ptr) {
    return reinterpret_cast<R>(ptr);
  }
};

template <typename _T>
struct into_abi<_T*> {
  using A = _T*;

  static inline int64_t into_i64(const A self, std::vector<buffer_t>& buffers) {
    return reinterpret_cast<int64_t>(self);
  }

  static inline int64_t into_i64_unown(A self) {
    return reinterpret_cast<int64_t>(self);
  }
};

namespace _detail {
template <typename _T>
struct extref_deleter {
  bool enabled = true;
  inline void operator()(_T* self) {
    if (enabled) {
      delete self;
    }
  }
};
}

template <typename _T>
struct from_abi<std::shared_ptr<_T>> {
  using R = std::shared_ptr<_T>;

  static inline R from_i64(int64_t ptr) {
    return {reinterpret_cast<_T*>(ptr), _detail::extref_deleter<_T>{false}};
  }

  static inline R from_i64_owned(int64_t ptr) {
    return {reinterpret_cast<_T*>(ptr), _detail::extref_deleter<_T>{true}};
  }
};

template <typename _T>
struct into_abi<std::shared_ptr<_T>> {
  using A = std::shared_ptr<_T>;

  static inline int64_t into_i64(const A self, std::vector<buffer_t>& buffers) {
    return reinterpret_cast<int64_t>(self.get());
  }

  static inline int64_t into_i64_unown(A self) {
    std::get_deleter<_detail::extref_deleter<_T>>(self)->enabled = false;
    return reinterpret_cast<int64_t>(self.get());
  }
};

template <>
struct from_abi<buffer_t> {
  using R = buffer_t;

  static inline R from_i64(int64_t ptr) {
    auto result = buffer_t::from(current_module_inst, ptr);
    return result;
  }

  static inline R from_i64_owned(int64_t ptr) {
    auto result = from_i64(ptr);
    result.set_owned(true);
    return result;
  }
};

template <>
struct into_abi<buffer_t> {
  using A = buffer_t;

  static inline int64_t into_i64(const A& self, std::vector<buffer_t>& buffers) {
    return self.wasm();
  }

  static inline int64_t into_i64_unown(A& self) {
    self.set_owned(false);
    return self.wasm();
  }
};

template <>
struct from_abi<std::string_view> {
  using R = std::string_view;

  static inline R from_i64(int64_t ptr) {
    auto buffer = from_abi<buffer_t>::from_i64(ptr);
    auto result = buffer.to_string();
    return result;
  }

  // NOTE: ownership does not change
  static inline R from_i64_owned(int64_t ptr) {
    return from_i64(ptr);
  }
};

template <>
struct into_abi<std::string_view> {
  using A = std::string_view;

  static inline int64_t into_i64(const A self, std::vector<buffer_t>& buffers) {
    auto buffer = buffer_t::malloc_from_string(current_module_inst, self);
    auto ptr = buffer.wasm();
    buffers.emplace_back(std::move(buffer));
    return ptr;
  }

  static inline int64_t into_i64_unown(A self) {
    auto buffer = buffer_t::malloc_from_string(current_module_inst, self);
    auto ptr = buffer.wasm();
    buffer.set_owned(false);
    return ptr;
  }
};

template <>
struct from_abi<std::string> {
  using R = std::string;

  static inline R from_i64(int64_t ptr) {
    auto buffer = from_abi<buffer_t>::from_i64(ptr);
    auto result = (std::string)buffer.to_string();
    return result;
  }

  static inline R from_i64_owned(int64_t ptr) {
    auto buffer = from_abi<buffer_t>::from_i64_owned(ptr);
    auto result = (std::string)buffer.to_string();
    return result;
  }
};

template <>
struct into_abi<std::string> {
  using A = std::string_view;

  static inline int64_t into_i64(const A self, std::vector<buffer_t>& buffers) {
    auto buffer = buffer_t::malloc_from_string(current_module_inst, self);
    auto ptr = buffer.wasm();
    buffers.emplace_back(std::move(buffer));
    return ptr;
  }

  static inline int64_t into_i64_unown(A self) {
    auto buffer = buffer_t::malloc_from_string(current_module_inst, self);
    auto ptr = buffer.wasm();
    buffer.set_owned(false);
    return ptr;
  }
};

template <>
struct from_abi<const char*> {
  using R = const char*;

  static inline R from_i64(int64_t ptr) {
    return from_abi<std::string_view>::from_i64(ptr).data();
  }

  // NOTE: ownership does not change
  static inline R from_i64_owned(int64_t ptr) {
    return from_abi<std::string_view>::from_i64_owned(ptr).data();
  }
};

template <>
struct into_abi<const char*> {
  using A = std::string_view;

  static inline int64_t into_i64(const A self, std::vector<buffer_t>& buffers) {
    return into_abi<A>::into_i64(self, buffers);
  }

  static inline int64_t into_i64_unown(A self) {
    return into_abi<A>::into_i64_unown(self);
  }
};

namespace _detail {
template <typename T>
constexpr bool is_lte64bit_v = sizeof(T) <= sizeof(int64_t);
}

template <typename T>
struct from_abi<T, std::enable_if_t<std::is_arithmetic_v<T> && _detail::is_lte64bit_v<T>>> {
  using R = T;

  static inline R from_i64(int64_t ptr) {
    return from_i64_owned(ptr);
  }

  static inline R from_i64_owned(int64_t ptr) {
    using _Re = std::conditional_t<std::is_integral_v<T>, std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>, double>;

    auto re = *reinterpret_cast<_Re*>(&ptr);
    auto ca = static_cast<R>(re);
    return ca;
  }
};

template <typename T>
struct into_abi<T, std::enable_if_t<std::is_arithmetic_v<T> && _detail::is_lte64bit_v<T>>> {
  using A = T;

  static inline int64_t into_i64(A self, std::vector<buffer_t>& buffers) {
    return into_i64_unown(self);
  }

  static inline int64_t into_i64_unown(A self) {
    using _Re = std::conditional_t<std::is_integral_v<T>, std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>, double>;

    auto ca = static_cast<_Re>(self);
    auto re = *reinterpret_cast<int64_t*>(&ca);
    return re;
  }
};

template <typename T>
struct from_abi<T, std::enable_if_t<std::is_enum_v<T>>> {
  using R = T;

  static inline R from_i64(int64_t ptr) {
    return from_i64_owned(ptr);
  }

  static inline R from_i64_owned(int64_t ptr) {
    return static_cast<R>(ptr);
  }
};

template <typename T>
struct into_abi<T, std::enable_if_t<std::is_enum_v<T>>> {
  using A = T;

  static inline int64_t into_i64(A self, std::vector<buffer_t>& buffers) {
    return into_i64_unown(self);
  }

  static inline int64_t into_i64_unown(A self) {
    return static_cast<int64_t>(self);
  }
};

template <typename T>
struct from_abi<std::vector<T>> {
  using R = std::vector<T>;

  static inline R from_i64(int64_t ptr) {
    auto buffer = from_abi<buffer_t>::from_i64(ptr);
    auto span = buffer.to_span();
    R result(span.size());
    for (uint32_t i = 0; i < span.size(); i++) {
      result[i] = from_abi<T>::from_i64(span[i]);
    }
    return result;
  }

  static inline R from_i64_owned(int64_t ptr) {
    auto buffer = from_abi<buffer_t>::from_i64_owned(ptr);
    auto span = buffer.to_span();
    R result(span.size());
    for (uint32_t i = 0; i < span.size(); i++) {
      result[i] = from_abi<T>::from_i64_owned(span[i]);
    }
    return result;
  }
};

template <typename T>
struct into_abi<std::vector<T>> {
  using A = std::span<T>;

  static inline int64_t into_i64(A self, std::vector<buffer_t>& buffers) {
    std::vector<int64_t> converted(self.size());
    for (uint32_t i = 0; i < self.size(); i++) {
      converted[i] = into_abi<T>::into_i64(self[i]);
    }
    auto buffer = buffer_t::malloc_from_span(current_module_inst, converted);
    auto ptr = buffer.wasm();
    buffers.emplace_back(std::move(buffer));
    return ptr;
  }

  static inline int64_t into_i64_unown(A self) {
    std::vector<int64_t> converted(self.size());
    for (uint32_t i = 0; i < self.size(); i++) {
      converted[i] = into_abi<T>::into_i64_unown(self[i]);
    }
    auto buffer = buffer_t::malloc_from_span(current_module_inst, converted);
    auto ptr = buffer.wasm();
    buffer.set_owned(false);
    return ptr;
  }
};

template <typename T>
struct global {
private:
  std::string _name;
  int64_t* _pointer = nullptr;

  inline void init(wasm_module_inst_t module_inst = current_module_inst) {
    if (_pointer) {
      return;
    }

    wasm_global_inst_t global;
    auto success = wasm_runtime_get_export_global_inst(module_inst, _name.data(), &global);
    if (!success) {
      return;
    }

    auto addr = *(int32_t*)global.global_data;

    _pointer = (int64_t*)wasm_runtime_addr_app_to_native(module_inst, addr);

    // if (_pointer != old_pointer) {
    //   printf("'%s' changed from [%ld as %d] to [%ld as %d]\n", _name.data(), (uint64_t)old_pointer, old_addr, (uint64_t)_pointer, _addr);
    // }
  }

public:
  using _Input = std::conditional_t<std::is_same_v<T, std::string>, std::string_view, const T&>;
  using _Delete = std::conditional_t<std::is_same_v<T, std::string>, buffer_t, T>;
  using _Output = std::conditional_t<std::is_same_v<T, std::string>, std::string_view, T>;

  explicit inline global(module_inst_t module, const char* name) : _name{name} {
    init(module.get());
  }

  explicit inline global(unit& file, const char* name) : global(file.instance, name) {
    // fdm
  }

  explicit inline global(const char* name) {
    _name = name;
  }

  inline operator bool() {
    init();

    return _pointer && *_pointer;
  }

  inline std::string_view name() const {
    return _name;
  }

  inline auto set(_Input value) {
    init();

    del();

    *_pointer = into_abi<T>::into_i64_unown(value);
  }

  inline auto del() {
    init();

    if (*_pointer) {
      from_abi<_Delete>::from_i64_owned(*_pointer);
    }

    *_pointer = 0;
  }

  inline auto get() {
    init();

    auto result = from_abi<_Output>::from_i64(*_pointer);
    return result;
  }
};

namespace _detail {
// thread_local static global<std::string> current_error{"WASM_ERROR"};
}

namespace _detail {
template <typename _T>
inline void set_wasmarg_at(int64_t* wasm_args, uint32_t i, _T&& arg, std::vector<buffer_t>& buffers) {
  using T = std::remove_reference_t<_T>;
  wasm_args[i] = into_abi<T>::into_i64(arg, buffers);
}

template <typename T>
inline T get_wasmret_at(int64_t* wasm_results, uint32_t i) {
  return from_abi<T>::from_i64_owned(wasm_results[i]);
}
}

struct function_ptr {
  module_inst_t instance;
  wasm_function_inst_t func = nullptr;
  uint32_t func_index = 0;

  explicit inline function_ptr(module_inst_t instance, const char *name) {
    this->instance = instance;
    this->func = wasm_runtime_lookup_function(instance.get(), name);
  }

  explicit inline function_ptr(module_inst_t instance, uint32_t func_index) {
    this->instance = instance;
    this->func_index = func_index;
  }
};

template <typename _Signature>
struct function {};

template <typename _Res, typename... _ArgTypes>
struct function<_Res(_ArgTypes...)> {
private:
  module_inst_t _instance;
  wasm_function_inst_t _func = nullptr;
  uint32_t _func_index = 0;

public:
  constexpr static uint32_t num_results = std::is_same_v<_Res, void> ? 0 : 1;
  constexpr static uint32_t num_args = sizeof...(_ArgTypes);

  explicit inline function(const function_ptr &ptr) {
    _instance = ptr.instance;
    _func = ptr.func;
    _func_index = ptr.func_index;
  }

  explicit inline function(module_inst_t instance, const char *name) {
    _instance = instance;
    _func = wasm_runtime_lookup_function(instance.get(), name);

    if (_func) {
      auto func_retc = wasm_func_get_result_count(_func, _instance.get());
      if (func_retc != num_results) {
        throw std::runtime_error{std::string{"invalid result count "} + std::to_string(num_results) + ", must be " + std::to_string(func_retc)};
      }

      auto func_argc = wasm_func_get_param_count(_func, _instance.get());
      if (func_argc != num_args) {
        throw std::runtime_error{std::string{"invalid argument count "} + std::to_string(num_args) + ", must be " + std::to_string(func_argc)};
      }
    }
  }

  explicit inline function(unit& file, const char* name) : function(file.instance, name) {
    // fdm
  }

  explicit inline function(module_inst_t instance, uint32_t index) {
    _instance = instance;
    _func_index = index;
  }

  explicit inline function(unit& file, uint32_t index) : function(file.instance, index) {
    // fdm
  }

  inline operator bool() const {
    return _func;
  }

  from_abi_res_t<_Res> operator()(into_abi_arg_t<_ArgTypes>... args) {
    _detail::set_global_exec_env _exec_env_lock{_instance.get()};

    auto current_exec_env = _detail::get_exec_env(current_module_inst).get();

    constexpr uint32_t args_array_len = std::max(num_args, num_results);
    int64_t args_array[args_array_len];

    std::vector<buffer_t> tmp_buffers;

    int i = 0;
    (_detail::set_wasmarg_at(args_array, i++, args, tmp_buffers), ...);

    bool success = false;
    if (_func) {
      success = wasm_runtime_call_wasm(current_exec_env, _func, args_array_len * 2, reinterpret_cast<uint32_t*>(args_array));
    } else {
      success = wasm_runtime_call_indirect(current_exec_env, _func_index, args_array_len * 2, reinterpret_cast<uint32_t*>(args_array));
    }

    if (!success) {
      constexpr std::string_view undefined_element_exception = "Exception: undefined element";

      std::string error = wasm_runtime_get_exception(current_module_inst);
      wasm_runtime_set_exception(current_module_inst, nullptr);

      if (error == undefined_element_exception) {
        // the runtime wrongly reports that the indirect_call does not exist but it still executed it beforehand
        // which is unfortunate because now there's no way to determine if it actually failed in this case
      } else {
        throw std::runtime_error{error};
      }
    }

    // if (_detail::current_error) {
    //   auto e = (std::string)_detail::current_error.get();
    //   _detail::current_error.del();
    //   throw std::runtime_error{e.data()};
    // }

    if constexpr (std::is_same_v<_Res, void>) {
      // void
    } else {
      auto result = _detail::get_wasmret_at<_Res>(args_array, 0);
      return result;
    }
  }
};

namespace _detail {
template <const std::string_view&... Strs>
struct join {
  static constexpr auto impl() noexcept {
    constexpr std::size_t len = (Strs.size() + ... + 0);
    std::array<char, len + 1> arr{};
    auto append = [i = 0, &arr](auto const& s) mutable {
      for (auto c : s)
        arr[i++] = c;
    };
    (append(Strs), ...);
    arr[len] = 0;
    return arr;
  }
  static constexpr auto arr = impl();
  static constexpr std::string_view value{arr.data(), arr.size() - 1};
};

template <const std::string_view&... Strs>
static constexpr auto join_v = join<Strs...>::value;

static constexpr std::string_view str_void{""};
static constexpr std::string_view str_i64{"I"};
static constexpr std::string_view str_opn{"("};
static constexpr std::string_view str_cls{")"};

template <typename T>
struct type_to_string {
  static constexpr auto value = str_i64;
};

template <>
struct type_to_string<void> {
  static constexpr auto value = str_void;
};

template <typename _Underlying>
struct type_to_string<_Underlying*> {
  static constexpr auto value = str_i64;
};

template <typename... _Elements>
struct type_to_string<std::tuple<_Elements...>> {
  static constexpr auto value = join_v<type_to_string<std::decay_t<_Elements>>::value...>;
};

template <typename T>
static constexpr std::string_view type_to_string_v = type_to_string<std::decay_t<T>>::value;

template <typename _Signature>
struct function_decltype {};

template <typename _Res, typename... _ArgTypes>
struct function_decltype<_Res(_ArgTypes...)> {
  static constexpr std::string_view create() {
    return join_v<str_opn, type_to_string_v<_ArgTypes>..., str_cls, type_to_string_v<_Res>>;
  }
};

template <typename F>
static constexpr const char* function_decltype_v = _detail::function_decltype<F>::create().data();

template <std::size_t I = 0, typename... _Elements>
inline void fill_nativearg_tuple(std::tuple<_Elements...>& args_tuple, int64_t* args) {
  if constexpr (I == sizeof...(_Elements)) {
    return;
  } else {
    using T = std::tuple_element_t<I, std::tuple<_Elements...>>;
    std::get<I>(args_tuple) = from_abi<T>::from_i64(args[I]);

    fill_nativearg_tuple<I + 1, _Elements...>(args_tuple, args);
  }
}

template <typename _T>
inline void set_nativeret_at(int64_t* wasm_results, uint32_t i, _T&& arg) {
  using T = std::remove_reference_t<_T>;
  wasm_results[i] = into_abi<T>::into_i64_unown(arg);
}

template <typename _Signature>
using function_pointer_t = _Signature*;

template <typename _Signature, function_pointer_t<_Signature> _FuncPtr>
struct function_wrapper {};

template <typename _Res, typename... _ArgTypes, function_pointer_t<_Res(_ArgTypes...)> _FuncPtr>
struct function_wrapper<_Res(_ArgTypes...), _FuncPtr> {
  static void execute(wasm_exec_env_t exec_env, int64_t* args) noexcept {
    _detail::set_global_exec_env _exec_env_lock{exec_env};

    std::tuple<std::remove_reference_t<_ArgTypes>...> args_tuple;
    fill_nativearg_tuple(args_tuple, args);

    try {
      if constexpr (std::is_same_v<_Res, void>) {
        std::apply(_FuncPtr, args_tuple);
      } else {
        _Res result = std::apply(_FuncPtr, args_tuple);
        set_nativeret_at(args, 0, result);
      }
    } catch (const std::exception &err) {
      wasm_runtime_set_exception(current_module_inst, err.what());

      // _detail::current_error.set(err.what());

      if constexpr (std::is_same_v<_Res, void>) {
        // do nothing
      } else {
        args[0] = 0;
      }
    }
  }
};
}

template <typename T>
inline T* get_attachment() {
  return static_cast<T*>(wasm_runtime_get_function_attachment(_detail::get_exec_env(current_module_inst).get()));
}

struct callback_ptr {
  wasm_module_inst_t instance;
  uint32_t func_idx = 0;
  uint32_t func_ctx = 0;
  uint32_t func_ctx_ctor = 0;

  inline callback_ptr() {}
  inline callback_ptr(wasm_module_inst_t inst, uint32_t idx, uint32_t ctx, uint32_t ctor = 0) {
    instance = inst;
    func_idx = idx;
    func_ctx = ctx;
    func_ctx_ctor = ctor;
  }

  inline callback_ptr(const callback_ptr&) = delete;
  inline callback_ptr& operator=(const callback_ptr&) = delete;

  inline callback_ptr(callback_ptr&& o) {
    *this = std::move(o);
  };
  inline callback_ptr &operator=(callback_ptr &&o) {
    instance = o.instance;
    func_idx = o.func_idx;
    func_ctx = o.func_ctx;
    func_ctx_ctor = o.func_ctx_ctor;

    o.instance = nullptr;
    o.func_idx = 0;
    o.func_ctx = 0;
    o.func_ctx_ctor = 0;

    return *this;
  }

  inline ~callback_ptr() {
    if (func_ctx_ctor) {
      wasm::function<void(uint32_t)> ctor{{instance, [](auto) {}}, func_ctx_ctor};
      ctor(func_ctx);
    }
  }
};

#define WASM_CALLBACK_PTR_ARGS                                                 \
  uint32_t func_idx, uint32_t func_ctx, uint32_t func_ctx_ctor

#define WASM_CALLBACK_PTR_FORWARD_ARGS                                         \
  func_idx, func_ctx, func_ctx_ctor

#define WASM_CALLBACK_PTR_CONSTRUCT(...)                                       \
  wasm::callback_ptr __VA_ARGS__ {                                             \
    wasm::current_module_inst, WASM_CALLBACK_PTR_FORWARD_ARGS                  \
  }

#define WASM_CALLBACK_PTR_MAKE_SHARED()                                        \
  std::make_shared<wasm::callback_ptr>(wasm::current_module_inst, WASM_CALLBACK_PTR_FORWARD_ARGS)

#define WASM_CALLBACK_PTR_AS_FUNCTION_ARGS(VAR)                                \
  wasm::module_inst_t{(VAR).instance, [](auto) {}}, (VAR).func_idx

using callback_map = std::unordered_multimap<std::string, callback_ptr>;

template <typename _Sig>
struct callback;

template <typename _Ret, typename... _Args>
struct callback<_Ret(_Args...)> {
  const callback_map& map;
  std::string name;

  constexpr static bool returns_null_v = std::is_same_v<_Ret, void>;
  using _RetOrNull = std::conditional_t<returns_null_v, std::nullopt_t, wasm::from_abi_res_t<_Ret>>;

  std::vector<_RetOrNull> operator()(wasm::into_abi_arg_t<_Args>... args) {
    std::vector<_RetOrNull> result;
    auto [start, end] = map.equal_range(name);
    for (auto& it = start; start != end; ++start) {
      auto& ptr = it->second;
      wasm::function<_Ret(uint32_t, _Args...)> cb{{ptr.instance, [](auto) {}}, ptr.func_idx};
      if constexpr (returns_null_v) {
        cb(ptr.func_ctx, args...);
        result.emplace_back(std::nullopt);
      } else {
        result.emplace_back(cb(ptr.func_ctx, args...));
      }
    }
    return result;
  }
};

template <typename _Sig>
struct multi_function;

template <typename _Ret, typename... _Args>
struct multi_function<_Ret(_Args...)> {
  const std::vector<wasm::function_ptr>& functions;

  constexpr static bool returns_null_v = std::is_same_v<_Ret, void>;
  using _RetOrNull = std::conditional_t<returns_null_v, std::nullopt_t, wasm::from_abi_res_t<_Ret>>;

  inline std::vector<_RetOrNull> operator()(wasm::into_abi_arg_t<_Args>... args) {
    std::vector<_RetOrNull> result;
    for (const auto &funcptr : functions) {
      wasm::function<_Ret(_Args...)> cb{funcptr};
      if constexpr (returns_null_v) {
        cb(args...);
        result.emplace_back(std::nullopt);
      } else {
        result.emplace_back(cb(args...));
      }
    }
    return result;
  }

  inline bool empty() const {
    return functions.empty();
  }
};
} // namespace wasm

#define _WASM_EXPORT_API_WITH_NAME_SIG_ATT(NAME, FUNC, ATT) \
  { NAME, reinterpret_cast<void*>(&wasm::_detail::function_wrapper<decltype(FUNC), &FUNC>::execute), wasm::_detail::function_decltype_v<decltype(FUNC)>, ATT }
#define _WASM_EXPORT_API_WITH_SIG_ATT(FUNC, ATT) \
  _WASM_EXPORT_API_WITH_NAME_SIG_ATT(#FUNC, FUNC, ATT)
#define _WASM_EXPORT_API_WITH_SIG(FUNC) \
  _WASM_EXPORT_API_WITH_SIG_ATT(FUNC, nullptr)

#define _WASM_GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define WASM_EXPORT(...) _WASM_GET_MACRO(__VA_ARGS__, _WASM_EXPORT_API_WITH_NAME_SIG_ATT, _WASM_EXPORT_API_WITH_SIG_ATT, _WASM_EXPORT_API_WITH_SIG)(__VA_ARGS__)

namespace wasm::_detail {
static void wasm_malloc(wasm_exec_env_t exec_env, uint32_t* args) {
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  args[0] = wasm::_detail::malloc(module_inst, args[0], nullptr);
}

static void wasm_free(wasm_exec_env_t exec_env, uint32_t* args) {
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  wasm::_detail::free(module_inst, args[0]);
}

static uint64_t wasm_random_u64() {
  std::random_device rd;

  uint64_t result;
  ((uint32_t*)&result)[0] = rd();
  ((uint32_t*)&result)[1] = rd();
  return result;
}

static void wasm_print(std::string_view string) {
  std::cout << string;
}

static std::string wasm_get_exception() {
  std::string result = wasm_runtime_get_exception(current_module_inst);
  wasm_runtime_set_exception(current_module_inst, nullptr);
  return result;
}

static void wasm_set_exception(std::string& string) {
  wasm_runtime_set_exception(current_module_inst, string.data());
}
}

#define WASM_EXPORT_CORE                                                           \
  { "wasm_malloc", (void*)&wasm::_detail::wasm_malloc, "(i)i", nullptr },          \
  { "wasm_free", (void*)&wasm::_detail::wasm_free, "(i)", nullptr },               \
  WASM_EXPORT("wasm_random_u64", wasm::_detail::wasm_random_u64, nullptr),         \
  WASM_EXPORT("wasm_print", wasm::_detail::wasm_print, nullptr),                   \
  WASM_EXPORT("wasm_get_exception", wasm::_detail::wasm_get_exception, nullptr),   \
  WASM_EXPORT("wasm_set_exception", wasm::_detail::wasm_set_exception, nullptr)
