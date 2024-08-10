/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU
 * AGPL v3 license:
 * https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "AllCommandScript.h"
#include "AllCreatureScript.h"
#include "AllGameObjectScript.h"
#include "AllItemScript.h"
#include "AllMapScript.h"
#include "Chat.h"
#include "Config.h"
#include "DBUpdater.h"
#include "DatabaseEnvFwd.h"
#include "DatabaseLoader.h"
#include "DatabaseWorkerPool.h"
#include "ObjectAccessor.h"
#include "PCQueue.h"
#include "Player.h"
#include "PlayerScript.h"
#include "SQLOperation.h"
#include "ScriptedGossip.h"
#include "SpellMgr.h"
#include "WorldDatabase.h"
#include "WorldObjectScript.h"
#include "WorldScript.h"
#include "foreach.hpp"
#include "wasm-Aura.hpp"
#include "wasm-Creature.hpp"
#include "wasm-Object.hpp"
#include "wasm-ObjectGuid.hpp"
#include "wasm-Player.hpp"
#include "wasm-QueryResult.hpp"
#include "wasm-Spell.hpp"
#include "wasm-Unit.hpp"
#include "wasm-WorldLocation.hpp"
#include "wasm-WorldObject.hpp"
#include "wasm-host.hpp"
#include "wasm_export.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>

// #define WOWASM_LOG_INFO(...)                                                   \
//   LOG_INFO("wowasm", "[WoWasm]: " _VA_FIRST_(__VA_ARGS__),                     \
//            _VA_SKIP0_(__VA_ARGS__));

#define LOG(FORMAT, ...)                                                       \
  printf("[WoWasm]: " FORMAT "\n" __VA_OPT__(, __VA_ARGS__))

inline auto read_file(const std::filesystem::path &path) {
  auto buffer_size = std::filesystem::file_size(path);
  std::ifstream input{path, std::ios::binary};

  std::vector<uint8_t> buffer(buffer_size);
  input.read((char *)buffer.data(), buffer_size);

  return buffer;
}

inline auto read_text_file(const std::filesystem::path &path) {
  auto buffer_size = std::filesystem::file_size(path);
  std::ifstream input{path, std::ios::binary};

  std::string buffer;
  buffer.resize(buffer_size);
  input.read(buffer.data(), buffer_size);

  return buffer;
}

struct wasm_vm {
private:
  static inline wasm_vm &_vm() { return *wasm::get_attachment<wasm_vm>(); }

  static void wasm_reload_vm() { _vm().run_reload(); }

  static void wasm_register_callback(std::string_view name, uint32_t cb_idx,
                                     uint32_t cb_ctx) {
    _vm().register_callback(name, cb_idx, cb_ctx);
  }

  static void wasm_remove_callback(std::string_view name, uint32_t cb_idx) {
    _vm().remove_callback(name, cb_idx);
  }

  std::filesystem::path _base_dir;

  //   wasm::native_module _native_module_env{
  //       "env",
  //       {
  //           WASM_EXPORT_CORE,
  //           WASM_EXPORT("wasm_reload_vm", wasm_reload_vm, this),
  //           WASM_EXPORT("wasm_register_callback", wasm_register_callback,
  //           this), WASM_EXPORT("wasm_remove_callback", wasm_remove_callback,
  //           this),
  //       }};

  bool did_init = false;

  std::vector<NativeSymbol> &_natives;

  wasm::heap _heap{MEGABYTE(32)};

  std::vector<wasm::unit> _files;

  wasm::callback_map _callbacks;

  void reload_files(const char *main_func) {
    _callbacks.clear();
    _files.clear();

    _heap.reset();

    if (did_init) {
      wasm::runtime_destroy();
    }

    wasm::runtime_init_with_heap(_heap);
    wasm::runtime_register_natives("env", _natives);
    did_init = true;

    if (!std::filesystem::exists(_base_dir)) {
      LOG("search directory `%s` does not exist", _base_dir.string().data());
      return;
    }

    for (const auto &entry : std::filesystem::directory_iterator{_base_dir}) {
      if (!entry.path().string().ends_with(".wasm")) {
        continue;
      }

      LOG("found wasm file `%s`", entry.path().string().data());
      _files.emplace_back(read_file(entry.path()), MEGABYTE(8));

      wasm::function<void()> init{_files.back(), "_initialize"};
      if (init) {
        init();
      }

      wasm::function<void()> main{_files.back(), main_func};
      if (main) {
        main();
      }
    }
  }

  void register_callback(std::string_view name, uint32_t func_idx,
                         uint32_t func_ctx) {
    _callbacks.emplace(
        (std::string)name,
        wasm::callback_ptr{wasm::current_module_inst, func_idx, func_ctx});
  }

  void remove_callback(std::string_view name, uint32_t func_idx) {
    // TODO: return func_ctx
    std::erase_if(_callbacks, [&](const auto &entry) {
      return (entry.first == name) && (entry.second.func_idx == func_idx);
    });
  }

public:
  explicit wasm_vm(std::vector<NativeSymbol> &natives)
      : _base_dir{"."}, _natives{natives} {}

  inline void set_directory(const std::filesystem::path &base_dir) {
    _base_dir = base_dir;
  }

  inline void run_setup() {
    LOG("running setup");
    reload_files("wasm_setup");
  }

  inline void run_reload() {
    LOG("reloading vm");
    reload_files("wasm_on_reload_vm");
  }

  inline void lookup_functions(const char *name,
                               std::vector<wasm::function_ptr> &result) {
    for (auto &unit : _files) {
      wasm::function_ptr funcptr{unit.instance, name};
      if (funcptr.func) {
        result.emplace_back(std::move(funcptr));
      }
    }
  }

  inline std::vector<wasm::function_ptr> lookup_functions(const char *name) {
    std::vector<wasm::function_ptr> result;
    lookup_functions(name, result);
    return result;
  }

  template <typename _Sig>
  inline wasm::callback<_Sig> get_callback(const char *name) {
    return {_callbacks, name};
  }

  static NativeSymbol get_wasm_reload_vm(wasm_vm *vm) {
    return WASM_EXPORT(wasm_reload_vm, vm);
  }

  static NativeSymbol get_wasm_register_callback(wasm_vm *vm) {
    return WASM_EXPORT(wasm_register_callback, vm);
  }

  static NativeSymbol get_wasm_remove_callback(wasm_vm *vm) {
    return WASM_EXPORT(wasm_remove_callback, vm);
  }
};

#define _DEFINE_ENUM_STRINGIFY_NAME(NAME, COMMA) "wasm_" #NAME COMMA

#define _DEFINE_ENUM_UNPACK_VALUES(...) __VA_ARGS__
#define _DEFINE_ENUM_UNPACK_NAMES(...)                                         \
  FOR_EACH(_DEFINE_ENUM_STRINGIFY_NAME, __VA_ARGS__)
#define DEFINE_ENUM(TYPE, VALUES)                                              \
  struct TYPE {                                                                \
    enum { _DEFINE_ENUM_UNPACK_VALUES VALUES, _SIZE };                         \
    constexpr static const char *_names[] = {                                  \
        _DEFINE_ENUM_UNPACK_NAMES VALUES};                                     \
  };

DEFINE_ENUM(
    ScriptEvent,
    (_NULL, CanCreatureGossipHello, CanCreatureGossipSelect,
     CanCreatureGossipSelectCode, OnCreatureAddWorld, OnCreatureRemoveWorld,
     CanCreatureQuestAccept, CanCreatureQuestReward, OnGameObjectAddWorld,
     OnGameObjectRemoveWorld, OnGameObjectUpdate, CanGameObjectGossipHello,
     OnGameObjectDamaged, OnGameObjectDestroyed, OnGameObjectLootStateChanged,
     OnGameObjectStateChanged, CanGameObjectQuestAccept,
     CanGameObjectGossipSelect, CanGameObjectGossipSelectCode,
     CanGameObjectQuestReward, CanItemQuestAccept, CanItemUse, CanItemExpire,
     CanItemRemove, OnItemGossipSelect, OnItemGossipSelectCode,
     OnBeforeCreateInstanceScript, OnDestroyInstance, OnCreateMap, OnDestroyMap,
     OnTryExecuteCommand, OnPlayerEnterAll, OnPlayerLeaveAll, OnMapUpdate,
     OnPlayerResurrect, CanPlayerUseChat0, CanPlayerUseChat1, CanPlayerUseChat2,
     CanPlayerUseChat3, CanPlayerUseChat4, OnLootItem, OnPlayerLearnTalents,
     CanUseItem, OnEquip, OnPlayerEnterCombat, OnPlayerLeaveCombat,
     CanRepopAtGraveyard, OnQuestAbandon, OnMapChanged, OnGossipSelect,
     OnGossipSelectCode, OnPVPKill, OnCreatureKill, OnPlayerKilledByCreature,
     OnLevelChanged, OnFreeTalentPointsChanged, OnTalentsReset, OnMoneyChanged,
     OnGiveXP, OnReputationChange, OnDuelRequest, OnDuelStart, OnDuelEnd,
     OnEmote, OnTextEmote, OnSpellCast, OnLogin, OnLogout, OnCreate, OnSave,
     OnDelete, OnBindToInstance, OnUpdateArea, OnUpdateZone, OnFirstLogin,
     OnLearnSpell, OnAchiComplete, OnFfaPvpStateUpdate, CanInitTrade,
     CanSendMail, CanJoinLfg, OnQuestRewardItem, OnGroupRollRewardItem,
     OnCreateItem, OnStoreNewItem, OnPlayerCompleteQuest, CanGroupInvite,
     OnBattlegroundDesertion, OnCreatureKilledByPet, OnOpenStateChange,
     OnAfterConfigLoad, OnShutdownInitiate, OnShutdownCancel, OnStartup,
     OnShutdown, OnAfterUnloadAllMaps, OnBeforeWorldInitialized));

#define _SCRIPT_HANDLER_ARG(TYPE, INDEX, COMMA) TYPE COMMA

#define _SCRIPT_HANDLER_UNPACK_ARGS(...)                                       \
  __VA_OPT__(FOR_EACH_INDEXED(_SCRIPT_HANDLER_ARG, __VA_ARGS__))

#define _SCRIPT_HANDLER_NAMED_ARG(TYPE, INDEX, COMMA) TYPE arg##INDEX COMMA

#define _SCRIPT_HANDLER_UNPACK_NAMED_ARGS(...)                                 \
  __VA_OPT__(FOR_EACH_INDEXED(_SCRIPT_HANDLER_NAMED_ARG, __VA_ARGS__))

#define _SCRIPT_HANDLER_FORWARDED_ARG(TYPE, INDEX, COMMA) arg##INDEX COMMA

#define _SCRIPT_HANDLER_UNPACK_FORWARDED_ARGS(...)                             \
  __VA_OPT__(FOR_EACH_INDEXED(_SCRIPT_HANDLER_FORWARDED_ARG, __VA_ARGS__))

// returns void
#define SCRIPT_HANDLER_VOID(NAME, ARGS)                                        \
  wasm::multi_function<void(_SCRIPT_HANDLER_UNPACK_ARGS ARGS)> handle##NAME{   \
      _handlers[(size_t)ScriptEvent::NAME]};                                   \
  void NAME(_SCRIPT_HANDLER_UNPACK_NAMED_ARGS ARGS) override {                 \
    if (handle##NAME.empty())                                                  \
      return;                                                                  \
    std::scoped_lock _vm_lock{_vm_mutex};                                      \
    handle##NAME(_SCRIPT_HANDLER_UNPACK_FORWARDED_ARGS ARGS);                  \
  }

#define SCRIPT_HANDLER_BOOL(NAME, ARGS, DEFAULT)                               \
  wasm::multi_function<bool(_SCRIPT_HANDLER_UNPACK_ARGS ARGS)> handle##NAME{   \
      _handlers[(size_t)ScriptEvent::NAME]};                                   \
  bool NAME(_SCRIPT_HANDLER_UNPACK_NAMED_ARGS ARGS) override {                 \
    if (handle##NAME.empty())                                                  \
      return DEFAULT(false);                                                   \
    std::scoped_lock _vm_lock{_vm_mutex};                                      \
    auto result = handle##NAME(_SCRIPT_HANDLER_UNPACK_FORWARDED_ARGS ARGS);    \
    for (auto r : result) {                                                    \
      if (DEFAULT(r)) {                                                        \
        return DEFAULT(true);                                                  \
      }                                                                        \
    }                                                                          \
    return DEFAULT(false);                                                     \
  }

#define DEFAULT_TRUE(V) !V
#define DEFAULT_FALSE(V) V

template <typename T>
void sql_format_helper(std::string &out, std::string_view &fmt,
                       T value) {
  using _T = std::decay_t<T>;
  std::size_t value_marker = fmt.find('?');
  if (value_marker == std::string::npos) {
    return;
  }
  out += (std::string)fmt.substr(0, value_marker);
  if constexpr (std::is_same_v<_T, std::nullptr_t>) {
    out += "NULL";
  } else if constexpr (std::is_arithmetic_v<_T>) {
    out += std::to_string(value);
  } else {
    WorldDatabase.EscapeString(value);
    out += "'" + value + "'";
  }
  fmt = fmt.substr(value_marker + 1);
}

std::string sql_format(std::string_view fmt, const PreparedStatementBase* stmt) {
  std::string out;
  for (auto &param : stmt->GetParameters()) {
    switch (param.data.index()) {
    case 8: // int64
      sql_format_helper(out, fmt, std::get<8>(param.data));
      break;
    case 10: // double
      sql_format_helper(out, fmt, std::get<10>(param.data));
      break;
    case 11: // string
      sql_format_helper(out, fmt, std::get<11>(param.data));
      break;
    case 13: // null
      sql_format_helper(out, fmt, nullptr);
      break;
    }
  }
  out += fmt;
  return out;
}

class WoWasm_Runtime : public AllCreatureScript,
                       public AllGameObjectScript,
                       public AllItemScript,
                       public AllMapScript,
                       public AllCommandScript,
                       public PlayerScript,
                       public WorldObjectScript,
                       public WorldScript {
private:
  constexpr static auto NAME = "WoWasm_Runtime";

  static inline WoWasm_Runtime *_self() {
    return wasm::get_attachment<WoWasm_Runtime>();
  }

#define WOWASM_LOCK                                                            \
  std::scoped_lock _vm_lock { self->_vm_mutex }

  static void wasm_reload_vm() { _self()->reload_vm(); }

  static uint64_t wasm_thread_id() { return pthread_self(); }

  static std::string wasm_get_cwd() {
    char cwd[256];
    if (::getcwd(cwd, sizeof(cwd)) != nullptr) {
      return cwd;
    } else {
      return "";
    }
  }

  static std::string wasm_read_text_file(std::string_view path) {
    return read_text_file(path);
  }

  static uint64_t wasm_get_unixtime() {
    auto clock = std::chrono::system_clock::now();
    auto result = std::chrono::duration_cast<std::chrono::milliseconds>(
                      clock.time_since_epoch())
                      .count();
    return result;
  }

  static uint64_t wasm_register_timeout(uint64_t ms, WASM_CALLBACK_PTR_ARGS) {
    auto self = _self();
    auto func = WASM_CALLBACK_PTR_MAKE_SHARED();
    self->_timeouts.emplace(self->_time + ms, [self, func]() {
      //   mutex locking happens at call site
      try {
        wasm::function<void(uint32_t)> cb{
            WASM_CALLBACK_PTR_AS_FUNCTION_ARGS(*func)};
        cb(func->func_ctx);
      } catch (const std::exception &error) {
        LOG("%s", error.what());
      }
    });

    return 0;
  }

  static void wasm_remove_timeout(uint64_t) {}

  //   static uint64_t wasm_register_interval() { return 0; }

  //   static void wasm_remove_interval(uint64_t) {}

  static void wasm_log_info(std::string &info) { LOG("%s", info.data()); }

  static Player *wasm_get_player_by_guid(const ObjectGuid guid) {
    return ObjectAccessor::FindPlayer(guid);
  }

  static void wasm_run_server_command(std::string &cmd) {
    sWorld->QueueCliCommand(
        new CliCommandHolder(nullptr, cmd.data(), nullptr, nullptr));
  }

  static void wasm_send_server_message(std::string &msg) {
    sWorld->SendServerMessage(SERVER_MSG_STRING, msg);
  }

  static void wasm_db_exec_sync(uint8_t db, std::string_view sql) {
    switch (db) {
    case 1:
      WorldDatabase.DirectExecute(sql);
      break;
    case 2:
      CharacterDatabase.DirectExecute(sql);
      break;
    case 3:
      LoginDatabase.DirectExecute(sql);
      break;
    }
  }

  static void wasm_db_query_async(uint8_t db, std::string_view sql,
                                  WASM_CALLBACK_PTR_ARGS) {
#define DO_ASYNC_QUERY(DATABASE)                                               \
  self->_query_callbacks.emplace_back(                                         \
      DATABASE.AsyncQuery(sql).WithCallback([self, func](QueryResult result) { \
        try {                                                                  \
          wasm::function<void(uint32_t, QueryResult)> cb{                      \
              WASM_CALLBACK_PTR_AS_FUNCTION_ARGS(*func)};                      \
          cb(func->func_ctx, result);                                          \
        } catch (const std::exception &error) {                                \
          LOG("%s", error.what());                                             \
        }                                                                      \
      }))

    auto self = _self();
    auto func = WASM_CALLBACK_PTR_MAKE_SHARED();
    switch (db) {
    case 1:
      DO_ASYNC_QUERY(WorldDatabase);
      break;
    case 2:
      DO_ASYNC_QUERY(CharacterDatabase);
      break;
    case 3:
      DO_ASYNC_QUERY(LoginDatabase);
      break;
    }
#undef DO_ASYNC_QUERY
  }

  static void wasm_db_prepare_statement(uint8_t db, uint32_t idx,
                                        std::string_view sql) {
    _self()->_stored_queries[idx] = (std::string)sql;
  }

  static PreparedStatementBase *wasm_db_get_prepared_statement(uint8_t db,
                                                               uint32_t idx) {
    return new PreparedStatementBase(0, 0);
  }

  static void wasm_db_query_async_prepared(uint8_t db,
                                           PreparedStatementBase *stmt,
                                           WASM_CALLBACK_PTR_ARGS) {
    auto sql = _self()->_stored_queries[stmt->GetIndex()];
    sql = sql_format(sql, stmt);
    wasm_db_query_async(db, sql, WASM_CALLBACK_PTR_FORWARD_ARGS);
  }

  static uint32_t wasm_get_spell_info_store_size() {
    return sSpellMgr->GetSpellInfoStoreSize();
  }

  static const SpellInfo *wasm_get_spell_info(uint32_t spell_id) {
    return sSpellMgr->GetSpellInfo(spell_id);
  }

  std::vector<NativeSymbol> _vm_symbols{
      WASM_EXPORT_CORE,
      WASM_EXPORT(wasm_reload_vm, this),
      WASM_EXPORT(wasm_thread_id, this),
      WASM_EXPORT(wasm_get_cwd, this),
      WASM_EXPORT(wasm_read_text_file, this),
      WASM_EXPORT(wasm_get_unixtime, this),
      WASM_EXPORT(wasm_register_timeout, this),
      WASM_EXPORT(wasm_remove_timeout, this),
      //   WASM_EXPORT(wasm_register_interval, this),
      //   WASM_EXPORT(wasm_remove_interval, this),
      WASM_EXPORT(wasm_log_info, this),
      WASM_EXPORT(wasm_get_player_by_guid, this),
      WASM_EXPORT(wasm_run_server_command, this),
      WASM_EXPORT(wasm_send_server_message, this),
      WASM_EXPORT(wasm_db_exec_sync, this),
      WASM_EXPORT(wasm_db_query_async, this),
      WASM_EXPORT(wasm_db_prepare_statement, this),
      WASM_EXPORT(wasm_db_get_prepared_statement, this),
      WASM_EXPORT(wasm_db_query_async_prepared, this),
      AURA_WASM_EXPORTS,
      CREATURE_WASM_EXPORTS,
      OBJECT_WASM_EXPORTS,
      PLAYER_WASM_EXPORTS,
      QUERYRESULT_WASM_EXPORTS,
      SPELL_WASM_EXPORTS,
      UNIT_WASM_EXPORTS,
      WORLDLOCATION_WASM_EXPORTS,
      WORLDOBJECT_WASM_EXPORTS,
  };

  wasm_vm _vm{_vm_symbols};
  std::mutex _vm_mutex;

  std::vector<std::vector<wasm::function_ptr>> _handlers{
      (size_t)ScriptEvent::_SIZE};

  void reload_vm(bool initial_setup = false) {
    std::scoped_lock _vm_lock{_vm_mutex};

    while (!_timeouts.empty()) {
      _timeouts.pop();
    }

    for (auto i = 0; i < _handlers.size(); i++) {
      _handlers[i].clear();
    }

    if (initial_setup) {
      _vm.run_setup();
    } else {
      _vm.run_reload();
    }

    for (auto i = 0; i < _handlers.size(); i++) {
      _vm.lookup_functions(ScriptEvent::_names[i], _handlers[i]);
    }
  }

  struct timeout_t {
    uint64_t when;
    std::function<void()> cb;

    timeout_t(uint64_t w, std::function<void()> &&c)
        : when(w), cb(std::move(c)) {}
  };
  struct is_greater_timeout {
    bool operator()(const timeout_t &x, const timeout_t &y) {
      return x.when > y.when;
    }
  };
  std::priority_queue<timeout_t, std::vector<timeout_t>, is_greater_timeout>
      _timeouts;

  uint64_t _time = 0;

  std::vector<QueryCallback> _query_callbacks;

  std::unordered_map<uint32_t, std::string> _stored_queries;

public:
  explicit WoWasm_Runtime()
      : AllCreatureScript(NAME), AllGameObjectScript(NAME), AllItemScript(NAME),
        AllMapScript(NAME), AllCommandScript(NAME), PlayerScript(NAME),
        WorldObjectScript(NAME), WorldScript(NAME) {}

  /*

    auto originalMenu = *pPlayer->PlayerTalkClass;
    pPlayer->PlayerTalkClass->ClearMenus();
    Push(pPlayer);
    Push(pCreature);
    Push(sender);
    Push(action);
    auto preventDefault = CallAllFunctionsBool(CreatureGossipBindings, key,
    true); if (!preventDefault) { *pPlayer->PlayerTalkClass = originalMenu;
    }*/

  SCRIPT_HANDLER_BOOL(CanCreatureGossipHello, (Player *, Creature *),
                      DEFAULT_FALSE);

  SCRIPT_HANDLER_BOOL(CanCreatureGossipSelect,
                      (Player *, Creature *, uint32_t, uint32_t),
                      DEFAULT_FALSE);

  SCRIPT_HANDLER_BOOL(CanCreatureGossipSelectCode,
                      (Player *, Creature *, uint32_t, uint32_t, const char *),
                      DEFAULT_FALSE);

  SCRIPT_HANDLER_VOID(OnCreatureAddWorld, (Creature *));

  SCRIPT_HANDLER_VOID(OnCreatureRemoveWorld, (Creature *));

  SCRIPT_HANDLER_BOOL(CanCreatureQuestAccept,
                      (Player *, Creature *, const Quest *), DEFAULT_FALSE);

  SCRIPT_HANDLER_BOOL(CanCreatureQuestReward,
                      (Player *, Creature *, const Quest *, uint32_t),
                      DEFAULT_FALSE);

  // CreatureAI *GetCreatureAI(Creature *creature) const override {
  //   if (CreatureAI *luaAI = sEluna->GetAI(creature))
  //     return luaAI;
  //   return nullptr;
  // }

  SCRIPT_HANDLER_VOID(OnGameObjectAddWorld, (GameObject *));

  SCRIPT_HANDLER_VOID(OnGameObjectRemoveWorld, (GameObject *));

  //   SCRIPT_HANDLER_VOID(OnGameObjectUpdate, (GameObject *, uint32_t));

  SCRIPT_HANDLER_BOOL(CanGameObjectGossipHello, (Player *, GameObject *),
                      DEFAULT_FALSE);

  SCRIPT_HANDLER_VOID(OnGameObjectDamaged, (GameObject *, Player *));

  SCRIPT_HANDLER_VOID(OnGameObjectDestroyed, (GameObject *, Player *));

  SCRIPT_HANDLER_VOID(OnGameObjectLootStateChanged,
                      (GameObject *, uint32_t, Unit *));

  SCRIPT_HANDLER_VOID(OnGameObjectStateChanged, (GameObject *, uint32_t));

  SCRIPT_HANDLER_BOOL(CanGameObjectQuestAccept,
                      (Player *, GameObject *, const Quest *), DEFAULT_FALSE);

  SCRIPT_HANDLER_BOOL(CanGameObjectGossipSelect,
                      (Player *, GameObject *, uint32_t, uint32_t),
                      DEFAULT_FALSE);

  SCRIPT_HANDLER_BOOL(CanGameObjectGossipSelectCode,
                      (Player *, GameObject *, uint32_t, uint32_t,
                       const char *),
                      DEFAULT_FALSE);

  SCRIPT_HANDLER_BOOL(CanGameObjectQuestReward,
                      (Player *, GameObject *, const Quest *, uint32_t),
                      DEFAULT_TRUE);

  //   GameObjectAI *GetGameObjectAI(GameObject *go) const override {
  //     sEluna->OnSpawn(go);
  //     return nullptr;
  //   }

  SCRIPT_HANDLER_BOOL(CanItemQuestAccept, (Player *, Item *, const Quest *),
                      DEFAULT_TRUE);

  //   TODO: implement SpellCastTargets
  //   SCRIPT_HANDLER_BOOL(CanItemUse, (Player *, Item *, const SpellCastTargets
  //   &), bool,
  //                  {SCRIPT_HANDLER_RETURN_BOOL({})});

  //   TODO: implement ItemTemplate
  //   SCRIPT_HANDLER_BOOL(CanItemExpire, (Player *, const ItemTemplate *),
  //   bool,
  //                  {SCRIPT_HANDLER_RETURN_BOOL({})});

  SCRIPT_HANDLER_BOOL(CanItemRemove, (Player *, Item *), DEFAULT_TRUE);

  SCRIPT_HANDLER_VOID(OnItemGossipSelect,
                      (Player *, Item *, uint32_t, uint32_t));

  SCRIPT_HANDLER_VOID(OnItemGossipSelectCode,
                      (Player *, Item *, uint32_t, uint32_t, const char *));

  //   SCRIPT_HANDLER_VOID(OnBeforeCreateInstanceScript,
  //                  (InstanceMap *, InstanceScript **, bool, std::string,
  //                  uint32));

  SCRIPT_HANDLER_VOID(OnDestroyInstance, (MapInstanced *, Map *));

  SCRIPT_HANDLER_VOID(OnCreateMap, (Map *));

  SCRIPT_HANDLER_VOID(OnDestroyMap, (Map *));

  bool OnTryExecuteCommand(ChatHandler &handler,
                           std::string_view cmdStr) override {
    Player *player =
        handler.IsConsole() ? nullptr : handler.GetSession()->GetPlayer();
    // If from console, player is NULL
    if (!player || player->GetSession()->GetSecurity() >= SEC_ADMINISTRATOR) {
      if (cmdStr == "reload wowasm") {
        sWorld->SendServerMessage(SERVER_MSG_STRING, "Reloading WoWasm...");
        reload_vm();
        return false;
      }
    }
    // TODO: forward to wasm
    return true;
  }

  SCRIPT_HANDLER_VOID(OnPlayerEnterAll, (Map *, Player *));

  SCRIPT_HANDLER_VOID(OnPlayerLeaveAll, (Map *, Player *));

  //   SCRIPT_HANDLER_VOID(OnMapUpdate, (Map *));

  SCRIPT_HANDLER_VOID(OnPlayerResurrect, (Player *, float, bool));

  //   SCRIPT_HANDLER_BOOL(CanPlayerUseChat,
  //                  (Player *, uint32_t, uint32_t, std::string &), bool,
  //                  {SCRIPT_HANDLER_RETURN_BOOL(DEFAULT_TRUE)});

  //   SCRIPT_HANDLER_BOOL(CanPlayerUseChat,
  //                  (Player *, uint32_t, uint32_t, std::string &, Player *),
  //                  bool, {SCRIPT_HANDLER_RETURN_BOOL(DEFAULT_TRUE)});

  //   SCRIPT_HANDLER_BOOL(CanPlayerUseChat,
  //                  (Player *, uint32_t, uint32_t, std::string &, Group *),
  //                  bool, {SCRIPT_HANDLER_RETURN_BOOL(DEFAULT_TRUE) });

  //   SCRIPT_HANDLER_BOOL(CanPlayerUseChat,
  //                  (Player *, uint32_t, uint32_t, std::string &, Guild *),
  //                  bool, {SCRIPT_HANDLER_RETURN_BOOL(DEFAULT_TRUE) });

  //   SCRIPT_HANDLER_BOOL(CanPlayerUseChat,
  //                  (Player *, uint32_t, uint32_t, std::string &, Channel *),
  //                  bool, {SCRIPT_HANDLER_RETURN_BOOL(DEFAULT_TRUE) });

  SCRIPT_HANDLER_VOID(OnLootItem, (Player *, Item *, uint32_t, ObjectGuid));

  SCRIPT_HANDLER_VOID(OnPlayerLearnTalents,
                      (Player *, uint32_t, uint32_t, uint32_t));

  //   TODO: implement InventoryResult
  //   SCRIPT_HANDLER_BOOL(CanUseItem, (Player *, const ItemTemplate *,
  //   InventoryResult &),
  //                  bool, {SCRIPT_HANDLER_RETURN_BOOL(DEFAULT_TRUE) });

  SCRIPT_HANDLER_VOID(OnEquip, (Player *, Item *, uint8_t, uint8_t, bool));

  SCRIPT_HANDLER_VOID(OnPlayerEnterCombat, (Player *, Unit *));

  SCRIPT_HANDLER_VOID(OnPlayerLeaveCombat, (Player *));

  SCRIPT_HANDLER_BOOL(CanRepopAtGraveyard, (Player *), DEFAULT_TRUE);

  SCRIPT_HANDLER_VOID(OnQuestAbandon, (Player *, uint32_t));

  SCRIPT_HANDLER_VOID(OnMapChanged, (Player *));

  SCRIPT_HANDLER_VOID(OnGossipSelect, (Player *, uint32_t, uint32_t, uint32_t));

  SCRIPT_HANDLER_VOID(OnGossipSelectCode,
                      (Player *, uint32_t, uint32_t, uint32_t, const char *));

  SCRIPT_HANDLER_VOID(OnPVPKill, (Player *, Player *));

  SCRIPT_HANDLER_VOID(OnCreatureKill, (Player *, Creature *));

  SCRIPT_HANDLER_VOID(OnPlayerKilledByCreature, (Creature *, Player *));

  SCRIPT_HANDLER_VOID(OnLevelChanged, (Player *, uint8_t));

  SCRIPT_HANDLER_VOID(OnFreeTalentPointsChanged, (Player *, uint32_t));

  SCRIPT_HANDLER_VOID(OnTalentsReset, (Player *, bool));

  SCRIPT_HANDLER_VOID(OnMoneyChanged, (Player *, int32_t &));

  SCRIPT_HANDLER_VOID(OnGiveXP, (Player *, uint32_t &, Unit *, uint8_t));

  SCRIPT_HANDLER_BOOL(OnReputationChange, (Player *, uint32_t, int32_t &, bool),
                      DEFAULT_TRUE);

  SCRIPT_HANDLER_VOID(OnDuelRequest, (Player *, Player *));

  SCRIPT_HANDLER_VOID(OnDuelStart, (Player *, Player *));

  SCRIPT_HANDLER_VOID(OnDuelEnd, (Player *, Player *, DuelCompleteType));

  SCRIPT_HANDLER_VOID(OnEmote, (Player *, uint32_t));

  SCRIPT_HANDLER_VOID(OnTextEmote, (Player *, uint32_t, uint32_t, ObjectGuid));

  SCRIPT_HANDLER_VOID(OnSpellCast, (Player *, Spell *, bool));

  SCRIPT_HANDLER_VOID(OnLogin, (Player *));

  SCRIPT_HANDLER_VOID(OnLogout, (Player *));

  SCRIPT_HANDLER_VOID(OnCreate, (Player *));

  //   SCRIPT_HANDLER_VOID(OnSave, (Player *));

  SCRIPT_HANDLER_VOID(OnDelete, (ObjectGuid, uint32_t));

  SCRIPT_HANDLER_VOID(OnBindToInstance, (Player *, Difficulty, uint32_t, bool));

  SCRIPT_HANDLER_VOID(OnUpdateArea, (Player *, uint32_t, uint32_t));

  SCRIPT_HANDLER_VOID(OnUpdateZone, (Player *, uint32_t, uint32_t));

  SCRIPT_HANDLER_VOID(OnFirstLogin, (Player *));

  SCRIPT_HANDLER_VOID(OnLearnSpell, (Player *, uint32_t));

  //   SCRIPT_HANDLER_VOID(OnAchiComplete, (Player *, const AchievementEntry
  //   *));

  SCRIPT_HANDLER_VOID(OnFfaPvpStateUpdate, (Player *, bool));

  SCRIPT_HANDLER_BOOL(CanInitTrade, (Player *, Player *), DEFAULT_TRUE);

  //   SCRIPT_HANDLER_BOOL(CanSendMail, (Player *, ObjectGuid, ObjectGuid,
  //   std::string &, std::string &, uint32_t, uint32_t, Item *), bool,
  //                  {SCRIPT_HANDLER_RETURN_BOOL(DEFAULT_TRUE)});

  //   SCRIPT_HANDLER_BOOL(CanJoinLfg, (Player *, uint8_t, lfg::LfgDungeonSet &,
  //   const std::string &), bool,
  //                  {SCRIPT_HANDLER_RETURN_BOOL(DEFAULT_TRUE)});

  SCRIPT_HANDLER_VOID(OnQuestRewardItem, (Player *, Item *, uint32_t));

  //   TODO: implement Roll
  //   SCRIPT_HANDLER_VOID(OnGroupRollRewardItem, (Player *, Item *, uint32_t,
  //   RollVote, Roll *));

  SCRIPT_HANDLER_VOID(OnCreateItem, (Player *, Item *, uint32_t));

  SCRIPT_HANDLER_VOID(OnStoreNewItem, (Player *, Item *, uint32_t));

  SCRIPT_HANDLER_VOID(OnPlayerCompleteQuest, (Player *, const Quest *));

  SCRIPT_HANDLER_BOOL(CanGroupInvite, (Player *, std::string &), DEFAULT_TRUE);

  SCRIPT_HANDLER_VOID(OnBattlegroundDesertion,
                      (Player *, BattlegroundDesertionType));

  SCRIPT_HANDLER_VOID(OnCreatureKilledByPet, (Player *, Creature *));

  void OnWorldObjectDestroy(WorldObject *object) override {
    // TODO: delete events for specific world object
    // delete object->elunaEvents;
    // object->elunaEvents = nullptr;
  }

  void OnWorldObjectCreate(WorldObject *object) override {
    // object->elunaEvents = nullptr;
  }

  void OnWorldObjectSetMap(WorldObject *object, Map * /*map*/) override {
    // if (!object->elunaEvents)
    //   object->elunaEvents = new ElunaEventProcessor(&Eluna::GEluna, object);
  }

  SCRIPT_HANDLER_VOID(OnOpenStateChange, (bool));

  void OnBeforeConfigLoad(bool reload) override {
    if (reload) {
      return;
    }

    auto wasm_scripts_dir =
        sConfigMgr->GetOption<std::string>("WoWasm.ScriptPath", "wasm_scripts");

    LOG("Searching .wasm files in `%s`", wasm_scripts_dir.data());

    _vm.set_directory(wasm_scripts_dir);

    reload_vm(true);
  }

  SCRIPT_HANDLER_VOID(OnAfterConfigLoad, (bool));

  SCRIPT_HANDLER_VOID(OnShutdownInitiate, (ShutdownExitCode, ShutdownMask));

  SCRIPT_HANDLER_VOID(OnShutdownCancel, ());

  void OnUpdate(uint32 diff) override {
    _time += diff;

    while (true) {
      if (_timeouts.empty()) {
        break;
      }

      const auto &timeout = _timeouts.top();
      if (_time < timeout.when) {
        break;
      }

      if (!_vm_mutex.try_lock()) {
        break;
      }

      timeout.cb();
      _timeouts.pop();

      _vm_mutex.unlock();
    }

    auto query_callback = _query_callbacks.begin();
    while (query_callback != _query_callbacks.end()) {
      if (!_vm_mutex.try_lock()) {
        break;
      }

      if (query_callback->InvokeIfReady()) {
        query_callback = _query_callbacks.erase(query_callback);
      }

      _vm_mutex.unlock();
    }
  }

  SCRIPT_HANDLER_VOID(OnStartup, ());

  SCRIPT_HANDLER_VOID(OnShutdown, ());

  SCRIPT_HANDLER_VOID(OnAfterUnloadAllMaps, ());

  SCRIPT_HANDLER_VOID(OnBeforeWorldInitialized, ());
};

// Add all scripts in one
void AddSC_WoWasmRuntime() { new WoWasm_Runtime(); }
