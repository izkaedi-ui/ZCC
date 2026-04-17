void R_InitColormaps (void)
{
    int lump, length;



    lump = W_GetNumForName("COLORMAP");
    length = W_LumpLength (lump) + 255;
    colormaps = Z_Malloc (length, 1, 0);
    colormaps = (byte *)( ((int)colormaps + 255)&~0xff);
    W_ReadLump (lump,colormaps);
}

