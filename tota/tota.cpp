#include <eosiolib/eosio.hpp>
#include <eosiolib/name.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;

class [[eosio::contract]] tota : public eosio::contract {

    public:
        tota(name receiver, name code, datastream<const char*> ds): contract(receiver, code, ds),
            _games(receiver, receiver.value), _histories(receiver, receiver.value) {}

    private:
        struct [[eosio::table]] game_info {
            uint64_t key;
            std::string game_name;
            uint64_t game_type;
            uint64_t start_time;
            uint64_t end_time;
            uint64_t result_time;
            uint64_t result;
            name team1;
            name team2;

            uint64_t primary_key() const { return key; }
        };

        struct [[eosio::table]] user_history {
            uint64_t key;
            name user;
            uint64_t game_key;
            uint64_t side;
            asset amount;
            uint64_t timestamp;

            uint64_t primary_key() const { return key; }
            uint64_t get_secondary_1() const { return user.value; }
        };
    
        typedef eosio::multi_index<name("games"), game_info> games_table;

        typedef eosio::multi_index<name("hitories"), user_history,
            indexed_by<name("byaccount"), const_mem_fun<user_history, uint64_t, &user_history::get_secondary_1>>
        > histories_table;

        games_table _games;
        histories_table _histories;
};

