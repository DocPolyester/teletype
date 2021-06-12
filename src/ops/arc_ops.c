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

static void op_ARC_MO_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_MO_set  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_SYN_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_SYN_set  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_VAL_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_VAL_set  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_RST_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_LEN_set  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_LEN_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_PHA_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_PHA_set  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_STP_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);
static void op_ARC_SCR_get  (const void *data, scene_state_t *ss, exec_state_t *es,  command_state_t *cs);

const tele_op_t op_ARC_MO   = MAKE_GET_SET_OP(ARC.MO, op_ARC_MO_get, op_ARC_MO_set, 1, true);
const tele_op_t op_ARC_SYN  = MAKE_GET_SET_OP(ARC.SYN, op_ARC_SYN_get, op_ARC_SYN_set, 0, true);
const tele_op_t op_ARC_RST  = MAKE_GET_OP(ARC.RST, op_ARC_RST_get, 0, true);
const tele_op_t op_ARC_VAL  = MAKE_GET_SET_OP(ARC.VAL, op_ARC_VAL_get,op_ARC_VAL_set, 1, true);
const tele_op_t op_ARC_LEN  = MAKE_GET_SET_OP(ARC.LEN, op_ARC_LEN_get, op_ARC_LEN_set, 1,  true);
const tele_op_t op_ARC_PHA  = MAKE_GET_SET_OP(ARC.PHA, op_ARC_PHA_get, op_ARC_PHA_set, 1, true);
const tele_op_t op_ARC_STP  = MAKE_GET_OP(ARC.STP, op_ARC_STP_get, 1, false);
const tele_op_t op_ARC_SCR  = MAKE_GET_OP(ARC.SCR, op_ARC_SCR_get, 2, false);


// clang-format on

static void op_ARC_MO_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
 int enc = cs_pop(cs);
if(enc >=1 || enc<=4){
    cs_push(cs, SA.encoder[enc-1].mode);
 }
}

static void op_ARC_MO_set(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
 int enc = cs_pop(cs);
 int mode = cs_pop(cs);

 if(enc==0){
   for(u8 i=0; i< monome_encs();i++){
     CLIP( mode , 0 , ARC_MODE_LAST-1);
     SA.encoder[i].mode = mode;
   }
 } else if(enc >=1 || enc<=4){
    CLIP( mode , 0 , ARC_MODE_LAST-1);
    SA.encoder[enc-1].mode = mode;
 }

}



static void op_ARC_SYN_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
   int i = SA.sync;
    cs_push(cs, i);
}
static void op_ARC_SYN_set(const void *NOTUSED(data), scene_state_t *ss,
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

 if(enc >=1 || enc<=4){
   int i = SA.encoder[enc-1].value;
   cs_push(cs, i);
 }
}

static void op_ARC_VAL_set(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
 int enc = cs_pop(cs);
 int value = cs_pop(cs);

 if(enc==0){
   SA.encoder[0].value = value;
   SA.encoder[1].value = value;
   SA.encoder[2].value = value;
   SA.encoder[3].value = value;
   SA.dirty = true;
 } else if(enc >=1 || enc<=4){
   SA.encoder[enc-1].value = value;
   SA.dirty = true;
 }
}


static void op_ARC_SCR_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
 int enc = cs_pop(cs);
 int script = cs_pop(cs);

 if(enc==0){
   for(u8 i=0; i< monome_encs();i++){
     if(script>0 && script<9){
       SA.encoder[i].connected_script = script-1;
     }
     if(script==0){
       // switch off (suppress) execution
       SA.encoder[i].connected_script = 64;
     }
     SA.dirty = true;
   }
 } else if(enc >=1 || enc<=4){
   if(script>0 && script<9){
     SA.encoder[enc-1].connected_script = script-1;
   }
   if(script==0){
     // switch off (suppress) execution
     SA.encoder[enc-1].connected_script = 64;
   }
   SA.dirty = true;
 }
}


static void op_ARC_LEN_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {

  int enc = cs_pop(cs);
  if(enc >=1 || enc<=4){
       cs_push(cs, SA.encoder[enc-1].length);
  }

}

static void op_ARC_LEN_set(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
  int enc = cs_pop(cs);
  int length = cs_pop(cs);

  if(length<1 || length>32)return;

  if(enc==0){
    for(u8 i=0; i< monome_encs();i++){
      SA.encoder[i].length = length;
      CLIP( SA.encoder[i].value, 0 , SA.encoder[i].length);
      SA.dirty = true;
    }
  } else if(enc >=1 || enc<=4){
    SA.encoder[enc-1].length = length;
    CLIP( SA.encoder[enc-1].value, 0 , SA.encoder[enc-1].length);
    SA.dirty = true;
  }
}

static void op_ARC_PHA_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
   int enc = cs_pop(cs);
   if(enc >=1 || enc<=4){
    cs_push(cs, SA.encoder[enc-1].phase_offset);
   }
}

static void op_ARC_PHA_set(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
  int enc = cs_pop(cs);
  int phase = cs_pop(cs);

  if(enc==0){
    for(u8 i=0; i< monome_encs();i++){
      CLIP( phase , 0 , SA.encoder[i].length-1);
      SA.encoder[i].phase_offset = phase;
      SA.dirty = true;
    }
  } else if(enc >=1 || enc<=4){
    CLIP( phase , 0 , SA.encoder[enc-1].length-1);
    SA.encoder[enc-1].phase_offset = phase;
    SA.dirty = true;
  }


}





static void op_ARC_STP_get(const void *NOTUSED(data), scene_state_t *ss,
                           exec_state_t *NOTUSED(es), command_state_t *cs) {
 int enc = cs_pop(cs);

 if(enc==0){
   if( SA.encoder[0].value<0
     ||SA.encoder[1].value<0
     ||SA.encoder[2].value<0
     ||SA.encoder[3].value<0)return;

   //identify enc with max length
   int max_enc = 0;
   for(u8 i=0; i< monome_encs();i++){
      if ((SA.encoder[i].mode == ARC_EUCL_FILL ||
           SA.encoder[i].mode == ARC_EUCL_PHASE ||
           SA.encoder[i].mode == ARC_EUCL_LENGTH) &&
           SA.encoder[i].length>SA.encoder[max_enc].length)
        max_enc = i;
   }

   if((SA.encoder[max_enc].cycle_step == 0) || SA.reset){
     arc_reset_(ss); // sync all
     }


   for(u8 i=0; i< monome_encs();i++){
     if(SA.encoder[i].mode == ARC_EUCL_FILL ||
          SA.encoder[i].mode == ARC_EUCL_PHASE ||
          SA.encoder[i].mode == ARC_EUCL_LENGTH){
     	u8 fill =SA.encoder[i].value ;
     	s8 step =SA.encoder[i].cycle_step - SA.encoder[i].phase_offset;
   	  u8 out = euclidean(fill, SA.encoder[i].length,step);
 	    if(out){
   	  	tele_tr(i,1);
 		    ss->tr_pulse_timer[i]=ARC_TRIGGER_TIME;
         if(SA.encoder[i].connected_script<8)
           run_script(ss,SA.encoder[i].connected_script);
     	}
       SA.encoder[i].next_step = true;
     }
   }
  SA.dirty = true;

 ///// Single ARC Step
 } else if(enc >=1 || enc<=4){

   if( SA.encoder[enc-1].value)return;
   if(SA.encoder[enc-1].mode != ARC_EUCL_FILL &&
      SA.encoder[enc-1].mode != ARC_EUCL_PHASE &&
      SA.encoder[enc-1].mode == ARC_EUCL_LENGTH) return;
   if(SA.reset) arc_reset_(ss);

 	u8 fill =SA.encoder[enc-1].value ;
 	s8 step =SA.encoder[enc-1].cycle_step - SA.encoder[enc-1].phase_offset;
 	u8 out = euclidean(fill, SA.encoder[enc-1].length,step);
 	if(out){
   		tele_tr(enc-1,1);
 		   ss->tr_pulse_timer[enc-1]=ARC_TRIGGER_TIME;
        if(SA.encoder[enc-1].connected_script<8)
          run_script(ss,SA.encoder[enc-1].connected_script);
     	}

  SA.encoder[enc-1].next_step = true;
  SA.dirty = true;
 }
}
