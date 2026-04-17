void R_InitSprites (char** namelist);
void R_ClearSprites (void);
void R_DrawMasked (void);

void
R_ClipVisSprite
( vissprite_t* vis,
  int xl,
  int xh );
# 46 "./DOOM/linuxdoom-1.10/r_local.h" 2
# 1 "./DOOM/linuxdoom-1.10/r_draw.h" 1
# 32 "./DOOM/linuxdoom-1.10/r_draw.h"
extern lighttable_t* dc_colormap;
extern int dc_x;
extern int dc_yl;
extern int dc_yh;
extern fixed_t dc_iscale;
extern fixed_t dc_texturemid;


extern byte* dc_source;





void R_DrawColumn (void);
void R_DrawColumnLow (void);


void R_DrawFuzzColumn (void);
void R_DrawFuzzColumnLow (void);




void R_DrawTranslatedColumn (void);
void R_DrawTranslatedColumnLow (void);

void
R_VideoErase
( unsigned ofs,
  int count );

extern int ds_y;
extern int ds_x1;
extern int ds_x2;

extern lighttable_t* ds_colormap;

extern fixed_t ds_xfrac;
extern fixed_t ds_yfrac;
extern fixed_t ds_xstep;
extern fixed_t ds_ystep;


extern byte* ds_source;

extern byte* translationtables;
extern byte* dc_translation;




void R_DrawSpan (void);


void R_DrawSpanLow (void);


void
R_InitBuffer
( int width,
  int height );




void R_InitTranslationTables (void);




void R_FillBackScreen (void);


void R_DrawViewBorder (void);
# 47 "./DOOM/linuxdoom-1.10/r_local.h" 2
# 28 "./DOOM/linuxdoom-1.10/p_local.h" 2
# 70 "./DOOM/linuxdoom-1.10/p_local.h"
extern thinker_t thinkercap;


void P_InitThinkers (void);
void P_AddThinker (thinker_t* thinker);
void P_RemoveThinker (thinker_t* thinker);





void P_SetupPsprites (player_t* curplayer);
void P_MovePsprites (player_t* curplayer);
void P_DropWeapon (player_t* player);





void P_PlayerThink (player_t* player);
# 101 "./DOOM/linuxdoom-1.10/p_local.h"
extern mapthing_t itemrespawnque[128];
extern int itemrespawntime[128];
extern int iquehead;
extern int iquetail;


void P_RespawnSpecials (void);

mobj_t*
P_SpawnMobj
( fixed_t x,
  fixed_t y,
  fixed_t z,
  mobjtype_t type );

void P_RemoveMobj (mobj_t* th);
boolean P_SetMobjState (mobj_t* mobj, statenum_t state);
void P_MobjThinker (mobj_t* mobj);

void P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z);
void P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage);
mobj_t* P_SpawnMissile (mobj_t* source, mobj_t* dest, mobjtype_t type);
void P_SpawnPlayerMissile (mobj_t* source, mobjtype_t type);





void P_NoiseAlert (mobj_t* target, mobj_t* emmiter);





typedef struct
{
    fixed_t x;
    fixed_t y;
    fixed_t dx;
    fixed_t dy;

} divline_t;

typedef struct
{
    fixed_t frac;
    boolean isaline;
    union {
 mobj_t* thing;
 line_t* line;
    } d;
} intercept_t;



extern intercept_t intercepts[128];
extern intercept_t* intercept_p;

typedef boolean (*traverser_t) (intercept_t *in);

fixed_t P_AproxDistance (fixed_t dx, fixed_t dy);
int P_PointOnLineSide (fixed_t x, fixed_t y, line_t* line);
int P_PointOnDivlineSide (fixed_t x, fixed_t y, divline_t* line);
void P_MakeDivline (line_t* li, divline_t* dl);
fixed_t P_InterceptVector (divline_t* v2, divline_t* v1);
int P_BoxOnLineSide (fixed_t* tmbox, line_t* ld);

extern fixed_t opentop;
extern fixed_t openbottom;
extern fixed_t openrange;
extern fixed_t lowfloor;

void P_LineOpening (line_t* linedef);

boolean P_BlockLinesIterator (int x, int y, boolean(*func)(line_t*) );
boolean P_BlockThingsIterator (int x, int y, boolean(*func)(mobj_t*) );





extern divline_t trace;

boolean
P_PathTraverse
( fixed_t x1,
  fixed_t y1,
  fixed_t x2,
  fixed_t y2,
  int flags,
