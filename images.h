#define allGood_width 60
#define allGood_height 36
const unsigned char allGood_bits[] PROGMEM = {
 
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFF, 
  0xFF, 0xF1, 0x03, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFF, 0xFB, 0x07, 0x00, 
  0x00, 0x00, 0xFF, 0x00, 0xE0, 0xFD, 0x07, 0x00, 0x00, 0xC0, 0x1F, 0x00, 
  0x00, 0xFE, 0x07, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0xFF, 0x03, 0x00, 
  0x00, 0xF0, 0x01, 0x00, 0x80, 0xFF, 0x01, 0x00, 0x00, 0xF8, 0x00, 0x00, 
  0xC0, 0xFF, 0x02, 0x00, 0x00, 0x7C, 0x00, 0x00, 0xF0, 0x3F, 0x07, 0x00, 
  0x00, 0x7C, 0x00, 0x00, 0xF8, 0x9F, 0x07, 0x00, 0x00, 0x3E, 0x7E, 0x00, 
  0xFC, 0x8F, 0x0F, 0x00, 0x00, 0x3E, 0xFF, 0x01, 0xFE, 0x87, 0x0F, 0x00, 
  0x00, 0x3E, 0xFF, 0x07, 0xFF, 0x83, 0x0F, 0x00, 0x00, 0x3E, 0xFF, 0x9F, 
  0xFF, 0x81, 0x0F, 0x00, 0x00, 0x3E, 0xFE, 0xFF, 0xFF, 0x80, 0x0F, 0x00, 
  0x00, 0x7C, 0xF8, 0xFF, 0x7F, 0xC0, 0x07, 0x00, 0x00, 0x7C, 0xC0, 0xFF, 
  0x1F, 0xC0, 0x07, 0x00, 0x00, 0xF8, 0x00, 0xFF, 0x0F, 0xE0, 0x03, 0x00, 
  0x00, 0xF8, 0x01, 0xFC, 0x07, 0xF0, 0x03, 0x00, 0x00, 0xF0, 0x03, 0xF0, 
  0x03, 0xF8, 0x01, 0x00, 0x00, 0xC0, 0x0F, 0x00, 0x00, 0x7E, 0x00, 0x00, 
  0x00, 0x80, 0x7F, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x07, 
  0xFC, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0x03, 0x00, 0x00, 
  0x00, 0x00, 0xC0, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
 
  };

#define noGood_width 60
#define noGood_height 36
const unsigned char noGood_bits [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0x3F, 0x00, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
  0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0xFC, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0x03, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 
  0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0xFE, 0x1F, 0x80, 0xFF, 
  0x1F, 0x80, 0xFF, 0x07, 0xFE, 0x1F, 0x00, 0xFF, 0x0F, 0x80, 0xFF, 0x07, 
  0xFE, 0x1F, 0x00, 0xFE, 0x07, 0xC0, 0xFF, 0x07, 0xFE, 0x3F, 0x00, 0xFC, 
  0x03, 0xE0, 0xFF, 0x07, 0xFE, 0x7F, 0x00, 0xF8, 0x01, 0xF0, 0xFF, 0x07, 
  0xFE, 0xFF, 0x00, 0xF0, 0x00, 0xF8, 0xFF, 0x07, 0xFE, 0xFF, 0x01, 0x60, 
  0x00, 0xFC, 0xFF, 0x07, 0xFE, 0xFF, 0x03, 0x00, 0x00, 0xFC, 0xFF, 0x07, 
  0xFE, 0xFF, 0x07, 0x00, 0x00, 0xFE, 0xFF, 0x07, 0xFE, 0xFF, 0x0F, 0x00, 
  0x00, 0xFF, 0xFF, 0x07, 0xFE, 0xFF, 0x1F, 0x00, 0x80, 0xFF, 0xFF, 0x07, 
  0xFE, 0xFF, 0x1F, 0x00, 0x80, 0xFF, 0xFF, 0x07, 0xFE, 0xFF, 0x0F, 0x00, 
  0x00, 0xFF, 0xFF, 0x07, 0xFE, 0xFF, 0x07, 0x00, 0x00, 0xFE, 0xFF, 0x07, 
  0xFE, 0xFF, 0x03, 0x00, 0x00, 0xFC, 0xFF, 0x07, 0xFE, 0xFF, 0x03, 0x60, 
  0x00, 0xF8, 0xFF, 0x07, 0xFE, 0xFF, 0x01, 0xF0, 0x00, 0xF8, 0xFF, 0x07, 
  0xFE, 0xFF, 0x00, 0xF8, 0x01, 0xF0, 0xFF, 0x07, 0xFE, 0x7F, 0x00, 0xFC, 
  0x03, 0xE0, 0xFF, 0x07, 0xFE, 0x3F, 0x00, 0xFE, 0x07, 0xC0, 0xFF, 0x07, 
  0xFE, 0x1F, 0x00, 0xFF, 0x0F, 0x80, 0xFF, 0x07, 0xFE, 0x1F, 0x80, 0xFF, 
  0x1F, 0xC0, 0xFF, 0x07, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 
  0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0xFC, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0x03, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 
  0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
 
  };