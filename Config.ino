#include <EEPROM.h>
#include <TrueRandom.h>

static char EEPROM_MAGIC[] = "OPN ";

void configGet(ArtNetConfig & config) {
  if(configCheckMagic()) {
    configRead(config);
  }
  else {
    //TrueRandom.mac(config.mac);
    configWriteMagic();
    configWrite(config);
  }
}

boolean configCheckMagic() {
  for(int i=0; i<4; i++) {
    if(EEPROM.read(i) != EEPROM_MAGIC[i])
      return false;
  }
  return true;
}

void configWriteMagic() {
  for(int i=0; i<4; i++)
    EEPROM.write(i, EEPROM_MAGIC[i]);
}

void configRead(ArtNetConfig & config) {
  byte *data = (byte*)&config;
  for(unsigned int i=0; i<sizeof(ArtNetConfig); i++)
    data[i] = EEPROM.read(4+i);
}

void configWrite(ArtNetConfig & config) {
  byte *data = (byte*)&config;
  for(unsigned int i=0; i<sizeof(ArtNetConfig); i++)
    EEPROM.write(4+i, data[i]);
}

