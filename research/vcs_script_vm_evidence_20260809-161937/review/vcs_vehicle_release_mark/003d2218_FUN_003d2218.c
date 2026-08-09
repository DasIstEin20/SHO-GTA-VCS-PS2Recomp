/* REVIEW-ONLY: FUN_003d2218 @ 0x003d2218 */


undefined8 FUN_003d2218(undefined8 param_1)

{
  undefined8 uVar1;
  
  FUN_00254d18(param_1,(int)param_1 + 0x10,1,0x50e1f8);
  uVar1 = FUN_00471b10(DAT_00487d98,DAT_0050e1f8);
  FUN_0025b300(uVar1);
  if (*(char *)((int)param_1 + 0x20a) != '\0') {
    FUN_00253f68(0x50cc70,DAT_0050e1f8,1,param_1,0);
  }
  return 0;
}

