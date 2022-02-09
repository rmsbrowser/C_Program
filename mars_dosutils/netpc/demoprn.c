/*
 * very simple demo for a printerclient running with mars_nwe
 * a real printerclient should be made in assembler
 * as a resident program.
 * the simple idea is to open a named pipe at the mars_nwe server
 * and spool all data to the printer.
 */

#include <stdio.h>

static int usage(char *progname)
{
  fprintf(stderr, "Usage:\t%s attachfile printer\n", progname);
  return(1);
}

static FILE *fopen_err(char *fn, char *openmode)
{
  FILE *f=fopen(fn, openmode);
  if (NULL == f) {
    fprintf(stderr, "Open error '%s' with mode='%s'\n", fn, openmode);
  }
  return(f);
}

int main(int argc, char *argv[])
{
  if (argc > 2) {
    FILE *fa=fopen_err(argv[1], "rb");
    if (NULL != fa) {
      FILE *fo=fopen_err(argv[2], "w+b");
      if (NULL != fo) {
        char buf[512];
        int k;
        while (1) {
          if (0 != (k=fread(buf, 1, sizeof(buf), fa))) {
            if (1 != fwrite(buf, k, 1, fo)) {
              fprintf(stderr, "Writerror in %s\n", argv[2]);
            }
          } else sleep(2);
        }
      } else
        return(usage(argv[0]));
      fclose(fa);
    } else
      return(usage(argv[0]));
  } else return(usage(argv[0]));
  return(0);
}
