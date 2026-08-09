/* REVIEW-ONLY: FUN_002c023c @ 0x002c023c */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_002c023c(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined1 *puVar3;
  int iVar4;
  
  uVar2 = 0;
  FUN_001313b0();
  FUN_003546fc(0x2d6ba0,param_1);
  FUN_0018c4c8();
  _DAT_ffff9e04 = 1;
  _DAT_ffff7bec = 0;
  uVar1 = FUN_001312cc();
  FUN_00131638("Loading the Game","Setup game variables",uVar1,0);
  FUN_00163100();
  FUN_0028f0a8();
  FUN_0021e5fc(0x1e0c0);
  FUN_0021950c(0x1e0c0,_DAT_ffffa2e0);
  FUN_0012ec94();
  FUN_00159638();
  FUN_002f7804();
  FUN_0016fb30();
  FUN_0033a65c();
  FUN_0014fbd0();
  FUN_002b4d04();
  if (_DAT_ffff9720 == 0) {
    FUN_003635b4();
  }
  FUN_00069b9c(_DAT_ffff9720);
  FUN_002b6e20();
  FUN_0018ce88();
  FUN_002905a8();
  FUN_0008d9fc();
  FUN_0017d584();
  FUN_000fb808();
  FUN_0033d478();
  FUN_000ee7e4();
  FUN_002e9814();
  FUN_000b5d3c();
  iVar4 = 0;
  puVar3 = &LAB_00034740;
  do {
    FUN_0015ca08(puVar3);
    iVar4 = iVar4 + 1;
    puVar3 = puVar3 + 0x160;
  } while (iVar4 < 1);
  FUN_0017207c(0x42f00000);
  _DAT_ffff9e9c = 0x43fa0000;
  FUN_00131638("Loading the Game",0xffff7c58,0,0);
  FUN_002cf9f0(0);
  FUN_002cf5ac();
  FUN_002cf50c();
  FUN_002cf9f0(0);
  FUN_002cf9f0(0);
  FUN_002b0e94();
  FUN_0017c198();
  FUN_002cf55c();
  FUN_002cf9f0(0);
  FUN_00275f40();
  FUN_00131638("Loading the Game","Setup game variables",0,0);
  if (_DAT_ffff96fc == 0) {
    FUN_00361c24();
  }
  FUN_00008f70(_DAT_ffff96fc);
  if (_DAT_ffff96fc == 0) {
    FUN_00361c24();
  }
  FUN_0000c1b8(_DAT_ffff96fc);
  FUN_0023ce90();
  FUN_00131638("Loading the Game","Setup traffic lights",0,0);
  FUN_0031f290();
  FUN_00131638("Loading the Game","Setup game variables",0,0);
  FUN_001c35dc();
  DAT_ffff9e98 = 0;
  FUN_00292408();
  FUN_00213cbc();
  FUN_0031c3e4();
  FUN_001cb520();
  FUN_0002366c();
  FUN_001f1304(0xbbe40);
  FUN_00131638("Loading the Game",0xffff7c68,0,0);
  FUN_0013dc50(1000);
  FUN_0018dac0();
  FUN_00169168();
  if (DAT_ffff6458 == '\0') {
    uVar2 = FUN_000650ac(DAT_ffff9093);
    DAT_ffff9093 = 0;
  }
  FUN_002837ec();
  FUN_00131638("Loading the Game","Setup game variables",0,0);
  FUN_001ea27c();
  FUN_0016c0ac();
  FUN_00158dcc();
  FUN_002318b8();
  FUN_0028ca64();
  FUN_00088cd4();
  FUN_000a7c44();
  FUN_0013bcf0();
  FUN_00282400();
  FUN_000666b4();
  FUN_00131638("Loading the Game","Position dynamic objects",0,0);
  FUN_00131638("Loading the Game","Initialise vehicle paths",0,0);
  FUN_001f9c34();
  FUN_0017bb34();
  FUN_0018ce98();
  uVar1 = FUN_001312cc();
  FUN_00131638("Loading the Game",0xffff7c78,uVar1,0);
  if (DAT_ffff6458 == '\0') {
    FUN_0005f4dc();
    FUN_00131638("Loading the Game","CTheScripts::Process",0,0);
    FUN_000657ec();
    FUN_00131638("Loading the Game","TheCamera.Process",0,0);
    FUN_0021e938(0x1e0c0);
    FUN_00131638("Loading the Game","CStreaming::LoadScene(TheCamera.GetPosition())",0,0);
    FUN_002cf938(0x1e0f0);
  }
  FUN_00131638("Loading the Game",0xffff7c88,0,0);
  _DAT_ffff9ed0 = _DAT_ffff9e04;
  uVar1 = FUN_00187760(0);
  FUN_00187d6c(uVar1,1);
  uVar1 = FUN_00187760(1);
  FUN_00187d6c(uVar1,1);
  FUN_00202760(0xffff9e78,1);
  return uVar2;
}

