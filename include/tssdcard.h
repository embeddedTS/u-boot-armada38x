#ifndef _TSSDCARD_H_
#define _TSSDCARD_H_

/* Set up memory and callbacks for SD controller */
int tssdcard_init(int luns, unsigned char *base);
/* Actually talk to SD card and verify its presence */
void tssdcard_probe(int lun);
uint32_t tssdcard_get_sectors(int dev, int force_rescan);
#endif
