/*
*  display.c:
* ȭ�鿡 ���� ������ ���
* ��, Ŀ��, �ý��� �޽���, ����â, �ڿ� ���� ���
* io.c�� �ִ� �Լ����� �����
*/
#include <stdlib.h>
#include "display.h"
#include "io.h"

// ����� ������� �»��(topleft) ��ǥ
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };
const POSITION sys_pos = { 20, 0 };
const POSITION status_pos = { 1, 62 };
const POSITION cmd_pos = { 20, 62 };
//�� ������ ��� ��ǥ
const POSITION sta_map_pos = { 2, 63 };
const POSITION cmd_map_pos = { 21,63 };
const POSITION sys_map_pos = { 20, 0 };
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

//display_map�� ��ġ�� ��
void formation(int i, int j, char backbuf[MAP_HEIGHT][MAP_WIDTH]);

//â�� �ִ� ���� ����� �Լ�
void clear_sta_map_area();

//��Ÿ��� (H, �����)
void sand_mob(POSITION friend_h, POSITION enemy_h);

//�Ϻ����� ��ǥ ã�� �Լ�
bool find_h_positions(POSITION* friend_h, POSITION* enemy_h);

//H �� ���� �迭
int color_map[MAP_HEIGHT][MAP_WIDTH] = { 7 }; //7 : �⺻ ��
//����� �ӵ�
int turn_counter = 0;
typedef struct {
	int id;           // ���� ID
	POSITION pos;     // ���� ��ġ
} Sandworm;

Sandworm sandworms[2] = {
	{.id = 1, .pos = {3, 8} },  // ù ��° ����� (�»��)
	{.id = 2, .pos = {13, 50} }  // �� ��° ����� (���ϴ�)
};


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

void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	project(map, backbuf);

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (frontbuf[i][j] != backbuf[i][j]) {
				POSITION pos = { i, j };
				formation(i, j, backbuf); // �⺻ ��
				gotoxy(padd(map_pos, pos));  // ��ġ �̵�
				printf("%c", backbuf[i][j]);  // ���� ���

				//����� ��ǥ Ȯ�� �� ����
				// �̰� ���� ��� X, ����� ��ǥ���� �޾ƿ��� �����δ����� ����
				if (backbuf[i][j] == 'W') { // ����� ǥ�ð� 'W'��� ����
					for (int k = 0; k < 2; k++) {
						if (sandworms[k].pos.row == i && sandworms[k].pos.column == j) {
							sandworms[k].pos = pos;  // ��ġ ����
						}
					}
				}


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
	// ���� ��� ��ġ�� 0���� �ʱ�ȭ
	memset(dest, 0, sizeof(char) * SYS_HEIGHT * SYS_WIDTH);

	// ���̾� 0�� �׵θ�('#') ���� ����
	for (int i = 0; i < SYS_HEIGHT; i++) {
		for (int j = 0; j < SYS_WIDTH; j++) {
			if (src[0][i][j] == '#') {
				dest[i][j] = '#';
			}
		}
	}

	// �� ���� ���̾� 1�� ������ ���� (�׵θ��� ����� ����)
	for (int i = 0; i < SYS_HEIGHT; i++) {
		for (int j = 0; j < SYS_WIDTH; j++) {
			if (dest[i][j] != '#' && src[1][i][j] >= 0) {
				dest[i][j] = src[1][i][j];
			}
		}
	}
}


void sys_map(char system_map[N_LAYER][SYS_HEIGHT][SYS_WIDTH]) {
	// �ý��� �޽��� ��� ���ۿ� �ý��� �޽��� �����͸� ����
	project_sys_map(system_map, sys_backbuf);

	POSITION friend_h, enemy_h;
	if (find_h_positions(&friend_h, &enemy_h))
	{
		if (turn_counter % 50 == 0) {
			sand_mob(friend_h, enemy_h); // ����� �̵�
		}
		turn_counter++;
	}

	display_system_messages();

	// ���� ������Ʈ
	for (int i = 0; i < SYS_HEIGHT; i++) {
		for (int j = 0; j < SYS_WIDTH; j++) {
			// # ���ڴ� ����� ����
			if (sys_backbuf[i][j] != ' ') {
				sys_frontbuf[i][j] = sys_backbuf[i][j];
			}
		}
	}
}

MessageQueue messageQueue = { .count = 0 };

void add_system_message(const char* message) {
	// �޽����� ���� á�� ��� ��� �޽����� �� ĭ�� ���� �̵�
	if (messageQueue.count >= MAX_MESSAGES) {
		for (int i = 0; i < MAX_MESSAGES - 1; i++) {
			strcpy_s(messageQueue.messages[i], MAX_MESSAGE_LENGTH, messageQueue.messages[i + 1]);
		}
		messageQueue.count--;
	}

	// �� �޽��� �߰�
	strcpy_s(messageQueue.messages[messageQueue.count], MAX_MESSAGE_LENGTH, message);
	messageQueue.count++;

	// �޽��� ���
	display_system_messages();
}

void display_system_messages() {
	// �ý��� �޽��� ���� �ʱ�ȭ (�׵θ��� ����)
	for (int i = 0; i < SYS_HEIGHT; i++) {  // ����ǥ�� ��µ� ���� ����
		gotoxy((POSITION) { sys_map_pos.row + i, sys_map_pos.column });
		for (int j = 0; j < SYS_WIDTH; j++) {  // ����ǥ�� ��µ� ���� �ʺ�
			if (i == 0 || i == SYS_HEIGHT - 1 || j == 0 || j == SYS_WIDTH - 1) {  // �׵θ� ��ġ
				printf("#");
			}
			else {
				printf(" ");
			}
		}
	}

	// ����� �޽������� ������� ���
	for (int i = 0; i < messageQueue.count; i++) {
		gotoxy((POSITION) { sys_map_pos.row + i + 1, sys_map_pos.column + 1 });  // �׵θ� ���ʺ��� ����
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


char get_key_non_blocking() {
	if (_kbhit()) {  // Ű�� ���ȴ��� Ȯ��
		char input = _getch();

		// ����Ű ó���� ���� �б�
		if (input == 72 || input == 75 || input == 77 || input == 80) {
			input = _getch(); 
			return 0;  // ����Ű�� ó������ ����
		}

		return input;  // ���������� �Էµ� Ű�� ��ȯ
	}
	return 0;  // �ƹ� Ű�� ������ �ʾ��� ���
}


//��ɾ� ��� �Լ�
void sys_text(const char* message) {
	add_system_message(message);
}

// �Ϻ����� �̵� �Լ� (���� ����)
void move_harvester(POSITION pos) {
	// �Ϻ����� �̵� ó�� (��ü���� ������ �߰��ؾ� ��)
	sys_text("�Ϻ����� �̵� ��...\n");
}

// �ڿ� ��Ȯ �Լ� (���� ����)
void harvest_resources(POSITION pos) {
	// �ڿ� ��Ȯ ó�� (��ü���� ������ �߰��ؾ� ��)
	sys_text("�ڿ� ��Ȯ ��...\n");
	// �ڿ� ��Ȯ ���� ����
}

//�����̽��� ������ ����â�� ���� ���, ���â�� ��ɾ� ���
void display_info_in_sta_map(char ch, POSITION pos, RESOURCE* resource) {
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
	printf("%s (��ǥ : %d, %d)\n", info, pos.row, pos.column);
	if (strcmp(info, "�����̽�") == 0)
	{
		gotoxy((POSITION) { sta_map_pos.row + 1, sta_map_pos.column });
		printf("�����̽��� �ڿ�����\n");
	}

	if (strcmp(info, "�Ʊ� ����") == 0) {
		gotoxy((POSITION) { cmd_map_pos.row, cmd_map_pos.column });
		printf(" H : �Ϻ����� ����\n");

		char input;
		while (1) {
			input = get_key_non_blocking();
			if (input == 27) {
				clear_sta_map_area();
				return;
			}
			if (input == 'H' || input == 'h') {
				if (resource->spice < 5) {
					sys_text("�����̽��� �����մϴ�.");
					printf("�����̽��� �����մϴ�( spice : %d )\n", resource->spice);
					break;
				}
				else {
					sys_text("�Ϻ����͸� �����ϰ� �ֽ��ϴ�.");
					resource->spice -= 5;
					for (int i = 10; i > 1; i--)
					{
						printf("�Ϻ����� �������� ���� �ð� : %d", i);
					}
					printf("�Ϻ����Ͱ� �����Ǿ����ϴ� ( spice : %d -> %d )\n", resource->spice + 5, resource->spice); 
					break;
				}
			}
		}

	}

	if (strcmp(info, "�Ϻ�����") == 0) {
		gotoxy((POSITION) { cmd_map_pos.row, cmd_map_pos.column });
		printf(" H : �ڿ� ��Ȯ");
		gotoxy((POSITION) { cmd_map_pos.row + 1, cmd_map_pos.column });
		printf(" M : �̵�");

		char input;
		while (1) {
			input = get_key_non_blocking();  // �񵿱������� Ű �Է� �ޱ�

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

//����â ������ ����� �Լ�
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
		set_color(7);  // �⺻ ���
	}
}


bool find_h_positions(POSITION* friend_h, POSITION* enemy_h) {
	bool friend_found = false;
	bool enemy_found = false;

	for (int row = 0; row < MAP_HEIGHT; row++) {
		for (int col = 0; col < MAP_WIDTH; col++) {
			// 'H'�� �ִ��� Ȯ���ϰ� �ش� ��ǥ�� ������ color_map���� ������
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
	return friend_found && enemy_found;  // �� ���� H�� ��� ã�� ��쿡�� true ��ȯ
}

int has_reached_target(Sandworm sandworm, POSITION target) {
	return sandworm.pos.row == target.row && sandworm.pos.column == target.column;
}

void move_sandworm_toward_target(Sandworm* sandworm, POSITION target) {
	// ���� ��ġ �ʱ�ȭ (ȭ�鿡�� �����)
	map[1][sandworm->pos.row][sandworm->pos.column] = -1;

	// Ÿ�ٱ����� ����/���� �Ÿ� ���
	int row_diff = target.row - sandworm->pos.row;
	int col_diff = target.column - sandworm->pos.column;
	// �̵� ���� ����
	if (abs(row_diff) > abs(col_diff)) {
		sandworm->pos.row += (row_diff > 0) ? 1 : -1;
	}
	else {
		sandworm->pos.column += (col_diff > 0) ? 1 : -1;
	}

	// ���ο� ��ġ ������Ʈ
	map[1][sandworm->pos.row][sandworm->pos.column] = 'W';
}

void sand_mob(POSITION friend_h, POSITION enemy_h) {
	// �»�� ������� �Ϻ����� �� �Ÿ� ���
	double friend_sandworm0 = sqrt(pow(friend_h.row - sandworms[0].pos.row, 2) + pow(friend_h.column - sandworms[0].pos.column, 2));
	double enemy_sandworm0 = sqrt(pow(enemy_h.row - sandworms[0].pos.row, 2) + pow(enemy_h.column - sandworms[0].pos.column, 2));

	if (!has_reached_target(sandworms[0], friend_h) && friend_sandworm0 < enemy_sandworm0) {
		move_sandworm_toward_target(&sandworms[0], friend_h);
	}
	else if (!has_reached_target(sandworms[0], enemy_h)) {
		move_sandworm_toward_target(&sandworms[0], enemy_h);
	}

	// ���ϴ� ������� �Ϻ����� �� �Ÿ� ���
	double friend_sandworm1 = sqrt(pow(friend_h.row - sandworms[1].pos.row, 2) + pow(friend_h.column - sandworms[1].pos.column, 2));
	double enemy_sandworm1 = sqrt(pow(enemy_h.row - sandworms[1].pos.row, 2) + pow(enemy_h.column - sandworms[1].pos.column, 2));

	if (!has_reached_target(sandworms[1], friend_h) && friend_sandworm1 < enemy_sandworm1) {
		move_sandworm_toward_target(&sandworms[1], friend_h);
	}
	else if (!has_reached_target(sandworms[1], enemy_h)) {
		move_sandworm_toward_target(&sandworms[1], enemy_h);
	}
}
