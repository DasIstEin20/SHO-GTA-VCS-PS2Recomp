/* REVIEW-ONLY: FUN_003b1520 @ 0x003b1520 */


undefined8 FUN_003b1520(undefined8 param_1)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = (int)param_1 + 0x10;
  piVar1 = (int *)FUN_002552f0(param_1,iVar2,0);
  FUN_00254d18(param_1,iVar2,1,0x50e1f8);
  *piVar1 = *piVar1 + DAT_0050e1f8;
  return 0;
}

