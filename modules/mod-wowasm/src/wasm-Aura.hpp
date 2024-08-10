#pragma once

#include "Define.h" // required because SpellAuras.h is donk
#include "SpellAuras.h"

static uint32_t wasm_Aura_get_id(const Aura* self) {
  return self->GetId();
}

static ObjectGuid wasm_Aura_get_caster_guid(const Aura* self) {
  return self->GetCasterGUID();
}

static Unit* wasm_Aura_get_caster(const Aura* self) {
  return self->GetCaster();
}

static WorldObject* wasm_Aura_get_owner(const Aura* self) {
  return self->GetOwner();
}

static void wasm_Aura_remove(Aura* self) {
  self->Remove();
}

static int32_t wasm_Aura_get_max_duration(const Aura* self) {
  return self->GetMaxDuration();
}

static void wasm_Aura_set_max_duration(Aura* self, int32_t duration) {
  self->SetMaxDuration(duration);
}

static int32_t wasm_Aura_get_duration(const Aura* self) {
  return self->GetDuration();
}

static void wasm_Aura_set_duration(Aura* self, int32_t duration) {
  self->SetDuration(duration);
}

static uint8_t wasm_Aura_get_stack_amount(const Aura* self) {
  return self->GetStackAmount();
}

static void wasm_Aura_set_stack_amount(Aura* self, uint8_t amount) {
  self->SetStackAmount(amount);
}

#define AURA_WASM_EXPORTS\
  WASM_EXPORT(wasm_Aura_get_id),\
  WASM_EXPORT(wasm_Aura_get_caster_guid),\
  WASM_EXPORT(wasm_Aura_get_caster),\
  WASM_EXPORT(wasm_Aura_get_owner),\
  WASM_EXPORT(wasm_Aura_remove),\
  WASM_EXPORT(wasm_Aura_get_max_duration),\
  WASM_EXPORT(wasm_Aura_set_max_duration),\
  WASM_EXPORT(wasm_Aura_get_duration),\
  WASM_EXPORT(wasm_Aura_set_duration),\
  WASM_EXPORT(wasm_Aura_get_stack_amount),\
  WASM_EXPORT(wasm_Aura_set_stack_amount)\
