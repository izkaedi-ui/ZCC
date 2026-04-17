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
  boolean (*trav) (intercept_t *));

void P_UnsetThingPosition (mobj_t* thing);
void P_SetThingPosition (mobj_t* thing);
# 203 "./DOOM/linuxdoom-1.10/p_local.h"
extern boolean floatok;
extern fixed_t tmfloorz;
extern fixed_t tmceilingz;


extern line_t* ceilingline;

boolean P_CheckPosition (mobj_t *thing, fixed_t x, fixed_t y);
boolean P_TryMove (mobj_t* thing, fixed_t x, fixed_t y);
boolean P_TeleportMove (mobj_t* thing, fixed_t x, fixed_t y);
void P_SlideMove (mobj_t* mo);
boolean P_CheckSight (mobj_t* t1, mobj_t* t2);
void P_UseLines (player_t* player);

boolean P_ChangeSector (sector_t* sector, boolean crunch);

extern mobj_t* linetarget;

fixed_t
P_AimLineAttack
( mobj_t* t1,
  angle_t angle,
  fixed_t distance );

void
P_LineAttack
( mobj_t* t1,
  angle_t angle,
  fixed_t distance,
  fixed_t slope,
  int damage );

void
P_RadiusAttack
( mobj_t* spot,
  mobj_t* source,
  int damage );






extern byte* rejectmatrix;
extern short* blockmaplump;
extern short* blockmap;
extern int bmapwidth;
extern int bmapheight;
extern fixed_t bmaporgx;
extern fixed_t bmaporgy;
extern mobj_t** blocklinks;






extern int maxammo[NUMAMMO];
extern int clipammo[NUMAMMO];

void
P_TouchSpecialThing
( mobj_t* special,
  mobj_t* toucher );

void
P_DamageMobj
( mobj_t* target,
  mobj_t* inflictor,
  mobj_t* source,
  int damage );





# 1 "./DOOM/linuxdoom-1.10/p_spec.h" 1
# 33 "./DOOM/linuxdoom-1.10/p_spec.h"
extern boolean levelTimer;
extern int levelTimeCount;







void P_InitPicAnims (void);


void P_SpawnSpecials (void);


void P_UpdateSpecials (void);


boolean
P_UseSpecialLine
( mobj_t* thing,
  line_t* line,
  int side );

void
P_ShootSpecialLine
( mobj_t* thing,
  line_t* line );

void
P_CrossSpecialLine
( int linenum,
  int side,
  mobj_t* thing );

void P_PlayerInSpecialSector (player_t* player);

int
twoSided
( int sector,
  int line );

sector_t*
getSector
( int currentSector,
  int line,
  int side );

side_t*
getSide
( int currentSector,
  int line,
  int side );

fixed_t P_FindLowestFloorSurrounding(sector_t* sec);
fixed_t P_FindHighestFloorSurrounding(sector_t* sec);

fixed_t
P_FindNextHighestFloor
( sector_t* sec,
  int currentheight );

fixed_t P_FindLowestCeilingSurrounding(sector_t* sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t* sec);

int
P_FindSectorFromLineTag
( line_t* line,
  int start );

int
P_FindMinSurroundingLight
( sector_t* sector,
  int max );

sector_t*
getNextSector
( line_t* line,
  sector_t* sec );





int EV_DoDonut(line_t* line);






typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int count;
    int maxlight;
    int minlight;

} fireflicker_t;



typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int count;
    int maxlight;
    int minlight;
    int maxtime;
    int mintime;

} lightflash_t;



typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int count;
    int minlight;
    int maxlight;
    int darktime;
    int brighttime;

} strobe_t;




typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int minlight;
    int maxlight;
    int direction;

} glow_t;







void P_SpawnFireFlicker (sector_t* sector);
void T_LightFlash (lightflash_t* flash);
void P_SpawnLightFlash (sector_t* sector);
void T_StrobeFlash (strobe_t* flash);

void
P_SpawnStrobeFlash
( sector_t* sector,
  int fastOrSlow,
  int inSync );

void EV_StartLightStrobing(line_t* line);
void EV_TurnTagLightsOff(line_t* line);

void
EV_LightTurnOn
( line_t* line,
  int bright );

void T_Glow(glow_t* g);
void P_SpawnGlowingLight(sector_t* sector);







typedef struct
{
    char name1[9];
    char name2[9];
    short episode;

} switchlist_t;


typedef enum
{
    top,
    middle,
    bottom

} bwhere_e;


typedef struct
{
    line_t* line;
    bwhere_e where;
    int btexture;
    int btimer;
    mobj_t* soundorg;

} button_t;
# 249 "./DOOM/linuxdoom-1.10/p_spec.h"
extern button_t buttonlist[16];

void
P_ChangeSwitchTexture
( line_t* line,
  int useAgain );

void P_InitSwitchList(void);





typedef enum
{
    up,
    down,
    waiting,
    in_stasis

} plat_e;



typedef enum
{
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS

} plattype_e;



typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    fixed_t speed;
    fixed_t low;
    fixed_t high;
    int wait;
    int count;
    plat_e status;
    plat_e oldstatus;
    boolean crush;
    int tag;
    plattype_e type;

} plat_t;
# 309 "./DOOM/linuxdoom-1.10/p_spec.h"
extern plat_t* activeplats[30];

void T_PlatRaise(plat_t* plat);

int
EV_DoPlat
( line_t* line,
  plattype_e type,
  int amount );

void P_AddActivePlat(plat_t* plat);
void P_RemoveActivePlat(plat_t* plat);
void EV_StopPlat(line_t* line);
void P_ActivateInStasis(int tag);





typedef enum
{
    normal,
    close30ThenOpen,
    close,
    open,
    raiseIn5Mins,
    blazeRaise,
    blazeOpen,
    blazeClose

} vldoor_e;



typedef struct
{
    thinker_t thinker;
    vldoor_e type;
    sector_t* sector;
    fixed_t topheight;
    fixed_t speed;


    int direction;


    int topwait;


    int topcountdown;

} vldoor_t;






void
EV_VerticalDoor
( line_t* line,
  mobj_t* thing );

int
EV_DoDoor
( line_t* line,
  vldoor_e type );

int
EV_DoLockedDoor
( line_t* line,
  vldoor_e type,
  mobj_t* thing );

void T_VerticalDoor (vldoor_t* door);
void P_SpawnDoorCloseIn30 (sector_t* sec);

void
P_SpawnDoorRaiseIn5Mins
( sector_t* sec,
  int secnum );
# 480 "./DOOM/linuxdoom-1.10/p_spec.h"
typedef enum
{
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise

} ceiling_e;



typedef struct
{
    thinker_t thinker;
    ceiling_e type;
    sector_t* sector;
    fixed_t bottomheight;
    fixed_t topheight;
    fixed_t speed;
    boolean crush;


    int direction;


    int tag;
    int olddirection;

} ceiling_t;
# 520 "./DOOM/linuxdoom-1.10/p_spec.h"
extern ceiling_t* activeceilings[30];

int
EV_DoCeiling
( line_t* line,
  ceiling_e type );

void T_MoveCeiling (ceiling_t* ceiling);
void P_AddActiveCeiling(ceiling_t* c);
void P_RemoveActiveCeiling(ceiling_t* c);
int EV_CeilingCrushStop(line_t* line);
void P_ActivateInStasisCeiling(line_t* line);





typedef enum
{

    lowerFloor,


    lowerFloorToLowest,


    turboLower,


    raiseFloor,


    raiseFloorToNearest,


    raiseToTexture,



    lowerAndChange,

    raiseFloor24,
    raiseFloor24AndChange,
    raiseFloorCrush,


    raiseFloorTurbo,
    donutRaise,
    raiseFloor512

} floor_e;




typedef enum
{
    build8,
    turbo16

} stair_e;



typedef struct
{
    thinker_t thinker;
    floor_e type;
    boolean crush;
    sector_t* sector;
    int direction;
    int newspecial;
    short texture;
    fixed_t floordestheight;
    fixed_t speed;

} floormove_t;





typedef enum
{
    ok,
    crushed,
    pastdest

} result_e;

result_e
T_MovePlane
( sector_t* sector,
  fixed_t speed,
  fixed_t dest,
  boolean crush,
  int floorOrCeiling,
  int direction );

int
EV_BuildStairs
( line_t* line,
  stair_e type );

int
EV_DoFloor
( line_t* line,
  floor_e floortype );

void T_MoveFloor( floormove_t* floor);




int
EV_Teleport
( line_t* line,
  int side,
  mobj_t* thing );
# 280 "./DOOM/linuxdoom-1.10/p_local.h" 2
# 33 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2
# 1 "./DOOM/linuxdoom-1.10/w_wad.h" 1
# 35 "./DOOM/linuxdoom-1.10/w_wad.h"
typedef struct
{

    char identification[4];
    int numlumps;
    int infotableofs;

} wadinfo_t;


typedef struct
{
    int filepos;
    int size;
    char name[8];

} filelump_t;




typedef struct
{
    char name[8];
    int handle;
    int position;
    int size;
} lumpinfo_t;


extern void** lumpcache;
extern lumpinfo_t* lumpinfo;
extern int numlumps;

void W_InitMultipleFiles (char** filenames);
void W_Reload (void);

int W_CheckNumForName (char* name);
int W_GetNumForName (char* name);

int W_LumpLength (int lump);
void W_ReadLump (int lump, void *dest);

void* W_CacheLumpNum (int lump, int tag);
void* W_CacheLumpName (char* name, int tag);
# 34 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2

# 1 "./DOOM/linuxdoom-1.10/m_cheat.h" 1
# 34 "./DOOM/linuxdoom-1.10/m_cheat.h"
typedef struct
{
    unsigned char* sequence;
    unsigned char* p;

} cheatseq_t;

int
cht_CheckCheat
( cheatseq_t* cht,
  char key );


void
cht_GetParam
( cheatseq_t* cht,
  char* buffer );
# 36 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2
# 1 "./DOOM/linuxdoom-1.10/i_system.h" 1
# 35 "./DOOM/linuxdoom-1.10/i_system.h"
void I_Init (void);




byte* I_ZoneBase (int *size);




int I_GetTime (void);
# 56 "./DOOM/linuxdoom-1.10/i_system.h"
void I_StartFrame (void);







void I_StartTic (void);
# 74 "./DOOM/linuxdoom-1.10/i_system.h"
ticcmd_t* I_BaseTiccmd (void);




void I_Quit (void);




byte* I_AllocLow (int length);

void I_Tactile (int on, int off, int total);


void I_Error (char *error, ...);
# 37 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2


# 1 "./DOOM/linuxdoom-1.10/v_video.h" 1
# 47 "./DOOM/linuxdoom-1.10/v_video.h"
extern byte* screens[5];

extern int dirtybox[4];

extern byte gammatable[5][256];
extern int usegamma;




void V_Init (void);


void
V_CopyRect
( int srcx,
  int srcy,
  int srcscrn,
  int width,
  int height,
  int destx,
  int desty,
  int destscrn );

void
V_DrawPatch
( int x,
  int y,
  int scrn,
  patch_t* patch);

void
V_DrawPatchDirect
( int x,
  int y,
  int scrn,
  patch_t* patch );



void
V_DrawBlock
( int x,
  int y,
  int scrn,
  int width,
  int height,
  byte* src );


void
V_GetBlock
( int x,
  int y,
  int scrn,
  int width,
  int height,
  byte* dest );


void
V_MarkRect
( int x,
  int y,
  int width,
  int height );
# 40 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2


# 1 "./DOOM/linuxdoom-1.10/doomstat.h" 1
# 34 "./DOOM/linuxdoom-1.10/doomstat.h"
# 1 "./DOOM/linuxdoom-1.10/d_net.h" 1
# 51 "./DOOM/linuxdoom-1.10/d_net.h"
typedef enum
{
    CMD_SEND = 1,
    CMD_GET = 2

} command_t;





typedef struct
{

    unsigned checksum;

    byte retransmitfrom;

    byte starttic;
    byte player;
    byte numtics;
    ticcmd_t cmds[12];

} doomdata_t;




typedef struct
{

    long id;


    short intnum;


    short command;

    short remotenode;


    short datalength;



    short numnodes;

    short ticdup;

    short extratics;

    short deathmatch;

    short savegame;
    short episode;
    short map;
    short skill;


    short consoleplayer;
    short numplayers;







    short angleoffset;

    short drone;


    doomdata_t data;

} doomcom_t;




void NetUpdate (void);



void D_QuitNetGame (void);


void TryRunTics (void);
# 35 "./DOOM/linuxdoom-1.10/doomstat.h" 2
# 49 "./DOOM/linuxdoom-1.10/doomstat.h"
extern boolean nomonsters;
extern boolean respawnparm;
extern boolean fastparm;

extern boolean devparm;






extern GameMode_t gamemode;
extern GameMission_t gamemission;


extern boolean modifiedgame;




extern Language_t language;







extern skill_t startskill;
extern int startepisode;
extern int startmap;

extern boolean autostart;


extern skill_t gameskill;
extern int gameepisode;
extern int gamemap;


extern boolean respawnmonsters;


extern boolean netgame;



extern boolean deathmatch;
# 109 "./DOOM/linuxdoom-1.10/doomstat.h"
extern int snd_SfxVolume;
extern int snd_MusicVolume;





extern int snd_MusicDevice;
extern int snd_SfxDevice;

extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;
# 130 "./DOOM/linuxdoom-1.10/doomstat.h"
extern boolean statusbaractive;

extern boolean automapactive;
extern boolean menuactive;
extern boolean paused;


extern boolean viewactive;

extern boolean nodrawers;
extern boolean noblit;

extern int viewwindowx;
extern int viewwindowy;
extern int viewheight;
extern int viewwidth;
extern int scaledviewwidth;
# 155 "./DOOM/linuxdoom-1.10/doomstat.h"
extern int viewangleoffset;


extern int consoleplayer;
extern int displayplayer;






extern int totalkills;
extern int totalitems;
extern int totalsecret;


extern int levelstarttic;
extern int leveltime;







extern boolean usergame;


extern boolean demoplayback;
extern boolean demorecording;


extern boolean singledemo;





extern gamestate_t gamestate;
# 208 "./DOOM/linuxdoom-1.10/doomstat.h"
extern int gametic;



extern player_t players[4];


extern boolean playeringame[4];




extern mapthing_t deathmatchstarts[10];
extern mapthing_t* deathmatch_p;


extern mapthing_t playerstarts[4];



extern wbstartstruct_t wminfo;




extern int maxammo[NUMAMMO];
# 244 "./DOOM/linuxdoom-1.10/doomstat.h"
extern char basedefault[1024];
extern FILE* debugfile;


extern boolean precache;




extern gamestate_t wipegamestate;

extern int mouseSensitivity;


extern boolean singletics;

extern int bodyqueslot;






extern int skyflatnum;






extern doomcom_t* doomcom;


extern doomdata_t* netbuffer;


extern ticcmd_t localcmds[12];
extern int rndindex;

extern int maketic;
extern int nettics[8];

extern ticcmd_t netcmds[4][12];
extern int ticdup;
# 43 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2



# 1 "./DOOM/linuxdoom-1.10/dstrings.h" 1
# 37 "./DOOM/linuxdoom-1.10/dstrings.h"
# 1 "./DOOM/linuxdoom-1.10/d_englsh.h" 1
# 38 "./DOOM/linuxdoom-1.10/dstrings.h" 2
# 58 "./DOOM/linuxdoom-1.10/dstrings.h"
extern char* endmsg[];
# 47 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2

# 1 "./DOOM/linuxdoom-1.10/am_map.h" 1
# 32 "./DOOM/linuxdoom-1.10/am_map.h"
boolean AM_Responder (event_t* ev);


void AM_Ticker (void);



void AM_Drawer (void);



void AM_Stop (void);
# 49 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2
# 128 "./DOOM/linuxdoom-1.10/doom_amalg.c"
typedef struct
{
    int x, y;
} fpoint_t;

typedef struct
{
    fpoint_t a, b;
} fline_t;

typedef struct
{
    fixed_t x,y;
} mpoint_t;

typedef struct
{
    mpoint_t a, b;
} mline_t;

typedef struct
{
    fixed_t slp, islp;
} islope_t;
# 161 "./DOOM/linuxdoom-1.10/doom_amalg.c"
mline_t player_arrow[] = {
    { { -((8*16*65536)/7)+((8*16*65536)/7)/8, 0 }, { ((8*16*65536)/7), 0 } },
    { { ((8*16*65536)/7), 0 }, { ((8*16*65536)/7)-((8*16*65536)/7)/2, 297808 } },
    { { ((8*16*65536)/7), 0 }, { ((8*16*65536)/7)-((8*16*65536)/7)/2, -297808 } },
    { { -((8*16*65536)/7)+((8*16*65536)/7)/8, 0 }, { -((8*16*65536)/7)-((8*16*65536)/7)/8, 297808 } },
    { { -((8*16*65536)/7)+((8*16*65536)/7)/8, 0 }, { -((8*16*65536)/7)-((8*16*65536)/7)/8, -297808 } },
    { { -((8*16*65536)/7)+3*((8*16*65536)/7)/8, 0 }, { -((8*16*65536)/7)+((8*16*65536)/7)/8, 297808 } },
    { { -((8*16*65536)/7)+3*((8*16*65536)/7)/8, 0 }, { -((8*16*65536)/7)+((8*16*65536)/7)/8, -297808 } }
};




mline_t cheat_player_arrow[] = {
    { { -((8*16*65536)/7)+((8*16*65536)/7)/8, 0 }, { ((8*16*65536)/7), 0 } },
    { { ((8*16*65536)/7), 0 }, { ((8*16*65536)/7)-((8*16*65536)/7)/2, 198539 } },
    { { ((8*16*65536)/7), 0 }, { ((8*16*65536)/7)-((8*16*65536)/7)/2, -198539 } },
    { { -((8*16*65536)/7)+((8*16*65536)/7)/8, 0 }, { -((8*16*65536)/7)-((8*16*65536)/7)/8, 198539 } },
    { { -((8*16*65536)/7)+((8*16*65536)/7)/8, 0 }, { -((8*16*65536)/7)-((8*16*65536)/7)/8, -198539 } },
    { { -((8*16*65536)/7)+3*((8*16*65536)/7)/8, 0 }, { -((8*16*65536)/7)+((8*16*65536)/7)/8, 198539 } },
    { { -((8*16*65536)/7)+3*((8*16*65536)/7)/8, 0 }, { -((8*16*65536)/7)+((8*16*65536)/7)/8, -198539 } },
    { { -((8*16*65536)/7)/2, 0 }, { -((8*16*65536)/7)/2, -198539 } },
    { { -((8*16*65536)/7)/2, -198539 }, { -56173, -198539 } },
    { { -56173, -198539 }, { -56173, 297808 } },
    { { -198539, 0 }, { -198539, -198539 } },
    { { -198539, -198539 }, { 0, -198539 } },
    { { 0, -198539 }, { 0, 297808 } },
    { { 198539, 297808 }, { 198539, -170176 } },
    { { 198539, -170176 }, { 235765, -207402 } },
    { { 235765, -207402 }, { 317662, -170176 } }
};




mline_t triangle_guy[] = {
    { { -56819, -32768 }, { 56819, -32768 } },
    { { 56819, -32768 } , { 0, 65536 } },
    { { 0, 65536 }, { -56819, -32768 } }
};




mline_t thintriangle_guy[] = {
    { { -32768, -45875 }, { 65536, 0 } },
    { { 65536, 0 }, { -32768, 45875 } },
    { { -32768, 45875 }, { -32768, -45875 } }
};






static int cheating = 0;
static int grid = 0;

static int leveljuststarted = 1;

boolean automapactive = false;
static int finit_width = 320;
static int finit_height = 200 - 32;


static int f_x;
static int f_y;


static int f_w;
static int f_h;

static int lightlev;
static byte* fb;
static int amclock;

static mpoint_t m_paninc;
static fixed_t mtof_zoommul;
static fixed_t ftom_zoommul;

static fixed_t m_x, m_y;
static fixed_t m_x2, m_y2;




static fixed_t m_w;
static fixed_t m_h;


static fixed_t min_x;
static fixed_t min_y;
static fixed_t max_x;
static fixed_t max_y;

static fixed_t max_w;
static fixed_t max_h;


static fixed_t min_w;
static fixed_t min_h;


static fixed_t min_scale_mtof;
static fixed_t max_scale_mtof;


static fixed_t old_m_w, old_m_h;
static fixed_t old_m_x, old_m_y;


static mpoint_t f_oldloc;


static fixed_t scale_mtof = (13107);

static fixed_t scale_ftom;

static player_t *plr;

static patch_t *marknums[10];
static mpoint_t markpoints[10];
static int markpointnum = 0;

static int followplayer = 1;

static unsigned char cheat_amap_seq[] = { 0xb2, 0x26, 0x26, 0x2e, 0xff };
static cheatseq_t cheat_amap = { cheat_amap_seq, 0 };

static boolean stopped = true;

extern boolean viewactive;




void
V_MarkRect
( int x,
  int y,
  int width,
  int height );





void
AM_getIslope
( mline_t* ml,
  islope_t* is )
{
    int dx, dy;

    dy = ml->a.y - ml->b.y;
    dx = ml->b.x - ml->a.x;
    if (!dy) is->islp = (dx<0?-((int)0x7fffffff):((int)0x7fffffff));
    else is->islp = FixedDiv(dx, dy);
    if (!dx) is->slp = (dy<0?-((int)0x7fffffff):((int)0x7fffffff));
    else is->slp = FixedDiv(dy, dx);

}

