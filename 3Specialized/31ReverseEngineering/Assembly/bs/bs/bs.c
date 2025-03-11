//#include <stdio.h>

int bs(int* a, int n, int x) {
  int l = 0;
  int r = n + 1;
  while (r - l > 1) {
    int mid = (l + r) >> 1;
    if (a[mid] < x)
      l = mid;
    else
      r = mid;
  }
  if (a[l] != x) return -1;
  return l;
}

int main() {
  int n = 6;
  int a[] = {1, 3, 6, 7, 10, 15};
  int x = 10;

  int res = bs(a, n, x);
  //printf(res);
}