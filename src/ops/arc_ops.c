#include "ops/arc_ops.h"

#include "monome.h"
#include "helpers.h"
#include "teletype.h"
#include "teletype_io.h"
#include "euclidean/euclidean.h"

#define ARC_TRIGGER_TIME 60

#define ARC_BRIGHTNESS_ON 13
#define ARC_BRIGHTNESS_ON2 7
#define ARC_BRIGHTNESS_OFF 0
#define ARC_BRIGHTNESS_DIM 4


#define SA ss->arc

#define CLIP(var, min, max) \
    if (var < (min)) { \
      var = (min); \
    } else if (var > 254) { \
      var = 0; \
    } else if (var > (max)) { \
      var = (max); \
    }

static void arc_reset_(scene_state_t *ss){
      SA.encoder[0].cycle_step = 0;
      SA.encoder[1].cycle_step = 0;
      SA.encoder[2].cycle_step = 0;
      SA.encoder[3].cycle_step = 0;
      SA.reset = false;
}



// clang-format off

static void op_ARC_MODE_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_MODE_set  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_SYNC_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_SYNC_set  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_VAL_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_RST_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_LEN_set  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_LEN_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_PHASE_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_PHASE_set  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_STEP_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_STEPN_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);


const tele_op_t op_ARC_MODE   = MAKE_GET_SET_OP(ARC.MODE, op_ARC_MODE_get, op_ARC_MODE_set, 1, true);
const tele_op_t op_ARC_SYNC   = MAKE_GET_SET_OP(ARC.SYNC, op_ARC_SYNC_get, op_ARC_SYNC_set, 0, true);
const tele_op_t op_ARC_RST    = MAKE_GET_OP(ARC.RST, op_ARC_RST_get, 0, true);
const tele_op_t op_ARC_VAL    = MAKE_GET_OP(ARC.VAL, op_ARC_VAL_get, 1, true);
const tele_op_t op_ARC_LEN    = MAKE_GET_SET_OP(ARC.LEN, op_ARC_LEN_get, op_ARC_LEN_set, 1,  true);
const tele_op_t op_ARC_PHASE  = MAKE_GET_SET_OP(ARC.PHASE, op_ARC_PHASE_get, op_ARC_PHASE_set, 1, true);
const tele_op_t op_ARC_STEP   = MAKE_GET_OP(ARC.STEP, op_ARC_STEP_get, 0, false);
const tele_op_t op_ARC_STEPN   = MAKE_GET_OP(ARC.STEPN, op_ARC_STEPN_get, 1, false);

// clang-format on

static void op_ARC_MODE_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
 int enc = cs_pop(cs);
 if(enc<0 || enc>3)return;
 cs_push(cs, SA.encoder[enc].mode);
}
static void op_ARC_MODE_set(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
 int enc = cs_pop(cs);
 int mode = cs_pop(cs);

 if(enc<0 || enc>3)return;

 CLIP( mode , 0 , ARC_MODE_LAST-1);
 SA.encoder[enc].mode = mode;

}

static void op_ARC_SYNC_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
   int i = SA.sync;
    cs_push(cs, i);
}
static void op_ARC_SYNC_set(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
  int i = cs_pop(cs);
  if(i==0)  SA.sync = false;
  if(i==1)  SA.sync = true;
}


static void op_ARC_RST_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
   SA.reset = true;
}


static void op_ARC_VAL_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
 int enc = cs_pop(cs);

 if(enc<0 || enc>3)return;

 int i = SA.encoder[enc].value;
 cs_push(cs, i);


}



static void op_ARC_LEN_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {

  int enc = cs_pop(cs);
  if(enc<0 || enc>3)return;

  cs_push(cs, SA.encoder[enc].length);

}

static void op_ARC_LEN_set(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
  int enc = cs_pop(cs);
  int length = cs_pop(cs);

  if(enc<0 || enc>3)return;
  if(length<1 || length>32)return;

  SA.encoder[enc].length = length;
  CLIP( SA.encoder[enc].value, 0 , SA.encoder[enc].length);

}

static void op_ARC_PHASE_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
   int enc = cs_pop(cs);
   if(enc<0 || enc>3)return;

   cs_push(cs, SA.encoder[enc].phase_offset);

}

static void op_ARC_PHASE_set(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
  int enc = cs_pop(cs);
  int phase = cs_pop(cs);

  if(enc<0 || enc>3)return;

	CLIP( phase , 0 , SA.encoder[enc].length-1);

  SA.encoder[enc].phase_offset = phase;

}




static void op_ARC_STEP_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs){
  if( SA.encoder[0].value<0
    ||SA.encoder[1].value<0
    ||SA.encoder[2].value<0
    ||SA.encoder[3].value<0)return;

  //identify enc with max length
  int max_enc = 0;
  for(u8 enc=0; enc< monome_encs();enc++){
     if (SA.encoder[enc].mode == ARC_EUCL_FILL && SA.encoder[enc].length>SA.encoder[max_enc].length)
       max_enc = enc;
  }

  if((SA.encoder[max_enc].cycle_step == 0) || SA.reset){
    arc_reset_(ss); // sync all
    }


  for(u8 enc=0; enc< monome_encs();enc++){
    if(SA.encoder[enc].mode == ARC_EUCL_FILL){
    	u8 fill =SA.encoder[enc].value ;
    	s8 step =SA.encoder[enc].cycle_step - SA.encoder[enc].phase_offset;
  	  u8 out = euclidean(fill, SA.encoder[enc].length,step);
	    if(out){
  	  	tele_tr(enc,1);
		    ss->tr_pulse_timer[enc]=ARC_TRIGGER_TIME;
    	}
      SA.encoder[enc].next_step = true;
    }
  }


   SA.dirty = true;
}


static void op_ARC_STEPN_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
 int enc = cs_pop(cs);
 if(enc<0 || enc>3 || enc >= monome_encs())return;



  if(SA.reset) arc_reset_(ss);

  if( SA.encoder[0].value<0
    ||SA.encoder[1].value<0
    ||SA.encoder[2].value<0
    ||SA.encoder[3].value<0)return;

	u8 fill =SA.encoder[enc].value ;
	s8 step =SA.encoder[enc].cycle_step - SA.encoder[enc].phase_offset;
	u8 out = euclidean(fill, SA.encoder[enc].length,step);
	if(out){
  		tele_tr(enc,1);
		   ss->tr_pulse_timer[enc]=ARC_TRIGGER_TIME;
    	}

 SA.encoder[enc].next_step = true;
 SA.dirty = true;
}
