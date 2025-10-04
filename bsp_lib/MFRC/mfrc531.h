#include <config.h>

#ifndef MFRC531_H
#define MFRC531_H

//------------------------------------------------------------------------------
//         Constants
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//         Types
//------------------------------------------------------------------------------
//=====================================================================
//                          Regieter Sets
//=====================================================================
// PAGE 0      Command and Status
#define     RegPage               0x00   //!< Page Select Register
#define     RegCommand            0x01   //!< Command Register
#define     RegFIFOData           0x02   //!< FiFo Register
#define     RegPrimaryStatus      0x03   //!< Modem State/IRQ/ERR/LoHiAlert Reg
#define     RegFIFOLength         0x04   //!< Buffer length Register
#define     RegSecondaryStatus    0x05   //!< diverse status flags
#define     RegInterruptEn        0x06   //!< IRQ enable Register
#define     RegInterruptRq        0x07   //!< IRQ bits Register
// PAGE 1      Control and Status
#define     RegControl            0x09   //!< processor control
#define     RegErrorFlag          0x0A   /*!< error flags showing the error 
                                         status of the last command executed */
#define     RegCollPos            0x0B   /*!< bit position of the first bit 
                                         collision detected on the 
                                         RF-interface */
#define     RegTimerValue         0x0C   //!< preload value of the timer
#define     RegCRCResultLSB       0x0D   //!< LSB of the CRC Coprocessor register
#define     RegCRCResultMSB       0x0E   //!< MSB of the CRC Coprocessor register
#define     RegBitFraming         0x0F   //!< Adjustments for bit oriented frames
// PAGE 2      Transmitter and Coder Control
#define     RegTxControl          0x11   //!< controls the logical behaviour of 
                                         //!< the antenna driver pins TX1 and TX2
#define     RegCwConductance      0x12   /*!< selects the conductance of the 
                                         antenna driver pins TX1 and TX2 */
#define     RFU13                 0x13   //!< RFU
#define     RegModConductance     0x13
#define     RegCoderControl       0x14   //!< selects coder rate
#define     RegModWidth           0x15   /*!< selects the width of the 
                                         modulation pulse */
#define     RFU16                 0x16   //!< RFU
#define     RegModWidthSOF        0x16
#define     RFU17                 0x17   //!< RFU
#define     RegTypeBFraming       0x17
// PAGE 3      Receiver and Decoder Control
#define     RegRxControl1         0x19   //!< controls receiver behaviour
#define     RegDecoderControl     0x1A   //!< controls decoder behaviour
#define     RegBitPhase           0x1B   /*!< selets the bit phase between 
                                         transmitter and receiver clock */
#define     RegRxThreshold        0x1C   /*!< selects thresholds for the bit 
                                         decoder */
#define     RFU1D                 0x1D   //!< RFU
#define     RegBPSKDemControl     0x1D
#define     RegRxControl2         0x1E   /*!< controls decoder behaviour and 
                                         defines the input source for the 
                                         receiver */
#define     RegClockQControl      0x1F   /*!< controls clock generation for the 
                                         90° phase shifted Q-channel clock */
// PAGE 4      RF-Timing and Channel Redundancy
#define     RegRxWait             0x21   /*!< selects the time interval after 
                                         transmission, before receiver starts */
#define     RegChannelRedundancy  0x22   /*!< selects the kind and mode of 
                                         checking the data integrity on the 
                                         RF-channel */
#define     RegCRCPresetLSB       0x23   /*!< LSB of the pre-set value for the 
                                         CRC register */
#define     RegCRCPresetMSB       0x24   /*!< MSB of the pre-set value for the 
                                         CRC register */
#define     RFU25                 0x25   //!< RFU
#define     RegTimeSlotPeriod     0x25
#define     RegMfOutSelect        0x26   /*!< selects internal signal applied to 
                                         pin MfOut */
#define     RFU27                 0x27   //!< RFU
// PAGE 5      FIFO, Timer and IRQ-Pin Configuration
#define     RegFIFOLevel          0x29   /*!< Defines level for FIFO over- and 
                                         underflow warning */
#define     RegTimerClock         0x2A   //!< selects the divider for the timer clock
#define     RegTimerControl       0x2B   /*!< selects start and stop conditions 
                                         for the timer */
#define     RegTimerReload        0x2C   /*!< defines the pre-set value for the 
                                         timer */
#define     RegIRqPinConfig       0x2D   /*!< configures the output stage of 
                                         pin IRq */
#define     RFU2E                 0x2E   //!< RFU
#define     RFU2F                 0x2F   //!< RFU
// PAGE 6      RFU
#define     RFU31                 0x31   //!< RFU
#define     RFU32                 0x32   //!< RFU
#define     RFU33                 0x33   //!< RFU
#define     RFU34                 0x34   //!< RFU
#define     RFU35                 0x35   //!< RFU
#define     RFU36                 0x36   //!< RFU
#define     RFU37                 0x37   //!< RFU
// PAGE 7      Test Control
#define     RFU39                 0x39   //!< RFU
#define     RegTestAnaSelect      0x3A   //!< selects analog test mode
#define     RFU3B                 0x3B   //!< RFU
#define     RFU3C                 0x3C   //!< RFU
#define     RegTestConfiguration  0x3C
#define     RegTestDigiSelect     0x3D   //!< selects digital test mode
#define     RFU3E                 0x3E   //!< RFU
#define     RegTestEE             0x3E
#define     RegTestDigiAccess     0x3F


//=====================================================================
//                      P C D - C O M M A N D S
//=====================================================================
#define  DEF_FIFO_LENGTH    64  //!< default FIFO size


#define PCD_IDLE           0x00 /*!< No action: cancel current command 
                                or home state */
#define PCD_WRITEE2        0x01 //!< Get data from FIFO and write it to the E2PROM
#define PCD_READE2         0x03 /*!< Read data from E2PROM and put it into the 
                                FIFO */
#define PCD_LOADCONFIG     0x07 /*!< Read data from E2PROM and initialise the 
                                registers */
#define PCD_LOADKEYE2      0x0B /*!< Read a master key from the E2PROM and put 
                                it into the master key buffer */
#define PCD_AUTHENT1       0x0C /*!< Perform the first part of the card 
                                authentication using the Crypto1 algorithm.
                                
                                Remark: The master key is automatically taken 
                                from the master key buffer. this implies, 
                                that the command LoadKeyE2 has to be executed
                                before using a certain key for card 
                                authentication */
#define PCD_CALCCRC        0x12 /*!< Activate the CRC-Coprocessor

                                Remark: The result of the CRC calculation can
                                be read from the register CRCResultXXX */
#define PCD_AUTHENT2       0x14 /*!< Perform the second part of the card 
                                authentication using the Crypto1 algorithm. */
#define PCD_RECEIVE        0x16 /*!< Activate Receiver Circuitry. Before the 
                                receiver actually starts, the state machine 
                                waits until the time configured in the 
                                register RxWait has passed.
                                
                                Remark: It is possible to read any received 
                                data from the FIFO while the Receive command
                                is active. Thus it is possible to receive an 
                                unlimited number of bytes by reading them 
                                from the FIFO in timer. */
#define PCD_LOADKEY        0x19 /*!< Read a master key from the FIFO and put it 
                                into the master key buffer
                                
                                Remark: The master key has to be prepared in
                                a certain format. Thus, 12 byte have to be 
                                passed to load a 6 byte master key */
#define PCD_TRANSMIT       0x1A /*!< Transmit data from FIFO to the card

                                Remark: If data is already in the FIFO when 
                                the command is activated, this data is 
                                transmitted immediately. It is possible to 
                                write data to the FIFO while the Transmit 
                                command is active. Thus it is possible to 
                                transmit an unlimited number of bytes in one
                                stream by writting them to the FIFO in time.*/
#define PCD_TRANSCEIVE     0x1E /*!< Transmits data from FIFO to the card and 
                                after that automatically activates the 
                                receiver. Before the receiver actually 
                                starts,the state machine waits until the 
                                time configured in the register RxWait has 
                                passed. 
                                
                                Remark: This command is the combination of 
                                Transmit and Receive.*/
#define PCD_RESETPHASE     0x3F /*!< Runs the Reset- and Initialisation Phase
                                Remark: This command can not be activated by 
                                software, but only by a Power-On or 
                                Hard Reset */



//=====================================================================
//                      P I C C - C O M M A N D S
//=====================================================================
#define PICC_REQIDL        0x26         //!< request idle
#define PICC_REQALL        0x52         //!< request all
#define PICC_ANTICOLL1     0x93         //!< anticollision level 1 106 kBaud
#define PICC_ANTICOLL11    0x92         //!< anticollision level 1 212 kBaud
#define PICC_ANTICOLL12    0x94         //!< anticollision level 1 424 kBaud
#define PICC_ANTICOLL13    0x98         //!< anticollision level 1 848 kBaud
#define PICC_ANTICOLL2     0x95         //!< anticollision level 2
#define PICC_ANTICOLL3     0x97         //!< anticollision level 3
#define PICC_AUTHENT1A     0x60         //!< authentication using key A
#define PICC_AUTHENT1B     0x61         //!< authentication using key B
#define PICC_READ16        0x30         //!< read 16 byte block
#define PICC_WRITE16       0xA0         //!< write 16 byte block
#define PICC_WRITE4        0xA2         //!< write 4 byte block
#define PICC_DECREMENT     0xC0         //!< decrement value
#define PICC_INCREMENT     0xC1         //!< increment value
#define PICC_RESTORE       0xC2         //!< restore command code
#define PICC_TRANSFER      0xB0         //!< transfer command code
#define PICC_HALT          0x50         //!< halt



//=====================================================================
//                           ERROR CODES
//=====================================================================
#define READER_ERR_BASE_START           (0)
#define MI_OK                           (0)
#define MI_CHK_OK                       (0)
#define MI_CRC_ZERO                     (0)
// ICODE1 Error   Codes
#define  I1_OK                          (0)
#define  I1_NO_ERR                      (0)

#define MI_NOTAGERR                     (-1)
#define MI_CHK_FAILED                   (-1)
#define MI_CRCERR                       (-2)
#define MI_CHK_COMPERR                  (-2)
#define MI_EMPTY                        (-3)
#define MI_AUTHERR                      (-4)
#define MI_PARITYERR                    (-5)
#define MI_CODEERR                      (-6)

#define MI_SERNRERR                     (-8)
#define MI_KEYERR                       (-9)
#define MI_NOTAUTHERR                   (-10)
#define MI_BITCOUNTERR                  (-11)
#define MI_BYTECOUNTERR                 (-12)
#define MI_IDLE                         (-13)
#define MI_TRANSERR                     (-14)
#define MI_WRITEERR                     (-15)
#define MI_INCRERR                      (-16)
#define MI_DECRERR                      (-17)
#define MI_READERR                      (-18)
#define MI_OVFLERR                      (-19)
#define MI_POLLING                      (-20)
#define MI_FRAMINGERR                   (-21)
#define MI_ACCESSERR                    (-22)
#define MI_UNKNOWN_COMMAND              (-23)
#define MI_COLLERR                      (-24)
#define MI_RESETERR                     (-25)
#define MI_INITERR                      (-25)
#define MI_INTERFACEERR                 (-26)
#define MI_ACCESSTIMEOUT                (-27)
#define MI_NOBITWISEANTICOLL            (-28)
#define MI_QUIT                         (-30)
#define MI_CODINGERR                    (-31)
#define MI_SENDBYTENR                   (-51)
#define MI_CASCLEVEX                    (-52)    
#define MI_SENDBUF_OVERFLOW             (-53)
#define MI_BAUDRATE_NOT_SUPPORTED       (-54)
#define MI_SAME_BAUDRATE_REQUIRED       (-55)

#define MI_WRONG_PARAMETER_VALUE        (-60)

// ICODE1 Error   Codes
#define  I1_WRONGPARAM                  (-61)
#define  I1_NYIMPLEMENTED               (-62)
#define  I1_TSREADY                     (-63)

#define  I1_TIMEOUT                     (-70)
#define  I1_NOWRITE                     (-71)
#define  I1_NOHALT                      (-72)
#define  I1_MISS_ANTICOLL               (-73) 

#define  I1_COMM_ABORT                  (-82)

#define MI_BREAK                        (-99)
#define MI_NY_IMPLEMENTED               (-100)
#define MI_NO_MFRC                      (-101)
#define MI_MFRC_NOTAUTH                 (-102)
#define MI_WRONG_DES_MODE               (-103)
#define MI_HOST_AUTH_FAILED             (-104)

#define MI_WRONG_LOAD_MODE              (-106)
#define MI_WRONG_DESKEY                 (-107)
#define MI_MKLOAD_FAILED                (-108)
#define MI_FIFOERR                      (-109)
#define MI_WRONG_ADDR                   (-110)
#define MI_DESKEYLOAD_FAILED            (-111)
#define MI_RECBUF_OVERFLOW              (-112)
#define MI_WRONG_SEL_CNT                (-114)

#define MI_WRONG_TEST_MODE              (-117)
#define MI_TEST_FAILED                  (-118)
#define MI_TOC_ERROR                    (-119)
#define MI_COMM_ABORT                   (-120)
#define MI_INVALID_BASE                 (-121)
#define MI_MFRC_RESET                   (-122)
#define MI_WRONG_VALUE                  (-123)
#define MI_VALERR                       (-124)


//=====================================================================
//                         Fuction Prototypes
//=====================================================================
extern unsigned char MFReaderStatus;

// PCD Configuration
extern signed char Mf500PcdConfig(void);

// Active Antenna Slave Configuration of the MF RC500.
extern signed char Mf500ActiveAntennaSlaveConfig(void);

// Active Antenna Master Configuration of the MF RC 500
extern signed char Mf500ActiveAntennaMasterConfig(void);

// Set default attributes for the baudrate divider
extern signed char Mf500PcdSetDefaultAttrib(void);

// Set attributes for the baudrate divider 
extern signed char Mf500PcdSetAttrib(unsigned char DSI,
                           unsigned char DRI);

// Get transmission properties of the PCD
extern signed char Mf500PcdGetAttrib(unsigned char *FSCImax,
                          unsigned char *FSDImax,
                          unsigned char *DSsupp,
                          unsigned char *DRsupp,
                          unsigned char *DREQDS);

// PICC Request command
extern signed char Mf500PiccRequest(unsigned char req_code, 
                       unsigned char *atq);
                       
// PICC Request command for ISO 14443 A-4 Command set
extern signed char Mf500PiccCommonRequest(unsigned char req_code, 
                             unsigned char *atq);  

// PICC Anticollision Command
extern signed char Mf500PiccAnticoll (unsigned char bcnt,
                         unsigned char *snr);

// PICC Cascaded Anticollision Command
extern signed char Mf500PiccCascAnticoll (unsigned char select_code,
                             unsigned char bcnt,
                             unsigned char *snr);                     

// PICC Select Command
extern signed char Mf500PiccSelect(unsigned char *snr, 
                      unsigned char *sak);

// PICC Select Command
extern signed char Mf500PiccCascSelect(unsigned char select_code, 
                             unsigned char *snr, 
                             unsigned char *sak); 

// Activation of a PICC in IDLE mode
extern signed char Mf500PiccActivateIdle(unsigned char br,
                           unsigned char *atq, 
                           unsigned char *sak, 
                           unsigned char *uid, 
                           unsigned char *uid_len);

// Activation of all PICC's in the RF field
extern signed char Mf500PiccActivateWakeup(unsigned char br,
                             unsigned char *atq, 
                             unsigned char *sak,
                             unsigned char *uid, 
                             unsigned char uid_len);

// MIFARE®  Authentication
extern signed char Mf500PiccAuth(unsigned char auth_mode, 
                      unsigned char key_sector, 
                      unsigned char block);   
  
// MIFARE ® Authentication with  keys stored  in the MF RC 500's EEPROM.
extern signed char Mf500PiccAuthE2( unsigned char auth_mode, 
                         unsigned char *snr,      
                         unsigned char key_sector,
                         unsigned char block); 

// Authentication Key Coding
extern signed char Mf500HostCodeKey(unsigned char *uncoded, 
                         unsigned char *coded); 

// Key Loading into the MF RC500's EEPROM.
extern signed char Mf500PcdLoadKeyE2(unsigned char key_type,
                          unsigned char sector,
                          unsigned char *uncoded_keys); 
                     
// Authentication with direct key loading form the microcontroller
extern signed char Mf500PiccAuthKey(unsigned char auth_mode,
                         unsigned char *snr,   
                         unsigned char *keys,  
                         unsigned char sector);   
                     
// PICC Read Block
extern signed char Mf500PiccRead(unsigned char addr,  
                       unsigned char* data);

// PICC Read Block of variable length
extern signed char Mf500PiccCommonRead(unsigned char cmd,
                             unsigned char addr,  
                             unsigned char datalen,
                             unsigned char *data);
                  
// PICC Write Block
extern signed char Mf500PiccWrite(unsigned char addr,
                        unsigned char *data);

// PICC Write 4 Byte Block
extern signed char Mf500PiccWrite4(unsigned char addr,
                         unsigned char *data);
                       
// PICC Write Block of variable length
extern signed char Mf500PiccCommonWrite(unsigned char cmd,
                              unsigned char addr,
                              unsigned char datalen,
                              unsigned char *data);

// PICC Value Block Operation
extern signed char Mf500PiccValue(unsigned char dd_mode, 
                       unsigned char addr, 
                       unsigned char *value,
                       unsigned char trans_addr);

// PICC Value Block Operation for Cards with automatic transfer
extern signed char Mf500PiccValueDebit(unsigned char dd_mode, 
                             unsigned char addr, 
                             unsigned char *value);

// Exchange Data Blocks PCD --> PICC --> PCD
extern signed char Mf500PiccExchangeBlock(unsigned char *send_data,
                               unsigned short send_bytelen,
                               unsigned char *rec_data,  
                               unsigned short *rec_bytelen,
                               unsigned char append_crc, 
                               unsigned long timeout );                  

// PICC Halt
extern signed char Mf500PiccHalt(void);

// Reset the reader ic 
extern signed char PcdReset(void);

// Exchange Data Stream PCD --> PICC --> PCD
extern signed char ExchangeByteStream(unsigned char Cmd,
                            unsigned char *send_data,
                            unsigned short send_bytelen,
                            unsigned char *rec_data,  
                            unsigned short *rec_bytelen);

// Set RF communication timeout 
extern signed char PcdSetTmo(unsigned long numberOfEtus);

// Read Serial Number from Reader IC
extern signed char PcdGetSnr(unsigned char *snr);

// Read EEPROM Memory Block
extern signed char PcdReadE2(unsigned short startaddr,  
                   unsigned char length,
                   unsigned char* data);

// Writes data to the reader IC's EEPROM blocks.
extern signed char PcdWriteE2(  unsigned int startaddr,
                      unsigned char length,
                      unsigned char* data);

// Turns ON/OFF RF field
extern signed char PcdRfReset(unsigned short ms);    

extern void ConfigureMFRC531(void);

extern void OpenRC(void);

//MFRC Registers
//Page 0: Command and Status
#define MFRC522_REG_RESERVED00			0x00    
#define MFRC522_REG_COMMAND				0x01    
#define MFRC522_REG_COMM_IE_N			0x02    
#define MFRC522_REG_DIV1_EN				0x03    
#define MFRC522_REG_COMM_IRQ			0x04    
#define MFRC522_REG_DIV_IRQ				0x05
#define MFRC522_REG_ERROR				0x06    
#define MFRC522_REG_STATUS1				0x07    
#define MFRC522_REG_STATUS2				0x08    
#define MFRC522_REG_FIFO_DATA			0x09
#define MFRC522_REG_FIFO_LEVEL			0x0A
#define MFRC522_REG_WATER_LEVEL			0x0B
#define MFRC522_REG_CONTROL				0x0C
#define MFRC522_REG_BIT_FRAMING			0x0D
#define MFRC522_REG_COLL				0x0E
#define MFRC522_REG_RESERVED01			0x0F
//Page 1: Command 
#define MFRC522_REG_RESERVED10			0x10
#define MFRC522_REG_MODE				0x11
#define MFRC522_REG_TX_MODE				0x12
#define MFRC522_REG_RX_MODE				0x13
#define MFRC522_REG_TX_CONTROL			0x14
#define MFRC522_REG_TX_AUTO				0x15
#define MFRC522_REG_TX_SELL				0x16
#define MFRC522_REG_RX_SELL				0x17
#define MFRC522_REG_RX_THRESHOLD		0x18
#define MFRC522_REG_DEMOD				0x19
#define MFRC522_REG_RESERVED11			0x1A
#define MFRC522_REG_RESERVED12			0x1B
#define MFRC522_REG_MIFARE				0x1C
#define MFRC522_REG_RESERVED13			0x1D
#define MFRC522_REG_RESERVED14			0x1E
#define MFRC522_REG_SERIALSPEED			0x1F
//Page 2: CFG    
#define MFRC522_REG_RESERVED20			0x20  
#define MFRC522_REG_CRC_RESULT_M		0x21
#define MFRC522_REG_CRC_RESULT_L		0x22
#define MFRC522_REG_RESERVED21			0x23
#define MFRC522_REG_MOD_WIDTH			0x24
#define MFRC522_REG_RESERVED22			0x25
#define MFRC522_REG_RF_CFG				0x26
#define MFRC522_REG_GS_N				0x27
#define MFRC522_REG_CWGS_PREG			0x28
#define MFRC522_REG__MODGS_PREG			0x29
#define MFRC522_REG_T_MODE				0x2A
#define MFRC522_REG_T_PRESCALER			0x2B
#define MFRC522_REG_T_RELOAD_H			0x2C
#define MFRC522_REG_T_RELOAD_L			0x2D
#define MFRC522_REG_T_COUNTER_VALUE_H	0x2E
#define MFRC522_REG_T_COUNTER_VALUE_L	0x2F
//Page 3:TestRegister 
#define MFRC522_REG_RESERVED30			0x30
#define MFRC522_REG_TEST_SEL1			0x31
#define MFRC522_REG_TEST_SEL2			0x32
#define MFRC522_REG_TEST_PIN_EN			0x33
#define MFRC522_REG_TEST_PIN_VALUE		0x34
#define MFRC522_REG_TEST_BUS			0x35
#define MFRC522_REG_AUTO_TEST			0x36
#define MFRC522_REG_VERSION				0x37
#define MFRC522_REG_ANALOG_TEST			0x38
#define MFRC522_REG_TEST_ADC1			0x39  
#define MFRC522_REG_TEST_ADC2			0x3A   
#define MFRC522_REG_TEST_ADC0			0x3B   
#define MFRC522_REG_RESERVED31			0x3C   
#define MFRC522_REG_RESERVED32			0x3D
#define MFRC522_REG_RESERVED33			0x3E   
#define MFRC522_REG_RESERVED34			0x3F
//Dummy byte
#define MFRC522_DUMMY					0x00

#define MFRC522_MAX_LEN					16

											//MF522 command bits
#define MFRC522_PCD_IDLE              0x00               //NO action; cancel current commands
#define MFRC522_PCD_AUTHENT           0x0E               //verify password key
#define MFRC522_PCD_RECEIVE           0x08               //receive data
#define MFRC522_PCD_TRANSMIT          0x04               //send data
#define MFRC522_PCD_TRANSCEIVE        0x0C               //send and receive data
#define MFRC522_PCD_RESETPHASE        0x0F               //reset
#define MFRC522_PCD_CALCCRC           0x03               //CRC check and caculation

//Mifare_One card command bits
#define PICC_REQIDL           0x26               //Search the cards that not into sleep mode in the antenna area 
#define PICC_REQALL           0x52               //Search all the cards in the antenna area
#define PICC_ANTICOLL         0x93               //prevent conflict
#define PICC_SElECTTAG        0x93               //select card
#define PICC_AUTHENT1A        0x60               //verify A password key
#define PICC_AUTHENT1B        0x61               //verify B password key
#define PICC_READ             0x30               //read 
#define PICC_WRITE            0xA0               //write
#define PICC_DECREMENT        0xC0               //deduct value
#define PICC_INCREMENT        0xC1               //charge up value
#define PICC_RESTORE          0xC2               //Restore data into buffer
#define PICC_TRANSFER         0xB0               //Save data into buffer
#define PICC_HALT             0x50               //sleep mode


//THe mistake code that return when communicate with MF522
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2


//------------------MFRC522 register ---------------
//Page 0:Command and Status
#define     Reserved00            0x00    
#define     CommandReg            0x01    
#define     CommIEnReg            0x02    
#define     DivlEnReg             0x03    
#define     CommIrqReg            0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     Reserved01            0x0F
//Page 1:Command     
#define     Reserved10            0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     Reserved11            0x1A
#define     Reserved12            0x1B
#define     MifareReg             0x1C
#define     Reserved13            0x1D
#define     Reserved14            0x1E
#define     SerialSpeedReg        0x1F
//Page 2:CFG    
#define     Reserved20            0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     Reserved21            0x23
#define     ModWidthReg           0x24
#define     Reserved22            0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg	          0x28
#define     ModGsPReg             0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
//Page 3:TestRegister     
#define     Reserved30            0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     Reserved31            0x3C   
#define     Reserved32            0x3D   
#define     Reserved33            0x3E   
#define     Reserved34			  0x3F
//-----------------------------------------------
unsigned char MFRC522_Request(unsigned char reqMode, unsigned char *TagType);
void MFRC522_Init(void);
unsigned char MFRC522_ToCard(unsigned char command, unsigned char *sendData, unsigned char sendLen, unsigned char *backData, unsigned int *backLen);
unsigned char MFRC522_Anticoll(unsigned char *serNum);
void MFRC522_Halt(void);

											
#endif											