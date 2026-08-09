/* REVIEW-ONLY: FUN_000657ec @ 0x000657ec */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000657ec(void)

{
  int iVar1;
  undefined1 *puVar2;
  int *piVar3;
  int iVar4;
  int *piVar5;
  uint uVar6;
  undefined1 auStack_80 [4];
  undefined4 local_7c;
  int *local_78;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  int local_60;
  undefined4 local_58;
  int local_50;
  ushort local_4c;
  int local_48;
  
  if (_DAT_ffff0e25 != 0) {
    if ((_DAT_ffff9e1d / 50.0) * 1000.0 < 2.1474836e+09) {
      iVar4 = (int)((_DAT_ffff9e1d / 50.0) * 1000.0);
    }
    else {
      iVar4 = (int)((_DAT_ffff9e1d / 50.0) * 1000.0 - 2.1474836e+09) + -0x80000000;
    }
    FUN_0005de2c(0x4b940,0);
    FUN_0005e0ac(0x4b970);
    FUN_00061220(0x4b7a8,0);
    FUN_00060628();
    if (DAT_ffff9f7a != '\0') {
      DAT_ffff9f7a = DAT_ffff9f7a + -1;
    }
    uVar6 = 0;
    local_50 = iVar4;
    do {
      local_4c = (ushort)uVar6;
      iVar4 = uVar6 * 0xa8;
      *(undefined4 *)(iVar4 + 0x4dfd8) = 0x3ef5c28f;
      *(undefined4 *)(iVar4 + 0x4dfdc) = 0x3f8f5c29;
      local_48 = iVar4;
      puVar2 = (undefined1 *)FUN_002318a0(auStack_80,0xe1,0xe1,0xe1);
      *(undefined1 *)(iVar4 + 0x4dfd0) = *puVar2;
      *(undefined1 *)(iVar4 + 0x4dfd1) = puVar2[1];
      *(undefined1 *)(iVar4 + 0x4dfd2) = puVar2[2];
      *(undefined1 *)(iVar4 + 0x4dfd3) = puVar2[3];
      *(byte *)(iVar4 + 0x4dff4) = *(byte *)(iVar4 + 0x4dff4) & 0xfe;
      *(byte *)(iVar4 + 0x4dff4) = *(byte *)(iVar4 + 0x4dff4) & 0xbf;
      *(byte *)(iVar4 + 0x4dff4) = *(byte *)(iVar4 + 0x4dff4) & 0xfd;
      *(byte *)(iVar4 + 0x4dff4) = *(byte *)(iVar4 + 0x4dff4) & 0xfb;
      *(byte *)(iVar4 + 0x4dff4) = *(byte *)(iVar4 + 0x4dff4) & 0xf7;
      *(undefined4 *)(iVar4 + 0x4dfe0) = 0x43ef0000;
      *(undefined4 *)(iVar4 + 0x4dfe4) = 0x44200000;
      puVar2 = (undefined1 *)FUN_002318a0(auStack_80,0x80,0x80,0x80);
      iVar1 = local_50;
      *(undefined1 *)(iVar4 + 0x4dfd4) = *puVar2;
      *(undefined1 *)(iVar4 + 0x4dfd5) = puVar2[1];
      *(undefined1 *)(iVar4 + 0x4dfd6) = puVar2[2];
      *(undefined1 *)(iVar4 + 0x4dfd7) = puVar2[3];
      *(byte *)(iVar4 + 0x4dff4) = *(byte *)(iVar4 + 0x4dff4) | 0x10;
      *(byte *)(iVar4 + 0x4dff4) = *(byte *)(iVar4 + 0x4dff4) & 0xdf;
      *(undefined1 *)(iVar4 + 0x4dfe8) = 1;
      *(undefined4 *)(iVar4 + 0x4dfec) = 0;
      *(undefined4 *)(iVar4 + 0x4dff0) = 0;
      uVar6 = 0;
      do {
        *(undefined2 *)(local_48 + uVar6 * 2 + 0x4dff6) = 0;
        piVar3 = _DAT_ffff9f35;
        uVar6 = uVar6 + 1 & 0xffff;
      } while (uVar6 < 0x40);
      uVar6 = local_4c + 1 & 0xffff;
    } while (uVar6 < 0x14);
    _DAT_ffff9f53 = 0;
    uVar6 = 0;
    do {
      *(undefined1 *)(uVar6 * 0x18 + 0x4ed04) = 0;
      *(undefined1 *)(uVar6 * 0x18 + 0x4ed05) = 0;
      uVar6 = uVar6 + 1 & 0xffff;
    } while (uVar6 < 0x10);
    _DAT_ffff9f57 = 0;
    DAT_ffff0e2d = 1;
    while (piVar3 != (int *)0x0) {
      piVar5 = (int *)*piVar3;
      piVar3[0x7d] = piVar3[0x7d] + iVar1;
      piVar3[0x7e] = piVar3[0x7e] + iVar1;
      FUN_0005e4a4(piVar3);
      piVar3 = piVar5;
      if ((piVar5 != (int *)0x0) && (*(char *)(piVar5 + 0x82) == '\0')) {
        piVar3 = (int *)0x0;
      }
    }
    DAT_ffff0e2d = 0;
    local_60 = _DAT_ffff9f85;
    for (local_78 = (int *)_DAT_ffff9f85; local_78 != (int *)0x0; local_78 = (int *)*local_78) {
      local_7c = 0;
      local_58 = 0;
      local_70 = local_78[2];
      local_6c = local_78[3];
      local_68 = local_78[4];
      FUN_0017d65c(-(float)local_78[5],0,local_78,*(undefined1 *)(local_78 + 8),
                   *(undefined1 *)((int)local_78 + 0x21),*(undefined1 *)((int)local_78 + 0x22));
    }
    DAT_ffff9e51 = 0;
  }
  return;
}

