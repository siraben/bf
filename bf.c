#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int stack;
uint8_t arr[30000] = {0};
long dp = 0;
long pc = 0;
char *prog;
// Jump table
long *jmp_tbl;
// Increment table
long *inc_tbl;
// Open file handler
FILE *f;
// Size of program
long s;

static void die(int i) {
  free(prog);
  free(jmp_tbl);
  free(inc_tbl);
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
static long calculate_size(FILE *file) {
  fseek(file, 0L, SEEK_END);
  long byteCnt = (long)ftell(file);
  rewind(file);
  return byteCnt;
}

static void load_prog(FILE *file) {
  uint8_t tmp[1024];
  long bytes_read = 0;
  long acc = 0;

  while ((bytes_read = fread(tmp, 1, sizeof(tmp), file)) > 0)
    for (long i = 0; i < bytes_read; i++)
      prog[acc++] = tmp[i];
  rewind(file);
}

static void backward_dp() { dp = dp ? dp - 1 : 29999; }

static void forward_dp() { dp = dp == 29999 ? 0 : dp + 1; }

static void forward_pc() {
  pc++;
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

static void compute_jmp_tbl() {
  for (long i = 0; i < s; i++) {
    pc = i;
    long old = pc;
    if (prog[i] == '[') {
      push();
      forward_pc();
      find_closing_brace();
    } else if (prog[i] == ']') {
      push();
      backward_pc();
      find_opening_brace();
    }
    jmp_tbl[old] = pc;
  }
  pc = 0;
}

// Compute increment table to optimize runs of + and -
static void compute_inc_dec() {
  for (long i = 0; i < s; i++) {
    pc = i;
    // If we encounter a + or -, we need to know how many times to increment
    // or decrement
    // Keep looping next instruction until we encounter a + or -
    if (prog[i] == '+' || prog[i] == '-') {
      // Keep track of the value of the increment
      long inc = prog[i] == '+' ? 1 : -1;
      long old = pc;
      forward_pc();
      while (prog[pc] == '+' || prog[pc] == '-') {
        inc += prog[pc] == '+' ? 1 : -1;
        forward_pc();
      }
      jmp_tbl[old] = pc;
      inc_tbl[old] = inc;
    }
  }
  pc = 0;
}

// Compute increment table to optimize runs of > and <
static void compute_left_right() {
  for (long i = 0; i < s; i++) {
    pc = i;
    // If we encounter a > or < we need to know how many times to increment
    // or decrement
    // Keep looping next instruction until we encounter a > or <
    if (prog[i] == '>' || prog[i] == '<') {
      // Keep track of the value of the increment
      long inc = prog[i] == '>' ? 1 : -1;
      long old = pc;
      forward_pc();
      while (prog[pc] == '>' || prog[pc] == '<') {
        inc += prog[pc] == '>' ? 1 : -1;
        forward_pc();
      }
      jmp_tbl[old] = pc;
      inc_tbl[old] = inc;
    }
  }
  pc = 0;
}

static void lbrac() {
  if (arr[dp] == 0) {
    pc = jmp_tbl[pc];
  } else {
    forward_pc();
  }
}

static void rbrac() {
  if (arr[dp] != 0) {
    pc = jmp_tbl[pc];
  } else {
    forward_pc();
  }
}

// Interpret + or -
static void inc_dec() {
  // Use the increment table to determine the increment
  long inc = inc_tbl[pc];
  arr[dp] += inc;
  // Use the jump table to determine the next instruction
  pc = jmp_tbl[pc];
}

// Interpret > or <
static void left_right() {
  // Use the increment table to determine the increment and ensure dp is in
  // range
  long inc = inc_tbl[pc];
  dp += inc;
  if (dp < 0)
    dp += 30000;
  else if (dp > 29999)
    dp %= 30000;
  // Use the jump table to determine the next instruction
  pc = jmp_tbl[pc];
}

static void interp() {
  while (1) {
    assert(!stack);
    switch (prog[pc]) {
    case '+':
    case '-':
      inc_dec();
      break;
    case '>':
    case '<':
      left_right();
      break;
    case '.':
      putchar(arr[dp]);
      forward_pc();
      break;
    case ',':
      fflush(stdout);
      arr[dp] = getc(stdin);
      forward_pc();
      break;
    case '[':
      // Detect "[-]" and write 0 to dp
      if (prog[pc + 1] == '-' && prog[pc + 2] == ']') {
        arr[dp] = 0;
        pc += 3;
      } else {
        lbrac();
      }
      break;
    case ']':
      rbrac();
      break;
    case 0:
      printf("\nReached end of program\n");
      return;
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
  f = fopen(fn, "rb");
  if (!f) {
    fprintf(stderr, "Unable to open %s for reading\n", fn);
    return -1;
  }
  s = calculate_size(f);
  // Add 2 to account for look-ahead when running
  prog = calloc(s+2, sizeof(uint8_t));
  jmp_tbl = calloc(s, sizeof(long));
  inc_tbl = calloc(s, sizeof(long));
  load_prog(f);
  fclose(f);
  compute_jmp_tbl();
  compute_inc_dec();
  compute_left_right();
  interp();
  die(0);
}
