@ 기능 추가
샌드웜이 스파이스를 배설할 때 배설하는 시간 카운트 (30초)


샌드웜 
1. 샌드웜의 이동 주기
  - sys_map 함수에서 ( turn_counter % 30 == 0 ) 일 때 sand_mob() 호출
  - 30틱마다 이동

====================================================================================

2. sand_mob 함수
void sand_mob() {
    // 하베스터 목록을 가져옴
    HarvesterList harvesters = find_harvesters();

    // 각 샌드웜별로 처리
    for (int i = 0; i < 2; i++) {
        Sandworm* sandworm = &sandworms[i];

        // 소화 중인지 체크
        if (sandworm->is_digesting) {
            // 소화 완료 시점인지 체크
            if (turn_counter >= sandworm->digestion_start_time + sandworm->digestion_time) {
                // 소화 완료 처리
                sandworm->is_digesting = false;
                map[0][sandworm->pos.row][sandworm->pos.column] = 'S';  // 스파이스 생성
                sandworm->harvester_eaten--;    // 먹은 수 감소
                sandworm->undigested_count--;   // 소화 대기 수 감소

                // 다음 소화 시작 여부 결정
                if (sandworm->undigested_count > 0 && rand() % 2 == 0) {
                    sandworm->is_digesting = true;
                    sandworm->digestion_start_time = turn_counter;
                }
            }
        }

        // 하베스터가 없으면 랜덤 이동
        if (harvesters.friend_count == 0 && harvesters.enemy_count == 0) {
            move_sandworm_toward_target(sandworm, {-1, -1});
            continue;
        }

        // 가장 가까운 하베스터 찾기
        POSITION closest = find_closest_harvester(sandworm, harvesters);
        
        // 그 하베스터를 향해 이동
        move_sandworm_toward_target(sandworm, closest);
    }
}

====================================================================================

3. move_sandworm_toward_target 함수
void move_sandworm_toward_target(Sandworm* sandworm, POSITION target) {
    // 현재 위치에서 샌드웜 제거
    map[1][sandworm->pos.row][sandworm->pos.column] = -1;

    if (target.row == -1) {  // 목표가 없는 경우 (랜덤 이동)
        // 4방향 이동 가능 위치 계산
        POSITION moves[4] = {
            {sandworm->pos.row - 1, sandworm->pos.column},    // 상
            {sandworm->pos.row + 1, sandworm->pos.column},    // 하
            {sandworm->pos.row, sandworm->pos.column - 1},    // 좌
            {sandworm->pos.row, sandworm->pos.column + 1}     // 우
        };

        // ID를 이용해 서로 다른 랜덤 동작
        int rand_index = (rand() + sandworm->id * 17) % 4;
        POSITION next_move = moves[rand_index];

        // 이동 가능하면 이동
        if (can_move_to(next_move)) {
            check_and_eat_harvester(sandworm, next_move);
            sandworm->prev_pos = sandworm->pos;
            sandworm->pos = next_move;
        }
    } else {  // 목표가 있는 경우
        // 4방향 중 목표에 가장 가까운 방향 선택
        POSITION best_move = find_best_move(sandworm, target);
        
        if (can_move_to(best_move)) {
            check_and_eat_harvester(sandworm, best_move);
            sandworm->prev_pos = sandworm->pos;
            sandworm->pos = best_move;
        }
    }

    // 새 위치에 샌드웜 표시
    map[1][sandworm->pos.row][sandworm->pos.column] = 'W';
}

====================================================================================

4. check_and_eat_harvester 함수
void check_and_eat_harvester(Sandworm* sandworm, POSITION pos) {
    if (map[1][pos.row][pos.column] == 'H') {
        sandworm->harvester_eaten++;     // 먹은 수 증가
        sandworm->undigested_count++;    // 소화 대기 수 증가

        // 현재 소화 중이 아니고 50% 확률이면 소화 시작
        if (!sandworm->is_digesting && rand() % 2 == 0) {
            sandworm->is_digesting = true;
            sandworm->digestion_start_time = turn_counter;
        }
    }
}
