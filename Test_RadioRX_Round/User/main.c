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
    uint32_t pair_ack_count;
    uint8_t pair_mode;
} LinkAppStatus;
volatile LinkAppStatus g_link;

#if !BOARD_IS_TX
#define PAIR_CAPACITY 8U
#define PAIR_BASE 0x08080040UL
static struct {
    uint32_t id,session;
    uint8_t sequence,seen;
} paired[PAIR_CAPACITY];
static uint8_t pair_count;

static uint8_t PairStore(uint8_t slot,uint32_t id)
{
    uint32_t address=PAIR_BASE+(uint32_t)slot*8UL;
    DATA_EEPROM_Unlock();
    if(DATA_EEPROM_ProgramWord(address,id)!=FLASH_COMPLETE ||
       DATA_EEPROM_ProgramWord(address+4UL,~id)!=FLASH_COMPLETE ||
       *(volatile const uint32_t *)address!=id ||
       *(volatile const uint32_t *)(address+4UL)!=~id) {
        DATA_EEPROM_Lock();
        return 0U;
    }
    DATA_EEPROM_Lock();
    paired[slot].id=id;
    ++pair_count;
    return 1U;
}
static void PairLoad(void)
{
    uint8_t i;
    uint32_t first_id=*(volatile const uint32_t *)PAIR_BASE;
    uint32_t first_inverse=*(volatile const uint32_t *)(PAIR_BASE+4UL);
    for(i=0;i<PAIR_CAPACITY;++i) {
        uint32_t address=PAIR_BASE+(uint32_t)i*8UL;
        uint32_t id=*(volatile const uint32_t *)address;
        uint32_t inverse=*(volatile const uint32_t *)(address+4UL);
        if(id!=0U && id!=0xFFFFFFFFUL && inverse==~id) {
            paired[i].id=id;
            ++pair_count;
        }
    }
    /* Preserve the already deployed sender on the first firmware upgrade. */
    if(pair_count==0U && (first_id==0U || first_id==0xFFFFFFFFUL) &&
       (first_inverse==0U || first_inverse==0xFFFFFFFFUL))
        (void)PairStore(0U,LINK_EXPECTED_SENDER_ID);
}
static int PairFind(uint32_t id)
{
    uint8_t i;
    for(i=0;i<PAIR_CAPACITY;++i) if(paired[i].id==id) return i;
    return -1;
}
static uint8_t PairAdd(uint32_t id)
{
    uint8_t i;
    if(id==0U || id==0xFFFFFFFFUL) return 0U;
    if(PairFind(id)>=0) return 1U;
    if(pair_count>=PAIR_CAPACITY) return 0U;
    for(i=0;i<PAIR_CAPACITY;++i)
        if(paired[i].id==0U)
            return PairStore(i,id);
    return 0U;
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
#if !BOARD_IS_TX
    uint32_t pair_held=0U,pair_changed=0U,pair_ready_at=0U;
    uint32_t last_pair_id=0U,last_pair_session=0U;
    uint8_t pair_key_raw=0U,pair_key_stable=0U,pair_latched=0U;
    uint8_t last_pair_sequence=0U,pair_ack_seen=0U;
#endif
#if BOARD_IS_TX
    uint32_t pulse_started=0;
    uint8_t pulse_active=0;
    uint32_t last_send;
    uint8_t pending_keys=0,manual=0;
#endif
    LinkMessage message;
    clock_ok=Board_Init();
    g_link.role=BOARD_IS_TX ? 1U : 2U;
    g_link.local_id=Board_DeviceId();
#if !BOARD_IS_TX
    PairLoad();
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
        {
            uint8_t key=(Board_Keys()&1U)!=0U; /* PA0 SW KEY is fitted. */
            if(key!=pair_key_raw) { pair_key_raw=key; pair_changed=now; }
            if(key!=pair_key_stable && (uint32_t)(now-pair_changed)>=25UL) {
                pair_key_stable=key;
                if(key) { pair_held=now; pair_latched=0U; }
            }
            if(pair_key_stable && !pair_latched &&
               (uint32_t)(now-pair_held)>=2000UL) {
                pair_latched=1U;
                g_link.pair_mode=(uint8_t)!g_link.pair_mode;
                if(g_link.pair_mode) {
                    pair_ready_at=now+2000UL;
                    Board_Sound(SOUND_PAIR_ENTER,0U);
                } else Board_Sound(SOUND_PAIR_EXIT,0U);
            }
        }
        {
            int status=Radio_PollReceive(packet);
            if(status<0) {
                Radio_Shutdown();
                g_link.online=0;
                last_retry=Board_Millis();
                Board_Leds(0);
                Board_Sound(SOUND_FAULT,g_radio.error);
            } else if(status>0) {
                for(i=0;i<LINK_PACKET_SIZE;++i) g_link.last_packet[i]=packet[i];
                if(Link_Decode(packet,&message)) {
                    int slot=PairFind(message.sender_id);
                    if(message.type==LINK_PAIR_REQUEST && g_link.pair_mode &&
                       (int32_t)(now-pair_ready_at)>=0) {
                        if(PairAdd(message.sender_id)) {
                            uint8_t new_request=(uint8_t)(!pair_ack_seen ||
                                last_pair_id!=message.sender_id ||
                                last_pair_session!=message.session ||
                                last_pair_sequence!=message.sequence);
                            slot=PairFind(message.sender_id);
                            if(new_request && slot>=0) paired[slot].seen=0U;
                            message.type=LINK_PAIR_ACK;
                            message.payload=LINK_PAIR_ACK_OK;
                            Link_Encode(packet,&message);
                            /* Let the square board switch from TX to RX first. */
                            Board_DelayMs(30U);
                            if(Radio_Send(packet) && Radio_StartReceive()) {
                                ++g_link.pair_ack_count;
                                if(new_request) {
                                    last_pair_id=message.sender_id;
                                    last_pair_session=message.session;
                                    last_pair_sequence=message.sequence;
                                    pair_ack_seen=1U;
                                    ++g_link.pair_success_count;
                                    Board_Sound(SOUND_PAIR_OK,0U);
                                }
                            } else {
                                Radio_Shutdown();
                                g_link.online=0U;
                                last_retry=Board_Millis();
                                Board_Sound(SOUND_FAULT,g_radio.error);
                            }
                        } else ++g_link.rx_rejected_count;
                    } else if(message.type==LINK_DATA && !g_link.pair_mode && slot>=0 &&
                              Link_IsNewer(message.session,message.sequence,
                                           paired[slot].session,paired[slot].sequence,
                                           paired[slot].seen)) {
                        paired[slot].seen=1U;
                        paired[slot].session=message.session;
                        paired[slot].sequence=message.sequence;
                        g_link.sender_locked=1U;
                        g_link.last_sender_id=message.sender_id;
                        g_link.last_sequence=message.sequence;
                        g_link.session=message.session;
                        ++g_link.rx_valid_count;
                        g_link.last_command=(uint8_t)(message.payload>>3);
                        g_link.last_keys=(uint8_t)(message.payload&7U);
                        g_link.last_valid_ms=Board_Millis();
                        Board_Sound(SOUND_PACKET,0U);
                    } else {
                        ++g_link.rx_rejected_count;
                    }
                } else ++g_link.rx_rejected_count;
            }
        }
        /* No LEDs fitted. Sound envelope is advanced by SysTick, so polling
         * continues during both short beeps and while the buzzer is silent. */
        Board_DelayMs(1);
    }
}
