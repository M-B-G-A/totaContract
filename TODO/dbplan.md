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


table user_history
===
**key** (uint64_t) 숫자

**user** (name) 

**game_key** (uint64_t) 숫자

**side** (uint64_t) 베팅 사이드 (1 or 2)

**amount** (asset) 베팅

**timestamp** (uint64_t) 시간 millis

