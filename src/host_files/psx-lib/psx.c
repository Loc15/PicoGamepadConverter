#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"

#include "clock.pio.h"


#define N_BYTES 9

static PIO psx_pio;
uint sm;
int dma_chan;
static uint32_t data_psx[N_BYTES];
const uint cmd = 0x01 | (0x42<<8);

void (*callback_psx)( uint32_t *) = {0}; //callback from the main


void __not_in_flash_func(dma_handler)() {

    // Clear the interrupt request.
    dma_hw->ints0 = 1u << dma_chan;
    // re-trigger it
    dma_channel_set_read_addr(dma_chan, &psx_pio->rxf[sm], false);
    dma_channel_set_write_addr(dma_chan, &data_psx[0], false);
    /*Reiniciar cuenta*/
    dma_channel_set_trans_count(dma_chan, N_BYTES, false);
    /*Reiniciar la channel trigger*/
    dma_channel_start(dma_chan);

    (*callback_psx)(data_psx);   //Call the callback function

    /*send cmd*/
    pio_sm_put_blocking(psx_pio, sm, cmd);

}


void psx_init(uint pio, uint gpio_output, uint gpio_input, void (*fn)(uint32_t *))
{

    //callback function
    callback_psx = fn;

    /*Which pio use*/
    psx_pio = pio ? pio1 : pio0;

    /*CONFIG PIO*/
    uint offset = pio_add_program(psx_pio, &clock_program);
    sm = pio_claim_unused_sm(psx_pio, true);
    pio_sm_config c = clock_program_get_default_config(offset);
    /*mapping set pins*/
    sm_config_set_set_pins(&c, gpio_output+1, 2);
    /*mapping out pins*/
    sm_config_set_out_pins(&c, gpio_output, 1);
    /*mapping side site pins*/
    sm_config_set_sideset_pins(&c, gpio_output);
    /*mapping in pins*/
    sm_config_set_in_pins(&c, gpio_input);
    /*Shift 8 bits autopush enabled*/
    sm_config_set_in_shift(&c, true, true, 8);
    pio_gpio_init(psx_pio, gpio_output);
    pio_gpio_init(psx_pio, gpio_output+1);
    pio_gpio_init(psx_pio, gpio_output+2);
    pio_gpio_init(psx_pio, gpio_input);
    gpio_pull_up(gpio_input);
    gpio_pull_up(gpio_output+1);
    gpio_pull_up(gpio_output+2);
    pio_sm_set_consecutive_pindirs(psx_pio, sm, gpio_output, 3, true);
    pio_sm_set_consecutive_pindirs(psx_pio, sm, gpio_input, 1, false);
    float div = (float)clock_get_hz(clk_sys) / (6 * 12500);
    sm_config_set_clkdiv(&c, div);
    pio_sm_init(psx_pio, sm, offset, &c);
    pio_sm_set_enabled(psx_pio, sm, true);
    /*FIN CONFIG PIO*/

    /*DMA*/
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c_dma = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c_dma, DMA_SIZE_32);
    channel_config_set_read_increment(&c_dma, false);
    channel_config_set_write_increment(&c_dma, true);
    channel_config_set_dreq(&c_dma, pio_get_dreq(psx_pio, sm, false));

    dma_channel_configure(dma_chan, &c_dma,
        &data_psx[0],        // Destination pointer
        &psx_pio->rxf[sm],      // Source pointer
        N_BYTES, // Number of transfers
        false                // Start immediately
    );

    // Tell the DMA to raise IRQ line 0 when the channel finishes a block
    dma_channel_set_irq0_enabled(dma_chan, true);

    // Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Manually call the handler once, to trigger the first transfer
    dma_handler();
    /*DMA*/

}



/*
PS1_CLOCK -> 12.5Khz
PIO_CLOCK = Freq_sys / (PS1_CLOCK * 6)
*/
