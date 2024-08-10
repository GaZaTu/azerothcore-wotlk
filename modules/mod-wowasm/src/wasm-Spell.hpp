#pragma once

#include "Spell.h"
#include "SpellInfo.h"

// Spell is a root type

static const uint32_t wasm_Spell_get_id(const Spell* self) {
  return self->m_spellInfo->Id;
}

// static void wasm_Spell_(Spell* self) {
//   self->Something();
// }

static uint32_t wasm_SpellInfo_get_id(const SpellInfo *self) {
  return self->Id;
}

static uint32_t wasm_SpellInfo_get_spell_family(const SpellInfo* self) {
  return self->SpellFamilyName;
}

static uint32_t wasm_SpellInfo_get_power_type(const SpellInfo *self) {
  return self->PowerType;
}

static auto wasm_SpellInfo_get_base_level(const SpellInfo *self) {
  return self->BaseLevel;
}

#define SPELL_WASM_EXPORTS\
  WASM_EXPORT(wasm_Spell_get_id)
