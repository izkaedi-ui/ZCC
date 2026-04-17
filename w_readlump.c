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

