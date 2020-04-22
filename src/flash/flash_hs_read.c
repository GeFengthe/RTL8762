/******************************************************************************
 * Copyright (c) 2017, Realtek Semiconductor Corporation. All rights reserved.
 ******************************************************************************/

#include <stdint.h>
#include <string.h>
#include "rtl876x.h"
#include "system_rtl876x.h"
#include "flash_device.h"
#include "otp.h"
#include "app_section.h"

/*============================================================================*
  *                                   Configuration
  *============================================================================*/
#define FLASH_HIGH_SPEED_READ           1   // open to support flash high speed read

#if (FLASH_HIGH_SPEED_READ == 1)
#define FLASH_AUTO_SEQ_TRANS            1

#define FLASH_USER_SPLIT_DMA_HW_HSK     0 // disable at present, so can't use flash_split_read_by_dma_locked
#endif

#if (FLASH_HIGH_SPEED_READ == 1)
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"

/*============================================================================*
  *                                   Macros
  *============================================================================*/

#define FLASH_OFFSET_TO_NO_CACHE    0x01000000 /*!< memory offset between no cache and cache flash address */
#define FLASH_DR_ADDR               (0x40080060)

/*****************************************************************************************
* Registers Definitions --------------------------------------------------------*
********************  Bits definition for SPIC_CTRLR0 register  *******************/
#define BIT_CMD_CH(x)           (((x) & 0x00000003) << 20)
#define BIT_DATA_CH(x)          (((x) & 0x00000003) << 18)
#define BIT_ADDR_CH(x)          (((x) & 0x00000003) << 16)
#define BIT_TMOD(x)             (((x) & 0x00000003) << 8)

/********************  Bits definition for SPIC_CTRLR1 register  *******************/
#define BIT_NDF(x)                  ((x) & 0xffff)

/********************  Bits definition for SPIC_SSIENR register  *******************/

/********************  Bits definition for SPIC_BAUDR register  *******************/

/********************  Bits definition for SPIC_SR register  *******************/

/********************  Bits definition for SPIC_IMR register  *******************/
#define BIT_RXSIM                               (0x00000001 << 10)

/********************  Bits definition for SPIC_ISR register  *******************/
#define BIT_RXSIS                               (0x00000001 << 10)

/********************  Bits definition for SPIC_RISR register  *******************/

/********************  Bits definition for SPIC_CTRLR2 register  *******************/
#define BIT_SEQ_EN              (0x00000001 << 3)


/* Indicate the WPn input pin of SPI Flash is connected to:
   0(default): WP=spi_sout[2], 1:WP=spi_sout[3]. */

/*write protect function.*/


/* SO pin of SPI Flash.0: SO connects to spi_sout[0]. 1(default): SO connects to spi_sout[1].*/


/********************  Bits definition for SPIC_ADDR_LENGTH register  *******************/
#define BIT_ADDR_PHASE_LENGTH(x)    ((x) & 0x00000003)


/********************  Bits definition for SPIC_AUTO_LENGTH register  *******************/

/********************  Bits definition for SPIC_VALID_CMD register  *******************/
#define BIT_SEQ_TRANS_EN        (0x00000001 << 14)

/*============================================================================*
  *                                   Types
  *============================================================================*/

/********************  Access mode of DR register in user mode  *******************/
typedef enum
{
    DATA_BYTE         = 0,
    DATA_HALF         = 1,
    DATA_WORD         = 2
} T_SPIC_BYTE_NUM;


typedef struct
{
    uint8_t              channel_num; /*!< Log DMA Channel Number          */
    GDMA_ChannelTypeDef *channel;     /*!< Log DMA Channel (Register Base) */
    IRQn_Type            irq_no;      /*!< Log DMA Channel IRQ Number      */
    VECTORn_Type         vector_no;   /*!< Log DMA Channel Vector Number   */
    IRQ_Fun              isr;         /*!< Log DMA Channel ISR Handler     */
} T_FLASH_DMA_CFG;


typedef struct
{
    uint32_t    ser;
    uint32_t    read_ofs;
    uint32_t    read_len;
    uint32_t    dma_src_addr;
    uint32_t    dma_dst_addr;
    uint32_t    bk_ctrlr0;
} T_FLASH_SPLIT_READ_INFO;

/* query_info */
typedef struct
{
    // Top/Bottom bit offset of status reg. (some chip may use BPx as TB)
    // set "0xFF" means BP only support lock / unlock all (no level)
    // set "0x7F" means BP support lock / unlock all, half, none
    uint8_t         tb_offset;
    uint8_t         manu_id;
    uint16_t        device_id;      //  memory_type + memory_density
    uint32_t        flash_size;
    uint8_t         bp_all_lv;      // Block Protect All Level (depends on flash_size)
    uint8_t         bp_mask;        // Block Protect (BPx~BP0); BP2~BP0=0x07, BP3~BP0=0x0F
    uint8_t         pkg_id;
    /* use pkg_info bit 0 for IS_EXT_flash flag */
    /* use pkg_info bit 1 for support 4 bit-mode flag */
    /* use pkg_info bit 7 to notify package ID is valid */
    uint8_t         pkg_info;
} T_FLASH_QUERY_TBL;

/* basic command set */
typedef struct
{
    /* force use command set from OTP */
    //uint8_t cfg_from_otp;           /*!< Specifies the Flash configuration source ( 0: QVL; 1: OTP) */

    uint8_t cmd_rd_data;            /*!< Specifies single data read cmd */
    uint8_t cmd_rd_dual_o;          /*!< Specifies dual data read cmd */
    uint8_t cmd_rd_dual_io;         /*!< Specifies dual data/addr read cmd */
    uint8_t cmd_pp;                 /*!<Specifies single page program cmd*/
    uint8_t cmd_wr_en;              /*!< Specifies the Write Enable(WREN) instruction*/
    uint8_t cmd_rd_id;              /*!< Specifies the Read ID instruction*/
    uint8_t cmd_rd_status;          /*!< Specifies the Read Status Register*/
    uint8_t cmd_wr_status;          /*!< Specifies the Write Status Register*/
    uint8_t cmd_chip_e;             /*!< Specifies the Erase Chip instruction which is for erasing whole chip*/
    uint8_t cmd_block_e;            /*!< Specifies the Erase Block instruction which is for erasing 64kB*/
    uint8_t cmd_sector_e;           /*!< Specifies the Erase Sector instruction which is for erasing 4kB*/
    uint8_t cmd_pwdn_release;       /*!< Specifies the Release from Deep Power Down instruction*/
    uint8_t cmd_pwdn;               /*!< Specifies the Deep Power Down instruction*/
    uint8_t cmd_en_reset;           /*!< Specifies the Enable Reset instruction*/
    uint8_t cmd_reset;              /*!< Specifies the Reset instruction*/

    uint8_t manu_id;
    uint16_t device_id;
} T_FLASH_BASIC_CFG;

/* advanced command set */
typedef struct
{
    uint8_t flash_qe_bit;           /*!< Specifies the QE bit in status register which is used to
                                            enable Quad I/O mode . */
    uint8_t cmd_rd_quad_o;          /*!< Specifies quad data read cmd */
    uint8_t cmd_rd_quad_io;         /*!< Specifies quad data/addr read cmd */
    uint8_t cmd_ppx4;               /*!< Specifies quad page program cmd */
    uint8_t cmd_wr_en_vol;          /*!< Specifies the Write Enable for Volatile Status Register instruction*/
    uint8_t cmd_rd_status2;         /*!< Specifies the Read Status Register2*/
    uint8_t cmd_wr_status2;         /*!< Specifies the Write Status Register2*/
    uint8_t cmd_rdcr;               /*!< Specifies read config command, it's used on MXIC for high power mode*/
    uint8_t cmd_hpm;                /*!< Specifies the high performance command.*/

    uint8_t manu_id;
    uint16_t device_id;
} T_FLASH_ADV_CFG;

typedef struct
{
    uint8_t in_use              : 1;
    uint8_t spec_addr_len       : 2;
    uint8_t spec_byte_map       : 4;        // used to identify which spec_byte is needed
    uint8_t rsvd                : 1;
    uint16_t front_delay_10us;              // delay max 655360 us
    uint16_t rear_delay_10us;               // delay max 655360 us
    uint8_t  spec_byte[4];
} __attribute__((packed)) T_FLASH_SPEC_CFG;

/**
  * @brief  FLASH Init structure definition
  */
typedef struct
{
    uint32_t flash_id;          /*!< Specifies the flash vendor ID.*/
    uint32_t valid_cmd;         /*!< to store valid command set for this flash*/
    uint16_t baud_rate;         /*!< Specifies the spi_sclk divider value.*/
    uint8_t current_read_cmd;   /*!< Specifies the current read cmd which is used to read data from
                                flash in current bitmode. */


    struct
    {
        uint8_t flash_sr2_exists: 1; /*!< Specifies whether flash chip has Status Register2 or not*/

        uint8_t flash_hpm_exists: 1; /*!< Specifies whether high power mode exists*/
        uint8_t flash_hpm_cmd_exists: 1; /*! Specifies whether using hpm command or wrst to set hpm*/
        uint8_t read_addr_ch: 2;  /*! Specifies flash address channel*/
        uint8_t read_data_ch: 2; /*! Specifies flash data channel*/
        uint8_t write_pp4_mode: 1;  /*! specifies use pp or 4pp to write. 1 if 4pp enable.*/
    };

    /* calibration data */
    uint8_t flash_rd_dummy_cyle; /*!< Specifies the read dummy cycle of multi chann bitmode */
    uint8_t flash_rd_st_dummy_cycle;

    T_FLASH_BASIC_CFG   basic_cfg;
    T_FLASH_ADV_CFG     adv_cfg;
    T_FLASH_QUERY_TBL   query_info;
} T_FLASH_DEVICE_INFO;

FlashCB flash_dma_cb = NULL;

extern T_FLASH_DEVICE_INFO flash_device_info;
extern T_FLASH_DMA_CFG flash_dma_cfg;
extern T_LOCK_TYPE  flash_lock_flag;
extern T_FLASH_SPLIT_READ_INFO flash_split_read_info;
extern void flash_unlock(T_LOCK_TYPE flash_lock_mode);
extern bool flash_lock(T_LOCK_TYPE flash_lock_mode);
extern uint32_t flash_split_read(uint32_t start_addr, uint32_t data_len, uint8_t *data);


void flash_seq_trans_enable(uint8_t enable);

/**
 * @brief set spic enable or disable.
 *
 * @param enable          DISABLE to disable cache, ENABLE to enable cache.
 * @return
 * @note: spic should be disabled before programming any register in user mode
*/
__STATIC_INLINE void spic_enable(uint32_t enable)
{
    SPIC->ssienr = enable;
}

#if (FLASH_USER_SPLIT_DMA_HW_HSK == 1)
/**
 * @brief set spic ctrl1 register.
 *
 * @param ndf   indicate number of data frames that spic will not stop receiving unitl date frame
 *              equals to NDF. This is used for user mode
 * @return
*/
__STATIC_INLINE void spic_set_ctrl1(uint32_t ndf)
{
    SPIC->ctrlr1 = BIT_NDF(ndf);
}

/**
 * @brief set RX mode in ctrl0 register in user mode.
 *
 * @return
*/
__STATIC_INLINE void spic_set_rx_mode(void)
{
    SPIC->ctrlr0 |= BIT_TMOD(3);
}

/**
 * @brief set spic dr register to implement user mode programming
 *
 * @param byte_num    byte number for every tx.
 * @param data        data to be write into fifo
 * @return
 * @note: dr0 is used to do programming, and endian issue is considered as well.
*/
__STATIC_INLINE void spic_set_dr(T_SPIC_BYTE_NUM byte_num, uint32_t data)
{
    uint32_t wr_data;
    if (byte_num == DATA_HALF)
    {
        wr_data = rtk_cpu_to_le16(data);
        SPIC->dr[0].half = wr_data;
    }
    else if (byte_num == DATA_WORD)
    {
        wr_data = rtk_cpu_to_le32(data);
        SPIC->dr[0].word = wr_data;
    }
    else
    {
        SPIC->dr[0].byte = data;
    }
    return;
}

/**
 * @brief set addr channel and data channel in ctrl0 register.
 *
 * @return
*/
__STATIC_INLINE void spic_set_multi_ch(uint32_t data_ch, uint32_t addr_ch)
{
    SPIC->ctrlr0 &= ~(BIT_CMD_CH(3) | BIT_ADDR_CH(3) | BIT_DATA_CH(3));

    SPIC->ctrlr0 |= (BIT_ADDR_CH(addr_ch) | BIT_DATA_CH(data_ch));
}

/**
 * @brief set seq_en in ctrlr2 register in user mode.
 *
 * @return
*/

__STATIC_INLINE void spic_set_seq_enable(uint8_t enable)
{
    if (enable)
    {
        SPIC->imr |= BIT_RXSIM;
        SPIC->ctrlr2 |= BIT_SEQ_EN;
    }
    else
    {
        SPIC->ctrlr2 &= ~ BIT_SEQ_EN;
        SPIC->imr &= ~ BIT_RXSIM;
    }
}
#endif

//flash_gdma0_ch(FLASH_GDMA_CH_NUM)_handler
void FLASH_GDMA_HANDLER(void) DATA_RAM_FUNCTION;
void FLASH_GDMA_HANDLER(void)
{
    if (flash_lock_flag == FLASH_LOCK_AUTO_MODE_READ)
    {
        flash_unlock(FLASH_LOCK_AUTO_MODE_READ);
    }
    else if (flash_lock_flag == FLASH_LOCK_USER_MODE_READ)
    {
        flash_unlock(FLASH_LOCK_USER_MODE_READ);
    }

#if (FLASH_AUTO_SEQ_TRANS == 1)
    if ((SPIC->valid_cmd & BIT_SEQ_TRANS_EN) == BIT_SEQ_TRANS_EN)
    {
        flash_seq_trans_enable(0);
    }
#endif  /* end of FLASH_AUTO_SEQ_TRANS */

    GDMA_INTConfig(flash_dma_cfg.channel_num, GDMA_INT_Transfer, (FunctionalState)DISABLE);
    GDMA_ClearINTPendingBit(flash_dma_cfg.channel_num, GDMA_INT_Transfer);

    spic_enable(DISABLE);
#if (FLASH_USER_SPLIT_DMA_HW_HSK == 1)
    if (flash_split_read_info.read_len > 0)
    {
        /* restore ctrlr0 */
        SPIC->ctrlr0 = flash_split_read_info.bk_ctrlr0;

        /* Receive DMA Disable */
        SPIC->dmacr   = 0;
        SPIC->dmardlr = 0x3F;
        spic_set_seq_enable(0);

        /* disable flash IRQ */
        NVIC_InitTypeDef nvic_init_struct;
        nvic_init_struct.NVIC_IRQChannel         = SPIFLASH_IRQ;
        nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)DISABLE;
        NVIC_Init(&nvic_init_struct);

        flash_split_read_info.read_len = 0;
    }
#endif  /* end of FLASH_USER_SPLIT_DMA_HW_HSK */

    if (flash_dma_cb != NULL)
    {
        flash_dma_cb();
        flash_dma_cb = NULL;
    }
}

/**
 * @init DMA for split read by DMA
 * @param dma_type      select the direction
 * @param src_addr      source address to read
 * @param dst_addr      destenation address to write
 * @param data_len      data length to be read
 * @return
*/
void flash_dma_init(T_FLASH_DMA_TYPE dma_type, uint32_t src_addr, uint32_t dst_addr,
                    uint32_t data_len)
{
    uint8_t dir = 0, src_hsk_idx = 0, dst_hsk_idx = 0, src_inc = 0, dest_inc = 0;
    uint8_t dma_ch_num = OTP->flash_dma_ch;
    GDMA_InitTypeDef gdma_init;
    uint32_t dma_ch_base          = GDMA_Channel0_BASE;
    uint32_t dma_irqn_base        = GDMA0_Channel0_IRQn;
    VECTORn_Type dma_vectorn_base = GDMA0_Channel0_VECTORn;

    flash_dma_cfg.channel_num = dma_ch_num;
    flash_dma_cfg.channel     = (GDMA_ChannelTypeDef *)(dma_ch_base +
                                                        dma_ch_num * sizeof(GDMA_ChannelTypeDef));
    if (dma_ch_num >= 6)
    {
#if 1
        /* bee2 only 6 channes */
        //FLASH_PRINT_ERROR0("Flash DMA init fail! only 6 CH");
        while (1);
#else
        /* The IRQs of GDMA channel are discontinuous in vector. */
        dma_irqn_base    = GDMA0_Channel4_IRQn;
        dma_vectorn_base = GDMA0_Channel4_VECTORn;
#endif
    }

    flash_dma_cfg.irq_no    = (IRQn_Type)(dma_irqn_base + dma_ch_num);
    flash_dma_cfg.vector_no = (VECTORn_Type)(dma_vectorn_base + dma_ch_num);
    flash_dma_cfg.isr       = FLASH_GDMA_HANDLER;


    //RamVectorTableUpdate(FLASH_GDMA_VECTORn, (IRQ_Fun)FLASH_GDMA_HANDLER);
    RamVectorTableUpdate(flash_dma_cfg.vector_no, flash_dma_cfg.isr);
    //flash_split_read_info.dma_dst_addr = dst_addr;

    /* DMA buf_size depens on unit of data size */
    /* if data size is word, so buf_size need div 4 */
    /* enable FIFO mode */
    flash_dma_cfg.channel->CFG_HIGH |= 0x02;

    //FLASH_PRINT_TRACE3("src=0x%lx, dst=0x%lx, len=0x%lx", src_addr, dst_addr,
    //                   data_len);  // reserved for debug
    switch (dma_type)
    {
    case FLASH_DMA_AUTO_F2R:
        dir         = GDMA_DIR_MemoryToMemory;
        src_addr    = (src_addr | FLASH_OFFSET_TO_NO_CACHE);      //Mapped to non-cache address
        break;
    case FLASH_DMA_AUTO_R2F:
        dir = GDMA_DIR_MemoryToMemory;
        dst_addr    = (dst_addr | FLASH_OFFSET_TO_NO_CACHE);      //Mapped to non-cache address
        break;
    case FLASH_DMA_USER_F2R:
        dir         = GDMA_DIR_PeripheralToMemory;
        src_inc     = DMA_SourceInc_Fix;
        src_hsk_idx = 21;
        break;
    case FLASH_DMA_USER_R2F:
        dir         = GDMA_DIR_MemoryToPeripheral;
        dest_inc    = DMA_DestinationInc_Dec;
        dst_hsk_idx = 20;
        break;
    default:
        //FLASH_PRINT_ERROR1("Invalid  DMA type %ld", dma_type);
        return;
    }

    /* Init GDMA */
    GDMA_StructInit(&gdma_init);
    gdma_init.GDMA_ChannelNum           = flash_dma_cfg.channel_num;
    gdma_init.GDMA_DIR = dir;
    gdma_init.GDMA_BufferSize           = data_len /
                                          4;             // div by 4 because DataSize use word
    gdma_init.GDMA_SourceInc            = src_inc;
    gdma_init.GDMA_DestinationInc       = dest_inc;
    gdma_init.GDMA_SourceDataSize       = GDMA_DataSize_Word;
    gdma_init.GDMA_DestinationDataSize  = GDMA_DataSize_Word;
    gdma_init.GDMA_SourceMsize          = GDMA_Msize_32;            // dependent to SPIC dmardlr
    gdma_init.GDMA_DestinationMsize     = GDMA_Msize_32;            // dependent to SPIC dmardlr
    gdma_init.GDMA_SourceAddr           = src_addr;
    gdma_init.GDMA_DestinationAddr      = (uint32_t)dst_addr;
    gdma_init.GDMA_ChannelPriority      = 3; //channel prority between 0 to 6
    gdma_init.GDMA_SourceHandshake      = src_hsk_idx;
    gdma_init.GDMA_DestHandshake        = dst_hsk_idx;
    GDMA_Init(flash_dma_cfg.channel, &gdma_init);

    /*  Enable GDMA IRQ  */
    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = flash_dma_cfg.irq_no;
    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 6;
    NVIC_Init(&nvic_init_struct);

    GDMA_INTConfig(flash_dma_cfg.channel_num, GDMA_INT_Transfer, (FunctionalState)ENABLE);

}

/**
 * @start to auto read by DMA
 * @param dma_type      select the direction
 * @param flash_cb      callback function after complete
 * @param src_addr      source address to read
 * @param dst_addr      destenation address to write
 * @param data_len      data length to be read
 * @return
*/
void flash_auto_dma_read(T_FLASH_DMA_TYPE dma_type, FlashCB flash_cb,
                         uint32_t src_addr, uint32_t dst_addr, uint32_t data_len)
{
    flash_dma_cb = flash_cb;
    flash_dma_init(dma_type, src_addr, dst_addr, data_len);
    GDMA_Cmd(flash_dma_cfg.channel_num, (FunctionalState)ENABLE);
}

#if (FLASH_AUTO_SEQ_TRANS == 1)
/**
 * @configure seq_trans_en
 * @param enable         enable / disable seq_trans
 * @return
*/
void flash_seq_trans_enable(uint8_t enable)
{
    spic_enable(DISABLE);
    if (enable)
    {
        SPIC->valid_cmd |= BIT_SEQ_TRANS_EN;
    }
    else
    {
        SPIC->valid_cmd &= ~BIT_SEQ_TRANS_EN;
    }
    spic_enable(ENABLE);
}
/**
 * @start to auto sequential transition read by DMA
 * @param dma_type      select the direction
 * @param flash_cb      callback function after complete
 * @param src_addr      source address to read
 * @param dst_addr      destenation address to write
 * @param data_len      data length to be read
 * @return
*/
void flash_auto_seq_trans_dma_read(T_FLASH_DMA_TYPE dma_type, FlashCB flash_cb,
                                   uint32_t src_addr, uint32_t dst_addr, uint32_t data_len)
{
    flash_dma_cb = flash_cb;
    flash_dma_init(dma_type, src_addr, dst_addr, data_len);
    flash_seq_trans_enable(1);
    GDMA_Cmd(flash_dma_cfg.channel_num, (FunctionalState)ENABLE);
}
#endif  /* end of FLASH_AUTO_SEQ_TRANS */


#if (FLASH_USER_SPLIT_DMA_HW_HSK == 1)
void SPI_Flash_Handler(void);
/**
 * @setup split read by DMA
 * @param start_addr    start address to read
 * @param data_len      data length to be read
 * @return
 * @note also need to call flash_dma_init() before kick!!
*/
void flash_split_read_by_dma_setup(uint32_t start_addr, uint32_t data_len)
{
    uint8_t cmd = flash_device_info.current_read_cmd;

    uint32_t rd_addr = 0;
    volatile uint32_t ser;

    flash_split_read_info.read_len = data_len;

    /* In order to enhance throughput, only word align (4 bytes) is allowed to be written.*/
    if ((data_len % 4) != 0)
    {
        return;
    }

    flash_split_read_info.bk_ctrlr0 = SPIC->ctrlr0;
    spic_enable(DISABLE);

    //FLASH_PRINT_TRACE2("start_addr=0x%lx, data_len=0x%lx", start_addr, data_len); // reserved for debug
    spic_set_multi_ch(flash_device_info.read_data_ch, flash_device_info.read_addr_ch);
    spic_set_rx_mode();
    spic_set_ctrl1(data_len);
    spic_set_seq_enable(1);

    flash_split_read_info.ser = SPIC->ser;
    SPIC->addr_length = BIT_ADDR_PHASE_LENGTH(3);

    rd_addr = (cmd & 0xff) |
              ((start_addr & 0x000000ff) << 24) |
              ((start_addr & 0x0000ff00) << 8) |
              ((start_addr & 0x00ff0000) >> 8) ;

    spic_set_dr(DATA_WORD, rd_addr);

    /* Receive DMA Enable */
    SPIC->dmacr   = 1;
    /* config DMA Receive Data Level */
    SPIC->dmardlr = 32;         //dependent to GDMA Msize (4, 8, 16, 32, 64)

}

/**
 * @start split read by DMA
 * @return
 * @note need to call flash_split_read_by_dma_setup()
 *  and flash_dma_init() before kick!!
*/
void flash_split_read_by_dma_kick(void)
{
    GDMA_Cmd(flash_dma_cfg.channel_num, (FunctionalState)ENABLE);
    spic_enable(ENABLE);
}

/**
 * @setup user split read by DMA + HW HSK
 * @param start_addr    start address to read
 * @param data_len      data length to be read
 * @return
 * @note also need to call flash_dma_init() before kick!!
*/
void flash_split_read_by_dma(FlashCB flash_cb, uint32_t start_addr, uint32_t dst_addr,
                             uint32_t data_len)
{
    flash_dma_cb = flash_cb;
    memset(&flash_split_read_info, 0, sizeof(T_FLASH_SPLIT_READ_INFO));
    flash_split_read_info.dma_src_addr = start_addr | FLASH_OFFSET_TO_NO_CACHE;
    flash_split_read_info.dma_dst_addr = dst_addr;
    flash_split_read_by_dma_setup(flash_split_read_info.dma_src_addr, data_len);
    flash_dma_init(FLASH_DMA_USER_F2R, FLASH_DR_ADDR, (uint32_t)dst_addr, data_len);

    RamVectorTableUpdate(SPIFlash_VECTORn, SPI_Flash_Handler);
    /* Enable SPIFLASH IRQ */
#if (FLASH_USER_SPLIT_DMA_HW_HSK == 1)
    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = SPIFLASH_IRQ;
    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 6;
    NVIC_Init(&nvic_init_struct);
#endif  /* end of FLASH_USER_SPLIT_DMA_HW_HSK */

    flash_split_read_by_dma_kick();
}

/**
* @SPI Flash interrupt handler
* @return
* @note currently only support RXSIS because can't get SPIC->isr
* @     value in handler.
*/
DATA_RAM_FUNCTION void SPI_Flash_Handler(void)
{
    volatile uint32_t tmp_isr = SPIC->isr;

    SPIC->icr = 1;

    if ((tmp_isr & BIT_RXSIS) == BIT_RXSIS)
    {
        flash_dma_cfg.channel->CFG_LOW |= 0x0100;           // suspend DMA
        uint32_t trans_len = (flash_dma_cfg.channel->CTL_HIGH & 0xFFFF);

        spic_enable(DISABLE);
        GDMA_Cmd(flash_dma_cfg.channel_num, (FunctionalState)DISABLE);

        SPIC->ser = flash_split_read_info.ser;
        SPIC->flush_fifo = 1;
        spic_set_multi_ch(flash_device_info.read_data_ch, flash_device_info.read_addr_ch);
        spic_set_rx_mode();

        flash_split_read_info.read_ofs += trans_len;

        /* setup SPIC and DMA with last data length and related address */
        flash_split_read_by_dma_setup((flash_split_read_info.dma_src_addr + flash_split_read_info.read_ofs),
                                      (flash_split_read_info.read_len - trans_len));
        flash_dma_init(FLASH_DMA_USER_F2R, FLASH_DR_ADDR,
                       (uint32_t)(flash_split_read_info.dma_dst_addr + trans_len), (flash_split_read_info.read_len));

        /* trigger SPIC and DMA */
        flash_split_read_by_dma_kick();
    }
}
#endif  /* end of FLASH_USER_SPLIT_DMA_HW_HSK */
#endif  /* end of FLASH_HIGH_SPEED_READ */


#if (FLASH_HIGH_SPEED_READ == 1)
/**
 * @brief safe version of flash auto dma read
 * @param dma_type      identify user / auto mode and flash_2_ram / ram_2_flash
 * @param flash_cb      callback function after DMA transmit completed if need
 * @param src_addr      address where is going to be read in flash
 * @param dst_addr      address where is going to write data to
 * @param data_len      data length to be read
 * @param data          data buffer to be read into
 * @return true: succeed to access flash;  false: fail to access flash because other flash access is not finished
 * @note    FLASH_GDMA_HANDLER() will be executed after flash transmit by DMA complete.
*/
bool flash_auto_dma_read_locked(T_FLASH_DMA_TYPE dma_type, FlashCB flash_cb,
                                uint32_t src_addr, uint32_t dst_addr, uint32_t data_len)
{
    if (!flash_lock(FLASH_LOCK_AUTO_MODE_READ))
    {
        return false;
    }

    flash_auto_dma_read(dma_type, flash_cb, src_addr, dst_addr, data_len);

    return true;
}

#if (FLASH_AUTO_SEQ_TRANS == 1)
/**
 * @brief safe version of flash auto seqential transition dma read
 * @param dma_type      identify user / auto mode and flash_2_ram / ram_2_flash
 * @param flash_cb      callback function after DMA transmit completed if need
 * @param src_addr      address where is going to be read in flash
 * @param dst_addr      address where is going to write data to
 * @param data_len      data length to be read
 * @param data          data buffer to be read into
 * @return true: succeed to access flash;  false: fail to access flash because other flash access is not finished
 * @note    FLASH_GDMA_HANDLER() will be executed after flash transmit by DMA complete.
*/
bool flash_auto_seq_trans_dma_read_locked(T_FLASH_DMA_TYPE dma_type, FlashCB flash_cb,
                                          uint32_t src_addr, uint32_t dst_addr, uint32_t data_len)
{
    if (!flash_lock(FLASH_LOCK_AUTO_MODE_READ))
    {
        return false;
    }

    flash_auto_seq_trans_dma_read(dma_type, flash_cb, src_addr, dst_addr, data_len);

    return true;
}
#endif  /* end of FLASH_AUTO_SEQ_TRANS */

#if (FLASH_USER_SPLIT_DMA_HW_HSK == 1)
/**
 * @setup user split read by DMA + HW HSK
 * @param start_addr    start address to read
 * @param data_len      data length to be read
 * @return
 * @note also need to call flash_dma_init() before kick!!
*/
bool flash_split_read_by_dma_locked(FlashCB flash_cb, uint32_t start_addr, uint32_t dst_addr,
                                    uint32_t data_len)
{
    if (!flash_lock(FLASH_LOCK_AUTO_MODE_READ))
    {
        return false;
    }

    flash_split_read_by_dma(flash_cb, start_addr, dst_addr, data_len);

    return true;
}
#endif  /* end of FLASH_USER_SPLIT_DMA_HW_HSK */
#endif  /* end of FLASH_HIGH_SPEED_READ */





