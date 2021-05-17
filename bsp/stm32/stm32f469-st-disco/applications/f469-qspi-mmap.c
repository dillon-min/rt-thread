#include "stm32f4xx.h"
void config_qspi_mmap(void)
{
  /****************************************************************************/
  /*                                                                          */
  /* Configuration of the IOs :                                               */
  /* --------------------------                                               */
  /* GPIOF10  : CLK                                                            */
  /* GPIOB6   : BK1_nCS                                                        */
  /* GPIOF8   : BK1_IO0/SO                                                     */
  /* GPIOF9   : BK1_IO1/SI                                                     */
  /* GPIOF7   : BK1_IO2                                                        */
  /* GPIOF6   : BK1_IO3                                                        */
  /*                                                                          */
  /* Configuration of the QSPI :                                              */
  /* ---------------------------                                              */
  /* - Instruction is on one single line                                      */
  /* - Address is 32-bits on four lines                                       */
  /* - No alternate bytes                                                     */
  /* - Ten dummy cycles                                                       */
  /* - Data is on four lines                                                  */
  /*                                                                          */
  /* If the clock is changed :                                                */
  /* -------------------------                                                */
  /* - Modify the prescaler in the control register                           */
  /* - Update the number of dummy cycles on the memory side and on            */
  /*   communication configuration register                                   */
  /*                                                                          */
  /* If the memory is changed :                                               */
  /* --------------------------                                               */
  /* - Update the device configuration register with the memory configuration */
  /* - Modify the instructions with the instruction set of the memory         */
  /* - Configure the number of dummy cycles as described in memory datasheet  */
  /* - Modify the data size and alternate bytes according memory datasheet    */
  /*                                                                          */
  /****************************************************************************/
  
  register uint32_t tmpreg = 0, datareg = 0,tmp = 0, timeout = 0xFFFF;
  
  /*--------------------------------------------------------------------------*/
  /*------------------ Activation of the peripheral clocks -------------------*/
  /*--------------------------------------------------------------------------*/      
  /* Enable GPIOB and GPIOF interface clock */ 
  /* Enable clock of the QSPI */
  RCC->AHB3ENR |= 0x00000002;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  
  /*--------------------------------------------------------------------------*/
  /*--------------------- Configuration of the I/O pins ----------------------*/
  /*--------------------------------------------------------------------------*/
  /* Configure alternate function selection for IO pins */
  GPIOF->AFR[0] = 0x99000000;
  GPIOF->AFR[1] = 0x000009AA;
  GPIOB->AFR[0] = 0x0A000000; 
  
  /* Configure alternate function mode for IO pins */
  GPIOF->MODER = 0x002AA000;
  GPIOB->MODER = 0x00002280;
  
  /* Configure output speed for IO pins */
  GPIOF->OSPEEDR = 0x003FF000;
  GPIOB->OSPEEDR = 0x000030C0;
  
  /* Configure pull-up or pull-down for IO pins */
  GPIOB->PUPDR   = 0x00001100;
  
  /*--------------------------------------------------------------------------*/
  /*----------------------- Initialization of the QSPI -----------------------*/
  /*--------------------------------------------------------------------------*/
  timeout = 0xFFFF;
  do
  {
    tmpreg = (QUADSPI->SR & QUADSPI_SR_BUSY); 
  } while((tmpreg != 0) && (timeout-- > 0));
  
  if (timeout != 0)
  {
    /* Configure device configuration register of QSPI */
    /* - FSIZE = 23 */
    QUADSPI->DCR = QUADSPI_DCR_CSHT_0|  23<<16;
    /* Configure control register of QSPI: precsaler, sample shift and enable QSPI */
    QUADSPI->CR = (1 << 24) | QUADSPI_CR_SSHIFT|QUADSPI_CR_EN;
  }  
  /*--------------------------------------------------------------------------*/
  /*----------- Configuration of the dummy cycles on flash side --------------*/
  /*--------------------------------------------------------------------------*/
  /* Configure communication register to read volatile configuration register */
  /* - FMODE = Indirect read
  - DMODE = Data on a single line
  - IMODE = Instruction on a single line
  - INSTRUCTION = READ_VOL_CFG_REG_CMD */
  tmp = QUADSPI->CCR;
  tmp = tmp& (~(QUADSPI_CCR_FMODE | QUADSPI_CCR_DMODE | QUADSPI_CCR_IMODE | QUADSPI_CCR_INSTRUCTION));
  tmp |= (QUADSPI_CCR_FMODE_0 | QUADSPI_CCR_DMODE_0 | QUADSPI_CCR_IMODE_0 | 0x85);
  QUADSPI->CCR = tmp;
  /* Wait that the transfer is complete */
  timeout = 0xFFFF;
  do
  {
    tmpreg = (QUADSPI->SR & QUADSPI_SR_TCF); 
  } while((tmpreg == 0) && (timeout-- > 0));
  
  if (timeout != 0)
  {
    /* Read received value */
    datareg = QUADSPI->DR;
    
    /* Clear transfer complete flag */
    QUADSPI->FCR = QUADSPI_FCR_CTCF;
    
    /* Perform abort (mandatory workaround for this version of QSPI) */
    tmp = QUADSPI->CR; 
    tmp = (tmp&(~QUADSPI_CR_ABORT));
    QUADSPI->CR = tmp|QUADSPI_CR_ABORT;
    
    /* Wait that the transfer is complete */
    timeout = 0xFFFF;
    do
    {
      tmpreg = (QUADSPI->SR & QUADSPI_SR_TCF); 
    } while((tmpreg == 0) && (timeout-- > 0));
    
    if (timeout != 0)
    {
      /* Clear transfer complete flag */
      QUADSPI->FCR = QUADSPI_FCR_CTCF;
      
      /* Configure communication register to enable write operations */
      tmp = QUADSPI->CCR;
      tmp = tmp& (~(QUADSPI_CCR_FMODE | QUADSPI_CCR_DMODE | QUADSPI_CCR_INSTRUCTION));
      tmp |= 0x06;
      QUADSPI->CCR = tmp;
      /* Wait that the transfer is complete */
      timeout = 0xFFFF;
      do
      {
        tmpreg = (QUADSPI->SR & QUADSPI_SR_TCF); 
      } while((tmpreg == 0) && (timeout-- > 0));
      
      if (timeout != 0)
      {
        /* Clear transfer complete flag */
        QUADSPI->FCR = QUADSPI_FCR_CTCF;
        
        /* Configure the mask for the auto-polling mode on write enable bit of status register */
        QUADSPI->PSMKR = 0x2;
        
        /* Configure the value for the auto-polling mode on write enable bit of status register */
        QUADSPI->PSMAR = 0x2;
        
        /* Configure the auto-polling interval */
        QUADSPI->PIR   = 0x10;
        
        /* Configure control register to automatically stop the auto-polling mode */
        QUADSPI->CR = (QUADSPI->CR&(~QUADSPI_CR_APMS));
        QUADSPI->CR |= QUADSPI_CR_APMS;
        
        /* Configure communication register to perform auto-polling mode on status register */           
        tmp = QUADSPI->CCR;
        tmp = tmp& (~(QUADSPI_CCR_FMODE | QUADSPI_CCR_DMODE | QUADSPI_CCR_INSTRUCTION));
        tmp |= (QUADSPI_CCR_FMODE_1 | QUADSPI_CCR_DMODE_0 | 0x05);
        QUADSPI->CCR = tmp;
        /* Wait that the status match occurs */
        timeout = 0xFFFF;
        do
        {
          tmpreg = (QUADSPI->SR & QUADSPI_SR_SMF); 
        } while((tmpreg == 0) && (timeout-- > 0));
        
        if (timeout != 0)
        {
          /* Clear status match flag */
          QUADSPI->FCR = QUADSPI_FCR_CSMF;
          
          /* Write volatile configuration register with new dummy cycles */  
          datareg = (datareg&0xF)| 10<<4;
          
          /* Configure communication register to write volatile configuration register */
          tmp = QUADSPI->CCR;
          tmp = tmp& (~(QUADSPI_CCR_FMODE | QUADSPI_CCR_INSTRUCTION));
          tmp |= 0x81;
          QUADSPI->CCR = tmp;
          /* Write the value to transmit */
          QUADSPI->DR = datareg;
          
          /* Wait that the transfer is complete */
          timeout = 0xFFFF;
          do
          {
            tmpreg = (QUADSPI->SR & QUADSPI_SR_TCF); 
          } while((tmpreg == 0) && (timeout-- > 0));
          
          if (timeout != 0)
          {
            /* Clear transfer complete flag */
            QUADSPI->FCR = QUADSPI_FCR_CTCF;
            
            /* Perform abort (mandatory workaround for this version of QSPI) */
            tmp = QUADSPI->CR; 
            tmp = (tmp&(~QUADSPI_CR_ABORT));
            QUADSPI->CR = tmp|QUADSPI_CR_ABORT;
            
            /* Wait that the transfer is complete */
            timeout = 0xFFFF;
            do
            {
              tmpreg = (QUADSPI->SR & QUADSPI_SR_TCF); 
            } while((tmpreg == 0) && (timeout-- > 0));
            
            if (timeout != 0)
            {
              /* Clear transfer complete flag */
              QUADSPI->FCR = QUADSPI_FCR_CTCF;
              
                /*------------------------------------------------------------*/
                /*--------- Configuration of the memory-mapped mode ----------*/
                /*------------------------------------------------------------*/
                /* Configure communication register for reading sequence in memory-mapped mode */
                /* - FMODE = Memory-mapped
                   - DMODE = Data on four lines
                   - DCYC = 10
                   - ADSIZE = 32-bit address
                   - ADMODE = Address on four lines
                   - IMODE = Instruction on a single line
                   - INSTRUCTION = QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD */
              tmp = QUADSPI->CCR;
              tmp = tmp& (~(QUADSPI_CCR_FMODE | QUADSPI_CCR_DMODE | QUADSPI_CCR_DCYC | QUADSPI_CCR_ADSIZE | QUADSPI_CCR_ADMODE | QUADSPI_CCR_INSTRUCTION));
              tmp |= (QUADSPI_CCR_FMODE | QUADSPI_CCR_DMODE |  (10 << POSITION_VAL(QUADSPI_CCR_DCYC)) | QUADSPI_CCR_ADSIZE_1 | QUADSPI_CCR_ADMODE | 0xEB);
              QUADSPI->CCR = tmp;
            }
          }
        }
      }
    }
  }
}
