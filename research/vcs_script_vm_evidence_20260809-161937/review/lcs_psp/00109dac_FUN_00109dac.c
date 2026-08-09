/* REVIEW-ONLY: FUN_00109dac @ 0x00109dac */


/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Removing unreachable block (ram,0x00109e44) */

undefined4 FUN_00109dac(undefined8 param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  uint uVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  
  fVar4 = (float)param_1 + 2048.0;
  fVar3 = (float)((ulonglong)param_1 >> 0x20) + 2048.0;
  iVar6 = (int)(fVar4 * 0.03125);
  iVar5 = (int)(fVar3 * 0.03125);
  uVar2 = (uint)*(byte *)(DAT_00331fe0 + iVar6 * 0x80 + iVar5 + 0x100c);
  if (uVar2 == 0x80) {
    return 0;
  }
  *param_2 = *(undefined4 *)(*(int *)(DAT_00331fe0 + 4) + uVar2 * 4);
  uVar1 = getCopReg(2,0);
  setCopReg(2,0,CONCAT44((float)(uRam00000e3c & 0xfff) * 0.0015339355 +
                         ((fVar4 - (float)iVar6 * 32.0) + (fVar3 - (float)iVar5 * 32.0)) *
                         0.19634375,uVar1));
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

