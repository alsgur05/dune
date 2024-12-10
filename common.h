#ifndef _COMMON_H_
#define _COMMON_H_

#define MAX_MESSAGES 6  // 최대 저장할 메시지 수
#define MAX_MESSAGE_LENGTH 100  // 각 메시지의 최대 길이

#define MAX_HARVESTERS 50 //아군, 적군 하베스터 총 합

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

/* ================= 위치와 방향 =================== */
// 맵에서 위치를 나타내는 구조체
typedef struct {
	int row, column;
} POSITION;

// 커서 위치
typedef struct {
	POSITION previous;  // 직전 위치
	POSITION current;   // 현재 위치
} CURSOR;

extern CURSOR cursor;

// 입력 가능한 키 종류.
// 수업에서 enum은 생략했는데, 크게 어렵지 않으니 예제 검색
typedef enum {
	// k_none: 입력된 키가 없음. d_stay(안 움직이는 경우)에 대응
	k_none = 0, k_up, k_right, k_left, k_down,
	k_quit, k_space,
	k_undef, // 정의되지 않은 키 입력	
} KEY;


// DIRECTION은 KEY의 부분집합이지만, 의미를 명확하게 하기 위해서 다른 타입으로 정의
typedef enum {
	d_stay = 0, d_up, d_right, d_left, d_down
} DIRECTION;


/* ================= 위치와 방향(2) =================== */
// 편의성을 위한 함수들. KEY, POSITION, DIRECTION 구조체들을 유기적으로 변환

// 편의성 함수
inline POSITION padd(POSITION p1, POSITION p2) {
	POSITION p = { p1.row + p2.row, p1.column + p2.column };
	return p;
}

// p1 - p2
inline POSITION psub(POSITION p1, POSITION p2) {
	POSITION p = { p1.row - p2.row, p1.column - p2.column };
	return p;
}

// 방향키인지 확인하는 함수
#define is_arrow_key(k)		(k_up <= (k) && (k) <= k_down)

// 화살표 '키'(KEY)를 '방향'(DIRECTION)으로 변환. 정수 값은 똑같으니 타입만 바꿔주면 됨
#define ktod(k)		(DIRECTION)(k)

// DIRECTION을 POSITION 벡터로 변환하는 함수
inline POSITION dtop(DIRECTION d) {
	static POSITION direction_vector[] = { {0, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 0} };
	return direction_vector[d];
}

// p를 d 방향으로 이동시킨 POSITION
#define pmove(p, d)		(padd((p), dtop(d)))

/* ================= game data =================== */
typedef struct {
	int spice;		// 현재 보유한 스파이스
	int spice_max;  // 스파이스 최대 저장량
	int population; // 현재 인구 수
	int population_max;  // 수용 가능한 인구 수
} RESOURCE;


// 대강 만들어 봤음. 기능 추가하면서 각자 수정할 것
typedef struct {
	POSITION pos;		// 현재 위치(position)
	POSITION dest;		// 목적지(destination)
	char repr;			// 화면에 표시할 문자(representation)
	int move_period;	// '몇 ms마다 한 칸 움직이는지'를 뜻함
	int next_move_time;	// 다음에 움직일 시간
	int speed;
} OBJECT_SAMPLE;

enum ColorType {
	RED = 12, //하코넨(AI)
	GRAY = 8, //돌
	BLUE = 9, //아트레이디스(플레이어)
	DARK_YELLOW = 6, //샌드웜
	DARK_RED = 4, //스파이스
	BLACK = 0 //장판

} COLOR ;

extern bool is_enemy_map[MAP_HEIGHT][MAP_WIDTH];
extern char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH];
extern int color_map[MAP_HEIGHT][MAP_WIDTH]; //H를 색으로 적군 아군 구별하기위해 만든 배열

extern const POSITION sys_map_pos; //engine.c에도 사용할라고

typedef struct {
	char sys_backbuf[SYS_HEIGHT][SYS_WIDTH];  // 시스템 창 텍스트를 저장하는 버퍼
} SysMap;

typedef struct {
	char messages[MAX_MESSAGES][MAX_MESSAGE_LENGTH];
	int count;  // 현재 저장된 메시지 수
} MessageQueue;

typedef struct {
	POSITION friend_harvesters[MAX_HARVESTERS];
	POSITION enemy_harvesters[MAX_HARVESTERS];
	int friend_count;
	int enemy_count;
} HarvesterList;


typedef struct {
	int id;           // 고유 ID
	POSITION pos;     // 현재 위치
	POSITION prev_pos; // 이전 위치를 저장
	bool is_digesting;  // 소화 중인지 여부
	int digestion_start_time;  // 소화 시작 시간
	int harvester_eaten;  // 먹은 하베스터 수
} Sandworm;

#endif
