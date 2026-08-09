/* REVIEW-ONLY: FUN_00306110 @ 0x00306110 */


/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00306110(int param_1,int param_2)

{
  uint uVar1;
  undefined4 in_zero;
  
  if (param_1 != 0) {
    setCopReg(2,in_zero,*(undefined8 *)(param_1 + 0x30));
    getCopReg(2,0);
    if (0 < param_2 + 0x20) {
      uVar1 = getCopReg(2,0);
      setCopReg(2,0,(ulonglong)uVar1);
                    /* WARNING: Bad instruction - Truncating control flow here */
      halt_baddata();
    }
  }
  return;
}

