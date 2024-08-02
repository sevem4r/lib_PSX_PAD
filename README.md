# Resources
https://store.curiousinventor.com/guides/PS2

# Usage example

```c
#include <avr/io.h>
#include <util/delay.h>
#include "psx_pad.h"
#include "spi.h"

#define ATT      (1<<PD6)
#define ATT_SET  PORTD |= ATT;
#define ATT_CLR  PORTD &= ~ATT;

// GND
// 3,3V/5V
// MDO	-> MOSI
// MDI	-> MISO (2,2k pull-up)
// CLK	-> SCK
// ACK
// CS	-> CS

static void psx_pad_att_set(void){
  ATT_SET;
}

static void psx_pad_att_clr(void){
  ATT_CLR;
}

static void psx_pad_delay_ms(uint16_t ms){
  while(ms--){
    _delay_ms(1);
  }
}

static uint8_t psx_pad_spi_transfer(uint8_t byte){
  return spi_transfer(byte);
}

PSX_pad_t pad = {
  ._att_clr = psx_pad_att_clr,
  ._att_set = psx_pad_att_set,
  ._spi_transfer = psx_pad_spi_transfer,
  ._delay_ms = psx_pad_delay_ms
};

int main(void){

  DDRD |= ATT;

  spi_init();

  psx_pad_init(&pad, PSX_PAD_MODE_ANALOG, PSX_PAD_LOCKED);

  while(1){
    PSX_pad_mode_t mode = psx_pad_poll(&pad, 0, 0);

    if(mode == PSX_PAD_MODE_ANALOG){
      // pad.data.map.button0,
      // pad.data.map.button1,
      // pad.data.map.right_h,
      // pad.data.map.right_v,
      // pad.data.map.left_h,
      // pad.data.map.left_v
    }

    _delay_ms(100);
  }
}
'''
