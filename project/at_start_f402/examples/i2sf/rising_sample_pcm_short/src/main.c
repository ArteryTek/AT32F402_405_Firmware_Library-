/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "at32f402_405_board.h"
#include "at32f402_405_clock.h"

/** @addtogroup AT32F402_periph_examples
  * @{
  */

/** @addtogroup 402_I2SF_extern_clock_pcm_long I2SF_extern_clock_pcm_long
  * @{
  */

#define TXBUF_SIZE                       32
#define RXBUF_SIZE                       TXBUF_SIZE

uint16_t i2sf5_buffer_tx[TXBUF_SIZE];
uint16_t i2sf5_buffer_rx[RXBUF_SIZE];
volatile error_status transfer_status1 = ERROR;

static void i2sf5_gpio_config(void);
static void i2sf5_config(void);
void tx_data_fill(void);
error_status buffer_compare(uint16_t* pbuffer1, uint16_t* pbuffer2, uint16_t buffer_length);

/**
  * @brief  buffer compare function.
  * @param  none
  * @retval the result of compare
  */
error_status buffer_compare(uint16_t* pbuffer1, uint16_t* pbuffer2, uint16_t buffer_length)
{
  while(buffer_length--)
  {
    if(*pbuffer1 != *pbuffer2)
    {
      return ERROR;
    }

    pbuffer1++;
    pbuffer2++;
  }
  return SUCCESS;
}

/**
  * @brief  transfer data fill.
  * @param  none
  * @retval none
  */
void tx_data_fill(void)
{
  uint32_t data_index = 0;
  for(data_index = 0; data_index < TXBUF_SIZE; data_index++)
  {
    i2sf5_buffer_tx[data_index] = data_index + 1;
  }
}

/**
  * @brief  i2sf5 configuration.
  * @param  none
  * @retval none
  */
static void i2sf5_config(void)
{
  dma_init_type dma_init_struct;
  i2s_init_type i2s_init_struct;

  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
  dmamux_enable(DMA1, TRUE);
  dma_reset(DMA1_CHANNEL3);
  dma_reset(DMA1_CHANNEL4);

  dmamux_init(DMA1MUX_CHANNEL3, DMAMUX_DMAREQ_ID_I2SF5_TX);
  dmamux_init(DMA1MUX_CHANNEL4, DMAMUX_DMAREQ_ID_I2SF5_RX);

  dma_default_para_init(&dma_init_struct);
  dma_init_struct.buffer_size = TXBUF_SIZE;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = FALSE;

  dma_init_struct.memory_base_addr = (uint32_t)i2sf5_buffer_tx;
  dma_init_struct.peripheral_base_addr = (uint32_t)&(I2SF5->dt);
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  dma_init(DMA1_CHANNEL3, &dma_init_struct);

  dma_init_struct.memory_base_addr = (uint32_t)i2sf5_buffer_rx;
  dma_init_struct.peripheral_base_addr = (uint32_t)&(I2SF5->dt);
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
  dma_init(DMA1_CHANNEL4, &dma_init_struct);

  crm_periph_clock_enable(CRM_I2SF5_PERIPH_CLOCK, TRUE);
  
  /* i2sf5 clock source select */
  crm_i2sf5_clock_select(CRM_I2SF5_CLOCK_SOURCE_SCLK);

  i2s_default_para_init(&i2s_init_struct);
  i2s_init_struct.operation_mode = I2S_MODE_MASTER_TX;
  i2s_init_struct.audio_protocol = I2S_AUDIO_PROTOCOL_PCM_SHORT;
  i2s_init_struct.audio_sampling_freq = I2S_AUDIO_FREQUENCY_8K;
  i2s_init_struct.data_channel_format = I2S_DATA_16BIT_CHANNEL_16BIT;
  i2s_init_struct.clock_polarity = I2S_CLOCK_POLARITY_LOW;
  i2s_init_struct.mclk_output_enable = TRUE;
  i2s_init_struct.i2s_ckin_value = 8000000;
  
  /* one method: set rising edge as  the i2s pcm mode sample clock */
  i2s_init_struct.pcm_sample_clock_selection = I2S_PCM_SAMPLE_CLOCK_RISING;
  i2s_init(I2SF5, &i2s_init_struct);
  
  /* another method: set rising edge as  the i2s pcm mode sample clock */
  i2sf_pcm_sample_clock_set(I2SF5, I2S_PCM_SAMPLE_CLOCK_RISING);
  
  /* i2s fullduplex mode enable */
  i2sf_full_duplex_mode_enable(I2SF5, TRUE);

  dma_channel_enable(DMA1_CHANNEL3, TRUE);
  dma_channel_enable(DMA1_CHANNEL4, TRUE);
  spi_i2s_dma_transmitter_enable(I2SF5, TRUE);
  spi_i2s_dma_receiver_enable(I2SF5, TRUE);
}

/**
  * @brief  i2sf5 gpio configuration.
  * @param  none
  * @retval none
  */
static void i2sf5_gpio_config(void)
{
  gpio_init_type gpio_initstructure;
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_initstructure);

  /* ws pin */
  gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_mode           = GPIO_MODE_MUX;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pins           = GPIO_PINS_6;
  gpio_init(GPIOB, &gpio_initstructure);
  gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE6, GPIO_MUX_6);

  /* ck pin */
  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_pins           = GPIO_PINS_7;
  gpio_init(GPIOB, &gpio_initstructure);
  gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE7, GPIO_MUX_6);

  /* mck pin */
  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_pins           = GPIO_PINS_8;
  gpio_init(GPIOC, &gpio_initstructure);
  gpio_pin_mux_config(GPIOC, GPIO_PINS_SOURCE8, GPIO_MUX_6);

  /* sd pin */
  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_pins           = GPIO_PINS_9;
  gpio_init(GPIOB, &gpio_initstructure);
  gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE9, GPIO_MUX_6);

  /* sdext pin */
  gpio_initstructure.gpio_pull           = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_pins           = GPIO_PINS_8;
  gpio_init(GPIOB, &gpio_initstructure);
  gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE8, GPIO_MUX_6);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();
  at32_board_init();

  tx_data_fill();
  i2sf5_gpio_config();
  i2sf5_config();
  i2s_enable(I2SF5, TRUE);
  
  while(dma_flag_get(DMA1_FDT3_FLAG) == RESET);
  while(dma_flag_get(DMA1_FDT4_FLAG) == RESET);

  /* test result:the data check */
  transfer_status1 = buffer_compare(i2sf5_buffer_rx, i2sf5_buffer_tx, TXBUF_SIZE);
  
  /* test result indicate:if success ,led2 lights */
  if(transfer_status1 == SUCCESS)
  {
    at32_led_on(LED2);
  }
  else
  {
    at32_led_off(LED2);
  }
  while(1)
  {
  }
}

/**
  * @}
  */

/**
  * @}
  */