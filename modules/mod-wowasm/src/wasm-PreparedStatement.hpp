#pragma once

// PreparedStatement is a root type

#include "PreparedStatement.h"

static void wasm_PreparedStatement_set_string(PreparedStatementBase* self, uint8_t index, std::string_view value) {
  self->SetData(index, value);
}

static void wasm_PreparedStatement_set_float(PreparedStatementBase* self, uint8_t index, double value) {
  self->SetData(index, value);
}

static void wasm_PreparedStatement_set_int64(PreparedStatementBase* self, uint8_t index, int64_t value) {
  self->SetData(index, value);
}

static void wasm_PreparedStatement_set_null(PreparedStatementBase* self, uint8_t index) {
  self->SetData(index, nullptr);
}

#define PREPAREDSTATEMENT_WASM_EXPORTS\
  WASM_EXPORT(wasm_PreparedStatement_set_string),\
  WASM_EXPORT(wasm_PreparedStatement_set_float),\
  WASM_EXPORT(wasm_PreparedStatement_set_int64),\
  WASM_EXPORT(wasm_PreparedStatement_set_null)\
