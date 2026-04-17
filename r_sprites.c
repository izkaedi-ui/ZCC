void R_InitSpriteDefs (char** namelist)
{
    char** check;
    int i;
    int l;
    int intname;
    int frame;
    int rotation;
    int start;
    int end;
    int patched;


    check = namelist;
    while (*check != 0)
 check++;

    numsprites = check-namelist;

    if (!numsprites)
 return;

    sprites = Z_Malloc(numsprites *sizeof(*sprites), 1, 0);

    start = firstspritelump-1;
    end = lastspritelump+1;




    for (i=0 ; i<numsprites ; i++)
    {
 spritename = namelist[i];
 memset (sprtemp,-1, sizeof(sprtemp));

 maxframe = -1;
 intname = *(int *)namelist[i];



 for (l=start+1 ; l<end ; l++)
 {
     if (*(int *)lumpinfo[l].name == intname)
     {
  frame = lumpinfo[l].name[4] - 'A';
  rotation = lumpinfo[l].name[5] - '0';

  if (modifiedgame)
      patched = W_GetNumForName (lumpinfo[l].name);
  else
      patched = l;

  R_InstallSpriteLump (patched, frame, rotation, false);

  if (lumpinfo[l].name[6])
  {
      frame = lumpinfo[l].name[6] - 'A';
      rotation = lumpinfo[l].name[7] - '0';
      R_InstallSpriteLump (l, frame, rotation, true);
  }
     }
 }


 if (maxframe == -1)
 {
     sprites[i].numframes = 0;
     continue;
 }

 maxframe++;

 for (frame = 0 ; frame < maxframe ; frame++)
 {
     switch ((int)sprtemp[frame].rotate)
     {
       case -1:

  I_Error ("R_InitSprites: No patches found "
    "for %s frame %c", namelist[i], frame+'A');
  break;

       case 0:

  break;

       case 1:

  for (rotation=0 ; rotation<8 ; rotation++)
      if (sprtemp[frame].lump[rotation] == -1)
   I_Error ("R_InitSprites: Sprite %s frame %c "
     "is missing rotations",
     namelist[i], frame+'A');
  break;
     }
 }


 sprites[i].numframes = maxframe;
 sprites[i].spriteframes =
     Z_Malloc (maxframe * sizeof(spriteframe_t), 1, 0);
 memcpy (sprites[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
    }

}

