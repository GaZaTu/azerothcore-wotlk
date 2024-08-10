#pragma once

#include "Object.h"

// WorldObject inherits from Object, WorldLocation

static uint32_t wasm_WorldObject_get_instance_id(const WorldObject* self) {
  return self->GetInstanceId();
}

static void wasm_WorldObject_set_phase_mask(WorldObject* self, uint32_t new_phase_mask, bool update) {
  return self->SetPhaseMask(new_phase_mask, update);
}

static uint32_t wasm_WorldObject_get_phase_mask(const WorldObject* self) {
  return self->GetPhaseMask();
}

static uint32_t wasm_WorldObject_get_zone_id(const WorldObject* self) {
  return self->GetZoneId();
}

static uint32_t wasm_WorldObject_get_area_id(const WorldObject* self) {
  return self->GetAreaId();
}

static bool wasm_WorldObject_is_outdoors(const WorldObject* self) {
  return self->IsOutdoors();
}

static std::string wasm_WorldObject_get_name(const WorldObject* self) {
  return self->GetName();
}

static void wasm_WorldObject_set_name(WorldObject* self, std::string_view name) {
  return self->SetName((std::string)name);
}

static float wasm_WorldObject_get_distance_to(const WorldObject* self, const WorldObject* other) {
  return self->GetDistance(other);
}

static float wasm_WorldObject_get_distance(const WorldObject* self, float x, float y, float z) {
  return self->GetDistance(x, y, z);
}

static float wasm_WorldObject_get_distance2d_to(const WorldObject* self, const WorldObject* other) {
  return self->GetDistance2d(other);
}

static float wasm_WorldObject_get_distance2d(const WorldObject* self, float x, float y) {
  return self->GetDistance2d(x, y);
}

// TODO: add flags
static bool wasm_WorldObject_is_within_los(const WorldObject* self, float x, float y, float z) {
  return self->IsWithinLOS(x, y, z);
}

// TODO: add flags
static bool wasm_WorldObject_is_within_los_to(const WorldObject* self, const WorldObject* other) {
  return self->IsWithinLOSInMap(other);
}

static bool wasm_WorldObject_is_in_front(const WorldObject* self, const WorldObject* target) {
  return self->isInFront(target);
}

static bool wasm_WorldObject_is_in_back(const WorldObject* self, const WorldObject* target) {
  return self->isInBack(target);
}

static bool wasm_WorldObject_is_in_between(const WorldObject* self, const WorldObject* first, const WorldObject* second) {
  return self->IsInBetween(first, second);
}

// target is nullable
static void wasm_WorldObject_play_distance_sound(WorldObject* self, uint32_t sound_id, Player* target) {
  self->PlayDistanceSound(sound_id, target);
}

// target is nullable
static void wasm_WorldObject_play_direct_sound(WorldObject* self, uint32_t sound_id, Player* target) {
  self->PlayDirectSound(sound_id, target);
}

static void wasm_WorldObject_play_radius_sound(WorldObject* self, uint32_t sound_id, float radius) {
  self->PlayRadiusSound(sound_id, radius);
}

static void wasm_WorldObject_set_map(WorldObject* self, Map* map) {
  self->SetMap(map);
}

static Map* wasm_WorldObject_get_map(const WorldObject* self) {
  return self->FindMap();
}

static TempSummon* wasm_WorldObject_summon_creature(WorldObject* self, uint32_t id, float x, float y, float z, float angle, TempSummonType spawn_type, uint32_t despawn_time, const SummonPropertiesEntry* properties, bool visible_by_summoner_only) {
  return self->SummonCreature(id, x, y, z, angle, spawn_type, despawn_time, properties, visible_by_summoner_only);
}

static GameObject* wasm_WorldObject_summon_gameobject(WorldObject* self, uint32_t id, float x, float y, float z, float angle, uint32_t respawn_time, GOSummonType summon_type) {
  return self->SummonGameObject(id, x, y, z, angle, 0, 0, 0, respawn_time, summon_type);
}

static Creature* wasm_WorldObject_find_nearest_creature(const WorldObject* self, uint32_t entry, float range, bool alive) {
  return self->FindNearestCreature(entry, range, alive);
}

static GameObject* wasm_WorldObject_find_nearest_gameobject(const WorldObject* self, uint32_t entry, float range, bool only_spawned) {
  return self->FindNearestGameObject(entry, range, only_spawned);
}

static Player* wasm_WorldObject_select_nearest_player(const WorldObject* self, float distance) {
  return self->SelectNearestPlayer(distance);
}

#define WORLDOBJECT_WASM_EXPORTS\
  WASM_EXPORT(wasm_WorldObject_get_instance_id),\
  WASM_EXPORT(wasm_WorldObject_set_phase_mask),\
  WASM_EXPORT(wasm_WorldObject_get_phase_mask),\
  WASM_EXPORT(wasm_WorldObject_get_zone_id),\
  WASM_EXPORT(wasm_WorldObject_get_area_id),\
  WASM_EXPORT(wasm_WorldObject_is_outdoors),\
  WASM_EXPORT(wasm_WorldObject_get_name),\
  WASM_EXPORT(wasm_WorldObject_set_name),\
  WASM_EXPORT(wasm_WorldObject_get_distance_to),\
  WASM_EXPORT(wasm_WorldObject_get_distance),\
  WASM_EXPORT(wasm_WorldObject_get_distance2d_to),\
  WASM_EXPORT(wasm_WorldObject_get_distance2d),\
  WASM_EXPORT(wasm_WorldObject_is_within_los),\
  WASM_EXPORT(wasm_WorldObject_is_within_los_to),\
  WASM_EXPORT(wasm_WorldObject_is_in_front),\
  WASM_EXPORT(wasm_WorldObject_is_in_back),\
  WASM_EXPORT(wasm_WorldObject_is_in_between),\
  WASM_EXPORT(wasm_WorldObject_play_distance_sound),\
  WASM_EXPORT(wasm_WorldObject_play_direct_sound),\
  WASM_EXPORT(wasm_WorldObject_play_radius_sound),\
  WASM_EXPORT(wasm_WorldObject_set_map),\
  WASM_EXPORT(wasm_WorldObject_get_map),\
  WASM_EXPORT(wasm_WorldObject_summon_creature),\
  WASM_EXPORT(wasm_WorldObject_summon_gameobject),\
  WASM_EXPORT(wasm_WorldObject_find_nearest_creature),\
  WASM_EXPORT(wasm_WorldObject_find_nearest_gameobject),\
  WASM_EXPORT(wasm_WorldObject_select_nearest_player)\
