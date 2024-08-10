#pragma once

#include "Creature.h"

// Creature inherits from Unit

static const CreatureTemplate* wasm_Creature_get_creature_template(const Creature* self) {
  return self->GetCreatureTemplate();
}

static const CreatureData* wasm_Creature_get_creature_data(const Creature* self) {
  return self->GetCreatureData();
}

static bool wasm_Creature_is_tapped_by(const Creature* self, const Player* player) {
  return self->isTappedBy(player);
}

static void wasm_Creature_do_flee_to_get_assistance(Creature* self) {
  self->DoFleeToGetAssistance();
}

// static void wasm_Creature_(Creature* self) {
//   self->Something();
// }

static const uint32_t wasm_Creature_get_creature_template_entry(const Creature* self) {
  return self->GetCreatureTemplate()->Entry;
}

#define CREATURE_WASM_EXPORTS\
  WASM_EXPORT(wasm_Creature_get_creature_template),\
  WASM_EXPORT(wasm_Creature_get_creature_data),\
  WASM_EXPORT(wasm_Creature_is_tapped_by),\
  WASM_EXPORT(wasm_Creature_do_flee_to_get_assistance),\
  WASM_EXPORT(wasm_Creature_get_creature_template_entry)\
