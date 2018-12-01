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
        eosio_assert(user.value == _code.value, "This action should be occured by contract permission");
        games_table games(_code, _code.value);
        games.emplace(_code, [&](auto& row) {
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

    [[eosio::action]]
    void pushresult(name user, uint64_t game_key, uint64_t result) {
        require_auth(user);
        eosio_assert(user.value == _code.value, "This action should be occured by contract permission");
        games_table games(_code, _code.value);
        auto game = games.find(game_key);
        eosio_assert(game != games.end(), "Game key does not match!");
        games.modify(game, _code, [&]( auto& row ) {
            row.result = result;
        });
    }

    [[eosio::action]]
    void insertcoin(name user, asset amount, uint64_t game_key, name _side) {
        require_auth(user);
        eosio_assert(_side == name("totaproxyno1") || _side == name("totaproxyno2"), "You bet at wrong side!");
        uint64_t side = 2;
        if(_side == name("totaproxyno1")) {
            side = 1;
        }
        const symbol sym(symbol_code("EOS"), 4);
        eosio_assert(amount.symbol == sym, "Your input balance is not match with symbol 'EOS'.");
        asset eos_get = eosio::token::get_balance(name("eosio.token"), user, sym.code());
        eosio_assert(amount.amount <= eos_get.amount, "Your EOS balance is too low.");
        games_table games(_code, _code.value);
        auto game = games.find(game_key);
        eosio_assert(game != games.end(), "Game key does not match!");
        auto timestamp = current_time() / uint64_t(1000);
        eosio_assert(timestamp >= game->start_time, "You can`t bet now, because time is ealier.");
        eosio_assert(timestamp <= game->end_time, "You can`t bet now, because time has passed.");
        games.modify(game, _code, [&](auto& row) {
            histories_table histories(_code, _code.value);
            auto iter = histories.begin();
            while(iter != histories.end()) {
                if(iter->user == user && iter->game_key == game_key) {
                    eosio_assert(iter->side == side, "If you want to bet more, you should bet at same side!");
                    receiveEOS(user, amount);
                    histories.modify(iter, _code, [&](auto& history) {
                        history.timestamp = timestamp;
                        history.amount += amount;
                    });
                    break;
                }
                ++iter;
            }
            if(iter == histories.end()) {
                receiveEOS(user, amount);
                histories.emplace(_code, [&](auto& history) {
                    history.key = histories.available_primary_key();
                    history.user = user;
                    history.game_key = game_key;
                    history.side = side;
                    history.amount = amount;
                    history.timestamp = timestamp;
                    history.status = 0;
                });
            }
            if(side == 1) {
                row.team1_asset += amount;
            } else if(side == 2) {
                row.team2_asset += amount;
            }
        });
    }

    [[eosio::action]]
    void dropcoin(name user, uint64_t game_key) {
        require_auth(user);
        histories_table histories(_code, _code.value);
        auto history = histories.begin();
        while(history != histories.end()) {
            if(history->user == user && history->game_key == game_key) {
                break;
            }
            ++history;
        }
        eosio_assert(history != histories.end(), "History key does not match!");
        eosio_assert(user == history->user, "You`re not owner of this history!");
        eosio_assert(history->status == 0, "Already refunded");
        games_table games(_code, _code.value);
        auto game = games.find(game_key);
        eosio_assert(game != games.end(), "Game key does not match!");
        auto result = game->result;
        eosio_assert(current_time() / uint64_t(1000) > game->result_time, "Result is not yet available!");
        eosio_assert(result != 0, "Result is not yet available!");
        if(history->side != result) {
            histories.modify(history, _code, [&](auto& row) {
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

            sendEOS(user, receiving);

            histories.modify(history, _code, [&](auto& row) {
                row.status = 1;
            });
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
            uint64_t by_account() const { return user.value; }
        };
    
        typedef eosio::multi_index<name("games2"), game_info> games_table;

        typedef eosio::multi_index<name("histories2"), user_history,
            indexed_by<name("byaccount"), const_mem_fun<user_history, uint64_t, &user_history::by_account>>
        > histories_table;

        void receiveEOS(name user, asset amount) {
            action(
                permission_level{user, name("active")},
                name("eosio.token"),
                name("transfer"),
                std::make_tuple(user, get_self(), amount, std::string("attend tota"))
            ).send();
        }

        void sendEOS(name user, asset amount) {
            action(
                permission_level{get_self(), name("active")},
                name("eosio.token"),
                name("transfer"),
                std::make_tuple(get_self(), user, amount, std::string("receiving from tota"))
            ).send();
        }
};

EOSIO_DISPATCH(tota, (insertgame)(pushresult)(insertcoin)(dropcoin))

