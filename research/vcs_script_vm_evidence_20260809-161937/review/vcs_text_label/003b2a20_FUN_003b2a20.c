/* REVIEW-ONLY: FUN_003b2a20 @ 0x003b2a20 */


undefined8 FUN_003b2a20(undefined8 param_1)

{
  undefined4 *puVar1;
  int iVar2;
  
  iVar2 = (int)param_1 + 0x10;
  puVar1 = (undefined4 *)FUN_002552f0(param_1,iVar2,0);
  FUN_00254d18(param_1,iVar2,1,0x50e1f8);
  *puVar1 = DAT_0050e1f8;
  return 0;
}

