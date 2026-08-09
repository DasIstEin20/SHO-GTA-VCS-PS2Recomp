/* REVIEW-ONLY: FUN_00256260 @ 0x00256260 */


undefined1 FUN_00256260(int param_1)

{
  byte bVar1;
  undefined1 uVar2;
  uint uVar3;
  undefined8 uVar4;
  byte *pbVar5;
  short sStack_20;
  short sStack_1e;
  code *pcStack_1c;
  
  pbVar5 = (byte *)(DAT_0048771c + *(int *)(param_1 + 0x10));
  bVar1 = *pbVar5;
  uVar3 = (int)(char)pbVar5[1] << 8;
  *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 2;
  if ((uVar3 & 0x8000) == 0) {
    *(undefined1 *)(param_1 + 0x20c) = 0;
  }
  else {
    *(undefined1 *)(param_1 + 0x20c) = 1;
  }
  uVar4 = *(undefined8 *)(&DAT_004bee20 + ((uint)bVar1 | uVar3 & 0x7fff) * 8);
  sStack_1e = (short)((ulong)uVar4 >> 0x10);
  uVar2 = 0xff;
  if (sStack_1e != 0) {
    sStack_20 = (short)uVar4;
    pcStack_1c = (code *)((ulong)uVar4 >> 0x20);
    uVar2 = (*pcStack_1c)(param_1 + sStack_20);
  }
  return uVar2;
}

