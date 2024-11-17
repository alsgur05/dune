/*
* raw(?) I/O
*/
#include "io.h"

#define DOUBLE_PRESS_INTERVAL 200

void gotoxy(POSITION pos) {
	COORD coord = { pos.column, pos.row }; // 행, 열 반대로 전달
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void set_color(int color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void printc(POSITION pos, char ch, int color) {
	if (color >= 0) {
		set_color(color);
	}
	gotoxy(pos);
	printf("%c", ch);
}

KEY get_key(void) {
	static int last_key = k_none;
	static clock_t last_time = 0;

	if (!_kbhit()) {  // 입력된 키가 있는지 확인
		return k_none;
	}

	int byte = _getch();    // 입력된 키를 전달 받기
	int key;
	switch (byte) {
	case 'q': return k_quit;  // 'q'를 누르면 종료
	case ' ': return k_space; //스페이스 바
	case 27: return k_esc;
	case 224:
		byte = _getch();  // MSB 224가 입력 되면 1바이트 더 전달 받기
		switch (byte) {
		case 72: key = k_up; break;
		case 75: key = k_left; break;
		case 77: key = k_right; break;
		case 80: key = k_down; break;
		default: return k_undef;
		}
		break;
	default: return k_undef;
	}

	clock_t current_time = clock();
	if (key == last_key && (current_time - last_time) < (DOUBLE_PRESS_INTERVAL * CLOCKS_PER_SEC / 1000)) {
		last_time = 0;  // 다음 입력을 기다림
		last_key = k_none;
		return (KEY)(key + 10);  // 예: k_up_double = k_up + 10
	}

	last_key = key;
	last_time = current_time;
	return key;
}
