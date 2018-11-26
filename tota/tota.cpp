#include <eosiolib/eosio.hpp>
#include <eosiolib/name.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;

class [[eosio::contract]] tota : public eosio::contract {

    public:
        tota(name receiver, name code, datastream<const char*> ds): contract(receiver, code, ds) {}

    [[eosio::action]]
    void insertgame(name user, std::string game_name, uint64_t game_type, uint64_t start_time,
                     uint64_t end_time, uint64_t result_time, std::string team1, std::string team2) {
        require_auth(user);
        if(user.to_string() == "totagamelist") {
            games_table games(_code, _code.value);
            games.emplace(user, [&](auto& row) {
                row.key = games.available_primary_key();
                row.game_name = game_name;
                row.game_type = game_type;
                row.start_time = start_time;
                row.end_time = end_time;
                row.result_time = result_time;
                row.result = 0;
                row.team1 = name(team1);
                row.team2 = name(team2);
            });
        }
    }

    [[eosio::action]]
    void pushresult(name user, uint64_t game_key, uint64_t result) {
        require_auth(user);
        if(user.to_string() == "totagamelist") {
            games_table games(_code, _code.value);
            auto game = games.find(game_key);
            if(game != games.end()) {
                games.modify(game, user, [&]( auto& row ) {
                    row.result = result;
                });
            }
        }
    }

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
};

EOSIO_DISPATCH(tota, (insertgame), (pushresult))

