path = 'h:/__DOWNLOADS/selforglinux/doom_pp.c'
with open(path, 'r', encoding='utf-8') as f:
    text = f.read()

target = """void I_ShutdownGraphics(void)
{

  if (!XShmDetach(X_display, &X_shminfo))
     I_Error("XShmDetach() failed in I_ShutdownGraphics()");


  shmdt(X_shminfo.shmaddr);
  shmctl(X_shminfo.shmid, 0, 0);


  image->data = 0;
}"""

replacement = """void I_ShutdownGraphics(void)
{
  if (X_display && doShm) {
      if (!XShmDetach(X_display, &X_shminfo))
         I_Error("XShmDetach() failed in I_ShutdownGraphics()");

      shmdt(X_shminfo.shmaddr);
      shmctl(X_shminfo.shmid, 0, 0);
  }

  if (image) image->data = 0;
}"""

if target in text:
    text = text.replace(target, replacement)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(text)
    print("Replaced successfully")
else:
    print("Target not found")
