/*
 * psx_pad.h
 *
 *  Created on: 7 wrz 2019
 *  Updated on: 30 lip 2024
 *      Author: Mariusz
 */

#ifndef PSX_PAD_H
#define PSX_PAD_H


// PSX_PAD v2.0
// https://store.curiousinventor.com/guides/PS2/
//
// 1: Data (MISO) ( 2,2k pull up )
// 2: Command (MOSI)
// 3: 7V Rumble motor
// 4: GND
// 5: 3,3V
// 6: ATT
// 7: CLK (SCK)
// 8: NC
// 9: ACK
//
// SPI
// SPI_MODE: 3
// AVR: (1<<CPHA) | (1<<CPOL)
// STM32: SPI_POLARITY_HIGH, SPI_PHASE_2EDGE
// SPI_BIT_ORDER: LSB first
// SPI_CLK_SPEED: <250kHz

#define PSX_PAD_DATA_MAX	(21)

typedef enum PSX_PAD_MODE{
	PSX_PAD_MODE_DIGITAL 	= 0x04,
	PSX_PAD_MODE_ANALOG 	= 0x07,
	PSX_PAD_MODE_CONFIG		= 0x0F
}PSX_pad_mode_t;

typedef enum PSX_PAD_LOCK{
	PSX_PAD_LOCKED,
	PSX_PAD_UNLOCKED
}PSX_pad_lock_t;

typedef enum PSX_PAD_INIT{
	PSX_PAD_INIT_ERROR,
	PSX_PAD_INIT_OK
}PSX_pad_init_t;

typedef void(*callback_pad_psx_att_set)(void);
typedef void(*callback_pad_psx_att_clr)(void);
typedef uint8_t(*callback_pad_psx_spi_transfer)(uint8_t byte);
typedef void(*callback_pad_psx_delay_ms)(uint16_t ms);

typedef struct{
	union{
		struct{
			uint8_t button0;
			uint8_t button1;
			uint8_t right_h;
			uint8_t right_v;
			uint8_t left_h;
			uint8_t left_v;
		}map;

		uint8_t raw[PSX_PAD_DATA_MAX];
	}data;

	callback_pad_psx_att_clr _att_clr;
	callback_pad_psx_att_set _att_set;
	callback_pad_psx_spi_transfer _spi_transfer;
	callback_pad_psx_delay_ms _delay_ms;
}PSX_pad_t;

PSX_pad_init_t psx_pad_init(PSX_pad_t* pad, PSX_pad_mode_t pad_mode, PSX_pad_lock_t pad_lock);
PSX_pad_mode_t psx_pad_poll(PSX_pad_t* pad, uint8_t motor_small, uint8_t motor_large);

#endif /* PSX_PAD_H */
