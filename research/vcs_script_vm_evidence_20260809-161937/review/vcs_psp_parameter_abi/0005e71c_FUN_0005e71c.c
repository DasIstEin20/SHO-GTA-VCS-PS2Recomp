/* REVIEW-ONLY: FUN_0005e71c @ 0x0005e71c */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_0005e71c(undefined4 param_1,int param_2)

{
  uint *puVar1;
  uint uVar2;
  int iVar3;
  int local_10 [2];
  
  iVar3 = param_2 + 1;
  switch(*(undefined1 *)(_DAT_ffff3534 + param_2)) {
  case 1:
    uVar2 = 0;
    break;
  case 2:
    uVar2 = 0;
    break;
  case 3:
    uVar2 = (uint)*(byte *)(_DAT_ffff3534 + iVar3) << 0x18;
    break;
  case 4:
    uVar2 = (uint)*(ushort *)(_DAT_ffff3534 + iVar3) << 0x10;
    break;
  case 5:
    uVar2 = (uint)*(byte *)(_DAT_ffff3534 + iVar3) << 8 |
            (uint)*(ushort *)(_DAT_ffff3534 + param_2 + 2) << 0x10;
    break;
  case 6:
  case 9:
    uVar2 = *(uint *)(_DAT_ffff3534 + iVar3);
    break;
  case 7:
    uVar2 = (uint)*(char *)(_DAT_ffff3534 + iVar3);
    break;
  case 8:
    uVar2 = (uint)*(short *)(_DAT_ffff3534 + iVar3);
    break;
  default:
    local_10[0] = param_2;
    puVar1 = (uint *)FUN_0005e3d0(param_1,local_10,0);
    uVar2 = *puVar1;
  }
  return uVar2;
}

