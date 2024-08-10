#pragma once

#include "Player.h"

// Player inherits from Unit

static void wasm_Player_teleport(Player* self, uint32_t map_id, float x, float y, float z, float o) {
  self->TeleportTo(map_id, x, y, z, o);
}

static void wasm_Player_teleport_to(Player* self, const WorldLocation* location) {
  self->TeleportTo(*location);
}

static void wasm_Player_teleport_to_entry_point(Player* self) {
  self->TeleportToEntryPoint();
}

static PlayerFlags wasm_Player_get_playerflags(Player* self) {
  return self->GetPlayerFlags();
}

static bool wasm_Player_has_playerflag(Player* self, PlayerFlags flags) {
  return self->HasPlayerFlag(flags);
}

static void wasm_Player_set_playerflag(Player* self, PlayerFlags flags) {
  self->SetPlayerFlag(flags);
}

static void wasm_Player_remove_playerflag(Player* self, PlayerFlags flags) {
  self->RemovePlayerFlag(flags);
}

static void wasm_Player_replace_all_playerflags(Player* self, PlayerFlags flags) {
  self->ReplaceAllPlayerFlags(flags);
}

static PlayerSocial* wasm_Player_get_social(Player* self) {
  return self->GetSocial();
}

static void wasm_Player_give_level(Player* self, uint8_t level) {
  self->GiveLevel(level);
}

static uint32_t wasm_Player_get_item_count(Player* self, uint32_t item_id) {
  return self->GetItemCount(item_id);
}

static Item* wasm_Player_get_item_by_guid(Player* self, ObjectGuid guid) {
  return self->GetItemByGuid(guid);
}

static Item* wasm_Player_get_item_by_entry(Player* self, uint32_t entry) {
  return self->GetItemByEntry(entry);
}

static Item* wasm_Player_get_item_by_pos(Player* self, uint16_t pos) {
  return self->GetItemByPos(pos);
}

// TODO: store item

static Item* wasm_Player_equip_new_item(Player* self, uint16_t pos, uint32_t item_id) {
  return self->EquipNewItem(pos, item_id, true);
}

// TODO: advanced item stuff

static bool wasm_Player_add_item(Player* self, uint32_t item_id, uint32_t count) {
  return self->AddItem(item_id, count);
}

// TODO: advanced gossip stuff

static void wasm_Player_complete_quest(Player* self, uint32_t quest_id) {
  self->CompleteQuest(quest_id);
}

static void wasm_Player_fail_quest(Player* self, uint32_t quest_id) {
  self->FailQuest(quest_id);
}

// TODO: advanced quest stuff

// TODO: character customization

static uint32_t wasm_Player_get_money(Player* self) {
  return self->GetMoney();
}

static void wasm_Player_modify_money(Player* self, int32_t amount) {
  self->ModifyMoney(amount);
}

static Unit* wasm_Player_get_selected_unit(Player* self) {
  return self->GetSelectedUnit();
}

// TODO: mail stuff

static bool wasm_Player_has_spell(Player* self, uint32_t spell_id) {
  return self->HasSpell(spell_id);
}

static void wasm_Player_learn_spell(Player* self, uint32_t spell_id) {
  self->learnSpell(spell_id);
}

static void wasm_Player_learn_talent(Player* self, uint32_t spell_id, uint32_t rank) {
  self->LearnTalent(spell_id, rank);
}

static bool wasm_Player_has_talent(Player* self, uint32_t spell_id) {
  return self->HasTalent(spell_id, self->GetActiveSpecMask());
}

static TeamId wasm_Player_get_team_id(Player* self) {
  return self->GetTeamId();
}

static void wasm_Player_set_drunk_value(Player* self, float value) {
  self->SetDrunkValue(value);
}

static float wasm_Player_get_drunk_value(Player* self) {
  return self->GetDrunkValue();
}

static std::string wasm_Player_get_player_name(Player* self) {
  return self->GetPlayerName();
}

static void wasm_Player_gossip_menu_clear(Player *self) {
  self->PlayerTalkClass->ClearMenus();
}

static void wasm_Player_gossip_menu_add_item(Player* self, uint8_t icon, std::string& msg, uint32_t sender, uint32_t action, std::string& prompt, uint32_t cost, bool coded) {
  auto& menu = self->PlayerTalkClass->GetGossipMenu();
  if (menu.GetMenuItemCount() >= GOSSIP_MAX_MENU_ITEMS) {
    throw std::runtime_error{"gossip menu is full"};
  }

  menu.AddMenuItem(-1, icon, msg, sender, action, prompt, cost, coded);
}

static void wasm_Player_gossip_menu_send(Player* self, uint32_t npctext_id, WorldObject* sender) {
  self->PlayerTalkClass->SendGossipMenu(npctext_id, sender->GetGUID());
}

static void wasm_Player_gossip_menu_close(Player *self) {
  self->PlayerTalkClass->SendCloseGossip();
}

// TODO: player settings

#define PLAYER_WASM_EXPORTS\
  WASM_EXPORT(wasm_Player_teleport),\
  WASM_EXPORT(wasm_Player_teleport_to),\
  WASM_EXPORT(wasm_Player_teleport_to_entry_point),\
  WASM_EXPORT(wasm_Player_get_playerflags),\
  WASM_EXPORT(wasm_Player_has_playerflag),\
  WASM_EXPORT(wasm_Player_set_playerflag),\
  WASM_EXPORT(wasm_Player_remove_playerflag),\
  WASM_EXPORT(wasm_Player_replace_all_playerflags),\
  WASM_EXPORT(wasm_Player_get_social),\
  WASM_EXPORT(wasm_Player_give_level),\
  WASM_EXPORT(wasm_Player_get_item_count),\
  WASM_EXPORT(wasm_Player_get_item_by_guid),\
  WASM_EXPORT(wasm_Player_get_item_by_entry),\
  WASM_EXPORT(wasm_Player_get_item_by_pos),\
  WASM_EXPORT(wasm_Player_equip_new_item),\
  WASM_EXPORT(wasm_Player_add_item),\
  WASM_EXPORT(wasm_Player_complete_quest),\
  WASM_EXPORT(wasm_Player_fail_quest),\
  WASM_EXPORT(wasm_Player_get_money),\
  WASM_EXPORT(wasm_Player_modify_money),\
  WASM_EXPORT(wasm_Player_get_selected_unit),\
  WASM_EXPORT(wasm_Player_has_spell),\
  WASM_EXPORT(wasm_Player_learn_spell),\
  WASM_EXPORT(wasm_Player_learn_talent),\
  WASM_EXPORT(wasm_Player_has_talent),\
  WASM_EXPORT(wasm_Player_get_team_id),\
  WASM_EXPORT(wasm_Player_set_drunk_value),\
  WASM_EXPORT(wasm_Player_get_drunk_value),\
  WASM_EXPORT(wasm_Player_get_player_name),\
  WASM_EXPORT(wasm_Player_gossip_menu_clear),\
  WASM_EXPORT(wasm_Player_gossip_menu_add_item),\
  WASM_EXPORT(wasm_Player_gossip_menu_send),\
  WASM_EXPORT(wasm_Player_gossip_menu_close)\
