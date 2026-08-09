/* REVIEW-ONLY: FUN_00256130 @ 0x00256130 */


void FUN_00256130(undefined8 param_1)

{
  char cVar1;
  int iVar2;
  long lVar3;
  int *piVar4;
  int iVar5;
  
  iVar5 = (int)param_1;
  DAT_004cd700 = iVar5;
  if (*(char *)(iVar5 + 0x20a) == '\0') {
    cVar1 = *(char *)(iVar5 + 0x217);
  }
  else {
    FUN_00258da0();
    cVar1 = *(char *)(iVar5 + 0x217);
  }
  if (((cVar1 != '\0') && (DAT_004cd6f0 == '\x01')) && (*(short *)(iVar5 + 0x204) == 1)) {
    *(undefined2 *)(iVar5 + 0x204) = 0;
    *(undefined4 *)(iVar5 + 0x10) = *(undefined4 *)(iVar5 + 0x14);
  }
  if (DAT_004cd404 < *(uint *)(iVar5 + 0x200)) {
    if ((*(char *)(iVar5 + 0x20b) != '\0') && (DAT_00514ad2 != 0 && DAT_00514b02 == 0)) {
      *(undefined4 *)(iVar5 + 0x200) = 0;
      piVar4 = &DAT_00740f10;
      iVar5 = 0x10000;
      do {
        if (*piVar4 != 0) {
          piVar4[3] = 0;
        }
        piVar4 = piVar4 + 0x30;
        iVar2 = iVar5 >> 0x10;
        iVar5 = iVar5 + 0x10000;
      } while (iVar2 < 8);
      if (DAT_00741510 != 0) {
        DAT_0074151c = 0;
      }
    }
  }
  else {
    do {
      lVar3 = FUN_00256260(param_1);
    } while (lVar3 == 0);
  }
  return;
}

