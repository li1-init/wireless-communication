#include "board.h"
#include "radio_link.h"
#include "link_protocol.h"
#include "stm32l1xx_flash.h"

typedef struct {
    uint32_t role; /* 1=TX square/rectangular, 2=RX round */
    uint32_t local_id;
    uint32_t sequence;
    uint32_t session;
    uint32_t tx_failed;
    uint32_t rx_valid_count;
    uint32_t rx_rejected_count;
    uint32_t last_sender_id;
    uint32_t last_sequence;
    uint32_t last_valid_ms;
    uint8_t last_command;
    uint8_t last_keys;
    uint8_t last_packet[LINK_PACKET_SIZE];
    uint8_t online;
    uint8_t sender_locked;
    uint8_t reserved[3];
    uint32_t pair_success_count;
    uint32_t pair_timeout_count;
    uint8_t pair_mode;
} LinkAppStatus;
volatile LinkAppStatus g_link;

#if BOARD_IS_TX
static uint8_t SessionRecord(uint32_t address,uint32_t *value)
{
    uint32_t count=*(volatile const uint32_t *)address;
    uint32_t inverse=*(volatile const uint32_t *)(address+4UL);
    if(count==0U || count==0xFFFFFFFFUL || inverse!=~count) return 0U;
    *value=count;
    return 1U;
}
static uint8_t NewSession(void)
{
    const uint32_t first=0x08080008UL,second=0x08080010UL;
    uint32_t a=0U,b=0U,previous=0U,next,address;
    uint8_t valid_a=SessionRecord(first,&a);
    uint8_t valid_b=SessionRecord(second,&b);
    if(valid_a && (!valid_b || (int32_t)(a-b)>0)) {
        previous=a; address=second;
    } else if(valid_b) {
        previous=b; address=first;
    } else {
        uint32_t legacy=*(volatile const uint32_t *)0x08080000UL;
        if(*(volatile const uint32_t *)(first+4UL)!=0xFFFFFFFFUL ||
           *(volatile const uint32_t *)second!=0xFFFFFFFFUL ||
           *(volatile const uint32_t *)(second+4UL)!=0xFFFFFFFFUL)
            return 0U;
        if(legacy!=0xFFFFFFFFUL) {
            if(legacy==0U) return 0U;
            previous=legacy;
        }
        address=first;
    }
    if(previous>=0xFFFFFFFEUL) return 0U;
    next=previous+1UL;
    DATA_EEPROM_Unlock();
    if(DATA_EEPROM_ProgramWord(address,next)!=FLASH_COMPLETE ||
       DATA_EEPROM_ProgramWord(address+4UL,~next)!=FLASH_COMPLETE ||
       !SessionRecord(address,&a) || a!=next) {
        DATA_EEPROM_Lock();
        return 0U;
    }
    DATA_EEPROM_Lock();
    g_link.session=next;
    return 1U;
}
#endif

static void StartupLights(void)
{
#if BOARD_IS_TX
    Board_Leds(LED_MASK_RED); Board_DelayMs(200);
    Board_Leds(0); Board_DelayMs(200);
    Board_Leds(LED_MASK_GREEN); Board_DelayMs(200);
    Board_Leds(0); Board_DelayMs(200);
#if BOARD_HAS_WHITE
    Board_Leds(LED_MASK_WHITE); Board_DelayMs(200);
    Board_Leds(0); Board_DelayMs(200);
#endif
#else
    Board_Sound(SOUND_BOOT,0U);
    Board_DelayMs(400U); /* before RX starts; subsequent sounds never block RX */
#endif
}
static void ErrorLights(uint32_t now)
{
#if BOARD_IS_TX
    uint32_t code=g_radio.error ? g_radio.error : RF_COMMAND_ERROR;
    uint32_t phase=now % (code*300UL+1200UL);
    Board_Leds((phase<code*300UL && phase%300UL<120UL) ? LED_MASK_RED : 0U);
#else
    (void)now; /* RX faults are announced once on entry, then on each retry. */
#endif
}
static uint8_t InitializeRadio(void)
{
    uint8_t ok=Radio_Init();
#if !BOARD_IS_TX
    if(ok) ok=Radio_StartReceive();
#endif
    g_link.online=ok;
#if !BOARD_IS_TX
    if(!ok) Board_Sound(SOUND_FAULT,g_radio.error);
#endif
    return ok;
}
#if BOARD_IS_TX
static uint8_t KeyEdges(uint32_t now)
{
    static uint8_t raw_old,stable;
    static uint32_t changed[3];
    uint8_t raw=Board_Keys(),i,bit,edges=0;
    for(i=0;i<3U;++i) {
        bit=(uint8_t)(1U<<i);
        if((raw^raw_old)&bit) changed[i]=now;
        if(((raw^stable)&bit) && (uint32_t)(now-changed[i])>=20UL) {
            stable ^= bit;
            if(stable&bit) edges |= bit;
        }
    }
    raw_old=raw;
    return edges;
}
#endif

int main(void)
{
    uint8_t clock_ok,packet[LINK_PACKET_SIZE],i;
    uint32_t now,last_retry;
#if BOARD_IS_TX
#define PAIR_TIMEOUT_MS 10000UL
#define PAIR_RETRY_MS 400UL
    uint32_t pulse_started=0;
    uint8_t pulse_active=0;
    uint32_t last_send;
    uint32_t pair_started=0,pair_last_send=0;
    uint8_t pending_keys=0,pair_active=0,pair_nonce=0;
#endif
    LinkMessage message;
    clock_ok=Board_Init();
    g_link.role=BOARD_IS_TX ? 1U : 2U;
    g_link.local_id=Board_DeviceId();
#if BOARD_IS_TX
    if(!NewSession()) clock_ok=0U; /* Never reuse a boot epoch. */
#endif
    StartupLights();
    if(clock_ok) (void)InitializeRadio();
    else {
        g_radio.error=RF_CLOCK_ERROR;
        Board_Sound(SOUND_FAULT,RF_CLOCK_ERROR);
    }
    last_retry=Board_Millis();
    if(g_link.online) {
#if BOARD_IS_TX
        /* TX initialization complete; later flashes require PACKET_SENT. */
        Board_Leds(LED_MASK_GREEN);
        Board_DelayMs(500);
        Board_Leds(0);
#endif
    }
#if BOARD_IS_TX
    last_send=Board_Millis()-LINK_INTERVAL_MS;
#endif
    for(;;) {
        now=Board_Millis();
        if(!g_link.online) {
            ErrorLights(now);
            if(clock_ok && (uint32_t)(now-last_retry)>=5000UL) {
                Board_Leds(0);
                (void)InitializeRadio();
                last_retry=Board_Millis();
#if BOARD_IS_TX
                pulse_active=0;
#endif
            }
            Board_DelayMs(1);
            continue;
        }
#if BOARD_IS_TX
        {
            uint8_t edges=KeyEdges(now);
            if((edges&4U) && !pair_active) {
                pair_active=1U;
                pair_started=now;
                pair_last_send=now-PAIR_RETRY_MS;
                pair_nonce=(uint8_t)(++g_link.sequence);
                pending_keys=0U;
                pulse_active=0U;
                g_link.pair_mode=1U;
            } else pending_keys|=(uint8_t)(edges&3U);
        }
        if(pair_active) {
            if((uint32_t)(now-pair_started)>=PAIR_TIMEOUT_MS) {
                pair_active=0U;
                g_link.pair_mode=0U;
                ++g_link.pair_timeout_count;
                Board_Leds(0U);
                last_send=now-LINK_INTERVAL_MS;
            } else {
                int status=0;
                Board_Leds(LED_MASK_WHITE);
                if((uint32_t)(now-pair_last_send)>=PAIR_RETRY_MS) {
                    message.type=LINK_PAIR_REQUEST;
                    message.session=g_link.session;
                    message.sender_id=g_link.local_id;
                    message.payload=0U;
                    message.sequence=pair_nonce;
                    Link_Encode(packet,&message);
                    for(i=0;i<LINK_PACKET_SIZE;++i) g_link.last_packet[i]=packet[i];
                    g_link.last_command=LINK_PAIR_REQUEST;
                    if(!Radio_Send(packet) || !Radio_StartReceive()) status=-1;
                    pair_last_send=Board_Millis();
                }
                if(status==0) status=Radio_PollReceive(packet);
                if(status<0) {
                    Radio_Shutdown();
                    ++g_link.tx_failed;
                    g_link.online=0U;
                    pair_active=0U;
                    g_link.pair_mode=0U;
                    last_retry=Board_Millis();
                    Board_Leds(0U);
                } else if(status>0 && Link_Decode(packet,&message) &&
                          message.type==LINK_PAIR_ACK &&
                          message.sender_id==g_link.local_id &&
                          message.session==g_link.session &&
                          message.sequence==pair_nonce) {
                    pair_active=0U;
                    g_link.pair_mode=0U;
                    ++g_link.pair_success_count;
                    Board_Leds(0U);
                    last_send=Board_Millis()-LINK_INTERVAL_MS;
                }
                Board_DelayMs(1);
                continue;
            }
        }
        if((uint32_t)(now-last_send)>=LINK_INTERVAL_MS ||
           (pending_keys && (uint32_t)(now-last_send)>=300UL)) {
            uint8_t command=LINK_BEACON;
            for(i=0;i<2U;++i) if(pending_keys&(1U<<i)) {
                command=i+1U;
                pending_keys &= (uint8_t)~(1U<<i);
                break;
            }
            message.session=g_link.session;
            message.sequence=(uint8_t)(++g_link.sequence);
            message.sender_id=g_link.local_id;
            message.type=LINK_DATA;
            message.payload=(uint8_t)((command<<3U)|(Board_Keys()&7U));
            Link_Encode(packet,&message);
            for(i=0;i<LINK_PACKET_SIZE;++i) g_link.last_packet[i]=packet[i];
            g_link.last_command=command;
            g_link.last_keys=(uint8_t)(message.payload&7U);
            if(Radio_Send(packet)) {
                pulse_started=Board_Millis();
                pulse_active=1U;
            } else {
                ++g_link.tx_failed;
                g_link.online=0;
                last_retry=Board_Millis();
                Board_Leds(0);
                Board_Sound(SOUND_FAULT,g_radio.error);
            }
            last_send=Board_Millis();
        }
        if(g_link.online) {
            if(pulse_active && (uint32_t)(Board_Millis()-pulse_started)<120UL)
                Board_Leds(LED_MASK_GREEN);
            else { pulse_active=0; Board_Leds(0); }
        }
#endif
        Board_DelayMs(1);
    }
}
