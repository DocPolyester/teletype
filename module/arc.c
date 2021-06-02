#include "arc.h"
#include "edit_mode.h"
#include "flash.h"
#include "font.h"
#include "globals.h"
#include "live_mode.h"
#include "pattern_mode.h"
#include "preset_r_mode.h"
#include "state.h"
#include "teletype.h"
#include "teletype_io.h"
#include "timers.h"
#include "util.h"
#include "print_funcs.h"
#include "euclidean/euclidean.h"

#define ARC_TRIGGER_TIME 60
#define ARC_MAX_PITCH 48



#define ARC_BRIGHTNESS_ON 14
#define ARC_BRIGHTNESS_ON2 10
#define ARC_BRIGHTNESS_OFF 0
#define ARC_BRIGHTNESS_DIM 3

// sensitiviy of arc clone , original arc must be larger
#define ARC_SENSITIVITY 4
#define ARC_SENSITIVITY_CONF 16
static s16 delta_buffer = 0;


#define CLIP_MAX_ROLL(var,max) if (var > max) var = 0;
#define CLIP_U16(var, min, max) \
  if (var < (min)) { \
    var = (min); \
  } else if (var > 65530) { \
    var = 0; \
  } else if (var > (max)) { \
    var = (max); \
  }
#define CLIP_U8(var, min, max) \
    if (var < (min)) { \
      var = (min); \
    } else if (var > 253) { \
      var = 0; \
    } else if (var > (max)) { \
      var = (max); \
    }


u8 DIM_LEVEL[8] = {3,4,5,6,8,10,12,14};


typedef struct {
    u8 on;
    scene_state_t *ss;
    softTimer_t timer;
} arc_timer_t;


typedef struct {
    u8 length;
    u8 shift;
} euclidean_lengths_t;


void arc_refresh(scene_state_t *ss);
void arc_process_enc(scene_state_t *ss, u8 enc, s8 delta);
void arc_process_key(scene_state_t *ss, u8 enc);

void arc_reset(scene_state_t *ss);


void arc_refresh_eucl(scene_state_t *ss, u8 enc);
void arc_refresh_pitch(scene_state_t *ss, u8 enc);
void arc_refresh_maxval(scene_state_t *ss, u8 enc);
void arc_refresh_minval(scene_state_t *ss, u8 enc);

void arc_process_enc_eucl(scene_state_t *ss, u8 enc);
void arc_process_enc_eucl_fill(scene_state_t *ss, u8 enc, s8 delta);
void arc_process_enc_eucl_length(scene_state_t *ss, u8 enc, s8 delta);
void arc_process_enc_eucl_phase(scene_state_t *ss, u8 enc, s8 delta);
void arc_process_enc_pitch(scene_state_t *ss, u8 enc, s8 delta);
void arc_process_enc_val(scene_state_t *ss, u8 enc, s8 delta);

void arc_process_key_eucl(scene_state_t *ss, u8 enc);



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void arc_refresh(scene_state_t *ss){
  for(u8 enc = 0; enc < monome_encs();enc++){
   switch(SA.encoder[enc].mode){
    case ARC_PITCH:
      arc_refresh_pitch(ss,enc);
      break;
    case ARC_MINVAL:
     arc_refresh_minval(ss,enc);
     break;
    case ARC_MAXVAL:
      arc_refresh_maxval(ss,enc);
     break;
    case ARC_EUCL_LENGTH:
    case ARC_EUCL_PHASE:
    case ARC_EUCL_FILL:
    default:
     arc_refresh_eucl(ss,enc);
     break;
   }
 }
 SA.dirty = 0;
}


void arc_process_enc(scene_state_t *ss, u8 enc, s8 delta){
  if(enc >= monome_encs()) return;
   switch(SA.encoder[enc].mode){
     case ARC_EUCL_LENGTH:
      arc_process_enc_eucl_length(ss, enc, delta);
      break;
     case ARC_EUCL_PHASE:
      arc_process_enc_eucl_phase(ss, enc, delta);
      break;
     case ARC_PITCH:
       arc_process_enc_pitch(ss, enc, delta);
       break;
     case ARC_MINVAL:
     case ARC_MAXVAL:
       arc_process_enc_val(ss, enc, delta);
       break;
     case ARC_EUCL_FILL:
     default:
       arc_process_enc_eucl_fill(ss, enc, delta);
       break;
     }

 }



void arc_process_key(scene_state_t *ss, u8 enc){
    if(enc >= monome_encs()) return;
    switch(SA.encoder[enc].mode){
      case ARC_EUCL_LENGTH:
      case ARC_EUCL_PHASE:
      case ARC_EUCL_FILL:
      default:
        arc_process_key_eucl(ss,enc);
        break;
    }

}




void arc_reset(scene_state_t *ss){
  SA.encoder[0].cycle_step = 0;
  SA.encoder[1].cycle_step = 0;
  SA.encoder[2].cycle_step = 0;
  SA.encoder[3].cycle_step = 0;
  SA.reset = false;
}



void arc_process_enc_eucl_fill(scene_state_t *ss, u8 enc, s8 delta) {
 delta_buffer += delta;

 if(delta_buffer>ARC_SENSITIVITY||delta_buffer<(-ARC_SENSITIVITY)){
  delta_buffer=0;
  SA.encoder[enc].value += (delta>0)?1:-1;

  CLIP_U16( SA.encoder[enc].value , 0 , SA.encoder[enc].length );

  arc_process_enc_eucl(ss,enc);
 }
}


void arc_process_enc_eucl(scene_state_t *ss, u8 enc) {
  u8 phase_offset_ = SA.encoder[enc].phase_offset;
  u8 length = SA.encoder[enc].length;

/*
  print_dbg("\r\nValue");
  print_dbg_hex(SA.encoder[enc].value);
  print_dbg("\r\nLength");
  print_dbg_hex(length);
*/

 for(u8 i=0;i<length<<1;i++){
		int eucl =euclidean(SA.encoder[enc].value, length,(i >>1) - phase_offset_);
 		SA.leds[enc][i] = eucl>0?ARC_BRIGHTNESS_ON:ARC_BRIGHTNESS_DIM;
 		SA.leds_layer2[enc][i] = eucl>0?ARC_BRIGHTNESS_ON2:ARC_BRIGHTNESS_DIM;
 }
 for(u8 i=length<<1;i<64;i++){
 		SA.leds[enc][i] = ARC_BRIGHTNESS_OFF;
 		SA.leds_layer2[enc][i] = ARC_BRIGHTNESS_OFF;
 }
  SA.dirty = 1;
}




void arc_process_enc_eucl_length(scene_state_t *ss, u8 enc, s8 delta) {
 delta_buffer += delta;

 if(delta_buffer>ARC_SENSITIVITY_CONF||(-delta_buffer)>ARC_SENSITIVITY_CONF){
  delta_buffer=0;
  SA.encoder[enc].length += (delta>0)?1:-1;

  CLIP_U8( SA.encoder[enc].length , 0 , 32);
  CLIP_U16( SA.encoder[enc].value , 0 , SA.encoder[enc].length );

  }
     arc_process_enc_eucl(ss,enc);
}


void arc_process_enc_eucl_phase(scene_state_t *ss, u8 enc, s8 delta) {
  delta_buffer += delta;

  if(delta_buffer>ARC_SENSITIVITY_CONF||delta_buffer<(-ARC_SENSITIVITY_CONF)){
    delta_buffer=0;
    SA.encoder[enc].phase_offset += (delta>0)?1:-1;

  	CLIP_U8( SA.encoder[enc].phase_offset , 0 , SA.encoder[enc].length-1);

  }
  arc_process_enc_eucl(ss,enc);
}

void arc_refresh_eucl(scene_state_t *ss, u8 enc) {

      for(u8 i=0;i<64;i++){
 	      if((i>>1) != (SA.encoder[enc].cycle_step)){
              	monomeLedBuffer[i + (enc << 6)] = SA.leds[enc][i];
 	      }else{
              	monomeLedBuffer[i + (enc << 6)] = SA.leds_layer2[enc][i];
 	      }
      }
     	   monomeFrameDirty |= (1 << enc);

   if(SA.encoder[enc].next_step){
     SA.encoder[enc].cycle_step++;
     SA.encoder[enc].next_step = false;
   }
   CLIP_MAX_ROLL( SA.encoder[enc].cycle_step , SA.encoder[enc].length-1);

}


/////////////////////// EUCL CONFIG ///////////////////////////

void arc_process_key_eucl(scene_state_t *ss, u8 enc) {

switch(SA.encoder[enc].mode){
  case ARC_EUCL_LENGTH:
  SA.encoder[enc].mode = ARC_EUCL_PHASE;
    break;
  case ARC_EUCL_PHASE:
  SA.encoder[enc].mode = ARC_EUCL_FILL;
    break;
  case ARC_EUCL_FILL:
  default:
    SA.encoder[enc].mode = ARC_EUCL_LENGTH;
    break;
  }
delta_buffer=0;
SA.dirty = 1;
}



////////////////////////////////////////////////////////////////
/////////////////////////   PITCH  /////////////////////////////
////////////////////////////////////////////////////////////////



void arc_process_enc_pitch(scene_state_t *ss, u8 enc, s8 delta) {
 delta_buffer += delta;

 if(delta_buffer>ARC_SENSITIVITY||delta_buffer<(-ARC_SENSITIVITY)){
  delta_buffer=0;
  SA.encoder[enc].value += (delta>0)?1:-1;

  CLIP_U16( SA.encoder[enc].value , 0 , ARC_MAX_PITCH );

  SA.dirty = 1;
 }
}


void arc_refresh_pitch(scene_state_t *ss, u8 enc) {

  for(u8 i2=0;i2<48;i2++)
				monomeLedBuffer[enc*64 + ((32 + i2) & 0x3f)] = ARC_BRIGHTNESS_DIM;
	monomeLedBuffer[enc*64 + ((32 + 0) & 0x3f)] = ARC_BRIGHTNESS_ON2;
	monomeLedBuffer[enc*64 + ((32 + 12) & 0x3f)] = ARC_BRIGHTNESS_ON2;
	monomeLedBuffer[enc*64 + ((32 + 24) & 0x3f)] = ARC_BRIGHTNESS_ON2;
	monomeLedBuffer[enc*64 + ((32 + 36) & 0x3f)] = ARC_BRIGHTNESS_ON2;
	monomeLedBuffer[enc*64 + ((32 + 48) & 0x3f)] = ARC_BRIGHTNESS_ON2;
	// show note
	monomeLedBuffer[enc*64 + ((32 + SA.encoder[enc].value) & 0x3f)] = ARC_BRIGHTNESS_ON;

  monomeFrameDirty |= (1 << enc);
}



////////////////////////////////////////////////////////////////
/////////////////////////   MAX/MINVAL  ////////////////////////
////////////////////////////////////////////////////////////////


void arc_process_enc_val(scene_state_t *ss, u8 enc, s8 delta) {
 delta_buffer += delta;

 if(delta_buffer>ARC_SENSITIVITY||delta_buffer<(-ARC_SENSITIVITY)){
  delta_buffer=0;
  SA.encoder[enc].value += (delta>0)?1:-1;

  CLIP_U16( SA.encoder[enc].value , 0 , 64 );

  SA.dirty = 1;
 }
}

////////////////////////////////////////////////////////////////
/////////////////////////   MAXVAL  /////////////////////////////
////////////////////////////////////////////////////////////////


void arc_refresh_maxval(scene_state_t *ss, u8 enc) {

  CLIP_U16( SA.encoder[enc].value , 0 , 64 );

  for(u8 i=0;i<SA.encoder[enc].value;i++)
      monomeLedBuffer[i + (enc << 6)] = DIM_LEVEL[i >> 3];

  for(u8 i=SA.encoder[enc].value;i<64;i++)
          monomeLedBuffer[i + (enc << 6)] = 0;


  monomeFrameDirty |= (1 << enc);

}


////////////////////////////////////////////////////////////////
/////////////////////////   MINVAL  /////////////////////////////
////////////////////////////////////////////////////////////////


void arc_refresh_minval(scene_state_t *ss, u8 enc) {

  CLIP_U16( SA.encoder[enc].value , 0 , 64 );

  for(s16 i=63;i>=SA.encoder[enc].value;i--)
      monomeLedBuffer[i + (enc << 6)] = DIM_LEVEL[i >> 3];

  for(u8 i=0;i<SA.encoder[enc].value;i++)
          monomeLedBuffer[i + (enc << 6)] = 0;


  monomeFrameDirty |= (1 << enc);

}
