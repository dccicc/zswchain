#include <eosio/state_history/create_deltas.hpp>
#include <eosio/state_history/serialization.hpp>
#include <eosio/chain/backing_store/db_combined.hpp>

namespace eosio {
namespace state_history {

template <typename T>
bool include_delta(const T& old, const T& curr) {
   return true;
}

bool include_delta(const chain::table_id_object& old, const chain::table_id_object& curr) {
   return old.payer != curr.payer;
}

bool include_delta(const chain::resource_limits::resource_limits_object& old,
                   const chain::resource_limits::resource_limits_object& curr) {
   return                                   //
       old.net_weight != curr.net_weight || //
       old.cpu_weight != curr.cpu_weight || //
       old.ram_bytes != curr.ram_bytes;
}

bool include_delta(const chain::resource_limits::resource_limits_state_object& old,
                   const chain::resource_limits::resource_limits_state_object& curr) {
   return                                                                                       //
       old.average_block_net_usage.last_ordinal != curr.average_block_net_usage.last_ordinal || //
       old.average_block_net_usage.value_ex != curr.average_block_net_usage.value_ex ||         //
       old.average_block_net_usage.consumed != curr.average_block_net_usage.consumed ||         //
       old.average_block_cpu_usage.last_ordinal != curr.average_block_cpu_usage.last_ordinal || //
       old.average_block_cpu_usage.value_ex != curr.average_block_cpu_usage.value_ex ||         //
       old.average_block_cpu_usage.consumed != curr.average_block_cpu_usage.consumed ||         //
       old.total_net_weight != curr.total_net_weight ||                                         //
       old.total_cpu_weight != curr.total_cpu_weight ||                                         //
       old.total_ram_bytes != curr.total_ram_bytes ||                                           //
       old.virtual_net_limit != curr.virtual_net_limit ||                                       //
       old.virtual_cpu_limit != curr.virtual_cpu_limit;
}

bool include_delta(const chain::account_metadata_object& old, const chain::account_metadata_object& curr) {
   return                                               //
       old.name != curr.name ||                         //
       old.is_privileged() != curr.is_privileged() ||   //
       old.last_code_update != curr.last_code_update || //
       old.vm_type != curr.vm_type ||                   //
       old.vm_version != curr.vm_version ||             //
       old.code_hash != curr.code_hash;
}

bool include_delta(const chain::code_object& old, const chain::code_object& curr) { //
   return false;
}

bool include_delta(const chain::protocol_state_object& old, const chain::protocol_state_object& curr) {
   return old.activated_protocol_features != curr.activated_protocol_features;
}

std::vector<table_delta> create_deltas(const chainbase::database& db, bool full_snapshot) {
   std::vector<table_delta>                          deltas;
   const auto&                                       table_id_index = db.get_index<chain::table_id_multi_index>();
   std::map<uint64_t, const chain::table_id_object*> removed_table_id;
   for (auto& rem : table_id_index.last_undo_session().removed_values)
      removed_table_id[rem.id._id] = &rem;

   auto get_table_id = [&](uint64_t tid) -> const chain::table_id_object& {
      auto obj = table_id_index.find(tid);
      if (obj)
         return *obj;
      auto it = removed_table_id.find(tid);
      EOS_ASSERT(it != removed_table_id.end(), chain::plugin_exception, "can not found table id ${tid}", ("tid", tid));
      return *it->second;
   };

   auto pack_row          = [&](auto& row) { return fc::raw::pack(make_history_serial_wrapper(db, row)); };
   auto pack_contract_row = [&](auto& row) {
      return fc::raw::pack(make_history_context_wrapper(db, get_table_id(row.t_id._id), row));
   };

   auto process_table = [&](auto* name, auto& index, auto& pack_row) {
      if (full_snapshot) {
         if (index.indices().empty())
            return;
         deltas.push_back({});
         auto& delta = deltas.back();
         delta.name  = name;
         for (auto& row : index.indices())
            delta.rows.obj.emplace_back(true, pack_row(row));
      } else {
         auto undo = index.last_undo_session();
         if (undo.old_values.empty() && undo.new_values.empty() && undo.removed_values.empty())
            return;
         deltas.push_back({});
         auto& delta = deltas.back();
         delta.name  = name;
         for (auto& old : undo.old_values) {
            auto& row = index.get(old.id);
            if (include_delta(old, row))
               delta.rows.obj.emplace_back(true, pack_row(row));
         }
         for (auto& old : undo.removed_values)
            delta.rows.obj.emplace_back(false, pack_row(old));
         for (auto& row : undo.new_values) {
            delta.rows.obj.emplace_back(true, pack_row(row));
         }
      }
   };

   process_table("account", db.get_index<chain::account_index>(), pack_row);
   process_table("account_metadata", db.get_index<chain::account_metadata_index>(), pack_row);
   process_table("code", db.get_index<chain::code_index>(), pack_row);

   process_table("contract_table", db.get_index<chain::table_id_multi_index>(), pack_row);
   process_table("contract_row", db.get_index<chain::key_value_index>(), pack_contract_row);
   process_table("contract_index64", db.get_index<chain::index64_index>(), pack_contract_row);
   process_table("contract_index128", db.get_index<chain::index128_index>(), pack_contract_row);
   process_table("contract_index256", db.get_index<chain::index256_index>(), pack_contract_row);
   process_table("contract_index_double", db.get_index<chain::index_double_index>(), pack_contract_row);
   process_table("contract_index_long_double", db.get_index<chain::index_long_double_index>(), pack_contract_row);

   process_table("key_value", db.get_index<chain::kv_index>(), pack_row);

   process_table("global_property", db.get_index<chain::global_property_multi_index>(), pack_row);
   process_table("generated_transaction", db.get_index<chain::generated_transaction_multi_index>(), pack_row);
   process_table("protocol_state", db.get_index<chain::protocol_state_multi_index>(), pack_row);

   process_table("permission", db.get_index<chain::permission_index>(), pack_row);
   process_table("permission_link", db.get_index<chain::permission_link_index>(), pack_row);

   process_table("resource_limits", db.get_index<chain::resource_limits::resource_limits_index>(), pack_row);
   process_table("resource_usage", db.get_index<chain::resource_limits::resource_usage_index>(), pack_row);
   process_table("resource_limits_state", db.get_index<chain::resource_limits::resource_limits_state_index>(),
                 pack_row);
   process_table("resource_limits_config", db.get_index<chain::resource_limits::resource_limits_config_index>(),
                 pack_row);

   return deltas;
}

class rocksdb_db_row_receiver {
public:

   rocksdb_db_row_receiver(std::vector<table_delta> &deltas, const chainbase::database& db) : deltas(deltas), db_(db) {}

   void add_row(const chain::backing_store::table_id_object_view& row) {
      deltas[get_delta_index("contract_table")].rows.obj.emplace_back(true, fc::raw::pack(make_history_serial_wrapper(db_, row)));
      current_table = row;
   }

   void add_row(const chain::backing_store::primary_index_view& row) {
      deltas[get_delta_index("contract_row")].rows.obj.emplace_back(true, fc::raw::pack(make_history_context_wrapper(db_, current_table, row)));
   }

   void add_row(const chain::backing_store::secondary_index_view<uint64_t>& row) {
      deltas[get_delta_index("contract_index64")].rows.obj.emplace_back(true, fc::raw::pack(make_history_context_wrapper(db_, current_table, row)));
   }

   void add_row(const chain::backing_store::secondary_index_view<chain::uint128_t>& row) {
      deltas[get_delta_index("contract_index128")].rows.obj.emplace_back(true, fc::raw::pack(make_history_context_wrapper(db_, current_table, row)));
   }

   void add_row(const chain::backing_store::secondary_index_view<chain::key256_t>& row) {
      deltas[get_delta_index("contract_index256")].rows.obj.emplace_back(true, fc::raw::pack(make_history_context_wrapper(db_, current_table, row)));
   }

   void add_row(const chain::backing_store::secondary_index_view<float64_t>& row) {
      deltas[get_delta_index("contract_index_double")].rows.obj.emplace_back(true, fc::raw::pack(make_history_context_wrapper(db_, current_table, row)));
   }

   void add_row(const chain::backing_store::secondary_index_view<float128_t>& row) {
      deltas[get_delta_index("contract_index_long_double")].rows.obj.emplace_back(true, fc::raw::pack(make_history_context_wrapper(db_, current_table, row)));
   }

   void add_row(fc::unsigned_int x) {

   }

private:
   int get_delta_index(std::string name){
      if(!m.count(name)){
         deltas.push_back({});
         deltas.back().name = name;
         m[name] = deltas.size() - 1;
      }
      return m[name];
   }

   std::map<std::string, int> m;
   std::vector<table_delta> &deltas;
   const chainbase::database& db_;

   chain::backing_store::table_id_object_view current_table;
};

std::vector<table_delta> create_deltas_rocksdb(const chainbase::database& db, const eosio::chain::kv_undo_stack_ptr &kv_undo_stack, bool full_snapshot){
   std::vector<table_delta>                          deltas;

   if(full_snapshot) {
      deltas.push_back({});
      auto& delta = deltas.back();
      delta.name  = "key_value";
      struct rocksdb_kv_row_receiver {
         rocksdb_kv_row_receiver(std::vector<std::pair<bool, bytes>>& obj, const chainbase::database& db) : obj_(obj), db_(db) {}
         void add_row(const chain::kv_object_view& row) {
            auto pack_row = [&](auto& row) { return fc::raw::pack(make_history_serial_wrapper(db_, row)); };
            obj_.emplace_back(true, pack_row(row));
         }

         std::vector<std::pair<bool, bytes>>& obj_;
         const chainbase::database& db_;
      };
//      auto rocksdb_kv_row_receiver = [&](uint64_t contract, const char* key, std::size_t key_size, const char* value, std::size_t value_size) {
//         auto pack_row = [&](auto& row) { return fc::raw::pack(make_history_serial_wrapper(db, row)); };
//
//         // In KV RocksDB, payer and actual data are packed together.
//         // Extract them.
//         chain::backing_store::payer_payload pp(value, value_size);
//         chain::kv_object_view row{chain::name(contract),
//                                   {{key, key + key_size}},
//                                   {{pp.value, pp.value + pp.value_size}},
//                                   pp.payer};
//
//         delta.rows.obj.emplace_back(true, pack_row(row));
//      };
      rocksdb_kv_row_receiver kv_receiver(delta.rows.obj, db);
      chain::backing_store::rocksdb_contract_kv_table_writer kv_writer(kv_receiver);
      auto begin_key = eosio::session::shared_bytes(&chain::backing_store::rocksdb_contract_kv_prefix, 1);
      auto end_key = begin_key.next();
      chain::backing_store::walk_rocksdb_entries_with_prefix(kv_undo_stack, begin_key, end_key, kv_writer);

      rocksdb_db_row_receiver db_receiver(deltas, db);
      using table_collector = chain::backing_store::rocksdb_whole_db_table_collector<rocksdb_db_row_receiver>;
      table_collector table_collector_receiver(db_receiver);
      chain::backing_store::rocksdb_contract_db_table_writer writer(table_collector_receiver);

      begin_key = eosio::session::shared_bytes(&chain::backing_store::rocksdb_contract_db_prefix, 1);
      end_key = begin_key.next();
      chain::backing_store::walk_rocksdb_entries_with_prefix(kv_undo_stack, begin_key, end_key, writer);
   } else {
      auto &session = kv_undo_stack->top();
      auto updated_keys = session.updated_keys();
      auto deleted_keys = session.deleted_keys();

      // TODO: get data from updated_keys and deleted_keys

      struct rocksdb_receiver_all {
         rocksdb_receiver_all(std::vector<table_delta> &deltas, const chainbase::database& db) : deltas(deltas), db_(db) {};

         void add_row(const chain::kv_object_view& row) {
            deltas[get_delta_index("key_value")].rows.obj.emplace_back(present, fc::raw::pack(make_history_serial_wrapper(db_, row)));
         }

         void add_row(const chain::backing_store::table_id_object_view& row) {
            table_ = row;
         }

         void add_row(const chain::backing_store::primary_index_view& row) {
            deltas[get_delta_index("contract_row")].rows.obj.emplace_back(present, fc::raw::pack(make_history_context_wrapper(db_, *table_, row)));
            table_ = {};
         }

         void add_row(const chain::backing_store::secondary_index_view<uint64_t>& row) {
            deltas[get_delta_index("contract_index64")].rows.obj.emplace_back(present, fc::raw::pack(make_history_context_wrapper(db_, *table_, row)));
            table_ = {};
         }

         void add_row(const chain::backing_store::secondary_index_view<chain::uint128_t>& row) {
            deltas[get_delta_index("contract_index128")].rows.obj.emplace_back(present, fc::raw::pack(make_history_context_wrapper(db_, *table_, row)));
            table_ = {};
         }

         void add_row(const chain::backing_store::secondary_index_view<chain::key256_t>& row) {
            deltas[get_delta_index("contract_index256")].rows.obj.emplace_back(present, fc::raw::pack(make_history_context_wrapper(db_, *table_, row)));
            table_ = {};
         }

         void add_row(const chain::backing_store::secondary_index_view<float64_t>& row) {
            deltas[get_delta_index("contract_index_double")].rows.obj.emplace_back(present, fc::raw::pack(make_history_context_wrapper(db_, *table_, row)));
            table_ = {};
         }

         void add_row(const chain::backing_store::secondary_index_view<float128_t>& row) {
            deltas[get_delta_index("contract_index_long_double")].rows.obj.emplace_back(present, fc::raw::pack(make_history_context_wrapper(db_, *table_, row)));
            table_ = {};
         }

         void add_row(fc::unsigned_int x) {

         }

         void maybe_process_table(){
            if(table_){
               deltas[get_delta_index("contract_table")].rows.obj.emplace_back(present, fc::raw::pack(make_history_serial_wrapper(db_, *table_)));
               table_ = {};
            }
         }

         int get_delta_index(std::string name){
            if(!m.count(name)){
               deltas.push_back({});
               deltas.back().name = name;
               m[name] = deltas.size() - 1;
            }
            return m[name];
         }

         std::map<std::string, int> m;
         std::vector<table_delta> &deltas;
         const chainbase::database& db_;
         bool present{false};

         std::optional<chain::backing_store::table_id_object_view> table_;
         } rec_all(deltas, db);

         for(auto &deleted_key: session.deleted_keys()) {
            std::visit([&](auto* p) {
               chain::backing_store::process_rocksdb_entry(*p, deleted_key, rec_all);
            }, session.parent());
            rec_all.maybe_process_table();
         }

         rec_all.present = true;
         rec_all.table_ = {};
         for(auto &updated_key: session.updated_keys()) {
            chain::backing_store::process_rocksdb_entry(session, updated_key, rec_all);
            rec_all.maybe_process_table();
         }



   }

   return deltas;
}

std::vector<table_delta> create_deltas(const chain::combined_database& db, bool full_snapshot){
   auto &chainbase_db = db.get_db();
   auto &kv_undo_stack = db.get_kv_undo_stack();

   std::vector<table_delta> deltas = create_deltas(chainbase_db, full_snapshot);

   if(kv_undo_stack && chainbase_db.get<chain::kv_db_config_object>().backing_store == chain::backing_store_type::ROCKSDB) {
      auto deltas_rocksdb = create_deltas_rocksdb(chainbase_db, kv_undo_stack, full_snapshot);
      deltas.insert( deltas.end(), deltas_rocksdb.begin(), deltas_rocksdb.end() );
   }

   return deltas;
}

} // namespace state_history
} // namespace eosio
