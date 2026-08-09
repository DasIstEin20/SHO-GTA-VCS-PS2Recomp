/* REVIEW-ONLY: FUN_00406240 @ 0x00406240 */


undefined8 FUN_00406240(undefined8 param_1)

{
  undefined8 uVar1;
  
  FUN_00254d18(param_1,(int)param_1 + 0x10,1,0x50e1f8);
  if (DAT_0050e1f8 == 0) {
    uVar1 = FUN_00471740();
    FUN_003b6be0(uVar1,0);
  }
  else {
    uVar1 = FUN_00471740();
    FUN_003b6be0(uVar1,1);
  }
  return 0;
}

