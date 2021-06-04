#ifndef _ARC_H_
#define _ARC_H_

#include "monome.h"
#include "state.h"

#define SA ss->arc

extern void arc_init(scene_state_t *ss);
extern void arc_refresh(scene_state_t *ss);
extern void arc_process_enc(scene_state_t *ss, u8 enc, s8 delta);


#endif
