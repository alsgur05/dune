/*
*  display.c:
* 화면에 게임 정보를 출력
* 맵, 커서, 시스템 메시지, 정보창, 자원 상태 등등
* io.c에 있는 함수들을 사용함
*/
#include <stdlib.h>
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
const POSITION sys_map_pos = { 20, 0 };
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
void formation(int i, int j, const char* info);

//창에 있는 문자 지우는 함수
void clear_sta_map_area();

//피타고라스 (H, 샌드웜)
void sand_mob();

//하베스터 좌표 찾는 함수
bool find_h_positions(POSITION* friend_h, POSITION* enemy_h);

//H 색 저장 배열
int color_map[MAP_HEIGHT][MAP_WIDTH] = { 7 }; //7 : 기본 색
//샌드웜 속도
int turn_counter = 0;
typedef struct {
	int id;           // 고유 ID
	POSITION pos;     // 현재 위치
	POSITION prev_pos; // 이전 위치를 저장
} Sandworm;

Sandworm sandworms[2] = {
	{ // 좌상단 샌드웜
		.id = 1,
		.pos = {3, 8},
		.prev_pos = {3, 8}
	},
	{ // 우하단 샌드웜
		.id = 2,
		.pos = {13, 50},
		.prev_pos = {13, 50}
	}
};

bool is_producing_harvester = false;
int harvester_production_time = 0;
POSITION production_pos = { 0, 0 };

bool is_showing_harvester_production;

void display(
	const RESOURCE* resource,
	char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
	CURSOR cursor,
	char system_map[N_LAYER][SYS_HEIGHT][SYS_WIDTH],
	char status_map[N_LAYER][STATUS_HEIGHT][STATUS_WIDTH],
	char command_map[N_LAYER][CMD_HEIGHT][CMD_WIDTH]
)
{
	display_resource(*resource);
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

void formation(int i, int j, const char* info) {
	if (strcmp(info, "장판") == 0) {
		set_color(BLACK);
		color_map[i][j] = BLACK;
	}
	else if (strcmp(info, "아군 진형") == 0) {
		set_color(BLUE);
		color_map[i][j] = BLUE;
	}
	else if (strcmp(info, "적 진형") == 0) {
		set_color(DARK_RED);
		color_map[i][j] = DARK_RED;
	}
	else if (strcmp(info, "아군 하베스터") == 0) {
		set_color(BLUE);
		color_map[i][j] = BLUE;
	}
	else if (strcmp(info, "적군 하베스터") == 0) {
		set_color(DARK_RED);
		color_map[i][j] = DARK_RED;
	}
	else if (strcmp(info, "생성중인 하베스터") == 0) {
		set_color(GRAY);
		color_map[i][j] = GRAY;
	}
	else if (strcmp(info, "스파이스") == 0) {
		set_color(RED);
		color_map[i][j] = RED;
	}
	else if (strcmp(info, "샌드웜") == 0) {
		set_color(DARK_YELLOW);
		color_map[i][j] = DARK_YELLOW;
	}
	else if (strcmp(info, "바위") == 0) {
		set_color(GRAY);
		color_map[i][j] = GRAY;
	}
	else {
		set_color(7); // 기본 흰색
		color_map[i][j] = 7;
	}
}

void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	static bool is_first_call = true;

	// 최초 실행시에만 하베스터 색상 초기화
	if (is_first_call) {
		initialize_harvester_colors();
		is_first_call = false;
	}

	project(map, backbuf);

	// 하베스터 생산 중일 때 깜빡임 효과
	if (is_producing_harvester) {
		backbuf[production_pos.row][production_pos.column] = 'H';
		// 커서 위치와 무관하게 깜빡임
		if (turn_counter % 50 < 25) {
			set_color(COLOR_DEFAULT);
			gotoxy(padd(map_pos, production_pos));
			printf("H");
		}
		else {
			set_color(GRAY);
			gotoxy(padd(map_pos, production_pos));
			printf("H");
		}
	}

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (frontbuf[i][j] != backbuf[i][j] && !(is_producing_harvester && i == production_pos.row && j == production_pos.column)) {
				const char* info = "사막 지형";  // 기본값 설정
				switch (backbuf[i][j]) {
				case 'P':
					info = "장판";
					break;
				case 'B':
					info = (i == 16 || i == 15) ? "아군 진형" : "적 진형";
					break;
				case 'H':
					if (color_map[i][j] == BLUE) {
						info = "아군 하베스터";
					}
					else if (color_map[i][j] == DARK_RED) {
						info = "적군 하베스터";
					}
					else if (color_map[i][j] == GRAY) {
						info = "생성중인 하베스터";
					}
					break;
				case 'W':
					info = "샌드웜";
					break;
				case 'R':
					info = "바위";
					break;
				case 'S':
					info = "스파이스";
					break;
				}

				formation(i, j, info);
				POSITION pos = { i, j };
				gotoxy(padd(map_pos, pos));

				// 생산 중인 하베스터의 경우 color_map의 색상 사용
				if (is_producing_harvester && i == production_pos.row && j == production_pos.column) {
					set_color(color_map[i][j]);
				}

				printf("%c", backbuf[i][j]);
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

	set_color(color);  // 이전위치 색상 설정
	printc(padd(map_pos, prev), ch, color);

	// 현재 커서 위치의 문자를 커서 색상으로 출력
	ch = frontbuf[curr.row][curr.column];
	set_color(COLOR_CURSOR);  // 커서 색상 설정
	printc(padd(map_pos, curr), ch, COLOR_CURSOR);

	// 색상 초기화
	set_color(COLOR_DEFAULT);
}


void initialize_harvester_colors() {
	// 아군 하베스터 초기 위치 (i = 14, 15, 16)
	POSITION friend_harvester_positions[] = {
		{14, 1},
		{14, 2},
		{15, 3},
		{16, 3}
	};

	POSITION enemy_harvester_positions[] = {
		{3, 58},
		{3, 57},
		{2, 56},
		{1, 56}
	};

	// 아군 하베스터 색상 설정
	for (int i = 0; i < sizeof(friend_harvester_positions) / sizeof(POSITION); i++) {
		POSITION pos = friend_harvester_positions[i];
		if (map[1][pos.row][pos.column] == 'H') {
			color_map[pos.row][pos.column] = BLUE;
		}
	}

	for (int i = 0; i < sizeof(enemy_harvester_positions) / sizeof(POSITION); i++) {
		POSITION pos = enemy_harvester_positions[i];
		if (map[1][pos.row][pos.column] == 'H') {
			color_map[pos.row][pos.column] = DARK_RED;
		}
	}
}
// 하베스터 이동 시 색상을 유지하는 함수
void move_harvester_with_color(POSITION from, POSITION to) {
	if (map[1][from.row][from.column] == 'H') {
		// 이동할 때 색상도 함께 이동
		int color = color_map[from.row][from.column];
		map[1][from.row][from.column] = -1;  // 이전 위치 비우기
		color_map[from.row][from.column] = 7; // 이전 위치 색상 초기화

		map[1][to.row][to.column] = 'H';     // 새 위치에 하베스터 배치
		color_map[to.row][to.column] = color; // 새 위치에 색상 설정
	}
}
// 새로운 하베스터 생성 시 색상 설정 함수
void create_new_harvester(POSITION pos, bool is_friend) {
	map[1][pos.row][pos.column] = 'H';
	color_map[pos.row][pos.column] = is_friend ? BLUE : DARK_RED;
}

void project_sys_map(char src[N_LAYER][SYS_HEIGHT][SYS_WIDTH], char dest[SYS_HEIGHT][SYS_WIDTH]) {
	// 먼저 모든 위치를 0으로 초기화
	memset(dest, 0, sizeof(char) * SYS_HEIGHT * SYS_WIDTH);

	// 레이어 0의 테두리('#') 먼저 복사
	for (int i = 0; i < SYS_HEIGHT; i++) {
		for (int j = 0; j < SYS_WIDTH; j++) {
			if (src[0][i][j] == '#') {
				dest[i][j] = '#';
			}
		}
	}

	// 그 다음 레이어 1의 내용을 복사 (테두리는 덮어쓰지 않음)
	for (int i = 0; i < SYS_HEIGHT; i++) {
		for (int j = 0; j < SYS_WIDTH; j++) {
			if (dest[i][j] != '#' && src[1][i][j] >= 0) {
				dest[i][j] = src[1][i][j];
			}
		}
	}
}

void sys_map(char system_map[N_LAYER][SYS_HEIGHT][SYS_WIDTH]) {
	project_sys_map(system_map, sys_backbuf);

	turn_counter++;

	if (is_producing_harvester && turn_counter >= harvester_production_time) {
		// 생산 완료
		is_producing_harvester = false;
		map[1][production_pos.row][production_pos.column] = 'H';

		// 즉시 아군 색상 설정
		color_map[production_pos.row][production_pos.column] = BLUE;

		// 색상이 즉시 반영되도록 frontbuf와 backbuf 모두 업데이트
		POSITION pos = production_pos;
		gotoxy(padd(map_pos, pos));
		set_color(BLUE);
		printf("%c", 'H');

		sys_text("하베스터가 생성되었습니다.");
	}
	// 하베스터 위치 찾기를 하베스터 생성 이후로 이동
	if (turn_counter % 10 == 0) {  // 매 10틱마다
		sand_mob();  // 샌드웜 이동
	}

	display_system_messages();
}

MessageQueue messageQueue = { .count = 0 };

void add_system_message(const char* message) {
	// 메시지가 가득 찼을 경우 모든 메시지를 한 칸씩 위로 이동
	if (messageQueue.count >= MAX_MESSAGES) {
		for (int i = 0; i < MAX_MESSAGES - 1; i++) {
			strcpy_s(messageQueue.messages[i], MAX_MESSAGE_LENGTH, messageQueue.messages[i + 1]);
		}
		messageQueue.count--;
	}

	// 새 메시지 추가
	strcpy_s(messageQueue.messages[messageQueue.count], MAX_MESSAGE_LENGTH, message);
	messageQueue.count++;

	// 메시지 출력
	display_system_messages();
}

void display_system_messages() {
	// 시스템 메시지 영역 초기화 (테두리는 유지)
	for (int i = 0; i < SYS_HEIGHT; i++) {  // 물음표가 출력된 실제 높이
		gotoxy((POSITION) { sys_map_pos.row + i, sys_map_pos.column });
		for (int j = 0; j < SYS_WIDTH; j++) {  // 물음표가 출력된 실제 너비
			if (i == 0 || i == SYS_HEIGHT - 1 || j == 0 || j == SYS_WIDTH - 1) {  // 테두리 위치
				printf("#");
			}
			else {
				printf(" ");
			}
		}
	}

	// 저장된 메시지들을 순서대로 출력
	for (int i = 0; i < messageQueue.count; i++) {
		gotoxy((POSITION) { sys_map_pos.row + i + 1, sys_map_pos.column + 1 });  // 테두리 안쪽부터 시작
		printf("%s", messageQueue.messages[i]);
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

	// 하베스터 생산 정보를 표시하기로 했다면 계속 업데이트
	if (is_showing_harvester_production && is_producing_harvester) {
		static int last_shown_seconds = -1;  // 마지막으로 표시한 시간

		// 남은 시간 계산
		int remaining_ticks = harvester_production_time - turn_counter;
		int remaining_seconds = remaining_ticks / 50;

		// 초가 변경됐을 때만 업데이트
		if (remaining_seconds != last_shown_seconds) {
			gotoxy((POSITION) { sta_map_pos.row, sta_map_pos.column });
			printf("생성중인 하베스터");
			gotoxy((POSITION) { sta_map_pos.row + 1, sta_map_pos.column });
			printf("생성까지 남은 시간: %d초    ", remaining_seconds);  // 공백 추가로 이전 숫자 지우기

			last_shown_seconds = remaining_seconds;
		}
	}

	// 나머지 상태창 업데이트 (기존 버퍼 업데이트 코드)
	for (int i = 0; i < STATUS_HEIGHT; i++) {
		for (int j = 0; j < STATUS_WIDTH; j++) {
			if (status_frontbuf[i][j] != status_backbuf[i][j]) {
				POSITION screen_pos = { status_pos.row + i, status_pos.column + j };
				gotoxy(screen_pos);
				printc(screen_pos, status_backbuf[i][j], COLOR_DEFAULT);
			}
			status_frontbuf[i][j] = status_backbuf[i][j];
		}
	}
}


char get_key_non_blocking() {
	if (_kbhit()) {  // 키가 눌렸는지 확인
		char input = _getch();

		// 방향키 처리를 위한 분기
		if (input == 72 || input == 75 || input == 77 || input == 80) {
			input = _getch();
			return 0;  // 방향키는 처리하지 않음
		}

		return input;  // 최종적으로 입력된 키를 반환
	}
	return 0;  // 아무 키도 눌리지 않았을 경우
}


//명령어 출력 함수
void sys_text(const char* message) {
	add_system_message(message);
}

// 하베스터 이동 함수 (임의 구현)
void move_harvester(POSITION pos) {
	// 하베스터 이동 처리 (구체적인 동작을 추가해야 함)
	sys_text("하베스터 이동 중...\n");
}

// 자원 수확 함수 (임의 구현)
void harvest_resources(POSITION pos) {
	// 자원 수확 처리 (구체적인 동작을 추가해야 함)
	sys_text("자원 수확 중...\n");
	// 자원 수확 로직 구현
}

// 아군 본진 주변의 빈 공간을 찾는 함수
bool find_empty_space_near_base(POSITION* pos) {
	// 아군 본진 위치 (15-16, 1-2)
	POSITION possible_spots[] = {
		{14, 2},
		{15, 3},
		{16, 3},
		{14, 1}
	};
	int num_spots = 4;  // 배치 가능한 위치 개수 수정

	// 가능한 위치들을 무작위로 섞기
	for (int i = num_spots - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		POSITION temp = possible_spots[i];
		possible_spots[i] = possible_spots[j];
		possible_spots[j] = temp;
	}

	// 빈 공간 찾기
	for (int i = 0; i < num_spots; i++) {
		if (map[1][possible_spots[i].row][possible_spots[i].column] == -1) {
			*pos = possible_spots[i];
			return true;
		}
	}
	return false;
}


//스페이스바 누르면 상태창에 내용 출력, 명령창에 명령어 출력
void display_info_in_sta_map(char ch, POSITION pos, RESOURCE* resource) {
	//sta_map 창을 지워서 이전 출력 지움
	clear_sta_map_area();
	// sta_map 창에 커서가 위치한 배치의 정보 출력
	const char* info = "아군 하베스터";
	switch (ch) {
	case 'P': info = "장판"; break;
	case 'B': info = is_enemy_map[pos.row][pos.column] ? "적 진형" : "아군 진형"; break;
	case 'R': info = "바위"; break; //나중에 2x2는 바위, 1x1은 돌맹이
	case 'S': info = "스파이스"; break;
	case 'W': info = "샌드웜"; break;
	case 'H':
		// 생성 중인 하베스터 처리를 먼저
		// 생성 중인 하베스터 처리
		if (ch == 'H' && is_producing_harvester &&
			pos.row == production_pos.row &&
			pos.column == production_pos.column) {

			is_showing_harvester_production = true;

			// 남은 시간 표시
			gotoxy((POSITION) { sta_map_pos.row, sta_map_pos.column });
			printf("생성중인 하베스터");

			char input = get_key_non_blocking();
			if (input == 'x' || input == 'X') {
				gotoxy((POSITION) { cmd_map_pos.row, cmd_map_pos.column });
				printf(" X : 생산 취소");

				// 실제 취소 입력 대기
				while (1) {
					input = get_key_non_blocking();
					display_system_messages();

					if (input == 'X' || input == 'x') {
						is_producing_harvester = false;
						map[1][production_pos.row][production_pos.column] = -1;
						resource->spice += 3;
						sys_text("하베스터 생산이 취소되었습니다.");
						clear_sta_map_area();
						is_showing_harvester_production = false;
						return;
					}
					else if (input == 27) {  // ESC
						clear_sta_map_area();
						return;
					}
					else if (input == 0) {
						Sleep(10);
						continue;
					}
				}
			}
			return;
		}
		// 일반 하베스터 처리
		else if (color_map[pos.row][pos.column] == BLUE) {
			info = "아군 하베스터";
		}
		else if (color_map[pos.row][pos.column] == DARK_RED) {
			info = "적군 하베스터";
		}
		break;
	default: info = "사막 지형"; break;
	}

	gotoxy((POSITION) { sta_map_pos.row, sta_map_pos.column }); // sta_map 창 위치로 이동
	printf("%s\n", info);
	if (strcmp(info, "스파이스") == 0)
	{
		gotoxy((POSITION) { sta_map_pos.row + 1, sta_map_pos.column }); //이거 나중에 sys_text 처럼 바꾸자
		printf("스파이스가 자원이지\n");
	}

	if (strcmp(info, "아군 진형") == 0) {
		gotoxy((POSITION) { cmd_map_pos.row, cmd_map_pos.column });
		printf(" H : 하베스터 생산\n");

		char input;
		while (1) {
			input = get_key_non_blocking();
			if (input == 27) {
				clear_sta_map_area();
				return;
			}
			if (input == 'H' || input == 'h') {
				if (resource->spice < 5) {
					sys_text("스파이스가 부족합니다.");
					break;
				}
				else {
					// 생산 가능한 위치 찾기
					if (!find_empty_space_near_base(&production_pos)) {
						sys_text("하베스터를 생산할 공간이 없습니다.");
						break;
					}

					sys_text("하베스터를 생성중...");
					resource->spice -= 5;

					// 생산 시작
					is_producing_harvester = true;
					harvester_production_time = turn_counter + 500; // 10초 (50틱/초 기준)
					break;
				}
			}
		}

	}

	if (strcmp(info, "아군 하베스터") == 0) {
		gotoxy((POSITION) { cmd_map_pos.row, cmd_map_pos.column });
		printf(" H : 자원 수확");
		gotoxy((POSITION) { cmd_map_pos.row + 1, cmd_map_pos.column });
		printf(" M : 이동");

		char input;
		while (1) {
			input = get_key_non_blocking();  // 비동기적으로 키 입력 받기

			if (input == 27) {
				clear_sta_map_area();
				return;
			}

			if (input == 'M' || input == 'm') {
				move_harvester(pos);
				break;
			}
			else if (input == 'H' || input == 'h') {
				harvest_resources(pos);
				break;
			}
			else if (input == 0) {
				continue;
			}
		}
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

bool find_h_positions(POSITION* friend_h, POSITION* enemy_h) {
	bool friend_found = false;
	bool enemy_found = false;

	for (int row = 0; row < MAP_HEIGHT; row++) {
		for (int col = 0; col < MAP_WIDTH; col++) {
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

bool can_move_to(POSITION pos) {
	// 맵 범위를 벗어나거나 바위가 있으면 이동 불가
	if (pos.row < 1 || pos.row >= MAP_HEIGHT - 1 ||
		pos.column < 1 || pos.column >= MAP_WIDTH - 1 ||
		map[0][pos.row][pos.column] == 'R'||
		map[0][pos.row][pos.column] == '#'||
		map[1][pos.row][pos.column] == 'B'||
		map[0][pos.row][pos.column] == 'S') {
		return false;
	}
	return true;
}

void move_sandworm_toward_target(Sandworm* sandworm, POSITION target) {
	// 현재 위치의 W 지우기
	map[1][sandworm->pos.row][sandworm->pos.column] = -1;

	// 목표가 없을 경우 랜덤 이동
	if (target.row == -1 && target.column == -1) {
		POSITION random_moves[4] = {
			{sandworm->pos.row - 1, sandworm->pos.column},
			{sandworm->pos.row + 1, sandworm->pos.column},
			{sandworm->pos.row, sandworm->pos.column - 1},
			{sandworm->pos.row, sandworm->pos.column + 1}
		};

		bool moved = false;
		for (int i = 0; i < 4; i++) {
			int rand_index = (rand() + sandworm->id * 17) % 4;
			POSITION next_move = random_moves[rand_index];

			if ((next_move.row != sandworm->prev_pos.row ||
				next_move.column != sandworm->prev_pos.column) &&
				can_move_to(next_move)) {
				sandworm->prev_pos = sandworm->pos;
				sandworm->pos = next_move;
				map[1][sandworm->pos.row][sandworm->pos.column] = 'W';
				moved = true;
				break;
			}
		}

		// 이동할 수 없으면 현재 위치에 다시 W 표시
		if (!moved) {
			map[1][sandworm->pos.row][sandworm->pos.column] = 'W';
		}
		return;
	}

	// 목표가 있을 때의 이동 로직도 같은 방식으로 수정
	POSITION moves[4] = {
		{sandworm->pos.row - 1, sandworm->pos.column},
		{sandworm->pos.row + 1, sandworm->pos.column},
		{sandworm->pos.row, sandworm->pos.column - 1},
		{sandworm->pos.row, sandworm->pos.column + 1}
	};

	double min_dist = 99999.0;
	POSITION best_move = sandworm->pos;  // 기본값으로 현재 위치 설정

	for (int i = 0; i < 4; i++) {
		if (moves[i].row == sandworm->prev_pos.row &&
			moves[i].column == sandworm->prev_pos.column) {
			continue;
		}
		if (can_move_to(moves[i])) {
			double next_dist = sqrt(pow(target.row - moves[i].row, 2) +
				pow(target.column - moves[i].column, 2));
			if (next_dist < min_dist) {
				min_dist = next_dist;
				best_move = moves[i];
			}
		}
	}

	sandworm->prev_pos = sandworm->pos;
	sandworm->pos = best_move;
	map[1][sandworm->pos.row][sandworm->pos.column] = 'W';
}

void sand_mob() {
	static bool first_run = true;
	if (first_run) {
		srand((unsigned int)time(NULL));
		first_run = false;
	}

	HarvesterList harvesters = find_harvesters();

	for (int i = 0; i < 2; i++) {
		Sandworm* sandworm = &sandworms[i];

		// 모든 하베스터가 없으면 랜덤 이동
		if (harvesters.friend_count == 0 && harvesters.enemy_count == 0) {
			move_sandworm_toward_target(sandworm, (POSITION) { -1, -1 });
			continue;
		}

		// 가장 가까운 하베스터 선택
		POSITION closest_harvester = { -1, -1 };
		double min_distance = 99999.0;

		// 아군 하베스터 중 가장 가까운 것 찾기
		for (int j = 0; j < harvesters.friend_count; j++) {
			double distance = sqrt(pow(harvesters.friend_harvesters[j].row - sandworm->pos.row, 2) +
				pow(harvesters.friend_harvesters[j].column - sandworm->pos.column, 2));
			if (distance < min_distance) {
				min_distance = distance;
				closest_harvester = harvesters.friend_harvesters[j];
			}
		}

		// 적군 하베스터 중 가장 가까운 것 찾기
		for (int j = 0; j < harvesters.enemy_count; j++) {
			double distance = sqrt(pow(harvesters.enemy_harvesters[j].row - sandworm->pos.row, 2) +
				pow(harvesters.enemy_harvesters[j].column - sandworm->pos.column, 2));
			if (distance < min_distance) {
				min_distance = distance;
				closest_harvester = harvesters.enemy_harvesters[j];
			}
		}

		// 목표로 이동
		move_sandworm_toward_target(sandworm, closest_harvester);
	}
}

HarvesterList find_harvesters() {
	HarvesterList harvester_list = { .friend_count = 0, .enemy_count = 0 };

	for (int row = 0; row < MAP_HEIGHT; row++) {
		for (int col = 0; col < MAP_WIDTH; col++) {
			if (map[1][row][col] == 'H') {
				if (color_map[row][col] == BLUE && harvester_list.friend_count < MAX_HARVESTERS) {
					// 아군 하베스터
					harvester_list.friend_harvesters[harvester_list.friend_count].row = row;
					harvester_list.friend_harvesters[harvester_list.friend_count].column = col;
					harvester_list.friend_count++;
				}
				else if (color_map[row][col] == DARK_RED && harvester_list.enemy_count < MAX_HARVESTERS) {
					// 적군 하베스터
					harvester_list.enemy_harvesters[harvester_list.enemy_count].row = row;
					harvester_list.enemy_harvesters[harvester_list.enemy_count].column = col;
					harvester_list.enemy_count++;
				}
			}
		}
	}

	return harvester_list;
}
