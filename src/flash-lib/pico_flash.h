#define FLASH_TARGET_OFFSET (1920 * 1024)

uint8_t read_flash(int pos);

void write_flash(uint8_t buffer[], uint32_t size);