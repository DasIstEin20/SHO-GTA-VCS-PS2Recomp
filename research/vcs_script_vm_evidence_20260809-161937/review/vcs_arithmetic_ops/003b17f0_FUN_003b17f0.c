/* REVIEW-ONLY: FUN_003b17f0 @ 0x003b17f0 */


undefined8 FUN_003b17f0(undefined8 param_1)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = (int)param_1 + 0x10;
  piVar1 = (int *)FUN_002552f0(param_1,iVar2,0);
  FUN_00254d18(param_1,iVar2,1,0x50e1f8);
  if (DAT_0050e1f8 == 0) {
    trap(7);
  }
  *piVar1 = *piVar1 / DAT_0050e1f8;
  return 0;
}

