/* REVIEW-ONLY: FUN_003b2320 @ 0x003b2320 */


undefined8 FUN_003b2320(undefined8 param_1)

{
  int iVar1;
  
  FUN_00254d18(param_1,(int)param_1 + 0x10,1,0x50e1f8);
  iVar1 = FUN_00255ad8(DAT_0050e1f8);
  *(undefined1 *)(iVar1 + 0x20a) = 1;
  return 0;
}

