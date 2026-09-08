/* A child started with SystemTagList() gets a copy of the parent's local
   variables (NP_CopyVars default): it must see them, and changes it makes
   must not reach the parent. With NP_CopyVars FALSE it must not see them.

   usage: dos_sysvar <own_path>      (parent)
          dos_sysvar <own_path> child (started by the parent)
*/
#include <dos/dos.h>
#include <dos/var.h>
#include <dos/dostags.h>
#include <proto/dos.h>
#include <utility/tagitem.h>
#include <string.h>

#define VAR   "VAMOSTEST"
#define FLAGS (GVF_LOCAL_ONLY | LV_VAR)

int main(int argc, char *argv[])
{
  char buf[64];
  char cmd[256];
  struct TagItem tags[] = {
    {TAG_DONE, 0}
  };
  struct TagItem nocopy_tags[] = {
    {NP_CopyVars, FALSE},
    {TAG_DONE, 0}
  };
  LONG rc;

  if(argc == 3 && strcmp(argv[2], "child") == 0) {
    if(GetVar(VAR, buf, sizeof(buf), FLAGS) < 0) {
      Printf("child: no " VAR "\n");
      return 2;
    }
    if(strcmp(buf, "parent") != 0) {
      Printf("child: " VAR " is '%s'\n", (ULONG)buf);
      return 3;
    }
    SetVar(VAR, "child", -1, FLAGS);
    return 0;
  }

  if(argc != 2) {
    Printf("Usage: %s <own_path>\n", (ULONG)argv[0]);
    return 1;
  }

  SetVar(VAR, "parent", -1, FLAGS);

  strcpy(cmd, argv[1]);
  strcat(cmd, " ");
  strcat(cmd, argv[1]);
  strcat(cmd, " child");
  rc = SystemTagList(cmd, tags);
  if(rc != 0) {
    Printf("SystemTagList returned: %ld\n", rc);
    return rc;
  }

  if(GetVar(VAR, buf, sizeof(buf), FLAGS) < 0 || strcmp(buf, "parent") != 0) {
    Printf("parent: " VAR " changed to '%s'\n", (ULONG)buf);
    return 4;
  }

  /* without NP_CopyVars the child must not see the variable: rc 2 */
  rc = SystemTagList(cmd, nocopy_tags);
  if(rc != 2) {
    Printf("SystemTagList without NP_CopyVars returned: %ld\n", rc);
    return 5;
  }

  DeleteVar(VAR, FLAGS);
  return 0;
}
