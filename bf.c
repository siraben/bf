#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int stack;
uint8_t arr[30000] = {0};
size_t dp = 0;
size_t pc = 0;
char *prog;

static void die(int i) {
  free(prog);
  exit(i);
}

static void push() { stack++; }

static void pop() {
  if (!stack) {
    fprintf(stderr, "Stack underflow");
    die(1);
  }
  stack--;
}

// Calculate size of a file
static uint32_t calculate_size(FILE *file) {
  fseek(file, 0L, SEEK_END);
  uint32_t byteCnt = (uint32_t)ftell(file);
  rewind(file);
  return byteCnt;
}

static void load_prog(FILE *file) {
  uint8_t tmp[1024];
  size_t bytes_read = 0;
  size_t acc = 0;

  while ((bytes_read = fread(tmp, 1, sizeof(tmp), file)) > 0)
    for (size_t i = 0; i < bytes_read; i++)
      prog[acc++] = tmp[i];
  rewind(file);
}

static void backward_dp() { dp = dp ? dp - 1 : 29999; }

static void forward_dp() { dp = dp == 29999 ? 0 : dp + 1; }

static void forward_pc() {
  pc++;
  if (!prog[pc]) {
    fprintf(stderr, "End of program\n");
    fflush(stderr);
    exit(1);
  }
}

static void backward_pc() {
  if (!pc) {
    fprintf(stderr, "Reached start of program\n");
    fflush(stderr);
    exit(1);
  }
  pc--;
}

static void find_closing_brace() {
  while (stack) {
    if (prog[pc] == ']') {
      pop();
    } else if (prog[pc] == '[') {
      push();
    }
    forward_pc();
  }
}

static void find_opening_brace() {
  while (stack) {
    if (prog[pc] == '[') {
      pop();
    } else if (prog[pc] == ']') {
      push();
    }
    backward_pc();
  }
  forward_pc();
  forward_pc();
}

static void lbrac() {
  if (arr[dp] == 0) {
    push();
    forward_pc();
    find_closing_brace();
  } else {
    forward_pc();
  }
}

static void rbrac() {
  if (arr[dp] != 0) {
    push();
    backward_pc();
    find_opening_brace();
  } else {
    forward_pc();
  }
}

static void interp() {
  while (1) {
    assert(!stack);
    switch (prog[pc]) {
    case '+':
      arr[dp]++;
      forward_pc();
      break;
    case '-':
      arr[dp]--;
      forward_pc();
      break;
    case '>':
      forward_dp();
      forward_pc();
      break;
    case '<':
      backward_dp();
      forward_pc();
      break;
    case '.':
      putchar(arr[dp]);
      fflush(stdout);
      forward_pc();
      break;
    case ',':
      arr[dp] = getc(stdin);
      forward_pc();
      break;
    case '[':
      lbrac();
      break;
    case ']':
      rbrac();
      break;
    case 0:
      printf("End of program\n");
      break;
    default:
      forward_pc();
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: ./bf <file>\n");
    return -1;
  }
  char *fn = argv[1];
  FILE *f = fopen(fn, "rb");
  if (!f) {
    fprintf(stderr, "Unable to open %s for reading\n", fn);
    return -1;
  }
  uint32_t s = calculate_size(f);
  prog = calloc(s, sizeof(uint8_t));
  load_prog(f);
  fclose(f);
  interp();
  die(0);
}
