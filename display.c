/*
*  display.c:
* 화면에 게임 정보를 출력
* 맵, 커서, 시스템 메시지, 정보창, 자원 상태 등등
* io.c에 있는 함수들을 사용함
*/

#include "display.h"
#include "io.h"

// 출력할 내용들의 좌상단(topleft) 좌표
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };
const POSITION sys_pos = { 20, 0 };
const POSITION status_pos = { 1, 62 };
const POSITION cmd_pos = { 20, 62 };
//이 밑으로 출력 좌표
const POSITION sta_map_pos = { 2, 63 };
const POSITION cmd_map_pos = { 21,63 };
const POSITION sys_map_pos = { 21, 1 };
//버퍼
char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
//시스템 메세지창
char sys_backbuf[SYS_HEIGHT][SYS_WIDTH] = { 0 };
char sys_frontbuf[SYS_HEIGHT][SYS_WIDTH] = { 0 };
//유닛 상태창
char status_backbuf[STATUS_HEIGHT][STATUS_WIDTH] = { 0 };
char status_frontbuf[STATUS_HEIGHT][STATUS_WIDTH] = { 0 };
//명령창
char cmd_backbuf[CMD_HEIGHT][CMD_WIDTH] = { 0 };
char cmd_frontbuf[CMD_HEIGHT][CMD_WIDTH] = { 0 };
//위치의 색을 저장하는 colorbuf
int colorbuf[MAP_HEIGHT][MAP_WIDTH] = { 7 };

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);
void sys_map(char system_map[N_LAYER][SYS_HEIGHT][SYS_WIDTH]);
void sta_map(char status_map[N_LAYER][STATUS_HEIGHT][STATUS_WIDTH]);
void cmd_map(char command_map[N_LAYER][CMD_HEIGHT][CMD_WIDTH]);
//display_map에 배치된 색
void formation(int i, int j, char backbuf[MAP_HEIGHT][MAP_WIDTH]);
//창에 있는 문자 지우는 함수
void clear_sta_map_area();
//피타고라스 (H, 샌드웜)
void sand_mob(POSITION friend_h, POSITION enemy_h);
//하베스터 좌표 찾는 함수
bool find_h_positions(POSITION* friend_h, POSITION* enemy_h);
//H 색 저장 배열
int color_map[MAP_HEIGHT][MAP_WIDTH] = { 7 }; //7 : 기본 색
//샌드웜 속도
int turn_counter = 0;
typedef struct {
	int id;           // 고유 ID
	POSITION pos;     // 현재 위치
} Sandworm;

Sandworm sandworms[2] = {
	{.id = 1, .pos = {3, 8} },  // 첫 번째 샌드웜 (좌상단)
	{.id = 2, .pos = {13, 50} }  // 두 번째 샌드웜 (우하단)
};

void display(
	RESOURCE resource,
	char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
	CURSOR cursor,
	char system_map[N_LAYER][SYS_HEIGHT][SYS_WIDTH],
	char status_map[N_LAYER][STATUS_HEIGHT][STATUS_WIDTH],
	char command_map[N_LAYER][CMD_HEIGHT][CMD_WIDTH]
)
{
	display_resource(resource);
	display_map(map);
	display_cursor(cursor);
	sys_map(system_map);
	sta_map(status_map);
	cmd_map(command_map);
	// display_system_message()
	// display_object_info()
	// display_commands()
	// ...
}

void display_resource(RESOURCE resource) {
	set_color(COLOR_RESOURCE);
	gotoxy(resource_pos);
	printf("spice = %d/%d, population=%d/%d\n",
		resource.spice, resource.spice_max,
		resource.population, resource.population_max
	);
}

// subfunction of draw_map()
void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]) {
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			for (int k = 0; k < N_LAYER; k++) {
				if (src[k][i][j] >= 0) {
					dest[i][j] = src[k][i][j];
				}
			}
		}
	}
}

void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	project(map, backbuf);

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (frontbuf[i][j] != backbuf[i][j]) {
				POSITION pos = { i, j };
				formation(i, j, backbuf); // 기본 색
				gotoxy(padd(map_pos, pos));  // 위치 이동
				printf("%c", backbuf[i][j]);  // 문자 출력

				//샌드웜 좌표 확인 및 갱신
				// 이거 아직 사용 X, 샌드웜 좌표먼저 받아오고 움직인다음에 갱신
				if (backbuf[i][j] == 'W') { // 샌드웜 표시가 'W'라고 가정
					for (int k = 0; k < 2; k++) {
						if (sandworms[k].pos.row == i && sandworms[k].pos.column == j) {
							sandworms[k].pos = pos;  // 위치 갱신
						}
					}
				}


			}
			frontbuf[i][j] = backbuf[i][j];
		}
	}
}

// frontbuf[][]에서 커서 위치의 문자를 색만 바꿔서 그대로 다시 출력

void display_cursor(CURSOR cursor) {
	POSITION prev = cursor.previous;
	POSITION curr = cursor.current;

	// 이전 위치의 문자와 색상 설정
	char ch = frontbuf[prev.row][prev.column];
	int color = get_color_for_char(ch, prev);

	set_color(color);  // 이전 위치 색상 설정
	printc(padd(map_pos, prev), ch, color);

	// 현재 커서 위치의 문자를 커서 색상으로 출력
	ch = frontbuf[curr.row][curr.column];
	set_color(COLOR_CURSOR);  // 커서 색상 설정
	printc(padd(map_pos, curr), ch, COLOR_CURSOR);

	// 색상 초기화
	set_color(COLOR_DEFAULT);
}


void project_sys_map(char src[N_LAYER][SYS_HEIGHT][SYS_WIDTH], char dest[SYS_HEIGHT][SYS_WIDTH]) {
	for (int k = 0; k < N_LAYER; k++) {
		for (int i = 0; i < SYS_HEIGHT; i++) {
			for (int j = 0; j < SYS_WIDTH; j++) {
				if (src[k][i][j] >= 0) {
					dest[i][j] = src[k][i][j];
				}
			}
		}
	}
}


void sys_map(char system_map[N_LAYER][SYS_HEIGHT][SYS_WIDTH]) {
	// 시스템 메시지 출력 버퍼에 시스템 메시지 데이터를 복사
	project_sys_map(system_map, sys_backbuf);

	POSITION friend_h, enemy_h;
	if (find_h_positions(&friend_h, &enemy_h))
	{
		gotoxy((POSITION) { sys_map_pos.row, sys_map_pos.column });
		if (turn_counter % 50 == 0) {
			sand_mob(friend_h, enemy_h); // 샌드웜 이동
		}

		// 턴 카운터 증가
		turn_counter++;
	}
	for (int i = 0; i < SYS_HEIGHT; i++) {
		for (int j = 0; j < SYS_WIDTH; j++) {
			if (sys_frontbuf[i][j] != sys_backbuf[i][j]) {
				// 시스템 화면의 실제 위치 계산
				POSITION screen_pos = { sys_pos.row + i, sys_pos.column + j };
				gotoxy(screen_pos);  // 화면에서 sys_pos를 기준으로 이동
				printc(screen_pos, sys_backbuf[i][j], COLOR_DEFAULT);
			}
			sys_frontbuf[i][j] = sys_backbuf[i][j];
		}
	}
}

void project_status_map(char src[N_LAYER][STATUS_HEIGHT][STATUS_WIDTH], char dest[STATUS_HEIGHT][STATUS_WIDTH]) {
	for (int k = 0; k < N_LAYER; k++) {
		for (int i = 0; i < STATUS_HEIGHT; i++) {
			for (int j = 0; j < STATUS_WIDTH; j++) {
				if (src[k][i][j] >= 0) {
					dest[i][j] = src[k][i][j];
				}
			}
		}
	}
}

void sta_map(char status_map[N_LAYER][STATUS_HEIGHT][STATUS_WIDTH]) {
	project_status_map(status_map, status_backbuf);
	for (int i = 0; i < STATUS_HEIGHT; i++) {
		for (int j = 0; j < STATUS_WIDTH; j++) {
			if (status_frontbuf[i][j] != status_backbuf[i][j]) {
				// 시스템 화면의 실제 위치 계산
				POSITION screen_pos = { status_pos.row + i, status_pos.column + j };
				gotoxy(screen_pos);
				printc(screen_pos, status_backbuf[i][j], COLOR_DEFAULT);
			}
			status_frontbuf[i][j] = status_backbuf[i][j];
		}
	}
}

//스페이스바 누르면 상태창에 내용 출력, 명령창에 명령어 출력
void display_info_in_sta_map(char ch, POSITION pos) {
	//sta_map 창을 지워서 이전 출력 지움
	clear_sta_map_area();
	// sta_map 창에 커서가 위치한 배치의 정보 출력
	const char* info;
	switch (ch) {
	case 'P': info = "장판"; break;
	case 'B':
		info = is_enemy_map[pos.row][pos.column] ? "적 진형" : "아군 진형";
		break;
	case 'R': info = "바위"; break; //나중에 2x2는 바위, 1x1은 돌맹이
	case 'S': info = "스파이스"; break;
	case 'W': info = "샌드웜"; break;
	case 'H': info = "하베스터"; break;
	default: info = "사막 지형"; break;
	}

	gotoxy((POSITION) { sta_map_pos.row, sta_map_pos.column }); // sta_map 창 위치로 이동
	printf("%s (좌표 : %d, %d)\n", info, pos.row, pos.column);
	if (strcmp(info, "스파이스") == 0)
	{
		gotoxy((POSITION) { sta_map_pos.row + 1, sta_map_pos.column });
		printf("스파이스가 자원이지");
	}

	if (strcmp(info, "하베스터") == 0) {
		gotoxy((POSITION) { cmd_map_pos.row, cmd_map_pos.column });
		printf(" H : 자원 수확");
		gotoxy((POSITION) { cmd_map_pos.row + 1, cmd_map_pos.column });
		printf(" M : 이동");

	}

}

//상태창 내용을 지우는 함수
void clear_sta_map_area() {
	for (int i = 0; i < STATUS_HEIGHT - 2; i++) {
		gotoxy((POSITION) { sta_map_pos.row + i, sta_map_pos.column });
		for (int j = 0; j < STATUS_WIDTH - 2; j++) {
			printf(" ");
		}
	}

	for (int i = 0; i < CMD_HEIGHT - 2; i++) {
		gotoxy((POSITION) { cmd_map_pos.row + i, cmd_map_pos.column });
		for (int j = 0; j < CMD_WIDTH - 2; j++)
		{
			printf(" ");
		}
	}
}


void project_cmd_map(char src[N_LAYER][CMD_HEIGHT][CMD_WIDTH], char dest[CMD_HEIGHT][CMD_WIDTH]) {
	for (int k = 0; k < N_LAYER; k++) {
		for (int i = 0; i < CMD_HEIGHT; i++) {
			for (int j = 0; j < CMD_WIDTH; j++) {
				if (src[k][i][j] >= 0) {
					dest[i][j] = src[k][i][j];
				}
			}
		}
	}
}

void cmd_map(char command_map[N_LAYER][CMD_HEIGHT][CMD_WIDTH]) {
	project_cmd_map(command_map, cmd_backbuf);
	for (int i = 0; i < CMD_HEIGHT; i++) {
		for (int j = 0; j < CMD_WIDTH; j++) {
			if (cmd_frontbuf[i][j] != cmd_backbuf[i][j]) {
				// 시스템 화면의 실제 위치 계산
				POSITION screen_pos = { cmd_pos.row + i, cmd_pos.column + j };
				gotoxy(screen_pos);
				printc(screen_pos, cmd_backbuf[i][j], COLOR_DEFAULT);
			}
			cmd_frontbuf[i][j] = cmd_backbuf[i][j];
		}
	}
}

void formation(int i, int j, char backbuf[MAP_HEIGHT][MAP_WIDTH]) {
	if (backbuf[i][j] == 'P') {
		set_color(BLACK);
	}
	else if (backbuf[i][j] == 'B') {
		if (i == 16 || i == 15) {
			set_color(BLUE);
		}
		else { set_color(DARK_RED); }
	}
	else if (backbuf[i][j] == 'H') {
		if (i == 14) {
			set_color(BLUE);
			color_map[i][j] = BLUE;
		}
		else {
			set_color(DARK_RED);
			color_map[i][j] = DARK_RED;
		}
	}
	else if (backbuf[i][j] == 'W') {
		set_color(DARK_YELLOW);
	}
	else if (backbuf[i][j] == 'R') {
		set_color(GRAY);
	}
	else if (backbuf[i][j] == 'S') {
		set_color(RED);
	}
	else {
		set_color(7);  // 기본 흰색
	}
}


/*
1. 하베스터 좌표 두개 가져옴
	ex ) H(아군) : 좌표(14, 1)
		 H(적군) : 좌표(3, 58)
2. 샌드웜과 두 하베스터의 거리 계산
3. 비교해서 더 가까운 하베스터 쪽으로 샌드웜 이동
*/

bool find_h_positions(POSITION* friend_h, POSITION* enemy_h) {
	bool friend_found = false;
	bool enemy_found = false;

	for (int row = 0; row < MAP_HEIGHT; row++) {
		for (int col = 0; col < MAP_WIDTH; col++) {
			// 'H'가 있는지 확인하고 해당 좌표의 색상을 color_map에서 가져옴
			if ((map[1][row][col] == 'H') && color_map[row][col] != 7) {
				int color = color_map[row][col];

				if (color == BLUE && !friend_found) {
					friend_h->row = row;
					friend_h->column = col;
					friend_found = true;
				}
				else if (color == DARK_RED && !enemy_found) {
					enemy_h->row = row;
					enemy_h->column = col;
					enemy_found = true;
				}
			}
		}
	}
	return friend_found && enemy_found;  // 두 개의 H를 모두 찾은 경우에만 true 반환
}

int has_reached_target(Sandworm sandworm, POSITION target) {
	return sandworm.pos.row == target.row && sandworm.pos.column == target.column;
}

void move_sandworm_toward_target(Sandworm* sandworm, POSITION target) {
	// 현재 위치 초기화 (화면에서 지우기)
	map[1][sandworm->pos.row][sandworm->pos.column] = -1;

	// 타겟까지의 가로/세로 거리 계산
	int row_diff = target.row - sandworm->pos.row;
	int col_diff = target.column - sandworm->pos.column;
	// 이동 방향 결정
	if (abs(row_diff) > abs(col_diff)) {
		sandworm->pos.row += (row_diff > 0) ? 1 : -1;
	}
	else {
		sandworm->pos.column += (col_diff > 0) ? 1 : -1;
	}

	// 새로운 위치 업데이트
	map[1][sandworm->pos.row][sandworm->pos.column] = 'W';
}

void sand_mob(POSITION friend_h, POSITION enemy_h) {
	// 좌상단 샌드웜과 하베스터 간 거리 계산
	double friend_sandworm0 = sqrt(pow(friend_h.row - sandworms[0].pos.row, 2) + pow(friend_h.column - sandworms[0].pos.column, 2));
	double enemy_sandworm0 = sqrt(pow(enemy_h.row - sandworms[0].pos.row, 2) + pow(enemy_h.column - sandworms[0].pos.column, 2));

	if (!has_reached_target(sandworms[0], friend_h) && friend_sandworm0 < enemy_sandworm0) {
		move_sandworm_toward_target(&sandworms[0], friend_h);
	}
	else if (!has_reached_target(sandworms[0], enemy_h)) {
		move_sandworm_toward_target(&sandworms[0], enemy_h);
	}

	// 우하단 샌드웜과 하베스터 간 거리 계산
	double friend_sandworm1 = sqrt(pow(friend_h.row - sandworms[1].pos.row, 2) + pow(friend_h.column - sandworms[1].pos.column, 2));
	double enemy_sandworm1 = sqrt(pow(enemy_h.row - sandworms[1].pos.row, 2) + pow(enemy_h.column - sandworms[1].pos.column, 2));

	if (!has_reached_target(sandworms[1], friend_h) && friend_sandworm1 < enemy_sandworm1) {
		move_sandworm_toward_target(&sandworms[1], friend_h);
	}
	else if (!has_reached_target(sandworms[1], enemy_h)) {
		move_sandworm_toward_target(&sandworms[1], enemy_h);
	}
}
