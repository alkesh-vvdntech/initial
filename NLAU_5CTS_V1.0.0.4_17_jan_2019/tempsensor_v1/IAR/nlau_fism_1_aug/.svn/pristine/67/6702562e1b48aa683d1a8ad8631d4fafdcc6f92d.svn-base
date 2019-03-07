/* Simple test program
 *
 *  gcc -o test test.c minIni.c
 */
#include <stdio.h>
#include <string.h>
#include "minIni.h"

#ifdef USE_MININI

#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

const char inifile[] = "test.ini";
const char inifile2[] = "testplain.ini";

int main_test()
{
#if 0

  char str[100];
  long n;
  int s, k;
  char section[50];

  /* string reading */
  n = ini_gets("first", "string", "dummy", str, sizearray(str), inifile);
  n = ini_gets("second", "string", "dummy", str, sizearray(str), inifile);
  n = ini_gets("first", "undefined", "dummy", str, sizearray(str), inifile);
  /* ----- */
  n = ini_gets("", "string", "dummy", str, sizearray(str), inifile2);
  n = ini_gets(NULL, "string", "dummy", str, sizearray(str), inifile2);
  /* ----- */

  /* value reading */
  n = ini_getl("first", "val", -1, inifile);
  n = ini_getl("second", "val", -1, inifile);
  n = ini_getl("first", "undefined", -1, inifile);

  /* ----- */
  n = ini_getl(NULL, "val", -1, inifile2);

  /* ----- */

  /* string writing */
  n = ini_puts("first", "alt", "flagged as \"correct\"", inifile);
  assert(n==1);
  n = ini_gets("first", "alt", "dummy", str, sizearray(str), inifile);
  assert(n==20 && strcmp(str,"flagged as \"correct\"")==0);
  /* ----- */
  n = ini_puts("second", "alt", "correct", inifile);
  assert(n==1);
  n = ini_gets("second", "alt", "dummy", str, sizearray(str), inifile);
  assert(n==7 && strcmp(str,"correct")==0);
  /* ----- */
  n = ini_puts("third", "test", "correct", inifile);
  assert(n==1);
  n = ini_gets("third", "test", "dummy", str, sizearray(str), inifile);
  assert(n==7 && strcmp(str,"correct")==0);
  /* ----- */
  n = ini_puts("second", "alt", "overwrite", inifile);
  assert(n==1);
  n = ini_gets("second", "alt", "dummy", str, sizearray(str), inifile);
  assert(n==9 && strcmp(str,"overwrite")==0);
  /* ----- */
  n = ini_puts(NULL, "alt", "correct", inifile2);
  assert(n==1);
  n = ini_gets(NULL, "alt", "dummy", str, sizearray(str), inifile2);
  assert(n==7 && strcmp(str,"correct")==0);
  /* ----- */
  printf("3. String writing tests passed\n");

  /* section/key enumeration */
  printf("4. Section/key enumertion, file contents follows\n");
  for (s = 0; ini_getsection(s, section, sizearray(section), inifile) > 0; s++) {
    printf("    [%s]\n", section);
    for (k = 0; ini_getkey(section, k, str, sizearray(str), inifile) > 0; k++) {
      printf("\t%s\n", str);
    } /* for */
  } /* for */

  /* browsing through the file */
  printf("5. browse through all settings, file contents follows\n");
  ini_browse(Callback, NULL, inifile);

  /* string deletion */
  n = ini_puts("first", "alt", NULL, inifile);
  assert(n==1);
  n = ini_puts("second", "alt", NULL, inifile);
  assert(n==1);
  n = ini_puts("third", NULL, NULL, inifile);
  assert(n==1);
  /* ----- */
  n = ini_puts(NULL, "alt", NULL, inifile2);
  assert(n==1);
  printf("6. String deletion tests passed\n");
#endif

  return 0;
}

#endif
