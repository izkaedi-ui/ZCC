void I_InitGraphics (void);


void I_ShutdownGraphics(void);


void I_SetPalette (byte* palette);

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);


void I_WaitVBL(int count);

void I_ReadScreen (byte* scr);

void I_BeginRead (void);
void I_EndRead (void);
# 1553 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2

# 1 "./DOOM/linuxdoom-1.10/g_game.h" 1
# 34 "./DOOM/linuxdoom-1.10/g_game.h"
void G_DeathMatchSpawnPlayer (int playernum);

void G_InitNew (skill_t skill, int episode, int map);




void G_DeferedInitNew (skill_t skill, int episode, int map);

void G_DeferedPlayDemo (char* demo);



void G_LoadGame (char* name);

void G_DoLoadGame (void);


void G_SaveGame (int slot, char* description);


void G_RecordDemo (char* name);

void G_BeginRecording (void);

void G_PlayDemo (char* name);
void G_TimeDemo (char* name);
boolean G_CheckDemoStatus (void);

void G_ExitLevel (void);
void G_SecretExitLevel (void);

void G_WorldDone (void);

void G_Ticker (void);
boolean G_Responder (event_t* ev);

void G_ScreenShot (void);
# 1555 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2

# 1 "./DOOM/linuxdoom-1.10/hu_stuff.h" 1
# 50 "./DOOM/linuxdoom-1.10/hu_stuff.h"
void HU_Init(void);
void HU_Start(void);

boolean HU_Responder(event_t* ev);

void HU_Ticker(void);
void HU_Drawer(void);
char HU_dequeueChatChar(void);
void HU_Erase(void);
# 1557 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2
# 1 "./DOOM/linuxdoom-1.10/wi_stuff.h" 1
# 31 "./DOOM/linuxdoom-1.10/wi_stuff.h"
typedef enum
{
    NoState = -1,
    StatCount,
    ShowNextLoc

} stateenum_t;


void WI_Ticker (void);



void WI_Drawer (void);


void WI_Start(wbstartstruct_t* wbstartstruct);
# 1558 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2



# 1 "./DOOM/linuxdoom-1.10/p_setup.h" 1
# 33 "./DOOM/linuxdoom-1.10/p_setup.h"
void
P_SetupLevel
( int episode,
  int map,
  int playermask,
  skill_t skill);


void P_Init (void);
# 1562 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2



# 1 "./DOOM/linuxdoom-1.10/d_main.h" 1
# 37 "./DOOM/linuxdoom-1.10/d_main.h"
extern char* wadfiles[20];

void D_AddFile (char *file);
# 49 "./DOOM/linuxdoom-1.10/d_main.h"
void D_DoomMain (void);


void D_PostEvent (event_t* ev);






void D_PageTicker (void);
void D_PageDrawer (void);
void D_AdvanceDemo (void);
void D_StartTitle (void);
# 1566 "./DOOM/linuxdoom-1.10/doom_amalg.c" 2
# 1576 "./DOOM/linuxdoom-1.10/doom_amalg.c"
void D_DoomLoop (void);


char* wadfiles[20];


boolean devparm;
boolean nomonsters;
boolean respawnparm;
boolean fastparm;

boolean drone;

boolean singletics = false;







extern boolean inhelpscreens;

skill_t startskill;
int startepisode;
int startmap;
boolean autostart;

FILE* debugfile;

boolean advancedemo;




char wadfile[1024];
char mapdir[1024];
char basedefault[1024];


void D_CheckNetGame (void);
void D_ProcessEvents (void);
void G_BuildTiccmd (ticcmd_t* cmd);
void D_DoAdvanceDemo (void);
# 1628 "./DOOM/linuxdoom-1.10/doom_amalg.c"
event_t events[64];
int eventhead;
int eventtail;






void D_PostEvent (event_t* ev)
{
    events[eventhead] = *ev;
    eventhead = (++eventhead)&(64 -1);
}

