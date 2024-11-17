/*
*  display.c:
* ȭ�鿡 ���� ������ ���
* ��, Ŀ��, �ý��� �޽���, ����â, �ڿ� ���� ���
* io.c�� �ִ� �Լ����� �����
*/

#include "display.h"
#include "io.h"

// ����� ������� �»��(topleft) ��ǥ
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };
const POSITION sys_pos = { 20, 0 };
const POSITION status_pos = { 1, 62 };
const POSITION cmd_pos = { 20, 62 };
const POSITION sta_map_pos = { 2, 63 };
//����
char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
//�ý��� �޼���â
char sys_backbuf[SYS_HEIGHT][SYS_WIDTH] = { 0 };
char sys_frontbuf[SYS_HEIGHT][SYS_WIDTH] = { 0 };
//���� ����â
char status_backbuf[STATUS_HEIGHT][STATUS_WIDTH] = { 0 };
char status_frontbuf[STATUS_HEIGHT][STATUS_WIDTH] = { 0 };
//���â
char cmd_backbuf[CMD_HEIGHT][CMD_WIDTH] = { 0 };
char cmd_frontbuf[CMD_HEIGHT][CMD_WIDTH] = { 0 };
//��ġ�� ���� �����ϴ� colorbuf
int colorbuf[MAP_HEIGHT][MAP_WIDTH] = { 7 };

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);
void sys_map(char system_map[N_LAYER][SYS_HEIGHT][SYS_WIDTH]);
void sta_map(char status_map[N_LAYER][STATUS_HEIGHT][STATUS_WIDTH]);
void cmd_map(char command_map[N_LAYER][CMD_HEIGHT][CMD_WIDTH]);
void formation(int i, int j, char backbuf[MAP_HEIGHT][MAP_WIDTH]);
void lear_sta_map_area();

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
				formation(i, j, backbuf); // ���� ��ġ
				gotoxy(padd(map_pos, pos));  // ��ġ �̵�
				printf("%c", backbuf[i][j]);  // ���� ���
			}
			frontbuf[i][j] = backbuf[i][j];
		}
	}
}

// frontbuf[][]���� Ŀ�� ��ġ�� ���ڸ� ���� �ٲ㼭 �״�� �ٽ� ���

void display_cursor(CURSOR cursor) {
	POSITION prev = cursor.previous;
	POSITION curr = cursor.current;

	// ���� ��ġ�� ���ڿ� ���� ����
	char ch = frontbuf[prev.row][prev.column];
	int color = get_color_for_char(ch, prev);

	set_color(color);  // ���� ��ġ ���� ����
	printc(padd(map_pos, prev), ch, color);

	// ���� Ŀ�� ��ġ�� ���ڸ� Ŀ�� �������� ���
	ch = frontbuf[curr.row][curr.column];
	set_color(COLOR_CURSOR);  // Ŀ�� ���� ����
	printc(padd(map_pos, curr), ch, COLOR_CURSOR);

	// ���� �ʱ�ȭ
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
	// �ý��� �޽��� ��� ���ۿ� �ý��� �޽��� �����͸� ����
	project_sys_map(system_map, sys_backbuf);
	for (int i = 0; i < SYS_HEIGHT; i++) {
		for (int j = 0; j < SYS_WIDTH; j++) {
			if (sys_frontbuf[i][j] != sys_backbuf[i][j]) {
				// �ý��� ȭ���� ���� ��ġ ���
				POSITION screen_pos = { sys_pos.row + i, sys_pos.column + j };
				gotoxy(screen_pos);  // ȭ�鿡�� sys_pos�� �������� �̵�
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
				// �ý��� ȭ���� ���� ��ġ ���
				POSITION screen_pos = { status_pos.row + i, status_pos.column + j };
				gotoxy(screen_pos);
				printc(screen_pos, status_backbuf[i][j], COLOR_DEFAULT);
			}
			status_frontbuf[i][j] = status_backbuf[i][j];
		}
	}
}

//�����̽��� ������ ����â�� ���� ���
void display_info_in_sta_map(char ch, POSITION pos) {
	//sta_map â�� ������ ���� ��� ����
	clear_sta_map_area();
	// sta_map â�� Ŀ���� ��ġ�� ��ġ�� ���� ���
	const char* info;
	switch (ch) {
	case 'P': info = "����"; break;
	case 'B':
		info = is_enemy_map[pos.row][pos.column] ? "�� ����" : "�Ʊ� ����";
		break;
	case 'R': info = "����"; break; //���߿� 2x2�� ����, 1x1�� ������
	case 'S': info = "�����̽�"; break;
	case 'W': info = "�����"; break;
	case 'H': info = "�Ϻ�����"; break;
	default: info = "�縷 ����"; break;
	}

	gotoxy((POSITION) { sta_map_pos.row, sta_map_pos.column }); // sta_map â ��ġ�� �̵�
	printf("%s (��ǥ : %d, %d)", info, pos.row, pos.column);
}

//����â ������ ����� �Լ�
void clear_sta_map_area() { 
	for (int i = 0; i < STATUS_HEIGHT; i++) {
		gotoxy((POSITION) { sta_map_pos.row, sta_map_pos.column });
		for (int j = 0; j < STATUS_WIDTH - 2; j++) {
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
				// �ý��� ȭ���� ���� ��ġ ���
				POSITION screen_pos = { cmd_pos.row + i, cmd_pos.column + j };
				gotoxy(screen_pos);
				printc(screen_pos, cmd_backbuf[i][j], COLOR_DEFAULT);
			}
			cmd_frontbuf[i][j] = cmd_backbuf[i][j];
		}
	}
}

void formation(int i, int j, char backbuf[MAP_HEIGHT][MAP_WIDTH]) {
	if (backbuf[i][j] == 'P') { set_color(BLACK); }
	else if (backbuf[i][j] == 'B') {
		if (i == 16 || i == 15) { set_color(BLUE); }
		else { set_color(DARK_RED); }
	}
	else if (backbuf[i][j] == 'H') {
		if (i == 14) { set_color(BLUE); }
		else { set_color(DARK_RED); }
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
		set_color(7);  // �⺻ ���
	}
}