#include "gameplay.cpp"

void cardgame::login(account_name username) {
    require_auth(username);

    auto user_iterator = _users.find(username);
    if (user_iterator == _users.end()) {
        user_iterator = _users.emplace(username, [&](auto& newuser) {
            new_user.name = username;
        });
    }
}

EOSIO_ABI(cardgame, (login))