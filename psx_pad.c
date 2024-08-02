/*
 * psx_pad.c
 *
 *  Created on: 7 wrz 2019
 *      Author: Mariusz
 */

#include <stdint.h>
#include <string.h>
#include "psx_pad.h"

#define PSX_PAD_COLD_START_DELAY	(1000)
#define PSX_PAD_DATA_DIGTAL			(2)
#define PSX_PAD_DATA_ANALOG			(6)
#define PSX_PAD_DATA_CONFIG			(6)

// Header
//		command			data
// 1: 	0x01			0xFF
// 2: 	0x41 - check	0x4n
//		0x42 - poll		0x4n - digital / 0x7n - analog | n - 16bit words
//		0x43 - cfg		0xFn
// 3:	0x00			0x5A
//
// Data
// n:	0x00			0xnn

static uint8_t psx_pad_mode(uint8_t data){
	return ((data >> 4) & 0x0F);
}

static uint8_t psx_pad_bytes(uint8_t data, uint8_t mode){
	uint8_t bytes;

	if(mode == PSX_PAD_MODE_CONFIG){
		bytes = PSX_PAD_DATA_CONFIG;
	}
	else{
		bytes = (data & 0x0F) * 2;
		bytes = (bytes > PSX_PAD_DATA_MAX) ? PSX_PAD_DATA_MAX : bytes;
	}

	return bytes;
}

PSX_pad_mode_t psx_pad_poll(PSX_pad_t* pad, uint8_t motor_small, uint8_t motor_large){
	uint8_t data, mode, bytes;

	pad->_att_clr();

	// header
	pad->_spi_transfer(0x01);
	data = pad->_spi_transfer(0x42);
	pad->_spi_transfer(0x00);

	mode = psx_pad_mode(data);
	bytes = psx_pad_bytes(data, mode);

	// data
	for(uint8_t i = 0; i < bytes; i++){
		uint8_t cmd;

		switch(i){
		case 0:
			cmd = motor_small;
			break;

		case 1:
			cmd = motor_large;
			break;

		default:
			cmd = 0x00;
			break;
		}

		if(
			(mode == PSX_PAD_MODE_DIGITAL && bytes == PSX_PAD_DATA_DIGTAL) ||
			(mode == PSX_PAD_MODE_ANALOG && bytes >= PSX_PAD_DATA_ANALOG)
		){
			pad->data.raw[i] = pad->_spi_transfer(cmd);
		}
		else{
			pad->_spi_transfer(cmd);
		}
	}

	pad->_att_set();

	return mode;
}

static void psx_pad_config_enter(PSX_pad_t* pad){
	uint8_t data, mode, bytes;

	pad->_att_clr();

	// header
	pad->_spi_transfer(0x01);
	data = pad->_spi_transfer(0x43);
	pad->_spi_transfer(0x00);

	mode = psx_pad_mode(data);
	bytes = psx_pad_bytes(data, mode);

	// data
	for(uint8_t i = 0; i < bytes; i++){
		pad->_spi_transfer((i == 0) ? 0x01 : 0x00);
	}

	pad->_att_set();
}

static void psx_pad_config_exit(PSX_pad_t* pad){
	uint8_t data, mode, bytes;

	pad->_att_clr();

	// header
	pad->_spi_transfer(0x01);
	data = pad->_spi_transfer(0x43);
	pad->_spi_transfer(0x00);

	mode = psx_pad_mode(data);
	bytes = psx_pad_bytes(data, mode);

	// data
	for(uint8_t i = 0; i < bytes; i++){
		pad->_spi_transfer(0x00);
	}

	pad->_att_set();
}

static void psx_pad_config_mode(PSX_pad_t* pad, PSX_pad_mode_t pad_mode, PSX_pad_lock_t pad_lock){
	uint8_t data, mode, bytes;

	pad->_att_clr();

	// header
	pad->_spi_transfer(0x01);
	data = pad->_spi_transfer(0x44);
	pad->_spi_transfer(0x00);

	mode = psx_pad_mode(data);
	bytes = psx_pad_bytes(data, mode);

	// data
	for(uint8_t i = 0; i < bytes; i++){
		uint8_t cmd;

		switch(i){
		case 0:
			// 0x01 - analog, 0x00 - digital
			cmd = (pad_mode == PSX_PAD_MODE_ANALOG) ? 0x01 : 0x00;
			break;

		case 1:
			// 0x03 - locked, 0x00 - unlocked
			cmd = (pad_lock == PSX_PAD_LOCKED) ? 0x03 : 0x00;
			break;

		default:
			cmd = 0x00;
			break;
		}

		pad->_spi_transfer(cmd);
	}

	pad->_att_set();
}

static void psx_pad_config_motor(PSX_pad_t* pad){
	uint8_t data, mode, bytes;

	pad->_att_clr();

	// header
	pad->_spi_transfer(0x01);
	data = pad->_spi_transfer(0x4D);
	pad->_spi_transfer(0x00);

	mode = psx_pad_mode(data);
	bytes = psx_pad_bytes(data, mode);

	// data
	for(uint8_t i = 0; i < bytes; i++){
		uint8_t cmd;

		switch(i){
		case 0:
			// 0x00 - map corresponding byte to control the small motor in 0x42 cmd
			// - 0xFF - turn on
			// - 0xnn - turn off
			cmd = 0x00;
			break;

		case 1:
			// 0x01 - map corresponding byte to controll the large motor in 0x42 cmd
			// - 0xnn - pwm value
			cmd = 0x01;
			break;

		default:
			cmd = 0xFF;
			break;
		}

		pad->_spi_transfer(cmd);
	}

	pad->_att_set();
}

PSX_pad_init_t psx_pad_init(PSX_pad_t* pad, PSX_pad_mode_t pad_mode, PSX_pad_lock_t pad_lock){
	if(
		pad == NULL ||
		pad->_att_clr == NULL ||
		pad->_att_set == NULL ||
		pad->_spi_transfer == NULL ||
		pad->_delay_ms == NULL
	){
		return PSX_PAD_INIT_ERROR;
	}

	pad->data.map.button0 = 255;
	pad->data.map.button1 = 255;
	pad->data.map.right_h = 127;
	pad->data.map.right_v = 127;
	pad->data.map.left_h = 127;
	pad->data.map.left_v = 127;

	pad->_att_set();

	pad->_delay_ms(PSX_PAD_COLD_START_DELAY);

	psx_pad_config_enter(pad);
	psx_pad_config_mode(pad, pad_mode, pad_lock);
	psx_pad_config_motor(pad);
	psx_pad_config_exit(pad);

	return PSX_PAD_INIT_OK;
}
