/* REVIEW-ONLY: FUN_002d123c @ 0x002d123c */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_002d123c(undefined4 param_1,undefined4 param_2)

{
  char cVar1;
  int iVar2;
  
  iVar2 = FUN_0028a29c(_DAT_ffff6b28);
  if (iVar2 == 0) {
    iVar2 = _DAT_ffff9e04;
  }
  _DAT_ffff9e04 = iVar2;
  if (_DAT_ffff9708 == 0) {
    FUN_00361eec();
  }
  FUN_001502cc(_DAT_ffff9708,param_2);
  if (_DAT_ffff9700 == 0) {
    FUN_00361e34();
  }
  FUN_00001340(_DAT_ffff9700);
  FUN_002d0368(param_1);
  DAT_ffff9df8 = 0;
  FUN_002d13dc(param_1);
  FUN_00278a3c();
  if (_DAT_ffff7bec == 0) {
    FUN_0016a670(param_2);
    iVar2 = 0;
    do {
      FUN_002d2168(param_1,param_2);
      iVar2 = iVar2 + 1;
    } while (iVar2 < 5);
  }
  FUN_002d3cbc(param_1,param_2);
  if (_DAT_ffff971c == 0) {
    FUN_0036319c();
    cVar1 = *(char *)(_DAT_ffff971c + 0x13);
  }
  else {
    cVar1 = *(char *)(_DAT_ffff971c + 0x13);
  }
  if (cVar1 == '\0') {
    FUN_00005600(param_2,0);
  }
  FUN_002d0d8c(param_1,1,0);
  if (DAT_ffff6458 == '\0') {
    if (_DAT_ffff9e28 == 0) {
      _DAT_ffff9e28 = 1;
      FUN_00029794(0x1f390);
      FUN_0035ca60(0xffff7d8c);
    }
    FUN_0002a9ac(0x1f390,0);
  }
  return;
}

