/* Run against production radio_link.c and link_protocol.c.
 * Models command framing, properties, FIFOs, IRQ flags and state transitions.
 * It is a software regression test, not a substitute for the two real boards. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../User/radio_port.h"
#include "../User/radio_link.h"
#include "../User/link_protocol.h"

enum { F_NONE,F_CTS,F_SPI,F_PART,F_COMMAND,F_PROPERTY,F_TX_TIMEOUT,F_TX_FIFO,
       F_TX_UNDERFLOW,F_READY,F_SPI_FINISH };
static unsigned fault,cases;
static uint32_t now;
static uint8_t selected,sdn,wire[64],count,reply[32],reply_len,props[256][256];
static uint8_t powered,state,ph,chip,error,error_id,tx_fifo[64],rx_fifo[64],tx_count,rx_count;
static uint8_t air[LINK_PACKET_SIZE],air_valid;
static unsigned tx_polls,set_property_count,read_fifo_count;

#define REQUIRE(x) do { if(!(x)) { \
 fprintf(stderr,"FAIL line %d: %s; fault=%u error=%u stage=%lu cmd=%02x\n",__LINE__,#x,fault, \
 g_radio.error,(unsigned long)g_radio.stage,g_radio.last_command);exit(2);} } while(0)
#define PASS(name) do { ++cases; printf("PASS %s\n",name); } while(0)

uint32_t Board_Millis(void) { return now; }
void Board_DelayMs(uint32_t ms) { now+=ms; }
void Board_RadioSpeed(uint8_t fast) { (void)fast; }
void Board_RadioInit(void) { selected=0;sdn=1;count=0; }
void Board_Sdn(uint8_t high)
{
    if(sdn&&!high) {
        powered=0;state=0;ph=chip=error=error_id=0;
        tx_count=rx_count=0;
        memset(props,0,sizeof(props));
    }
    sdn=high;
}
static void Process(void)
{
    unsigned i;
    uint8_t old_error=error,old_id=error_id;
    REQUIRE(count>0);
    if(wire[0]==0x44 || wire[0]==0x77) return;
    memset(reply,0,sizeof(reply));reply_len=0;
    if(wire[0]!=0x02) REQUIRE(powered);
    switch(wire[0]) {
    case 0x02:
        REQUIRE(count==7);
        REQUIRE(wire[1]==1&&wire[2]==0&&wire[3]==1&&wire[4]==0xC9&&wire[5]==0xC3&&wire[6]==0x80);
        powered=1;state=3;
        break;
    case 0x01:
        REQUIRE(count==1);
        reply_len=8;reply[0]=0x22;reply[1]=0x44;reply[2]=(fault==F_PART)?0x63:0x38;
        reply[3]=1;reply[4]=0x12;reply[5]=0x34;reply[7]=6;
        break;
    case 0x13:
        REQUIRE(count==8);
        REQUIRE(wire[1]==1&&wire[2]==1&&wire[3]==1&&wire[4]==1&&wire[5]==0&&wire[6]==0);
        reply_len=7;memcpy(reply,wire+1,7);
        break;
    case 0x11:
        REQUIRE(count==wire[2]+4U&&wire[2]<=12U);
        ++set_property_count;
        for(i=0;i<wire[2];++i) props[wire[1]][wire[3]+i]=wire[4+i];
        if(fault==F_COMMAND) { error=0x40;error_id=0x11;chip|=8; }
        break;
    case 0x12:
        REQUIRE(count==4&&wire[2]<=12U);
        reply_len=wire[2];
        for(i=0;i<reply_len;++i) reply[i]=props[wire[1]][wire[3]+i];
        if(fault==F_PROPERTY) reply[0]^=1;
        break;
    case 0x23:
        REQUIRE(count==2);
        reply_len=4;reply[0]=old_error?8:2;reply[1]=reply[0];
        reply[2]=old_error;reply[3]=old_id;
        error=error_id=0;chip&=(uint8_t)~8U;
        break;
    case 0x34:
        REQUIRE(count==2&&wire[1]==3);
        state=(fault==F_READY)?7:3;
        break;
    case 0x33:
        REQUIRE(count==1);
        reply_len=2;reply[0]=state;reply[1]=LINK_CHANNEL;
        break;
    case 0x20:
        REQUIRE(count==4&&wire[1]==0&&wire[2]==0&&wire[3]==0);
        if(state==7&&++tx_polls>=3U&&fault!=F_TX_TIMEOUT&&fault!=F_READY) {
            if(fault==F_TX_UNDERFLOW) chip|=0x20;
            else { ph|=0x20;memcpy(air,tx_fifo,LINK_PACKET_SIZE);air_valid=1; }
            state=3;
        }
        reply_len=8;reply[2]=ph;reply[6]=chip;
        ph=chip=0;
        break;
    case 0x15:
        REQUIRE(count==2&&state!=8);
        if(wire[1]&1) tx_count=0;
        if(wire[1]&2) rx_count=0;
        reply_len=2;reply[0]=rx_count;reply[1]=(fault==F_TX_FIFO)?4U:64U-tx_count;
        break;
    case 0x66:
        REQUIRE(count==LINK_PACKET_SIZE+1U&&state==3);
        tx_count=LINK_PACKET_SIZE;memcpy(tx_fifo,wire+1,LINK_PACKET_SIZE);
        break;
    case 0x31:
        REQUIRE(count==7&&wire[1]==0&&wire[2]==0x30&&wire[3]==0&&wire[4]==0&&wire[5]==0&&wire[6]==0);
        REQUIRE(tx_count==LINK_PACKET_SIZE&&props[0x12][0x0E]==LINK_PACKET_SIZE);
        tx_polls=0;state=7;
        break;
    case 0x32:
        REQUIRE(count==8&&wire[1]==0&&wire[2]==0&&wire[3]==0&&wire[4]==0&&wire[5]==8&&wire[6]==3&&wire[7]==3);
        REQUIRE(state==3&&rx_count==0);
        state=8;
        break;
    default: REQUIRE(0);
    }
}
void Board_Nsel(uint8_t high)
{
    if(!high) { REQUIRE(!selected);selected=1;count=0; }
    else {
        if(selected&&count&&fault!=F_SPI&&fault!=F_SPI_FINISH) Process();
        selected=0;
    }
}
uint8_t Board_SpiByte(uint8_t tx,uint8_t *rx)
{
    uint8_t offset=count;
    REQUIRE(selected&&!sdn&&count<sizeof(wire));
    if(fault==F_SPI) return 0;
    wire[count++]=tx;*rx=0;
    if(wire[0]==0x44) {
        if(offset==1) *rx=(fault==F_CTS)?0:0xFF;
        if(offset>=2) { REQUIRE(offset-2U<reply_len);*rx=reply[offset-2U]; }
    } else if(wire[0]==0x77&&offset>=1) {
        REQUIRE(state==3&&rx_count>0);
        *rx=rx_fifo[0];
        memmove(rx_fifo,rx_fifo+1,--rx_count);
        ++read_fifo_count;
    }
    return 1;
}
uint8_t Board_SpiFinish(void) { return fault!=F_SPI_FINISH; }
static void Reset(unsigned f)
{
    fault=f;now=0;set_property_count=read_fifo_count=0;
    memset((void*)&g_radio,0,sizeof(g_radio));
}
static void AssertInit(void)
{
    REQUIRE(Radio_Init());
    REQUIRE(g_radio.error==0&&g_radio.part==0x4438&&state==3&&!selected&&!sdn);
    REQUIRE(set_property_count>30);
    REQUIRE(props[0x12][0]==0x85&&props[0x12][6]==2&&props[0x12][8]==0);
    REQUIRE(props[0x12][0x0D]==0&&props[0x12][0x0E]==16&&props[0x12][0x0F]==0&&props[0x12][0x10]==0xAA);
    REQUIRE(props[0x01][0]==5&&props[0x01][1]==0x38&&props[0x01][3]==0x28);
    REQUIRE(props[0x20][0]==3&&props[0x22][1]==0x10);
}
static void Inject(const uint8_t *packet,uint8_t length,uint8_t ph_flags,uint8_t chip_flags)
{
    REQUIRE(state==8);
    if(length) memcpy(rx_fifo,packet,length);
    rx_count=length;ph=ph_flags;chip=chip_flags;state=3;
}
static void InitFault(unsigned f,unsigned expected,const char *name)
{
    Reset(f);
    REQUIRE(!Radio_Init()&&g_radio.error==expected&&sdn&&!selected&&now<2000);
    PASS(name);
}
int main(void)
{
    uint8_t packet[32],received[16],original[16];
    LinkMessage message={0x01020304UL,0x11223344UL,LINK_CODE,5},decoded;
    unsigned i,b;
    REQUIRE(Link_Crc16((const uint8_t*)"123456789",9)==0x29B1);
    PASS("CRC16 known-answer 123456789");
    Link_Encode(packet,&message);memcpy(original,packet,16);
    REQUIRE(packet[0]==0x54&&packet[1]==0x52&&packet[4]==4&&packet[7]==1&&packet[8]==0x44&&packet[11]==0x11);
    REQUIRE(Link_Decode(packet,&decoded)&&decoded.sequence==message.sequence&&decoded.sender_id==message.sender_id);
    PASS("payload format / endian / roundtrip");
    for(i=0;i<16;++i) for(b=0;b<8;++b) {
        memcpy(packet,original,16);packet[i]^=(uint8_t)(1U<<b);
        REQUIRE(!Link_Decode(packet,&decoded));
    }
    PASS("all 128 single-bit corruptions rejected");
    for(i=0;i<4;++i) {
        message.command=(uint8_t)i;
        Link_Encode(packet,&message);REQUIRE(Link_Decode(packet,&decoded)&&decoded.command==i);
    }
    PASS("all four command types");
    message.command=4;Link_Encode(packet,&message);REQUIRE(!Link_Decode(packet,&decoded));
    message.command=0;message.keys=8;Link_Encode(packet,&message);REQUIRE(!Link_Decode(packet,&decoded));
    PASS("invalid fields rejected even with valid application CRC");
    memcpy(packet,original,16);

    Reset(F_NONE);AssertInit();PASS("WDS properties and all test overrides verified");
    REQUIRE(Radio_Send(packet)&&g_radio.tx_count==1&&air_valid&&memcmp(air,packet,16)==0);
    REQUIRE(state==3&&!sdn&&!selected);PASS("TX FIFO -> PACKET_SENT -> READY");
    /* Fresh receiver instance consumes the frame emitted by the transmitter. */
    Reset(F_NONE);AssertInit();REQUIRE(Radio_StartReceive()&&state==8);
    REQUIRE(Radio_PollReceive(received)==0);PASS("RX idle has no false packet");
    Inject(air,16,0x10,0);
    REQUIRE(Radio_PollReceive(received)==1&&state==8&&read_fifo_count==16);
    REQUIRE(Link_Decode(received,&decoded)&&decoded.command==LINK_CODE&&decoded.sequence==0x01020304UL);
    REQUIRE(g_radio.rx_count==1);PASS("TX frame -> RX payload -> decode -> RX rearmed");
    Inject(packet,16,0x08,0);
    REQUIRE(Radio_PollReceive(received)==0&&state==8&&g_radio.crc_errors==1&&read_fifo_count==16);
    PASS("hardware CRC error discarded and RX recovered");
    Inject(packet,16,0x18,0);
    REQUIRE(Radio_PollReceive(received)==0&&g_radio.crc_errors==2&&state==8);
    PASS("CRC error overrides simultaneous PACKET_RX");
    Inject(packet,15,0x10,0);
    REQUIRE(Radio_PollReceive(received)==0&&state==8&&g_radio.fifo_errors==1);
    PASS("short FIFO flushed and RX recovered");
    packet[16]=0;
    Inject(packet,17,0x10,0);
    REQUIRE(Radio_PollReceive(received)==0&&state==8&&g_radio.fifo_errors==2);
    PASS("overlong FIFO flushed and RX recovered");
    Inject(packet,0,0,0x20);
    REQUIRE(Radio_PollReceive(received)==0&&state==8&&g_radio.fifo_errors==3);
    PASS("FIFO overflow recovery");
    packet[0]^=1;
    Inject(packet,16,0x10,0);
    REQUIRE(Radio_PollReceive(received)==1&&!Link_Decode(received,&decoded)&&state==8);
    PASS("foreign payload cannot pass application validation");
    memcpy(packet,original,16);

    InitFault(F_CTS,RF_CTS_TIMEOUT,"bounded CTS timeout + shutdown");
    InitFault(F_SPI,RF_SPI_TIMEOUT,"SPI byte fault + shutdown");
    InitFault(F_SPI_FINISH,RF_SPI_TIMEOUT,"SPI busy fault + shutdown");
    InitFault(F_PART,RF_WRONG_PART,"wrong chip rejected");
    InitFault(F_COMMAND,RF_COMMAND_ERROR,"WDS command error identified");
    InitFault(F_PROPERTY,RF_CONFIG_ERROR,"property readback mismatch rejected");
    InitFault(F_READY,RF_COMMAND_ERROR,"READY transition bounded");

    Reset(F_NONE);AssertInit();fault=F_TX_TIMEOUT;
    REQUIRE(!Radio_Send(packet)&&g_radio.error==RF_TX_TIMEOUT&&sdn&&now<2000);
    PASS("missing PACKET_SENT times out");
    Reset(F_NONE);AssertInit();fault=F_TX_FIFO;
    REQUIRE(!Radio_Send(packet)&&g_radio.error==RF_FIFO_ERROR&&sdn);
    PASS("insufficient TX FIFO space rejected");
    Reset(F_NONE);AssertInit();fault=F_TX_UNDERFLOW;
    REQUIRE(!Radio_Send(packet)&&g_radio.error==RF_FIFO_ERROR&&sdn);
    PASS("TX FIFO underflow rejected");
    Reset(F_NONE);AssertInit();REQUIRE(Radio_StartReceive());
    fault=F_CTS;REQUIRE(Radio_PollReceive(received)==-1&&g_radio.error==RF_CTS_TIMEOUT);
    PASS("runtime RX bus failure surfaced");
    fault=F_NONE;REQUIRE(Radio_Init()&&Radio_StartReceive());
    Inject(packet,16,0x10,0);REQUIRE(Radio_PollReceive(received)==1&&Link_Decode(received,&decoded));
    PASS("reinitialize and receive after communication fault");
    printf("%u cases passed; 0 failures\n",cases);
    return 0;
}
