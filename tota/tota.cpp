#include <eosiolib/eosio.hpp>
#include <eosiolib/name.hpp>
#include <eosiolib/asset.hpp>
#include "eosio.token.hpp"

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
                row.team1_asset = asset(0, symbol(symbol_code("EOS"), 4));
                row.team2_asset = asset(0, symbol(symbol_code("EOS"), 4));
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

    [[eosio::action]]
    void insertcoin(name user, asset amount, uint64_t game_key, uint64_t side) {
        require_auth(user);
        const symbol sym(symbol_code("EOS"), 4);
        asset eos_get = eosio::token::get_balance(name("eosio.token"), user, sym.code());
        if(amount.amount <= eos_get.amount) {
            games_table games(_code, _code.value);
            auto game = games.find(game_key);
            if(game != games.end()) {
                games.modify(game, user, [&]( auto& row ) {
                    action(
                        permission_level{user, name("active")},
                        name("eosio.token"),
                        name("transfer"),
                        std::make_tuple(user, get_self(), amount, std::string("attend tota"))
                    ).send();
                    auto timestamp = current_time() / uint64_t(1000);
                    histories_table histories(_code, _code.value);
                    histories.emplace(user, [&](auto& row) {
                        row.key = histories.available_primary_key();
                        row.user = user;
                        row.game_key = game_key;
                        row.side = side;
                        row.amount = amount;
                        row.timestamp = timestamp;
                        row.status = 0;
                    });
                    if(side == 1) {
                        row.team1_asset += amount;
                    } else if(side == 2) {
                        row.team2_asset += amount;
                    }
                });
            }
        }
    }

    [[eosio::action]]
    void dropcoin(name user, uint64_t game_key, uint64_t history_key) {
        require_auth(user);
        histories_table histories(_code, _code.value);
        auto history = histories.find(history_key);
        if(history != histories.end()) {
            if(user == history->user && history->status == 0) {
                games_table games(_code, _code.value);
                auto game = games.find(game_key);
                if(game != games.end()) {
                    auto result = game->result;
                    if(result == 0) {
                        return;
                    }
                    if(history->side != result) {
                        histories.modify(history, user, [&](auto& row) {
                            row.status = 1;
                        });
                    } else {
                        auto team1_asset = game->team1_asset;
                        auto team2_asset = game->team2_asset;
                        asset receiving =  history->amount / 1000 * 999;
                        if(result == 1) {
                            receiving = (receiving * (team1_asset.amount + team2_asset.amount)) / team1_asset.amount;
                        } else if(result == 2) {
                            receiving = (receiving * (team1_asset.amount + team2_asset.amount)) / team2_asset.amount;
                        }

                        action(
                            permission_level{get_self(), name("active")},
                            name("eosio.token"),
                            name("transfer"),
                            std::make_tuple(get_self(), user, receiving, std::string("receiving from tota"))
                        ).send();

                        histories.modify(history, user, [&](auto& row) {
                            row.status = 1;
                        });
                    }
                }
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
            asset team1_asset;
            asset team2_asset;

            uint64_t primary_key() const { return key; }
        };

        struct [[eosio::table]] user_history {
            uint64_t key;
            name user;
            uint64_t game_key;
            uint64_t side;
            asset amount;
            uint64_t timestamp;
            uint64_t status;

            uint64_t primary_key() const { return key; }
            uint64_t get_secondary_1() const { return user.value; }
        };
    
        typedef eosio::multi_index<name("games2"), game_info> games_table;

        typedef eosio::multi_index<name("histories2"), user_history,
            indexed_by<name("byaccount"), const_mem_fun<user_history, uint64_t, &user_history::get_secondary_1>>
        > histories_table;
};

EOSIO_DISPATCH(tota, (insertgame)(pushresult)(insertcoin)(dropcoin))

