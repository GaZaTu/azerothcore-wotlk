#pragma once

#include "Unit.h"

// Unit inherits from WorldObject

static bool wasm_Unit_is_within_combat_range(const Unit* self, const Unit* other, float dist2compare) {
  return self->IsWithinCombatRange(other, dist2compare);
}

static bool wasm_Unit_is_within_melee_range(const Unit* self, const Unit* other, float dist) {
  return self->IsWithinMeleeRange(other, dist);
}

static float wasm_Unit_get_melee_range(const Unit* self, const Unit* target) {
  return self->GetMeleeRange(target);
}

static bool wasm_Unit_attack(Unit* self, Unit* victim, bool melee) {
  return self->Attack(victim, melee);
}

// if_not_spell_id is nullable
static void wasm_Unit_cast_stop(Unit* self, uint32_t if_not_spell_id) {
  self->CastStop(if_not_spell_id);
}

static bool wasm_Unit_attack_stop(Unit* self) {
  return self->AttackStop();
}

static void wasm_Unit_remove_all_attackers(Unit* self) {
  self->RemoveAllAttackers();
}

static std::vector<Unit*> wasm_Unit_get_attackers(const Unit* self) {
  std::vector<Unit*> result;
  for (auto v : self->getAttackers()) {
    result.push_back(v);
  }
  return result;
}

static Unit* wasm_Unit_get_victim(const Unit* self) {
  return self->GetVictim();
}

static void wasm_Unit_combat_stop(Unit* self) {
  self->CombatStop();
}

static void wasm_Unit_combat_stop_with_pets(Unit* self) {
  self->CombatStopWithPets();
}

static void wasm_Unit_stop_attack_faction(Unit* self, uint32_t faction_id) {
  self->StopAttackFaction(faction_id);
}

static void wasm_Unit_stop_attacking_invalid_target(Unit* self) {
  self->StopAttackingInvalidTarget();
}

// exclude is nullable
static Unit* wasm_Unit_select_nearby_target(const Unit* self, Unit* exclude, float dist) {
  return self->SelectNearbyTarget(exclude, dist);
}

// exclude is nullable
static Unit* wasm_Unit_select_nearby_no_totem_target(const Unit* self, Unit* exclude, float dist) {
  return self->SelectNearbyNoTotemTarget(exclude, dist);
}

static void wasm_Unit_add_unit_state(Unit* self, uint32_t f) {
  self->AddUnitState(f);
}

static bool wasm_Unit_has_unit_state(const Unit* self, uint32_t f) {
  return self->HasUnitState(f);
}

static void wasm_Unit_clear_unit_state(Unit* self, uint32_t f) {
  self->ClearUnitState(f);
}

static uint32_t wasm_Unit_get_unit_state(const Unit *self) {
  return self->GetUnitState();
}

static bool wasm_Unit_can_free_move(const Unit* self) {
  return self->CanFreeMove();
}

static bool wasm_Unit_has_unit_type_mask(const Unit* self, uint32_t mask) {
  return self->HasUnitTypeMask(mask);
}

static uint32_t wasm_Unit_get_unit_type_mask(const Unit* self) {
  return self->GetUnitTypeMask();
}

static uint8_t wasm_Unit_get_level(const Unit* self) {
  return self->GetLevel();
}

static void wasm_Unit_set_level(Unit* self, uint8_t level) {
  self->SetLevel(level);
}

static uint8_t wasm_Unit_get_race(const Unit* self) {
  return self->getRace();
}

static void wasm_Unit_set_race(Unit* self, uint8_t race) {
  self->setRace(race);
}

static uint32_t wasm_Unit_get_race_mask(const Unit* self) {
  return self->getRaceMask();
}

static uint8_t wasm_Unit_get_class(const Unit* self) {
  return self->getClass();
}

static uint32_t wasm_Unit_get_class_mask(const Unit* self) {
  return self->getClassMask();
}

static uint8_t wasm_Unit_get_gender(const Unit* self) {
  return self->getGender();
}

static float wasm_Unit_get_stat(const Unit* self, Stats stat) {
  return self->GetStat(stat);
}

static void wasm_Unit_set_stat(Unit* self, Stats stat, int32_t value) {
  self->SetStat(stat, value);
}

static uint32_t wasm_Unit_get_armor(const Unit* self) {
  return self->GetArmor();
}

static void wasm_Unit_set_armor(Unit* self, int32_t value) {
  self->SetArmor(value);
}

static uint32_t wasm_Unit_get_resistance_for_school(const Unit* self, SpellSchools school) {
  return self->GetResistance(school);
}

static uint32_t wasm_Unit_get_resistance_for_school_mask(const Unit* self, SpellSchoolMask school_mask) {
  return self->GetResistance(school_mask);
}

static void wasm_Unit_set_resistance(Unit* self, SpellSchools school, int32_t value) {
  self->SetResistance(school, value);
}

static uint32_t wasm_Unit_get_health(const Unit* self) {
  return self->GetHealth();
}

static uint32_t wasm_Unit_get_max_health(const Unit* self) {
  return self->GetMaxHealth();
}

static float wasm_Unit_get_health_percent(const Unit* self) {
  return self->GetHealthPct();
}

static float wasm_Unit_get_power_percent(const Unit* self, Powers power) {
  return self->GetPowerPct(power);
}

static void wasm_Unit_set_health(Unit* self, uint32_t value) {
  self->SetHealth(value);
}

static void wasm_Unit_set_max_health(Unit* self, uint32_t value) {
  self->SetMaxHealth(value);
}

static void wasm_Unit_set_full_health(Unit* self) {
  self->SetFullHealth();
}

static int32_t wasm_Unit_modify_health(Unit* self, int32_t value) {
  return self->ModifyHealth(value);
}

static Powers wasm_Unit_get_power_type(const Unit* self) {
  return self->getPowerType();
}

static void wasm_Unit_set_power_type(Unit* self, Powers power) {
  self->setPowerType(power);
}

static uint32_t wasm_Unit_get_power(const Unit* self, Powers power) {
  return self->GetPower(power);
}

static uint32_t wasm_Unit_get_max_power(const Unit* self, Powers power) {
  return self->GetMaxPower(power);
}

static void wasm_Unit_set_power(Unit* self, Powers power, uint32_t value) {
  self->SetPower(power, value);
}

static void wasm_Unit_set_max_power(Unit* self, Powers power, uint32_t value) {
  self->SetMaxPower(power, value);
}

static int32_t wasm_Unit_modify_power(Unit* self, Powers power, int32_t value) {
  return self->ModifyPower(power, value);
}

static int32_t wasm_Unit_modify_power_by_percent(Unit* self, Powers power, float percent) {
  return self->ModifyPowerPct(power, percent);
}

static UnitFlags wasm_Unit_get_unitflags(const Unit* self) {
  return self->GetUnitFlags();
}

static bool wasm_Unit_has_unitflag(const Unit* self, UnitFlags flags) {
  return self->HasUnitFlag(flags);
}

static void wasm_Unit_set_unitflag(Unit* self, UnitFlags flags) {
  self->SetUnitFlag(flags);
}

static void wasm_Unit_remove_unitflag(Unit* self, UnitFlags flags) {
  self->RemoveUnitFlag(flags);
}

static void wasm_Unit_replace_all_unitflags(Unit* self, UnitFlags flags) {
  self->ReplaceAllUnitFlags(flags);
}

static UnitFlags2 wasm_Unit_get_unitflags2(const Unit* self) {
  return self->GetUnitFlags2();
}

static bool wasm_Unit_has_unitflag2(Unit* self, UnitFlags2 flags) {
  return self->HasUnitFlag2(flags);
}

static void wasm_Unit_set_unitflag2(Unit* self, UnitFlags2 flags) {
  self->SetUnitFlag2(flags);
}

static void wasm_Unit_remove_unitflag2(Unit* self, UnitFlags2 flags) {
  self->RemoveUnitFlag2(flags);
}

static void wasm_Unit_replace_all_unitflags2(Unit* self, UnitFlags2 flags) {
  self->ReplaceAllUnitFlags2(flags);
}

static uint32_t wasm_Unit_get_faction(const Unit* self) {
  return self->GetFaction();
}

static void wasm_Unit_set_faction(Unit* self, uint32_t faction) {
  self->SetFaction(faction);
}

static bool wasm_Unit_is_hostile_to(const Unit* self, const Unit* other) {
  return self->IsHostileTo(other);
}

static bool wasm_Unit_is_hostile_to_players(const Unit* self) {
  return self->IsHostileToPlayers();
}

static bool wasm_Unit_is_friendly_to(const Unit* self, const Unit* other) {
  return self->IsFriendlyTo(other);
}

static bool wasm_Unit_is_neutral_to_all(const Unit* self) {
  return self->IsNeutralToAll();
}

static bool wasm_Unit_is_in_party_with(const Unit* self, const Unit* other) {
  return self->IsInPartyWith(other);
}

static bool wasm_Unit_is_in_raid_with(const Unit* self, const Unit* other) {
  return self->IsInRaidWith(other);
}

static std::vector<Unit*> wasm_Unit_get_party_members(Unit* self) {
  std::list<Unit*> members;
  self->GetPartyMembers(members);
  std::vector<Unit*> result;
  for (auto v : members) {
    result.push_back(v);
  }
  return result;
}

static bool wasm_Unit_is_in_sanctuary(const Unit* self) {
  return self->IsInSanctuary();
}

static bool wasm_Unit_is_pvp(const Unit* self) {
  return self->IsPvP();
}

static bool wasm_Unit_is_ffa_pvp(const Unit* self) {
  return self->IsFFAPvP();
}

static uint32_t wasm_Unit_get_creature_type(const Unit* self) {
  return self->GetCreatureType();
}

static uint32_t wasm_Unit_get_creature_type_mask(const Unit* self) {
  return self->GetCreatureTypeMask();
}

static uint8_t wasm_Unit_get_stand_state(const Unit* self) {
  return self->getStandState();
}

static void wasm_Unit_set_stand_state(Unit* self, uint8_t state) {
  self->SetStandState(state);
}

static bool wasm_Unit_is_mounted(const Unit* self) {
  return self->IsMounted();
}

static void wasm_Unit_dismount(Unit* self) {
  self->Dismount();
}

// TODO: implement combat stuff

static NPCFlags wasm_Unit_get_npcflags(const Unit* self) {
  return self->GetNpcFlags();
}

static bool wasm_Unit_has_npcflag(const Unit* self, NPCFlags flags) {
  return self->HasNpcFlag(flags);
}

static void wasm_Unit_set_npcflag(Unit* self, NPCFlags flags) {
  self->SetNpcFlag(flags);
}

static void wasm_Unit_remove_npcflag(Unit* self, NPCFlags flags) {
  self->RemoveNpcFlag(flags);
}

static void wasm_Unit_replace_all_npcflags(Unit* self, NPCFlags flags) {
  self->ReplaceAllNpcFlags(flags);
}

static void wasm_Unit_set_immune_to_pc(Unit* self, bool apply, bool keep_combat) {
  self->SetImmuneToPC(apply, keep_combat);
}

static bool wasm_Unit_is_immune_to_pc(const Unit* self) {
  return self->IsImmuneToPC();
}

static void wasm_Unit_set_immune_to_npc(Unit* self, bool apply, bool keep_combat) {
  self->SetImmuneToNPC(apply, keep_combat);
}

static bool wasm_Unit_is_immune_to_npc(const Unit* self) {
  return self->IsImmuneToNPC();
}

static bool wasm_Unit_is_in_combat(const Unit* self) {
  return self->IsInCombat();
}

static bool wasm_Unit_is_in_combat_with(const Unit* self, const Unit* other) {
  return self->IsInCombatWith(other);
}

static void wasm_Unit_combat_start(Unit* self, Unit* target) {
  self->CombatStart(target);
}

static void wasm_Unit_set_in_combat_with(Unit* self, Unit* enemy) {
  self->SetInCombatWith(enemy);
}

static void wasm_Unit_clear_in_combat(Unit* self) {
  self->ClearInCombat();
}

// TODO: Aura stuff

// from is nullable
static bool wasm_Unit_is_targetable_for_attack(const Unit* self, const Unit* from) {
  return self->isTargetableForAttack(true, from);
}

static bool wasm_Unit_is_valid_attack_target(const Unit* self, const Unit* target) {
  return self->IsValidAttackTarget(target);
}

static bool wasm_Unit_is_valid_assist_target(const Unit* self, const Unit* target) {
  return self->IsValidAssistTarget(target);
}

// TODO: cast spell stuff

// target is nullable
static Aura* wasm_Unit_add_aura(Unit* self, uint32_t spell_id, Unit* target) {
  if (target == nullptr) {
    target = self;
  }
  return self->AddAura(spell_id, target);
}

static void wasm_Unit_send_play_spell_visual(Unit* self, uint32_t id) {
  self->SendPlaySpellVisual(id);
}

static void wasm_Unit_send_play_spell_impact(Unit* self, uint32_t id, ObjectGuid object_guid) {
  self->SendPlaySpellImpact(object_guid, id);
}

static void wasm_Unit_demorph(Unit* self) {
  self->DeMorph();
}

static void wasm_Unit_near_teleport_to(Unit* self, float x, float y, float z, float o) {
  self->NearTeleportTo(x, y, z, o);
}

static void wasm_Unit_knockback_from(Unit* self, float x, float y, float speed_xy, float speed_z) {
  self->KnockbackFrom(x, y, speed_xy, speed_z);
}

static bool wasm_Unit_set_walk(Unit* self, bool enable) {
  return self->SetWalk(enable);
}

static bool wasm_Unit_set_disable_gravity(Unit* self, bool enable) {
  return self->GetVictim();
}

static bool wasm_Unit_set_swim(Unit* self, bool enable) {
  return self->SetSwim(enable);
}

static bool wasm_Unit_set_can_fly(Unit* self, bool enable) {
  return self->SetCanFly(enable);
}

static bool wasm_Unit_set_water_walking(Unit* self, bool enable) {
  return self->SetWaterWalking(enable);
}

static bool wasm_Unit_set_slow_fall(Unit* self, bool enable) {
  return self->SetFeatherFall(enable);
}

static bool wasm_Unit_set_hover(Unit* self, bool enable) {
  return self->SetHover(enable);
}

static void wasm_Unit_set_in_front(Unit* self, const WorldObject* target) {
  self->SetInFront(target);
}

static void wasm_Unit_set_facing_to_object(Unit* self, WorldObject* target) {
  self->SetFacingToObject(target);
}

static DeathState wasm_Unit_get_death_state(Unit* self) {
  return self->getDeathState();
}

// TODO: relation guids

// TODO: possession and charm stuff

// TODO: owned and applied auras

static void wasm_Unit_remove_aura(Unit* self, uint32_t spell_id) {
  self->RemoveAura(spell_id);
}

// TODO: advanced aura removal

static Aura* wasm_Unit_get_aura(const Unit* self, uint32_t spell_id) {
  return self->GetAura(spell_id);
}

static bool wasm_Unit_has_aura(Unit* self, uint32_t spell_id) {
  return self->HasAura(spell_id);
}

// TODO: advanced aura selection / modifiers

static ShapeshiftForm wasm_Unit_get_shapeshift_form(const Unit* self) {
  return self->GetShapeshiftForm();
}

static void wasm_Unit_set_shapeshift_form(Unit* self, ShapeshiftForm form) {
  self->SetShapeshiftForm(form);
}

// TODO: advanced stuff like stats and threat

static uint32_t wasm_Unit_get_display_id(const Unit* self) {
  return self->GetDisplayId();
}

static void wasm_Unit_set_display_id(Unit* self, uint32_t display_id) {
  self->SetDisplayId(display_id);
}

static uint32_t wasm_Unit_get_native_display_id(const Unit* self) {
  return self->GetNativeDisplayId();
}

static void wasm_Unit_set_native_display_id(Unit* self, uint32_t display_id) {
  self->SetNativeDisplayId(display_id);
}

// TODO: more complicated stuff

static float wasm_Unit_get_speed(const Unit* self, UnitMoveType move_type) {
  return self->GetSpeed(move_type);
}

static void wasm_Unit_set_speed(Unit* self, UnitMoveType move_type, float speed) {
  self->SetSpeed(move_type, speed);
}

static void wasm_Unit_stop_moving(Unit* self) {
  self->StopMoving();
}

static void wasm_Unit_pause_movement(Unit* self) {
  self->PauseMovement();
}

static void wasm_Unit_resume_movement(Unit* self) {
  self->ResumeMovement();
}

static void wasm_Unit_add_unit_movement_flag(Unit* self, uint32_t f) {
  self->AddUnitMovementFlag(f);
}

static void wasm_Unit_remove_unit_movement_flag(Unit* self, uint32_t f) {
  self->RemoveUnitMovementFlag(f);
}

static bool wasm_Unit_has_unit_movement_flag(const Unit* self, uint32_t f) {
  return self->HasUnitMovementFlag(f);
}

static uint32_t wasm_Unit_get_unit_movement_flags(const Unit* self) {
  return self->GetUnitMovementFlags();
}

static void wasm_Unit_set_unit_movement_flags(Unit* self, uint32_t f) {
  self->SetUnitMovementFlags(f);
}

static ObjectGuid wasm_Unit_get_target_guid(const Unit* self) {
  return self->GetTarget();
}

static void wasm_Unit_set_target_guid(Unit* self, ObjectGuid guid) {
  self->SetTarget(guid);
}

// target is nullable
static void wasm_Unit_say(Unit* self, std::string_view text, Language language, const WorldObject* target) {
  self->Say(text, language, target);
}

// target is nullable
static void wasm_Unit_yell(Unit* self, std::string_view text, Language language, const WorldObject* target) {
  self->Yell(text, language, target);
}

// target is nullable
static void wasm_Unit_textemote(Unit* self, std::string_view text, const WorldObject* target, bool is_boss_emote) {
  self->TextEmote(text, target, is_boss_emote);
}

static void wasm_Unit_whisper(Unit* self, std::string_view text, Language language, Player* target, bool is_boss_whisper) {
  self->Whisper(text, language, target, is_boss_whisper);
}

#define UNIT_WASM_EXPORTS\
  WASM_EXPORT(wasm_Unit_is_within_combat_range),\
  WASM_EXPORT(wasm_Unit_is_within_melee_range),\
  WASM_EXPORT(wasm_Unit_get_melee_range),\
  WASM_EXPORT(wasm_Unit_attack),\
  WASM_EXPORT(wasm_Unit_cast_stop),\
  WASM_EXPORT(wasm_Unit_attack_stop),\
  WASM_EXPORT(wasm_Unit_remove_all_attackers),\
  WASM_EXPORT(wasm_Unit_get_attackers),\
  WASM_EXPORT(wasm_Unit_get_victim),\
  WASM_EXPORT(wasm_Unit_combat_stop),\
  WASM_EXPORT(wasm_Unit_combat_stop_with_pets),\
  WASM_EXPORT(wasm_Unit_stop_attack_faction),\
  WASM_EXPORT(wasm_Unit_stop_attacking_invalid_target),\
  WASM_EXPORT(wasm_Unit_select_nearby_target),\
  WASM_EXPORT(wasm_Unit_select_nearby_no_totem_target),\
  WASM_EXPORT(wasm_Unit_add_unit_state),\
  WASM_EXPORT(wasm_Unit_has_unit_state),\
  WASM_EXPORT(wasm_Unit_clear_unit_state),\
  WASM_EXPORT(wasm_Unit_get_unit_state),\
  WASM_EXPORT(wasm_Unit_can_free_move),\
  WASM_EXPORT(wasm_Unit_has_unit_type_mask),\
  WASM_EXPORT(wasm_Unit_get_unit_type_mask),\
  WASM_EXPORT(wasm_Unit_get_level),\
  WASM_EXPORT(wasm_Unit_set_level),\
  WASM_EXPORT(wasm_Unit_get_race),\
  WASM_EXPORT(wasm_Unit_set_race),\
  WASM_EXPORT(wasm_Unit_get_race_mask),\
  WASM_EXPORT(wasm_Unit_get_class),\
  WASM_EXPORT(wasm_Unit_get_class_mask),\
  WASM_EXPORT(wasm_Unit_get_gender),\
  WASM_EXPORT(wasm_Unit_get_stat),\
  WASM_EXPORT(wasm_Unit_set_stat),\
  WASM_EXPORT(wasm_Unit_get_armor),\
  WASM_EXPORT(wasm_Unit_set_armor),\
  WASM_EXPORT(wasm_Unit_get_resistance_for_school),\
  WASM_EXPORT(wasm_Unit_get_resistance_for_school_mask),\
  WASM_EXPORT(wasm_Unit_set_resistance),\
  WASM_EXPORT(wasm_Unit_get_health),\
  WASM_EXPORT(wasm_Unit_get_max_health),\
  WASM_EXPORT(wasm_Unit_get_health_percent),\
  WASM_EXPORT(wasm_Unit_get_power_percent),\
  WASM_EXPORT(wasm_Unit_set_health),\
  WASM_EXPORT(wasm_Unit_set_max_health),\
  WASM_EXPORT(wasm_Unit_set_full_health),\
  WASM_EXPORT(wasm_Unit_modify_health),\
  WASM_EXPORT(wasm_Unit_get_power_type),\
  WASM_EXPORT(wasm_Unit_set_power_type),\
  WASM_EXPORT(wasm_Unit_get_power),\
  WASM_EXPORT(wasm_Unit_get_max_power),\
  WASM_EXPORT(wasm_Unit_set_power),\
  WASM_EXPORT(wasm_Unit_set_max_power),\
  WASM_EXPORT(wasm_Unit_modify_power),\
  WASM_EXPORT(wasm_Unit_modify_power_by_percent),\
  WASM_EXPORT(wasm_Unit_get_unitflags),\
  WASM_EXPORT(wasm_Unit_has_unitflag),\
  WASM_EXPORT(wasm_Unit_set_unitflag),\
  WASM_EXPORT(wasm_Unit_remove_unitflag),\
  WASM_EXPORT(wasm_Unit_replace_all_unitflags),\
  WASM_EXPORT(wasm_Unit_get_unitflags2),\
  WASM_EXPORT(wasm_Unit_has_unitflag2),\
  WASM_EXPORT(wasm_Unit_set_unitflag2),\
  WASM_EXPORT(wasm_Unit_remove_unitflag2),\
  WASM_EXPORT(wasm_Unit_replace_all_unitflags2),\
  WASM_EXPORT(wasm_Unit_get_faction),\
  WASM_EXPORT(wasm_Unit_set_faction),\
  WASM_EXPORT(wasm_Unit_is_hostile_to),\
  WASM_EXPORT(wasm_Unit_is_hostile_to_players),\
  WASM_EXPORT(wasm_Unit_is_friendly_to),\
  WASM_EXPORT(wasm_Unit_is_neutral_to_all),\
  WASM_EXPORT(wasm_Unit_is_in_party_with),\
  WASM_EXPORT(wasm_Unit_is_in_raid_with),\
  WASM_EXPORT(wasm_Unit_get_party_members),\
  WASM_EXPORT(wasm_Unit_is_in_sanctuary),\
  WASM_EXPORT(wasm_Unit_is_pvp),\
  WASM_EXPORT(wasm_Unit_is_ffa_pvp),\
  WASM_EXPORT(wasm_Unit_get_creature_type),\
  WASM_EXPORT(wasm_Unit_get_creature_type_mask),\
  WASM_EXPORT(wasm_Unit_get_stand_state),\
  WASM_EXPORT(wasm_Unit_set_stand_state),\
  WASM_EXPORT(wasm_Unit_is_mounted),\
  WASM_EXPORT(wasm_Unit_dismount),\
  WASM_EXPORT(wasm_Unit_get_npcflags),\
  WASM_EXPORT(wasm_Unit_has_npcflag),\
  WASM_EXPORT(wasm_Unit_set_npcflag),\
  WASM_EXPORT(wasm_Unit_remove_npcflag),\
  WASM_EXPORT(wasm_Unit_replace_all_npcflags),\
  WASM_EXPORT(wasm_Unit_set_immune_to_pc),\
  WASM_EXPORT(wasm_Unit_is_immune_to_pc),\
  WASM_EXPORT(wasm_Unit_set_immune_to_npc),\
  WASM_EXPORT(wasm_Unit_is_immune_to_npc),\
  WASM_EXPORT(wasm_Unit_is_in_combat),\
  WASM_EXPORT(wasm_Unit_is_in_combat_with),\
  WASM_EXPORT(wasm_Unit_combat_start),\
  WASM_EXPORT(wasm_Unit_set_in_combat_with),\
  WASM_EXPORT(wasm_Unit_clear_in_combat),\
  WASM_EXPORT(wasm_Unit_is_targetable_for_attack),\
  WASM_EXPORT(wasm_Unit_is_valid_attack_target),\
  WASM_EXPORT(wasm_Unit_is_valid_assist_target),\
  WASM_EXPORT(wasm_Unit_add_aura),\
  WASM_EXPORT(wasm_Unit_send_play_spell_visual),\
  WASM_EXPORT(wasm_Unit_send_play_spell_impact),\
  WASM_EXPORT(wasm_Unit_demorph),\
  WASM_EXPORT(wasm_Unit_near_teleport_to),\
  WASM_EXPORT(wasm_Unit_knockback_from),\
  WASM_EXPORT(wasm_Unit_set_walk),\
  WASM_EXPORT(wasm_Unit_set_disable_gravity),\
  WASM_EXPORT(wasm_Unit_set_swim),\
  WASM_EXPORT(wasm_Unit_set_can_fly),\
  WASM_EXPORT(wasm_Unit_set_water_walking),\
  WASM_EXPORT(wasm_Unit_set_slow_fall),\
  WASM_EXPORT(wasm_Unit_set_hover),\
  WASM_EXPORT(wasm_Unit_set_in_front),\
  WASM_EXPORT(wasm_Unit_set_facing_to_object),\
  WASM_EXPORT(wasm_Unit_get_death_state),\
  WASM_EXPORT(wasm_Unit_remove_aura),\
  WASM_EXPORT(wasm_Unit_get_aura),\
  WASM_EXPORT(wasm_Unit_has_aura),\
  WASM_EXPORT(wasm_Unit_get_shapeshift_form),\
  WASM_EXPORT(wasm_Unit_set_shapeshift_form),\
  WASM_EXPORT(wasm_Unit_get_display_id),\
  WASM_EXPORT(wasm_Unit_set_display_id),\
  WASM_EXPORT(wasm_Unit_get_native_display_id),\
  WASM_EXPORT(wasm_Unit_set_native_display_id),\
  WASM_EXPORT(wasm_Unit_get_speed),\
  WASM_EXPORT(wasm_Unit_set_speed),\
  WASM_EXPORT(wasm_Unit_stop_moving),\
  WASM_EXPORT(wasm_Unit_pause_movement),\
  WASM_EXPORT(wasm_Unit_resume_movement),\
  WASM_EXPORT(wasm_Unit_add_unit_movement_flag),\
  WASM_EXPORT(wasm_Unit_remove_unit_movement_flag),\
  WASM_EXPORT(wasm_Unit_has_unit_movement_flag),\
  WASM_EXPORT(wasm_Unit_get_unit_movement_flags),\
  WASM_EXPORT(wasm_Unit_set_unit_movement_flags),\
  WASM_EXPORT(wasm_Unit_get_target_guid),\
  WASM_EXPORT(wasm_Unit_set_target_guid),\
  WASM_EXPORT(wasm_Unit_say),\
  WASM_EXPORT(wasm_Unit_yell),\
  WASM_EXPORT(wasm_Unit_textemote),\
  WASM_EXPORT(wasm_Unit_whisper)\
