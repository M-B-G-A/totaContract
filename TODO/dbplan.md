DB table structure(at now)
==
table game_info
===
**key** (uint64_t) 숫자

**game_name** (std::string) 스트링

**game_type** (uint64_t) 숫자, 아직 별의미 없을듯

**start_time** (uint64_t) 시간, millis형식

**end_time** (uint64_t) 시간, millis형식

**result_time** (uint64_t) 시간, millis형식

**result** (uint64_t) 숫자, 이긴팀번호 (0: 결과 미입력, 1: 1팀 승리, 2: 2팀 승리, 3: 무승부)

**team1** (name) 팀 이름

**team2** (name) 팀 이름

**team1_asset** (asset) 팀1에 누적된 자산

**team2_asset** (asset) 팀2에 누적된 자산

table user_history
===
**key** (uint64_t) 숫자

**user** (name) 

**game_key** (uint64_t) 숫자

**side** (uint64_t) 베팅 사이드 (1 or 2)

**amount** (asset) 베팅

**timestamp** (uint64_t) 시간 millis

**status** (uint64_t) 환불 상태(0: 환불 안됨, 1: 환불 됨)

Action
===
**게임 넣기** : 컨트랙트 유저 권한(totatestgame와 같이)

insertgame(name user, std::string game_name, uint64_t game_type, uint64_t start_time,
        uint64_t end_time, uint64_t result_time, std::string team1, std::string team2)

**게임 결과 넣기** : 컨트랙트 유저 권한(totatestgame와 같이)

pushresult(name user, uint64_t game_key, uint64_t result)


**베팅하기** : 일반 유저 권한, 컨트랙트의 가상권한 eosio.code 승인 필요

insertcoin(name user, asset amount, uint64_t game_key, uint64_t side)

**배당받기** : 일반 유저 권한

dropcoin(name user, uint64_t game_key)


위에서 name의 경우 `"totatestgame"`와 같이 12글자 문자열이고, std::string의 경우 그냥 제한없는 문자열이고, uint64_t의 경우 숫자, asset의 경우 `"12.0000 EOS"`와 같이 소숫점 4자리까지 넣고 한칸 띄고 EOS를 넣는 문자열입니다.

기타
===
**배당율 계산하기**(현재)
팀1의 경우 : * 0.999 * (team1_asset.amount + team2_asset.amount) / team1_asset.amount

팀2의 경우 : * 0.999 * (team1_asset.amount + team2_asset.amount) / team2_asset.amount
