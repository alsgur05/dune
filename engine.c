#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"

void init(void);
void sys_init(void);
void sta_init(void);
void cmd_init(void);
void intro(void);
void outro(void);
void cursor_move(DIRECTION dir);
void sample_obj_move(void);
POSITION sample_obj_next_position(void);
void init_map(void);

extern const POSITION map_pos;
extern char frontbuf[MAP_HEIGHT][MAP_WIDTH];
extern char colorbuf[MAP_HEIGHT][MAP_WIDTH];

/* ================= control =================== */
int sys_clock = 0;		// system-wide clock(ms)
CURSOR cursor = { { 1, 1 }, {1, 1} };


/* ================= game data =================== */
char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };
char system_map[N_LAYER][SYS_HEIGHT][SYS_WIDTH] = { 0 };
char status_map[N_LAYER][STATUS_HEIGHT][STATUS_WIDTH] = { 0 };
char command_map[N_LAYER][CMD_HEIGHT][CMD_WIDTH] = { 0 };
bool is_enemy_map[MAP_HEIGHT][MAP_WIDTH] = { {false} }; 

RESOURCE resource = {
	.spice = 0,
	.spice_max = 0,
	.population = 0,
	.population_max = 0
};

OBJECT_SAMPLE obj = {
	.pos = {1, 1},
	.dest = {MAP_HEIGHT - 2, MAP_WIDTH - 2},
	.repr = 'o',
	.speed = 300,
	.next_move_time = 300
};

/* ================= main() =================== */
int main(void) {
	srand((unsigned int)time(NULL));

	init();
	sys_init();
	sta_init();
	cmd_init();
	intro();
	init_map();
	display(resource, map, cursor, system_map, status_map, command_map);


	while (1) {
		// loop �� ������(��, TICK==10ms����) Ű �Է� Ȯ��
		KEY key = get_key();

		// Ű �Է��� ������ ó��
		if (is_arrow_key(key) || (key >= k_up_double && key <= k_down_double)) {
			DIRECTION dir = (key >= k_up_double) ? ktod(key - 10) : ktod(key);  // ���� �Է� ó��
			cursor_move(dir);
			if (key >= k_up_double) {  // ���� �Է��̸� �߰��� 2ĭ �̵�
				cursor_move(dir);
				cursor_move(dir);
			}
		}
		else if (key == k_space) {
			// �����̽��ٰ� ������ ��, ���� ��ġ�� ������ sta_map�� ǥ��
			char ch = frontbuf[cursor.current.row][cursor.current.column];
			display_info_in_sta_map(ch, cursor.current);
		}
		else if (key == k_esc) {
			clear_sta_map_area();
		}
		else {
			// ����Ű ���� �Է�
			switch (key) {
			case k_quit: outro();
			case k_none:
			case k_undef:
			default: break;
			}
		}

		// ���� ������Ʈ ����
		sample_obj_move();

		// ȭ�� ���
		display(resource, map, cursor, system_map, status_map, command_map);
		Sleep(TICK);
		sys_clock += 10;
	}
}


/* ================= subfunctions =================== */
void intro(void) {
	printf("DUNE 1.5\n");
	Sleep(2000);
	system("cls");
}

void outro(void) {
	printf("exiting...\n");
	exit(0);
}

void init(void) {
	// layer 0(map[0])�� ���� ����
	for (int j = 0; j < MAP_WIDTH; j++) {
		map[0][0][j] = '#';
		map[0][MAP_HEIGHT - 1][j] = '#';
	}

	for (int i = 1; i < MAP_HEIGHT - 1; i++) {
		map[0][i][0] = '#';
		map[0][i][MAP_WIDTH - 1] = '#';
		for (int j = 1; j < MAP_WIDTH - 1; j++) {
			map[0][i][j] = ' ';
		}
	}

	// layer 1(map[1])�� ��� �α�(-1�� ä��)
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[1][i][j] = -1;
		}
	}

	// object sample
	map[1][obj.pos.row][obj.pos.column] = 'o';
}
void sys_init(void) {
	for (int j = 0; j < SYS_WIDTH; j++) {
		system_map[0][0][j] = '#';
		system_map[0][SYS_HEIGHT - 1][j] = '#';
	}

	for (int i = 1; i < SYS_HEIGHT - 1; i++) {
		system_map[0][i][0] = '#';
		system_map[0][i][SYS_WIDTH - 1] = '#';
		for (int j = 1; j < SYS_WIDTH - 1; j++) {
			system_map[0][i][j] = ' ';
		}
	}

	// layer 1(map[1])�� ��� �α�(-1�� ä��)
	for (int i = 0; i < SYS_HEIGHT; i++) {
		for (int j = 0; j < SYS_WIDTH; j++) {
			system_map[1][i][j] = -1;
		}
	}
}
void sta_init(void) {
	for (int j = 0; j < STATUS_WIDTH; j++) {
		status_map[0][0][j] = '#';
		status_map[0][STATUS_HEIGHT - 1][j] = '#';
	}

	for (int i = 1; i < STATUS_HEIGHT - 1; i++) {
		status_map[0][i][0] = '#';
		status_map[0][i][STATUS_WIDTH - 1] = '#';
		for (int j = 1; j < STATUS_WIDTH - 1; j++) {
			status_map[0][i][j] = ' ';
		}
	}

	// layer 1(map[1])�� ��� �α�(-1�� ä��)
	for (int i = 0; i < STATUS_HEIGHT; i++) {
		for (int j = 0; j < STATUS_WIDTH; j++) {
			status_map[1][i][j] = -1;
		}
	}
}
void cmd_init(void) {
	for (int j = 0; j < CMD_WIDTH; j++) {
		command_map[0][0][j] = '#';
		command_map[0][CMD_HEIGHT - 1][j] = '#';
	}

	for (int i = 1; i < CMD_HEIGHT - 1; i++) {
		command_map[0][i][0] = '#';
		command_map[0][i][CMD_WIDTH - 1] = '#';
		for (int j = 1; j < CMD_WIDTH - 1; j++) {
			command_map[0][i][j] = ' ';
		}
	}

	// layer 1(map[1])�� ��� �α�(-1�� ä��)
	for (int i = 0; i < CMD_HEIGHT; i++) {
		for (int j = 0; j < CMD_WIDTH; j++) {
			command_map[1][i][j] = -1;
		}
	}
}

int get_color_for_char(char ch, POSITION pos) {
	int color = COLOR_DEFAULT;

	if (ch == 'P') {
		color = BLACK;
	}
	else if (ch == 'B') {
		color = (pos.row == 16 || pos.row == 15) ? BLUE : DARK_RED;
	}
	else if (ch == 'R') {
		color = GRAY;
	}
	else if (ch == 'S') {
		color = RED;
	}
	else if (ch == 'W') {
		color = DARK_YELLOW;
	}
	else if (ch == 'H') {
		color = (pos.row == 14) ? BLUE : DARK_RED;
	}

	return color;
}

// (�����ϴٸ�) ������ �������� Ŀ�� �̵�
void cursor_move(DIRECTION dir) {
	POSITION curr = cursor.current;
	POSITION new_pos = pmove(curr, dir);

	// ���� ��ġ �ʱ�ȭ (���� ���� ����)
	char ch = frontbuf[cursor.previous.row][cursor.previous.column];
	int prev_color = get_color_for_char(ch, cursor.previous);
	printc(padd(map_pos, cursor.previous), ch, prev_color);

	// validation check
	if (1 <= new_pos.row && new_pos.row <= MAP_HEIGHT - 2 &&
		1 <= new_pos.column && new_pos.column <= MAP_WIDTH - 2) {
		cursor.previous = cursor.current;
		cursor.current = new_pos;
	}

	if (dir == k_up_double) {
		new_pos = pmove(cursor.current, d_up);
		new_pos = pmove(new_pos, d_up);
	}
	else if (dir == k_down_double) {
		new_pos = pmove(cursor.current, d_down);
		new_pos = pmove(new_pos, d_down);
	}
	else if (dir == k_left_double) {
		new_pos = pmove(cursor.current, d_left);
		new_pos = pmove(new_pos, d_left);
	}
	else if (dir == k_right_double) {
		new_pos = pmove(cursor.current, d_right);
		new_pos = pmove(new_pos, d_right);
	}

	// ���� ������ Ȯ�� �� ���� ��ġ�� �̵�
	if (1 <= new_pos.row && new_pos.row <= MAP_HEIGHT - 2 &&
		1 <= new_pos.column && new_pos.column <= MAP_WIDTH - 2) {
		cursor.previous = cursor.current;
		cursor.current = new_pos;
	}
}



/* ================= sample object movement =================== */
POSITION sample_obj_next_position(void) {
	// ���� ��ġ�� �������� ���ؼ� �̵� ���� ����	
	POSITION diff = psub(obj.dest, obj.pos);
	DIRECTION dir;

	// ������ ����. ������ �ܼ��� ���� �ڸ��� �պ�
	if (diff.row == 0 && diff.column == 0) {
		if (obj.dest.row == 1 && obj.dest.column == 1) {
			// topleft --> bottomright�� ������ ����
			POSITION new_dest = { MAP_HEIGHT - 2, MAP_WIDTH - 2 };
			obj.dest = new_dest;
		}
		else {
			// bottomright --> topleft�� ������ ����
			POSITION new_dest = { 1, 1 };
			obj.dest = new_dest;
		}
		return obj.pos;
	}

	// ������, ������ �Ÿ��� ���ؼ� �� �� �� ������ �̵�
	if (abs(diff.row) >= abs(diff.column)) {
		dir = (diff.row >= 0) ? d_down : d_up;
	}
	else {
		dir = (diff.column >= 0) ? d_right : d_left;
	}

	// validation check
	// next_pos�� ���� ����� �ʰ�, (������ ������)��ֹ��� �ε����� ������ ���� ��ġ�� �̵�
	// ������ �浹 �� �ƹ��͵� �� �ϴµ�, ���߿��� ��ֹ��� ���ذ��ų� ���� ������ �ϰų�... ���
	POSITION next_pos = pmove(obj.pos, dir);
	if (1 <= next_pos.row && next_pos.row <= MAP_HEIGHT - 2 && \
		1 <= next_pos.column && next_pos.column <= MAP_WIDTH - 2 && \
		map[1][next_pos.row][next_pos.column] < 0) {

		return next_pos;
	}
	else {
		return obj.pos;  // ���ڸ�
	}
}

void sample_obj_move(void) {
	if (sys_clock <= obj.next_move_time) {
		// ���� �ð��� �� ����
		return;
	}

	// ������Ʈ(�ǹ�, ���� ��)�� layer1(map[1])�� ����
	map[1][obj.pos.row][obj.pos.column] = -1;
	obj.pos = sample_obj_next_position();
	map[1][obj.pos.row][obj.pos.column] = obj.repr;

	obj.next_move_time = sys_clock + obj.speed;
}

void init_map(void) {
	//���ϴ� ���� ����
	//B, P layer 0 �̶�� �ؼ� �ϴ� �ּ�
	/*
	map[0][16][1] = 'P';
	map[0][16][2] = 'P';
	map[0][15][1] = 'P';
	map[0][15][2] = 'P';

	//���� ���� ����
	map[0][1][58] = 'P';
	map[0][2][58] = 'P';
	map[0][1][57] = 'P';
	map[0][2][57] = 'P';
	*/
	//���ϴ� ����
	map[0][16][3] = 'P';
	map[0][16][4] = 'P';
	map[0][15][3] = 'P';
	map[0][15][4] = 'P';
	//���ϴ� ����
	map[0][16][1] = 'B';
	map[0][16][2] = 'B';
	map[0][15][1] = 'B';
	map[0][15][2] = 'B';
	
	//���� ����
	map[1][1][58] = 'B';
	map[1][2][58] = 'B';
	map[1][1][57] = 'B';
	map[1][2][57] = 'B';

	// ���ϴ� ������ �Ʊ����� ����
	is_enemy_map[16][1] = false;
	is_enemy_map[16][2] = false;
	is_enemy_map[15][1] = false;
	is_enemy_map[15][2] = false;

	// ���� ������ �������� ����
	is_enemy_map[1][58] = true;
	is_enemy_map[2][58] = true;
	is_enemy_map[1][57] = true;
	is_enemy_map[2][57] = true;

	//���ϴ� �Ϻ�����
	map[1][14][1] = 'H';
	//���� �Ϻ�����
	map[1][3][58] = 'H';
	
	//���ϴ� �����̽�
	map[0][12][1] = 'S';
	//���� �����̽�
	map[0][5][58] = 'S';

	//���� (2x2)
	map[0][13][15] = 'R';
	map[0][12][15] = 'R';
	map[0][13][16] = 'R';
	map[0][12][16] = 'R';

	map[0][8][33] = 'R';
	map[0][9][33] = 'R';
	map[0][8][32] = 'R';
	map[0][9][32] = 'R';

	map[0][4][49] = 'R';
	map[0][4][48] = 'R';
	map[0][5][49] = 'R';
	map[0][5][48] = 'R';

	//���� (1x1)
	map[0][14][32] = 'R';
	map[0][6][15] = 'R';
	map[0][3][23]= 'R';
	map[0][15][48]= 'R';

	//�����
	map[1][3][8] = 'W';
	map[1][13][50] = 'W';
}