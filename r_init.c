void R_InitTextures (void)
{
    maptexture_t* mtexture;
    texture_t* texture;
    mappatch_t* mpatch;
    texpatch_t* patch;

    int i;
    int j;

    int* maptex;
    int* maptex2;
    int* maptex1;

    char name[9];
    char* names;
    char* name_p;

    int* patchlookup;

    int totalwidth;
    int nummappatches;
    int offset;
    int maxoff;
    int maxoff2;
    int numtextures1;
    int numtextures2;

    int* directory;

    int temp1;
    int temp2;
    int temp3;



    name[8] = 0;
    names = W_CacheLumpName ("PNAMES", 1);
    nummappatches = (*((int *)names));
    name_p = names+4;
    patchlookup = alloca (nummappatches*sizeof(*patchlookup));

    for (i=0 ; i<nummappatches ; i++)
    {
 strncpy (name,name_p+i*8, 8);
 patchlookup[i] = W_CheckNumForName (name);
    }
    Z_Free (names);




    maptex = maptex1 = W_CacheLumpName ("TEXTURE1", 1);
    numtextures1 = (*maptex);
    maxoff = W_LumpLength (W_GetNumForName ("TEXTURE1"));
    directory = maptex+1;

    if (W_CheckNumForName ("TEXTURE2") != -1)
    {
 maptex2 = W_CacheLumpName ("TEXTURE2", 1);
 numtextures2 = (*maptex2);
 maxoff2 = W_LumpLength (W_GetNumForName ("TEXTURE2"));
    }
    else
    {
 maptex2 = 0;
 numtextures2 = 0;
 maxoff2 = 0;
    }
    numtextures = numtextures1 + numtextures2;

    textures = Z_Malloc (numtextures*4, 1, 0);
    texturecolumnlump = Z_Malloc (numtextures*4, 1, 0);
    texturecolumnofs = Z_Malloc (numtextures*4, 1, 0);
    texturecomposite = Z_Malloc (numtextures*4, 1, 0);
    texturecompositesize = Z_Malloc (numtextures*4, 1, 0);
    texturewidthmask = Z_Malloc (numtextures*4, 1, 0);
    textureheight = Z_Malloc (numtextures*4, 1, 0);

    totalwidth = 0;


    temp1 = W_GetNumForName ("S_START");
    temp2 = W_GetNumForName ("S_END") - 1;
    temp3 = ((temp2-temp1+63)/64) + ((numtextures+63)/64);
    printf("[");
    for (i = 0; i < temp3; i++)
 printf(" ");
    printf("         ]");
    for (i = 0; i < temp3; i++)
 printf("\x8");
    printf("\x8\x8\x8\x8\x8\x8\x8\x8\x8\x8");

    for (i=0 ; i<numtextures ; i++, directory++)
    {
 if (!(i&63))
     printf (".");

 if (i == numtextures1)
 {

     maptex = maptex2;
     maxoff = maxoff2;
     directory = maptex+1;
 }

 offset = (*directory);

 if (offset > maxoff)
     I_Error ("R_InitTextures: bad texture directory");

 mtexture = (maptexture_t *) ( (byte *)maptex + offset);

 texture = textures[i] =
     Z_Malloc (sizeof(texture_t)
        + sizeof(texpatch_t)*((mtexture->patchcount)-1),
        1, 0);

 texture->width = (mtexture->width);
 texture->height = (mtexture->height);
 texture->patchcount = (mtexture->patchcount);

 memcpy (texture->name, mtexture->name, sizeof(texture->name));
 mpatch = &mtexture->patches[0];
 patch = &texture->patches[0];

 for (j=0 ; j<texture->patchcount ; j++, mpatch++, patch++)
 {
     patch->originx = (mpatch->originx);
     patch->originy = (mpatch->originy);
     patch->patch = patchlookup[(mpatch->patch)];
     if (patch->patch == -1)
     {
  I_Error ("R_InitTextures: Missing patch in texture %s",
    texture->name);
     }
 }
 texturecolumnlump[i] = Z_Malloc (texture->width*2, 1,0);
 texturecolumnofs[i] = Z_Malloc (texture->width*2, 1,0);

 j = 1;
 while (j*2 <= texture->width)
     j<<=1;

 texturewidthmask[i] = j-1;
 textureheight[i] = texture->height<<16;

 totalwidth += texture->width;
    }

    Z_Free (maptex1);
    if (maptex2)
 Z_Free (maptex2);


    for (i=0 ; i<numtextures ; i++)
 R_GenerateLookup (i);


    texturetranslation = Z_Malloc ((numtextures+1)*4, 1, 0);

    for (i=0 ; i<numtextures ; i++)
 texturetranslation[i] = i;
}

