/*
 * 7_SEGMENT.c
 *
 *  Created on: Oct 27, 2025
 *      Author: mtri1
 */

#include "7_SEGMENT.h"

static int seg_duration = 0;
static int seg_mode = 0;

const uint8_t seg_pattern[10][7] = { { 0, 0, 0, 0, 0, 0, 1 }, //0
		{ 1, 0, 0, 1, 1, 1, 1 }, //1
		{ 0, 0, 1, 0, 0, 1, 0 }, //2
		{ 0, 0, 0, 0, 1, 1, 0 }, //3
		{ 1, 0, 0, 1, 1, 0, 0 }, //4
		{ 0, 1, 0, 0, 1, 0, 0 }, //5
		{ 0, 1, 0, 0, 0, 0, 0 }, //6
		{ 0, 0, 0, 1, 1, 1, 1 }, //7
		{ 0, 0, 0, 0, 0, 0, 0 }, //8
		{ 0, 0, 0, 0, 1, 0, 0 }, //9
		};

uint16_t segPins_A[7] = { Seg1_a_Pin, Seg1_b_Pin, Seg1_c_Pin, Seg1_d_Pin,
Seg1_e_Pin, Seg1_f_Pin, Seg1_g_Pin };

uint16_t segPins_B[7] = { Seg2_a_Pin, Seg2_b_Pin, Seg2_c_Pin, Seg2_d_Pin,
Seg2_e_Pin, Seg2_f_Pin, Seg2_g_Pin };

void display7SEG(int num, uint16_t *segPins) {

	if (num < 0 || num > 9)
		num = 0;

	for (int i = 0; i < 7; i++) {
		HAL_GPIO_WritePin(GPIOB, segPins[i],
				seg_pattern[num][i] ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}

void set7SEGValues(int duration, int mode) {
    seg_duration = duration;
    seg_mode = mode;
}

void update7SEG(void) {
    // Logic multiplexing (quét LED)
	static int state = 0;

    // Lấy giá trị từ biến static
	int tensA = seg_duration / 10;
	int onesA = seg_duration % 10;
	int tensB = seg_mode / 10;
	int onesB = seg_mode % 10;

	if (state == 1) {
		// Display Ten digits
		HAL_GPIO_WritePin(GPIOA, EN_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, EN_B_Pin, GPIO_PIN_RESET);
		display7SEG(tensA, segPins_A);
		display7SEG(tensB, segPins_B);
	} else if (state == 0) {
		// Display One digits
		HAL_GPIO_WritePin(GPIOA, EN_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, EN_B_Pin, GPIO_PIN_SET);
		display7SEG(onesA, segPins_A);
		display7SEG(onesB, segPins_B);
	}
	state = 1 - state; // Đổi trạng thái quét
}
