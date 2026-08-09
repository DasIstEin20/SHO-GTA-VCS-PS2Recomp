/* REVIEW-ONLY: FUN_003d0b60 @ 0x003d0b60 */


undefined8 FUN_003d0b60(undefined8 param_1)

{
  long lVar1;
  
  FUN_00254d18(param_1,(int)param_1 + 0x10,1,0x50e1f8);
  lVar1 = FUN_00471b10(DAT_00487d98,DAT_0050e1f8);
  if (*(char *)((int)param_1 + 0x20a) != '\0') {
    FUN_00253f68(0x50cc70,DAT_0050e1f8,1,param_1,0);
  }
  if (lVar1 != 0) {
    FUN_0018fa30(lVar1,1);
    FUN_00196920(lVar1);
    (**(code **)(*(int *)((int)lVar1 + 0x5c) + 0x10))(lVar1,3);
  }
  return 0;
}

