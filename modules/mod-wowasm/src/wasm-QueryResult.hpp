#pragma once

#include "QueryResult.h"

// QueryResult is a root type

static bool wasm_QueryResult_next_row(ResultSet* self) {
  return self->NextRow();
}

static uint64_t wasm_QueryResult_get_row_count(ResultSet* self) {
  return self->GetRowCount();
}

static uint32_t wasm_QueryResult_get_field_count(ResultSet* self) {
  return self->GetFieldCount();
}

static std::string wasm_QueryResult_get_field_name(ResultSet* self, uint32_t index) {
  return self->GetFieldName(index);
}

static DatabaseFieldTypes wasm_QueryResult_get_field_type(ResultSet* self, uint32_t index) {
  return self->Fetch()[index].GetType();
}

static std::string wasm_QueryResult_get_string(ResultSet* self, uint32_t index) {
  return self->Fetch()[index].Get<std::string>();
}

static double wasm_QueryResult_get_float(ResultSet* self, uint32_t index) {
  return self->Fetch()[index].Get<double>();
}

static int64_t wasm_QueryResult_get_int64(ResultSet* self, uint32_t index) {
  return self->Fetch()[index].Get<int64_t>();
}

#define QUERYRESULT_WASM_EXPORTS\
  WASM_EXPORT(wasm_QueryResult_next_row),\
  WASM_EXPORT(wasm_QueryResult_get_row_count),\
  WASM_EXPORT(wasm_QueryResult_get_field_count),\
  WASM_EXPORT(wasm_QueryResult_get_field_name),\
  WASM_EXPORT(wasm_QueryResult_get_field_type),\
  WASM_EXPORT(wasm_QueryResult_get_string),\
  WASM_EXPORT(wasm_QueryResult_get_float),\
  WASM_EXPORT(wasm_QueryResult_get_int64)\
