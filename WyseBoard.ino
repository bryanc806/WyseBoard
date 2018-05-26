#include <PS2KeyAdvanced.h>

PS2KeyAdvanced  keyboard;

volatile unsigned char keys[32];

volatile unsigned char keyindex;
unsigned char lastKeyIndex;
unsigned long lastKeyEvent;

// handle the SPI interrupt for keyscan
ISR(SPI_STC_vect)
{
  keyindex++;
  if (keyindex > 12)
    keyindex = 0;
  SPDR = ~keys[keyindex];
}

// set/clear bits in the keyboard matrix.
// this array is sent via SPI in slave mode
#define PRESS_KEY(a)  (keys[(a) >> 3] |= (1 << ((a) & 7)))
#define RELEASE_KEY(a) (keys[(a) >> 3] &= ~(1 << ((a) & 7)))

// keymap of PS2 code to keyboard matrix index.
uint8_t keymap2[256];

// keymap of keyboard matrix (WYSE ASCII) to PS2 code.  This is converted to the keymap2 in setup()
const uint8_t PROGMEM keymap[104] =
  {
    PS2_KEY_PRTSCR,  0, PS2_KEY_ESC, PS2_KEY_BREAK, PS2_KEY_Y, PS2_KEY_KP_MINUS, PS2_KEY_COMMA, PS2_KEY_APOS,
    0, 0, 0, 0, PS2_KEY_CLOSE_SQ, PS2_KEY_SEMI, 0, PS2_KEY_UP_ARROW,
    0, PS2_KEY_SPACE, 0, 0, PS2_KEY_O, 0, PS2_KEY_ENTER, PS2_KEY_PGDN,
    PS2_KEY_KP_COMMA,   0,   PS2_KEY_KP_ENTER, PS2_KEY_KP_DOT, PS2_KEY_KP9, PS2_KEY_OPEN_SQ, PS2_KEY_SINGLE, 0,
    PS2_KEY_2, PS2_KEY_1, PS2_KEY_INSERT, PS2_KEY_MINUS, PS2_KEY_I, PS2_KEY_L, PS2_KEY_Z, PS2_KEY_KP8,
    PS2_KEY_F7, PS2_KEY_F14, PS2_KEY_0, PS2_KEY_BACK, PS2_KEY_U, PS2_KEY_K, PS2_KEY_DOT, PS2_KEY_KP7,
    PS2_KEY_F6, PS2_KEY_F13, PS2_KEY_9, PS2_KEY_HOME, PS2_KEY_T, PS2_KEY_J, PS2_KEY_B, PS2_KEY_KP6,
    PS2_KEY_F5, PS2_KEY_F12, PS2_KEY_8, PS2_KEY_DN_ARROW, PS2_KEY_P, PS2_KEY_H, PS2_KEY_M, PS2_KEY_KP5,
    PS2_KEY_F4, PS2_KEY_F11, PS2_KEY_7, PS2_KEY_R_ARROW, PS2_KEY_R, PS2_KEY_G, PS2_KEY_N, PS2_KEY_KP4,
    PS2_KEY_F3, PS2_KEY_F10, PS2_KEY_6, PS2_KEY_L_ARROW, PS2_KEY_W, PS2_KEY_D, PS2_KEY_V, PS2_KEY_KP3,
    PS2_KEY_F2, PS2_KEY_F9, PS2_KEY_5, PS2_KEY_F16, PS2_KEY_E, PS2_KEY_F, PS2_KEY_X, PS2_KEY_KP2,
    PS2_KEY_F1, PS2_KEY_F8, PS2_KEY_4, PS2_KEY_F15, PS2_KEY_Q, PS2_KEY_S, PS2_KEY_C, PS2_KEY_KP1,
    PS2_KEY_DELETE, PS2_KEY_EQUAL, PS2_KEY_3, PS2_KEY_BS, PS2_KEY_TAB, PS2_KEY_A, PS2_KEY_DIV, PS2_KEY_KP0

  };

void setup() {
  pinMode(10, INPUT); // SPI SS
  pinMode(12, OUTPUT); // SPI MISO
  pinMode(13, INPUT); // SPI Clock
  pinMode(9, OUTPUT); // SPI SS out (for resetting SPI)
  digitalWrite(9, HIGH); // deselect

  //configure SPI
  SPCR = (1 <<SPIE) | (1 << SPE) | (1 << DORD) | (1 << CPOL);// | (1 << CPHA);
  Serial.begin(115200);
  PRESS_KEY(104);  // set the ID bits.  Does not appear to affect anything
  PRESS_KEY(110);

  keyboard.begin( 5, 3 );

  // build the keymap2 array from the keymap array.
  uint8_t j;
  int i;

  for (i = 0; i <256; i++)
    keymap2[i] = 0xff;

  for (i = 0; i < 104; i++)
  {
    j = pgm_read_byte(&keymap[i]);
    if (j)
      keymap2[j] = i;
    else
      keymap2[j] = 0xff;
  }

  // start the SPI transaction
  digitalWrite(9, LOW);
  SPDR = ~keys[0];
}

uint16_t  cLast = 0;
uint16_t keyindexCount;

void loop() {

  if (keyindex == lastKeyIndex && digitalRead(13) == LOW)  // if we have not had an SPI interrupt in 2 ms, reset things.  Preferably use micro89 for this, but does not work well.
  {
    keyindexCount++;
    if (keyindexCount > 100) // ~200 counts per millisecond
    {
        //Serial.print("timeout ");
        //Serial.println(keycount);
        digitalWrite(9, HIGH);
        _delay_us(10);
        keyindex = 0;
        SPDR = ~keys[0];
        digitalWrite(9, LOW);
        keyindexCount = 0;
      }
  }
  else
  {
    lastKeyIndex = keyindex;
    keyindexCount=0;
  }
  
  if( keyboard.available() )
  {
    uint16_t  c;
    // read the next key
    c = keyboard.read();
    if( c > 0 )
      {
        (c & PS2_SHIFT) ? PRESS_KEY(8) : RELEASE_KEY(8);
        (c & PS2_CTRL) ? PRESS_KEY(18) : RELEASE_KEY(18);
        if (c & PS2_CAPS && !(cLast & PS2_CAPS))
          PRESS_KEY(9);
        else if (cLast & PS2_CAPS && !(c & PS2_CAPS))
          RELEASE_KEY(9);
        if (c & PS2_ALT)
          ;
        if (c & PS2_ALT_GR)
          ;
        if (c & PS2_GUI)
          ;
        if (c & PS2_FUNCTION)
         ;
        uint8_t i;
        uint8_t j;
        j = c & 0xff;
        i = keymap2[j];
        if (i != 0xff)
          (c & PS2_BREAK) ? RELEASE_KEY(i) : PRESS_KEY(i);
        cLast = c;
        //Serial.print( i );
        //Serial.print(" ");
        //Serial.println( c, HEX );
        //Serial.print( F( " - Status Bits " ) );
       // Serial.print( c >> 8, HEX );
        //Serial.print( F( "  Code " ) );
        //Serial.println( c & 0xFF, HEX );
        lastKeyEvent = micros();
      }
  }
  else
  {
    if ((cLast & PS2_CAPS) && micros() - lastKeyEvent > 100000)
       RELEASE_KEY(9);
  }
}

