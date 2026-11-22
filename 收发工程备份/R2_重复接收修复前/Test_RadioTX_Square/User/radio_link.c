#include "radio_link.h"
#include "radio_port.h"
#include "radio_config_b1_10k.h"

/* R2: the real TX returned PART=4438, CHIPREV=11, ROMID=03 and rejected
 * the R1 C2 table at offset 62 (SYNC_CONFIG2, property 1105).
 * Use a complete WDS B1-API modem table instead of dropping arbitrary bytes
 * from the C2 table. The imported Si4464 B1 table is a compatibility starting
 * point for this observed EZRadioPRO API; RF operation still needs board test.
 * Retune 434 -> 433MHz; retain board XO tuning and reduced PA level.
 * Both projects use exactly the same 10kbps / 20kHz deviation configuration. */
#undef RF_GPIO_PIN_CFG
#define RF_GPIO_PIN_CFG 0x13,1,1,1,1,0,0,0
#undef RF_GLOBAL_XO_TUNE_2
#define RF_GLOBAL_XO_TUNE_2 0x11,0x00,2,0,0x45,0x00
#undef RF_FREQ_CONTROL_INTE_8
#define RF_FREQ_CONTROL_INTE_8 0x11,0x40,8,0,0x38,0x0D,0xDD,0xDD,0x44,0x44,0x20,0xFE
#undef RF_PA_MODE_4
#define RF_PA_MODE_4 0x11,0x22,4,0,0x08,0x10,0x00,0x3D
static const uint8_t configuration[]=RADIO_CONFIGURATION_DATA_ARRAY;
volatile RadioLinkStatus g_radio;

static uint8_t Fail(uint8_t error)
{
    if(g_radio.error==RF_OK) g_radio.error=error;
    return 0U;
}
static uint8_t Byte(uint8_t tx,uint8_t *rx)
{
    return Board_SpiByte(tx,rx) ? 1U : Fail(RF_SPI_TIMEOUT);
}
static uint8_t End(void)
{
    uint8_t ok=Board_SpiFinish();
    Board_Nsel(1U);
    return ok ? 1U : Fail(RF_SPI_TIMEOUT);
}
static uint8_t Response(uint8_t *reply,uint8_t length)
{
    uint32_t start=Board_Millis();
    uint8_t cts,discard,i;
    do {
        Board_Nsel(0U);
        if(!Byte(0x44U,&discard)||!Byte(0xFFU,&cts)) {
            Board_Nsel(1U); return 0U;
        }
        g_radio.last_cts=cts;
        if(cts==0xFFU) {
            for(i=0;i<length;++i)
                if(!Byte(0xFFU,reply+i)) { Board_Nsel(1U); return 0U; }
            return End();
        }
        if(!End()) return 0U;
        Board_DelayMs(1U);
    } while((uint32_t)(Board_Millis()-start)<500UL);
    return Fail(RF_CTS_TIMEOUT);
}
static uint8_t Command(const uint8_t *cmd,uint8_t length,uint8_t *reply,uint8_t reply_length)
{
    uint8_t i,discard;
    g_radio.last_command=cmd[0];
    if(cmd[0]!=0x23U) {
        g_radio.request_length=length;
        for(i=0;i<16U;++i) g_radio.request_bytes[i]=(i<length)?cmd[i]:0U;
    }
    if(!Response(0,0)) return 0U;
    Board_Nsel(0U);
    for(i=0;i<length;++i)
        if(!Byte(cmd[i],&discard)) { Board_Nsel(1U); return 0U; }
    if(!End()) return 0U;
    return Response(reply,reply_length);
}
static uint8_t CheckError(void)
{
    const uint8_t cmd[]={0x23,0};
    uint8_t r[4];
    if(!Command(cmd,sizeof(cmd),r,sizeof(r))) return 0U;
    g_radio.command_error=r[2];
    g_radio.command_error_id=r[3];
    if(((r[0]|r[1])&0x08U)||r[2]) return Fail(RF_COMMAND_ERROR);
    return 1U;
}
static uint8_t Checked(const uint8_t *cmd,uint8_t length)
{
    return Command(cmd,length,0,0) && CheckError();
}
static uint8_t Interrupts(uint8_t r[8])
{
    const uint8_t cmd[]={0x20,0,0,0};
    if(!Command(cmd,sizeof(cmd),r,8)) return 0U;
    g_radio.last_ph_pending=r[2];
    g_radio.last_chip_pending=r[6];
    if(r[6]&0x08U) {
        (void)CheckError();
        return Fail(RF_COMMAND_ERROR);
    }
    return 1U;
}
static uint8_t FifoInfo(uint8_t reset,uint8_t reply[2])
{
    const uint8_t cmd[]={0x15,reset};
    return Command(cmd,sizeof(cmd),reply,2U);
}
static uint8_t FifoTransfer(uint8_t opcode,const uint8_t *tx,uint8_t *rx)
{
    uint8_t i,b;
    if(!Response(0,0)) return 0U;
    Board_Nsel(0U);
    if(!Byte(opcode,&b)) { Board_Nsel(1U); return 0U; }
    for(i=0;i<LINK_PACKET_SIZE;++i) {
        if(!Byte(tx ? tx[i] : 0xFFU,&b)) { Board_Nsel(1U); return 0U; }
        if(rx) rx[i]=b;
    }
    return End();
}
static uint8_t Ready(void)
{
    const uint8_t cmd[]={0x34,3};
    const uint8_t query=0x33;
    uint8_t reply[2];
    uint32_t start;
    if(!Command(cmd,sizeof(cmd),0,0)) return 0U;
    start=Board_Millis();
    do {
        if(!Command(&query,1,reply,2)) return 0U;
        g_radio.device_state=reply[0];
        if((reply[0]&0x0FU)==3U) return 1U;
        Board_DelayMs(1);
    } while((uint32_t)(Board_Millis()-start)<100UL);
    return Fail(RF_COMMAND_ERROR);
}
static uint8_t Verify(uint8_t group,uint8_t offset,const uint8_t *expected,uint8_t length)
{
    uint8_t cmd[]={0x12,group,length,offset}, reply[12],i;
    if(length>sizeof(reply)) return Fail(RF_CONFIG_ERROR);
    if(!Command(cmd,sizeof(cmd),reply,length)) return 0U;
    for(i=0;i<length;++i)
        if(reply[i]!=expected[i]) return Fail(RF_CONFIG_ERROR);
    return CheckError();
}
static uint8_t ConfigurePacket(void)
{
    /* B1 packet handler uses one shared field table. Property 1207
     * (PKT_CONFIG2) and C2's separate RX fields must never be addressed. */
    const uint8_t crc[]={0x11,0x12,1,0,0x85};
    const uint8_t packet[]={0x11,0x12,1,6,0x02};
    const uint8_t fixed_length[]={0x11,0x12,3,8,0,0,0};
    const uint8_t field[]={0x11,0x12,4,0x0D,0,LINK_PACKET_SIZE,0,0xAA};
    const uint8_t field2[]={0x11,0x12,4,0x11,0,0,0,0};
    const uint8_t field3[]={0x11,0x12,4,0x15,0,0,0,0};
    const uint8_t field4[]={0x11,0x12,4,0x19,0,0,0,0};
    const uint8_t field5[]={0x11,0x12,4,0x1D,0,0,0,0};
    const uint8_t interrupts[]={0x11,0x01,4,0,0x05,0x38,0x00,0x28};
    const uint8_t expected_freq[]={0x38,0x0D,0xDD,0xDD,0x44,0x44,0x20,0xFE};
    const uint8_t expected_sync[]={1,0xB4,0x2B};
    const uint8_t expected_mod[]={3,0,7,0x06,0x1A,0x80};
    const uint8_t expected_pa[]={0x08,0x10,0,0x3D};
    if(!Checked(crc,sizeof(crc)) || !Checked(packet,sizeof(packet)) ||
       !Checked(fixed_length,sizeof(fixed_length)) || !Checked(field,sizeof(field)) ||
       !Checked(field2,sizeof(field2)) || !Checked(field3,sizeof(field3)) ||
       !Checked(field4,sizeof(field4)) || !Checked(field5,sizeof(field5)) ||
       !Checked(interrupts,sizeof(interrupts))) return 0U;
    g_radio.stage=4;
    return Verify(0x12,0,crc+4,1) && Verify(0x12,6,packet+4,1) &&
           Verify(0x12,8,fixed_length+4,3) &&
           Verify(0x12,0x0D,field+4,4) && Verify(0x01,0,interrupts+4,4) &&
           Verify(0x40,0,expected_freq,8) && Verify(0x11,0,expected_sync,3) &&
           Verify(0x20,0,expected_mod,6) && Verify(0x22,0,expected_pa,4);
}
void Radio_Shutdown(void)
{
    Board_Nsel(1U);
    Board_Sdn(1U);
}
uint8_t Radio_Init(void)
{
    uint16_t pos=0;
    uint8_t n,reply[8],fifo[2],i;
    const uint8_t part=0x01;
    ++g_radio.init_count;
    g_radio.error=RF_OK;
    g_radio.command_error=g_radio.command_error_id=0;
    g_radio.config_offset=0;
    g_radio.profile_version=2U;
    g_radio.device_state=0U;
    g_radio.stage=1;
    Board_RadioInit();
    Board_Sdn(1U); Board_DelayMs(10);
    Board_Sdn(0U); Board_DelayMs(20);
    /* First WDS command is exactly POWER_UP (7 bytes). */
    if(configuration[0]!=7 || configuration[1]!=0x02) {
        Fail(RF_CONFIG_ERROR); goto failed;
    }
    if(!Checked(configuration+1,7)) goto failed;
    pos=8;
    g_radio.stage=2;
    if(!Command(&part,1,reply,8)) goto failed;
    for(i=0;i<8;++i) g_radio.part_info[i]=reply[i];
    g_radio.part=(uint16_t)((uint16_t)reply[1]<<8)|reply[2];
    if(g_radio.part!=0x4438U) { Fail(RF_WRONG_PART); goto failed; }
    if(reply[0]!=0x11U || reply[7]!=0x03U) {
        Fail(RF_UNSUPPORTED_REV); goto failed;
    }
    g_radio.stage=3;
    while(pos<sizeof(configuration)) {
        g_radio.config_offset=pos;
        n=configuration[pos++];
        if(n==0U) break;
        if(n>16U || (uint32_t)pos+n>sizeof(configuration)) {
            Fail(RF_CONFIG_ERROR); goto failed;
        }
        if(!Checked(configuration+pos,n)) goto failed;
        pos=(uint16_t)(pos+n);
    }
    if(pos!=sizeof(configuration) || configuration[pos-1]!=0) {
        Fail(RF_CONFIG_ERROR); goto failed;
    }
    if(!ConfigurePacket() || !Ready() || !FifoInfo(3,fifo) || !Interrupts(reply))
        goto failed;
    g_radio.stage=5;
    return 1U;
failed:
    Radio_Shutdown();
    return 0U;
}
uint8_t Radio_Send(const uint8_t packet[LINK_PACKET_SIZE])
{
    const uint8_t start_tx[]={0x31,LINK_CHANNEL,0x30,0,0,0,0};
    uint8_t irq[8],fifo[2];
    uint32_t started;
    if(g_radio.error!=RF_OK) return 0U;
    g_radio.stage=6;
    if(!Ready() || !Interrupts(irq) || !FifoInfo(1,fifo)) goto failed;
    if(fifo[1]<LINK_PACKET_SIZE) { Fail(RF_FIFO_ERROR); goto failed; }
    if(!FifoTransfer(0x66,packet,0) || !Command(start_tx,sizeof(start_tx),0,0)) goto failed;
    started=Board_Millis();
    do {
        if(!Interrupts(irq)) goto failed;
        if(irq[6]&0x20U) { ++g_radio.fifo_errors; Fail(RF_FIFO_ERROR); goto failed; }
        if(irq[2]&0x20U) { ++g_radio.tx_count; g_radio.stage=5; return 1U; }
        Board_DelayMs(1);
    } while((uint32_t)(Board_Millis()-started)<1000UL);
    Fail(RF_TX_TIMEOUT);
failed:
    Radio_Shutdown();
    return 0U;
}
uint8_t Radio_StartReceive(void)
{
    const uint8_t start_rx[]={0x32,LINK_CHANNEL,0,0,0,8,3,3};
    uint8_t irq[8],fifo[2];
    if(g_radio.error!=RF_OK) return 0U;
    if(!Ready() || !FifoInfo(2,fifo) || !Interrupts(irq) ||
       !Command(start_rx,sizeof(start_rx),0,0)) return 0U;
    g_radio.stage=7;
    return 1U;
}
int Radio_PollReceive(uint8_t packet[LINK_PACKET_SIZE])
{
    uint8_t irq[8],fifo[2];
    if(g_radio.error!=RF_OK) return -1;
    if(!Interrupts(irq)) return -1;
    if(irq[6]&0x20U) {
        ++g_radio.fifo_errors;
        return Radio_StartReceive() ? 0 : -1;
    }
    /* CRC failure has priority over PACKET_RX if both flags are latched. */
    if(irq[2]&0x08U) {
        ++g_radio.crc_errors;
        return Radio_StartReceive() ? 0 : -1;
    }
    if(!(irq[2]&0x10U)) return 0;
    if(!FifoInfo(0,fifo)) return -1;
    g_radio.last_fifo_count=fifo[0];
    if(fifo[0]!=LINK_PACKET_SIZE) {
        ++g_radio.fifo_errors;
        return Radio_StartReceive() ? 0 : -1;
    }
    if(!FifoTransfer(0x77,0,packet)) return -1;
    if(!Radio_StartReceive()) return -1; /* re-arm before indicating success */
    ++g_radio.rx_count;
    return 1;
}
