#ifndef HAPD_KAY_H
#define HAPD_KAY_H


void * ieee802_1x_create_hapd_mka(struct hostapd_data *hapd, u8 *cak, u8 *ckn);

void ieee802_1x_dealloc_hapd_mka(struct hostapd_data *hapd);


#endif
