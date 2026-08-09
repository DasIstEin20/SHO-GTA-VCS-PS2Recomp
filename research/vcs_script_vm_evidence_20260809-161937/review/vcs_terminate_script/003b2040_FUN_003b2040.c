/* REVIEW-ONLY: FUN_003b2040 @ 0x003b2040 */


undefined4 FUN_003b2040(undefined8 param_1)

{
  if (*(char *)((int)param_1 + 0x217) != '\0') {
    DAT_004cd6e7 = 0;
  }
  FUN_00255a78(param_1,0x4cd6c4);
  FUN_00255ab0(param_1,0x4cd6c8);
  *(undefined1 *)((int)param_1 + 0x208) = 0;
  return 1;
}

