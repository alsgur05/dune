#ifndef _COMMON_H_
#define _COMMON_H_

#define MAX_MESSAGES 6  // �ִ� ������ �޽��� ��
#define MAX_MESSAGE_LENGTH 100  // �� �޽����� �ִ� ����

#define MAX_HARVESTERS 50 //�Ʊ�, ���� �Ϻ����� �� ��

#define FRIEND_BASE_START_ROW 15
#define FRIEND_BASE_END_ROW 16
#define FRIEND_BASE_START_COL 1
#define FRIEND_BASE_END_COL 2

#define ENEMY_BASE_START_ROW 1
#define ENEMY_BASE_END_ROW 2
#define ENEMY_BASE_START_COL 55
#define ENEMY_BASE_END_COL 56

#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include <conio.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <string.h>

/* ================= system parameters =================== */
#define TICK 10		// time unit(ms)

#define N_LAYER 2
#define MAP_WIDTH	60
#define MAP_HEIGHT	18

#define SYS_WIDTH 60
#define SYS_HEIGHT 8

#define STATUS_WIDTH 45
#define STATUS_HEIGHT 18

#define CMD_WIDTH 45
#define CMD_HEIGHT 8

/* ================= ��ġ�� ���� =================== */
// �ʿ��� ��ġ�� ��Ÿ���� ����ü
typedef struct {
	int row, column;
} POSITION;

// Ŀ�� ��ġ
typedef struct {
	POSITION previous;  // ���� ��ġ
	POSITION current;   // ���� ��ġ
} CURSOR;

extern CURSOR cursor;

// �Է� ������ Ű ����.
// �������� enum�� �����ߴµ�, ũ�� ����� ������ ���� �˻�
typedef enum {
	// k_none: �Էµ� Ű�� ����. d_stay(�� �����̴� ���)�� ����
	k_none = 0, k_up, k_right, k_left, k_down,
	k_quit, k_space,
	k_undef, // ���ǵ��� ���� Ű �Է�	
} KEY;


// DIRECTION�� KEY�� �κ�����������, �ǹ̸� ��Ȯ�ϰ� �ϱ� ���ؼ� �ٸ� Ÿ������ ����
typedef enum {
	d_stay = 0, d_up, d_right, d_left, d_down
} DIRECTION;


/* ================= ��ġ�� ����(2) =================== */
// ���Ǽ��� ���� �Լ���. KEY, POSITION, DIRECTION ����ü���� ���������� ��ȯ

// ���Ǽ� �Լ�
inline POSITION padd(POSITION p1, POSITION p2) {
	POSITION p = { p1.row + p2.row, p1.column + p2.column };
	return p;
}

// p1 - p2
inline POSITION psub(POSITION p1, POSITION p2) {
	POSITION p = { p1.row - p2.row, p1.column - p2.column };
	return p;
}

// ����Ű���� Ȯ���ϴ� �Լ�
#define is_arrow_key(k)		(k_up <= (k) && (k) <= k_down)

// ȭ��ǥ 'Ű'(KEY)�� '����'(DIRECTION)���� ��ȯ. ���� ���� �Ȱ����� Ÿ�Ը� �ٲ��ָ� ��
#define ktod(k)		(DIRECTION)(k)

// DIRECTION�� POSITION ���ͷ� ��ȯ�ϴ� �Լ�
inline POSITION dtop(DIRECTION d) {
	static POSITION direction_vector[] = { {0, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 0} };
	return direction_vector[d];
}

// p�� d �������� �̵���Ų POSITION
#define pmove(p, d)		(padd((p), dtop(d)))

/* ================= game data =================== */
typedef struct {
	int spice;		// ���� ������ �����̽�
	int spice_max;  // �����̽� �ִ� ���差
	int population; // ���� �α� ��
	int population_max;  // ���� ������ �α� ��
} RESOURCE;


// �밭 ����� ����. ��� �߰��ϸ鼭 ���� ������ ��
typedef struct {
	POSITION pos;		// ���� ��ġ(position)
	POSITION dest;		// ������(destination)
	char repr;			// ȭ�鿡 ǥ���� ����(representation)
	int move_period;	// '�� ms���� �� ĭ �����̴���'�� ����
	int next_move_time;	// ������ ������ �ð�
	int speed;
} OBJECT_SAMPLE;

enum ColorType {
	RED = 12, //���ڳ�(AI)
	GRAY = 8, //��
	BLUE = 9, //��Ʈ���̵�(�÷��̾�)
	DARK_YELLOW = 6, //�����
	DARK_RED = 4, //�����̽�
	BLACK = 0 //����

} COLOR ;

extern bool is_enemy_map[MAP_HEIGHT][MAP_WIDTH];
extern char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH];
extern int color_map[MAP_HEIGHT][MAP_WIDTH]; //H�� ������ ���� �Ʊ� �����ϱ����� ���� �迭

extern const POSITION sys_map_pos; //engine.c���� ����Ҷ��

typedef struct {
	char sys_backbuf[SYS_HEIGHT][SYS_WIDTH];  // �ý��� â �ؽ�Ʈ�� �����ϴ� ����
} SysMap;

typedef struct {
	char messages[MAX_MESSAGES][MAX_MESSAGE_LENGTH];
	int count;  // ���� ����� �޽��� ��
} MessageQueue;

typedef struct {
	POSITION friend_harvesters[MAX_HARVESTERS];
	POSITION enemy_harvesters[MAX_HARVESTERS];
	int friend_count;
	int enemy_count;
} HarvesterList;


typedef struct {
	int id;           // ���� ID
	POSITION pos;     // ���� ��ġ
	POSITION prev_pos; // ���� ��ġ�� ����
	bool is_digesting;  // ��ȭ ������ ����
	int digestion_start_time;  // ��ȭ ���� �ð�
	int harvester_eaten;  // ���� �Ϻ����� ��
} Sandworm;

#endif
