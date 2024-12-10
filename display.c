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
void formation(int i, int j, const char* info);

//â�� �ִ� ���� ����� �Լ�
void clear_sta_map_area();

//��Ÿ��� (H, �����)
void sand_mob();

//�Ϻ����� ��ǥ ã�� �Լ�
bool find_h_positions(POSITION* friend_h, POSITION* enemy_h);

//H �� ���� �迭
int color_map[MAP_HEIGHT][MAP_WIDTH] = { 7 }; //7 : �⺻ ��
//����� �ӵ�
int turn_counter = 0;
typedef struct {
	int id;           // ���� ID
	POSITION pos;     // ���� ��ġ
	POSITION prev_pos; // ���� ��ġ�� ����
} Sandworm;

Sandworm sandworms[2] = {
	{ // �»�� �����
		.id = 1,
		.pos = {3, 8},
		.prev_pos = {3, 8}
	},
	{ // ���ϴ� �����
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
	if (strcmp(info, "����") == 0) {
		set_color(BLACK);
		color_map[i][j] = BLACK;
	}
	else if (strcmp(info, "�Ʊ� ����") == 0) {
		set_color(BLUE);
		color_map[i][j] = BLUE;
	}
	else if (strcmp(info, "�� ����") == 0) {
		set_color(DARK_RED);
		color_map[i][j] = DARK_RED;
	}
	else if (strcmp(info, "�Ʊ� �Ϻ�����") == 0) {
		set_color(BLUE);
		color_map[i][j] = BLUE;
	}
	else if (strcmp(info, "���� �Ϻ�����") == 0) {
		set_color(DARK_RED);
		color_map[i][j] = DARK_RED;
	}
	else if (strcmp(info, "�������� �Ϻ�����") == 0) {
		set_color(GRAY);
		color_map[i][j] = GRAY;
	}
	else if (strcmp(info, "�����̽�") == 0) {
		set_color(RED);
		color_map[i][j] = RED;
	}
	else if (strcmp(info, "�����") == 0) {
		set_color(DARK_YELLOW);
		color_map[i][j] = DARK_YELLOW;
	}
	else if (strcmp(info, "����") == 0) {
		set_color(GRAY);
		color_map[i][j] = GRAY;
	}
	else {
		set_color(7); // �⺻ ���
		color_map[i][j] = 7;
	}
}

void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	static bool is_first_call = true;

	// ���� ����ÿ��� �Ϻ����� ���� �ʱ�ȭ
	if (is_first_call) {
		initialize_harvester_colors();
		is_first_call = false;
	}

	project(map, backbuf);

	// �Ϻ����� ���� ���� �� ������ ȿ��
	if (is_producing_harvester) {
		backbuf[production_pos.row][production_pos.column] = 'H';
		// Ŀ�� ��ġ�� �����ϰ� ������
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
				const char* info = "�縷 ����";  // �⺻�� ����
				switch (backbuf[i][j]) {
				case 'P':
					info = "����";
					break;
				case 'B':
					info = (i == 16 || i == 15) ? "�Ʊ� ����" : "�� ����";
					break;
				case 'H':
					if (color_map[i][j] == BLUE) {
						info = "�Ʊ� �Ϻ�����";
					}
					else if (color_map[i][j] == DARK_RED) {
						info = "���� �Ϻ�����";
					}
					else if (color_map[i][j] == GRAY) {
						info = "�������� �Ϻ�����";
					}
					break;
				case 'W':
					info = "�����";
					break;
				case 'R':
					info = "����";
					break;
				case 'S':
					info = "�����̽�";
					break;
				}

				formation(i, j, info);
				POSITION pos = { i, j };
				gotoxy(padd(map_pos, pos));

				// ���� ���� �Ϻ������� ��� color_map�� ���� ���
				if (is_producing_harvester && i == production_pos.row && j == production_pos.column) {
					set_color(color_map[i][j]);
				}

				printf("%c", backbuf[i][j]);
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

	set_color(color);  // ������ġ ���� ����
	printc(padd(map_pos, prev), ch, color);

	// ���� Ŀ�� ��ġ�� ���ڸ� Ŀ�� �������� ���
	ch = frontbuf[curr.row][curr.column];
	set_color(COLOR_CURSOR);  // Ŀ�� ���� ����
	printc(padd(map_pos, curr), ch, COLOR_CURSOR);

	// ���� �ʱ�ȭ
	set_color(COLOR_DEFAULT);
}


void initialize_harvester_colors() {
	// �Ʊ� �Ϻ����� �ʱ� ��ġ (i = 14, 15, 16)
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

	// �Ʊ� �Ϻ����� ���� ����
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
// �Ϻ����� �̵� �� ������ �����ϴ� �Լ�
void move_harvester_with_color(POSITION from, POSITION to) {
	if (map[1][from.row][from.column] == 'H') {
		// �̵��� �� ���� �Բ� �̵�
		int color = color_map[from.row][from.column];
		map[1][from.row][from.column] = -1;  // ���� ��ġ ����
		color_map[from.row][from.column] = 7; // ���� ��ġ ���� �ʱ�ȭ

		map[1][to.row][to.column] = 'H';     // �� ��ġ�� �Ϻ����� ��ġ
		color_map[to.row][to.column] = color; // �� ��ġ�� ���� ����
	}
}
// ���ο� �Ϻ����� ���� �� ���� ���� �Լ�
void create_new_harvester(POSITION pos, bool is_friend) {
	map[1][pos.row][pos.column] = 'H';
	color_map[pos.row][pos.column] = is_friend ? BLUE : DARK_RED;
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
	project_sys_map(system_map, sys_backbuf);

	turn_counter++;

	if (is_producing_harvester && turn_counter >= harvester_production_time) {
		// ���� �Ϸ�
		is_producing_harvester = false;
		map[1][production_pos.row][production_pos.column] = 'H';

		// ��� �Ʊ� ���� ����
		color_map[production_pos.row][production_pos.column] = BLUE;

		// ������ ��� �ݿ��ǵ��� frontbuf�� backbuf ��� ������Ʈ
		POSITION pos = production_pos;
		gotoxy(padd(map_pos, pos));
		set_color(BLUE);
		printf("%c", 'H');

		sys_text("�Ϻ����Ͱ� �����Ǿ����ϴ�.");
	}
	// �Ϻ����� ��ġ ã�⸦ �Ϻ����� ���� ���ķ� �̵�
	if (turn_counter % 10 == 0) {  // �� 10ƽ����
		sand_mob();  // ����� �̵�
	}

	display_system_messages();
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

	// �Ϻ����� ���� ������ ǥ���ϱ�� �ߴٸ� ��� ������Ʈ
	if (is_showing_harvester_production && is_producing_harvester) {
		static int last_shown_seconds = -1;  // ���������� ǥ���� �ð�

		// ���� �ð� ���
		int remaining_ticks = harvester_production_time - turn_counter;
		int remaining_seconds = remaining_ticks / 50;

		// �ʰ� ������� ���� ������Ʈ
		if (remaining_seconds != last_shown_seconds) {
			gotoxy((POSITION) { sta_map_pos.row, sta_map_pos.column });
			printf("�������� �Ϻ�����");
			gotoxy((POSITION) { sta_map_pos.row + 1, sta_map_pos.column });
			printf("�������� ���� �ð�: %d��    ", remaining_seconds);  // ���� �߰��� ���� ���� �����

			last_shown_seconds = remaining_seconds;
		}
	}

	// ������ ����â ������Ʈ (���� ���� ������Ʈ �ڵ�)
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

// �Ʊ� ���� �ֺ��� �� ������ ã�� �Լ�
bool find_empty_space_near_base(POSITION* pos) {
	// �Ʊ� ���� ��ġ (15-16, 1-2)
	POSITION possible_spots[] = {
		{14, 2},
		{15, 3},
		{16, 3},
		{14, 1}
	};
	int num_spots = 4;  // ��ġ ������ ��ġ ���� ����

	// ������ ��ġ���� �������� ����
	for (int i = num_spots - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		POSITION temp = possible_spots[i];
		possible_spots[i] = possible_spots[j];
		possible_spots[j] = temp;
	}

	// �� ���� ã��
	for (int i = 0; i < num_spots; i++) {
		if (map[1][possible_spots[i].row][possible_spots[i].column] == -1) {
			*pos = possible_spots[i];
			return true;
		}
	}
	return false;
}


//�����̽��� ������ ����â�� ���� ���, ���â�� ��ɾ� ���
void display_info_in_sta_map(char ch, POSITION pos, RESOURCE* resource) {
	//sta_map â�� ������ ���� ��� ����
	clear_sta_map_area();
	// sta_map â�� Ŀ���� ��ġ�� ��ġ�� ���� ���
	const char* info = "�Ʊ� �Ϻ�����";
	switch (ch) {
	case 'P': info = "����"; break;
	case 'B': info = is_enemy_map[pos.row][pos.column] ? "�� ����" : "�Ʊ� ����"; break;
	case 'R': info = "����"; break; //���߿� 2x2�� ����, 1x1�� ������
	case 'S': info = "�����̽�"; break;
	case 'W': info = "�����"; break;
	case 'H':
		// ���� ���� �Ϻ����� ó���� ����
		// ���� ���� �Ϻ����� ó��
		if (ch == 'H' && is_producing_harvester &&
			pos.row == production_pos.row &&
			pos.column == production_pos.column) {

			is_showing_harvester_production = true;

			// ���� �ð� ǥ��
			gotoxy((POSITION) { sta_map_pos.row, sta_map_pos.column });
			printf("�������� �Ϻ�����");

			char input = get_key_non_blocking();
			if (input == 'x' || input == 'X') {
				gotoxy((POSITION) { cmd_map_pos.row, cmd_map_pos.column });
				printf(" X : ���� ���");

				// ���� ��� �Է� ���
				while (1) {
					input = get_key_non_blocking();
					display_system_messages();

					if (input == 'X' || input == 'x') {
						is_producing_harvester = false;
						map[1][production_pos.row][production_pos.column] = -1;
						resource->spice += 3;
						sys_text("�Ϻ����� ������ ��ҵǾ����ϴ�.");
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
		// �Ϲ� �Ϻ����� ó��
		else if (color_map[pos.row][pos.column] == BLUE) {
			info = "�Ʊ� �Ϻ�����";
		}
		else if (color_map[pos.row][pos.column] == DARK_RED) {
			info = "���� �Ϻ�����";
		}
		break;
	default: info = "�縷 ����"; break;
	}

	gotoxy((POSITION) { sta_map_pos.row, sta_map_pos.column }); // sta_map â ��ġ�� �̵�
	printf("%s\n", info);
	if (strcmp(info, "�����̽�") == 0)
	{
		gotoxy((POSITION) { sta_map_pos.row + 1, sta_map_pos.column }); //�̰� ���߿� sys_text ó�� �ٲ���
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
					break;
				}
				else {
					// ���� ������ ��ġ ã��
					if (!find_empty_space_near_base(&production_pos)) {
						sys_text("�Ϻ����͸� ������ ������ �����ϴ�.");
						break;
					}

					sys_text("�Ϻ����͸� ������...");
					resource->spice -= 5;

					// ���� ����
					is_producing_harvester = true;
					harvester_production_time = turn_counter + 500; // 10�� (50ƽ/�� ����)
					break;
				}
			}
		}

	}

	if (strcmp(info, "�Ʊ� �Ϻ�����") == 0) {
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
	return friend_found && enemy_found;  // �� ���� H�� ��� ã�� ��쿡�� true ��ȯ
}

int has_reached_target(Sandworm sandworm, POSITION target) {
	return sandworm.pos.row == target.row && sandworm.pos.column == target.column;
}

bool can_move_to(POSITION pos) {
	// �� ������ ����ų� ������ ������ �̵� �Ұ�
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
	// ���� ��ġ�� W �����
	map[1][sandworm->pos.row][sandworm->pos.column] = -1;

	// ��ǥ�� ���� ��� ���� �̵�
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

		// �̵��� �� ������ ���� ��ġ�� �ٽ� W ǥ��
		if (!moved) {
			map[1][sandworm->pos.row][sandworm->pos.column] = 'W';
		}
		return;
	}

	// ��ǥ�� ���� ���� �̵� ������ ���� ������� ����
	POSITION moves[4] = {
		{sandworm->pos.row - 1, sandworm->pos.column},
		{sandworm->pos.row + 1, sandworm->pos.column},
		{sandworm->pos.row, sandworm->pos.column - 1},
		{sandworm->pos.row, sandworm->pos.column + 1}
	};

	double min_dist = 99999.0;
	POSITION best_move = sandworm->pos;  // �⺻������ ���� ��ġ ����

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

		// ��� �Ϻ����Ͱ� ������ ���� �̵�
		if (harvesters.friend_count == 0 && harvesters.enemy_count == 0) {
			move_sandworm_toward_target(sandworm, (POSITION) { -1, -1 });
			continue;
		}

		// ���� ����� �Ϻ����� ����
		POSITION closest_harvester = { -1, -1 };
		double min_distance = 99999.0;

		// �Ʊ� �Ϻ����� �� ���� ����� �� ã��
		for (int j = 0; j < harvesters.friend_count; j++) {
			double distance = sqrt(pow(harvesters.friend_harvesters[j].row - sandworm->pos.row, 2) +
				pow(harvesters.friend_harvesters[j].column - sandworm->pos.column, 2));
			if (distance < min_distance) {
				min_distance = distance;
				closest_harvester = harvesters.friend_harvesters[j];
			}
		}

		// ���� �Ϻ����� �� ���� ����� �� ã��
		for (int j = 0; j < harvesters.enemy_count; j++) {
			double distance = sqrt(pow(harvesters.enemy_harvesters[j].row - sandworm->pos.row, 2) +
				pow(harvesters.enemy_harvesters[j].column - sandworm->pos.column, 2));
			if (distance < min_distance) {
				min_distance = distance;
				closest_harvester = harvesters.enemy_harvesters[j];
			}
		}

		// ��ǥ�� �̵�
		move_sandworm_toward_target(sandworm, closest_harvester);
	}
}

HarvesterList find_harvesters() {
	HarvesterList harvester_list = { .friend_count = 0, .enemy_count = 0 };

	for (int row = 0; row < MAP_HEIGHT; row++) {
		for (int col = 0; col < MAP_WIDTH; col++) {
			if (map[1][row][col] == 'H') {
				if (color_map[row][col] == BLUE && harvester_list.friend_count < MAX_HARVESTERS) {
					// �Ʊ� �Ϻ�����
					harvester_list.friend_harvesters[harvester_list.friend_count].row = row;
					harvester_list.friend_harvesters[harvester_list.friend_count].column = col;
					harvester_list.friend_count++;
				}
				else if (color_map[row][col] == DARK_RED && harvester_list.enemy_count < MAX_HARVESTERS) {
					// ���� �Ϻ�����
					harvester_list.enemy_harvesters[harvester_list.enemy_count].row = row;
					harvester_list.enemy_harvesters[harvester_list.enemy_count].column = col;
					harvester_list.enemy_count++;
				}
			}
		}
	}

	return harvester_list;
}
