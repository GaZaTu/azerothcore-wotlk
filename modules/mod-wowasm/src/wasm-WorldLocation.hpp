#pragma once

#include "Position.h"

// WorldLocation is a root type

static float wasm_WorldLocation_get_position_x(const WorldLocation* self) {
  return self->GetPositionX();
}

static float wasm_WorldLocation_get_position_y(const WorldLocation* self) {
  return self->GetPositionY();
}

static float wasm_WorldLocation_get_position_z(const WorldLocation* self) {
  return self->GetPositionZ();
}

static float wasm_WorldLocation_get_orientation(const WorldLocation* self) {
  return self->GetOrientation();
}

static void wasm_WorldLocation_relocate(WorldLocation* self, uint32_t map_id, float x, float y, float z, float o) {
  if (map_id != MAPID_INVALID) {
    self->SetMapId(map_id);
  }

  self->Relocate(x, y, z, o);
}

static uint32_t wasm_WorldLocation_get_map_id(const WorldLocation* self) {
  return self->GetMapId();
}

#define WORLDLOCATION_WASM_EXPORTS\
  WASM_EXPORT(wasm_WorldLocation_get_position_x),\
  WASM_EXPORT(wasm_WorldLocation_get_position_y),\
  WASM_EXPORT(wasm_WorldLocation_get_position_z),\
  WASM_EXPORT(wasm_WorldLocation_get_orientation),\
  WASM_EXPORT(wasm_WorldLocation_relocate),\
  WASM_EXPORT(wasm_WorldLocation_get_map_id)\
