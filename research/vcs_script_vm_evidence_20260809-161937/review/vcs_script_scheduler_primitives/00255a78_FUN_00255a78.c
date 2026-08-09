/* REVIEW-ONLY: FUN_00255a78 @ 0x00255a78 */


void FUN_00255a78(int *param_1,int *param_2)

{
  if ((int *)param_1[1] == (int *)0x0) {
    *param_2 = *param_1;
  }
  else {
    *(int *)param_1[1] = *param_1;
  }
  if (*param_1 != 0) {
    *(int *)(*param_1 + 4) = param_1[1];
  }
  return;
}

