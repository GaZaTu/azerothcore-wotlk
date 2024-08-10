#pragma once

#include "Object.h"

// Object is a root type

// ^static (const )?[\w:*<>]+ (wasm_\w+)[^}]+}\n

static void wasm_Object_free(Object* self) {
  delete self;
}

static bool wasm_Object_is_in_world(const Object* self) {
  return self->IsInWorld();
}

static void wasm_Object_add_to_world(Object* self) {
  self->AddToWorld();
}

static void wasm_Object_remove_from_world(Object* self) {
  self->RemoveFromWorld();
}

static ObjectGuid wasm_Object_get_guid(const Object* self) {
  return self->GetGUID();
}

static uint32_t wasm_Object_get_entry(const Object* self) {
  return self->GetEntry();
}

static void wasm_Object_set_entry(Object* self, uint32_t entry) {
  self->SetEntry(entry);
}

static float wasm_Object_get_scale(const Object* self) {
  return self->GetObjectScale();
}

static void wasm_Object_set_scale(Object* self, float scale) {
  self->SetObjectScale(scale);
}

static TypeID wasm_Object_get_type_id(const Object* self) {
  return self->GetTypeId();
}

static bool wasm_Object_has_quest(const Object* self, uint32_t quest_id) {
  return self->hasQuest(quest_id);
}

static bool wasm_Object_has_involved_quest(const Object* self, uint32_t quest_id) {
  return self->hasInvolvedQuest(quest_id);
}

#define OBJECT_WASM_EXPORTS\
  WASM_EXPORT(wasm_Object_free),\
  WASM_EXPORT(wasm_Object_is_in_world),\
  WASM_EXPORT(wasm_Object_add_to_world),\
  WASM_EXPORT(wasm_Object_remove_from_world),\
  WASM_EXPORT(wasm_Object_get_guid),\
  WASM_EXPORT(wasm_Object_get_entry),\
  WASM_EXPORT(wasm_Object_set_entry),\
  WASM_EXPORT(wasm_Object_get_scale),\
  WASM_EXPORT(wasm_Object_set_scale),\
  WASM_EXPORT(wasm_Object_get_type_id),\
  WASM_EXPORT(wasm_Object_has_quest),\
  WASM_EXPORT(wasm_Object_has_involved_quest)\
