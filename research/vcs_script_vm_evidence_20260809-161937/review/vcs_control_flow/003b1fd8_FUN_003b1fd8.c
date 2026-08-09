/* REVIEW-ONLY: FUN_003b1fd8 @ 0x003b1fd8 */


undefined8 FUN_003b1fd8(undefined8 param_1)

{
  int iVar1;
  
  iVar1 = (int)param_1;
  FUN_00254d18(param_1,iVar1 + 0x10,1,0x50e1f8);
  if (*(char *)(iVar1 + 0x209) == '\0') {
    if (DAT_0050e1f8 < 0) {
      *(int *)(iVar1 + 0x10) = DAT_004cd6d8 - DAT_0050e1f8;
    }
    else {
      *(int *)(iVar1 + 0x10) = DAT_0050e1f8;
    }
  }
  return 0;
}

