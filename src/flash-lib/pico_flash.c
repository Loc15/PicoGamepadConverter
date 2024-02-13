#include "pico/stdlib.h"
#include <hardware/flash.h>
#include "hardware/sync.h"

#include "pico_flash.h"

/*FLASH*/
const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
/*FLASH*/

uint8_t read_flash(int pos){
  return flash_target_contents[pos];
}

void write_flash(uint8_t buffer[], uint32_t size){
  uint32_t ints = save_and_disable_interrupts();
  flash_range_erase (FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
  flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE*size);
  restore_interrupts(ints);
}