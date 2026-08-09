/* REVIEW-ONLY: FUN_0006189c @ 0x0006189c */


/* WARNING: Removing unreachable block (ram,0x00061938) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0006189c(undefined4 param_1,int *param_2,undefined4 param_3,int param_4)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  byte *pbVar3;
  int iVar4;
  undefined **ppuVar5;
  int unaff_gp;
  undefined4 unaff_retaddr;
  
  uVar1 = getCopReg(2,0x1c);
  setCopReg(2,0x1c,CONCAT44(param_1,uVar1));
  uVar1 = getCopReg(2,0x1d);
  setCopReg(2,0x1d,CONCAT44(param_2,uVar1));
  iVar4 = *(int *)(unaff_gp + -0x71dc);
  ppuVar5 = &PTR_LAB_00375c88;
  pbVar3 = (byte *)(*param_2 + iVar4);
  while( true ) {
    if (*pbVar3 < 0xb) break;
    getCopReg(2,0x1c);
    uVar1 = getCopReg(2,0x5d);
    setCopReg(2,0x5d,CONCAT44(unaff_retaddr,uVar1));
    uVar1 = getCopReg(2,0x5c);
    setCopReg(2,0x5c,CONCAT44(param_3,uVar1));
    uVar1 = getCopReg(2,0x7c);
    setCopReg(2,0x7c,CONCAT44(param_4 + 4,uVar1));
    uVar1 = getCopReg(2,0x1e);
    setCopReg(2,0x1e,CONCAT44(iVar4,uVar1));
    uVar1 = getCopReg(2,0x3e);
    setCopReg(2,0x3e,CONCAT44(ppuVar5,uVar1));
    _FUN_00000000 = pbVar3 + -iVar4;
    getCopReg(2,0x1d);
    puVar2 = (undefined4 *)FUN_0005da7c(0,0,0,param_4 + 4);
    uRamfffffffc = *puVar2;
    getCopReg(2,0x1d);
    getCopReg(2,0x5c);
    getCopReg(2,0x7c);
    param_4 = 0;
    getCopReg(2,0x1e);
    iVar4 = 0;
    getCopReg(2,0x3e);
    ppuVar5 = (undefined **)0x0;
    getCopReg(2,0x5d);
    unaff_retaddr = 0;
    param_3 = 0xffffffff;
    pbVar3 = _FUN_00000000;
  }
                    /* WARNING: Could not recover jumptable at 0x000618d4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar1 = (*(code *)ppuVar5[*pbVar3])();
  return uVar1;
}

