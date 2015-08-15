// What pins do what
#define LED PB2
#define IRLED PB0

// Shortcut to insert single, non-optimized-out nop
#define NOP __asm__ __volatile__ ("nop")

// Tweak this if neccessary to change timing
#define DELAY_CNT 11

// Makes the codes more readable. the OCRA is actually
// programmed in terms of 'periods' not 'freqs' - that
// is, the inverse!
#define freq_to_timerval(x) ((F_CPU / x - 1)/ 2)

void xmitCodeElement(uint16_t ontime, uint16_t offtime );
void delay_us(uint16_t us);
