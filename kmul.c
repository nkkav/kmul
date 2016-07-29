/*
 * File       : kmul.c                                                          
 * Description: Generator and calculator for multiplication by integer constant 
 *              routines. Based on the implementation of Bernstein's algorithm
 *              in the technical report (Multiplication by Integer Constants) 
 *              by:
 *              Preston Briggs <preston@cs.rice.edu>
 *              Tim Harvey     <harv@cs.rice.edu>
 *              July 13, 1994
 * Author     : Nikolaos Kavvadias <nikolaos.kavvadias@gmail.com>                
 * Copyright  : (C) Nikolaos Kavvadias 2006, 2007, 2008, 2009, 2010, 2011, 2012, 
 *              2013, 2014, 2015, 2016                
 * Website    : http://www.nkavvadias.com                            
 *                                                                          
 * This file is part of kmul, and is distributed under the terms of the  
 * Modified BSD License.
 *
 * A copy of the Modified BSD License is included with this distrubution 
 * in the files COPYING.BSD.
 * kmul is free software: you can redistribute it and/or modify it under the
 * terms of the Modified BSD License. 
 * kmul is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the Modified BSD License for more details.
 * 
 * You should have received a copy of the Modified BSD License along with 
 * kmul. If not, see <http://www.gnu.org/licenses/>. 
 */

// Include files <3b>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* dprintf: debugging printf enable by "enable" flag. */
#define dprintf(enable, debug_f, args...) \
if (enable==1) fprintf(debug_f, args);
#define _d_(arg) arg

// Function definitions <3c>, ...
// Function definitions <3c>
#define IS_ODD(c)         ((c) & 1)
#define IS_EVEN(c)        (!IS_ODD(c))
/* Absolute value of an integer. */
#define ABS(x)            ((x) >  0 ? (x) : (-x))

// Constant definitions <10b>
#define HASH_SIZE 2047
// Maximum constant multiplication steps
#define MAX_STEPS   16

// ------------------------------ cut ----------------------------------
// Type definitions <7a>, ...
// Type definitions <7a>
typedef enum
{
  IDENTITY,   /* used for n = 1 */
  NEGATE,     /* used for n = 1 */
  SHIFT_ADD,  /* used for makeOdd(n  1) */
  SHIFT_SUB,  /* used for makeOdd(n + 1) */
  SHIFT_REV,  /* used for makeOdd(1  n) */
  FACTOR_ADD, /* used for n/(2i  1) */
  FACTOR_SUB, /* used for n/(2i + 1) */
  FACTOR_REV  /* used for n/(1  2i) */
} MulOp;

// Type definitions <7c>
typedef struct node
{
  struct node *parent;
  int value;
  double cost;
  MulOp opcode;
  // Other fields in Node <10a>
  struct node *next;
} Node;

// Function definitions <15b>
// The final version of try <12b>
static void do_try(int factor, Node *node, MulOp opcode);
// The final version of find_sequence <12a>
static Node *find_sequence(int c, double limit);
// ---------------------------- end cut --------------------------------

int multiplier_val=1, width_val=32, lo_val=0, hi_val=65535;
int enable_debug=0, enable_errors=0;
int is_signed=0;
int enable_nac=1, enable_ansic=0, enable_gnu89=0, enable_c99=0, enable_cany=0, enable_print=0;
FILE *fout;

// Variable definitions <7b>, ...
// Variable definitions <7b>
double ADD_COST = 1.0, SUB_COST = 1.0, NEG_COST = 1.0, SHIFT_COST = 0.0, MULT_COST = 8.0;
//static unsigned int costs[] =
static double costs[8];
// Variable definitions <10c>
static Node *hash_table[HASH_SIZE];
// Delay cost of the single-constant multiplier (SCM)
double const_mul_cost = 0.0;
// Counter enumerating the number of operations for a const. mult.
int count = 0, kmul_steps = 0;


/* print_spaces:
 * Print a configurable number of space characters to an output file (specified 
 * by the given filename; the file is assumed already opened).
 */
void print_spaces(FILE *f, int nspaces)
{
  int i;  
  for (i = 0; i < nspaces; i++)
  {
    fprintf(f, " ");
  }
}

/* pfprintf: 
 * fprintf prefixed by a number of space characters. 
 */
void pfprintf(FILE *f, int nspaces, char *fmt, ...)
{
  va_list args;
  print_spaces(f, nspaces);
  va_start(args, fmt);
  vfprintf(f, fmt, args);
  va_end(args);
}

/* init_costs_for_mult_const_optimization:
 */
void init_costs_for_mult_const_optimization()
{
//  int i;
  costs[0] = 0.0;                      /* for IDENTITY  */
  costs[1] = NEG_COST;                 /* for NEGATE    */
  costs[2] = SHIFT_COST + ADD_COST;    /* for SHIFTADD  */
  costs[3] = SHIFT_COST + SUB_COST;    /* for SHIFTSUB  */
  costs[4] = SHIFT_COST + SUB_COST;    /* for SHIFTREV  */
  costs[5] = SHIFT_COST + ADD_COST;    /* for FACTORADD */
  costs[6] = SHIFT_COST + SUB_COST;    /* for FACTORSUB */
  costs[7] = SHIFT_COST + SUB_COST;    /* for FACTORREV */
//  costs[5] = 99999;                    /* for FACTORADD */
//  costs[6] = 99999;                    /* for FACTORSUB */
//  costs[7] = 99999;                    /* for FACTORREV */
}

// Function definitions <3d>
static int makeOdd(int c)
{
  do
  {
    c = c / 2;
  } while (IS_EVEN(c));

  return c;
}

// Function definitions <10d>
static Node *lookup(int c)
{
  int hash = ABS(c) % HASH_SIZE;
  Node *node = hash_table[hash];

  while (node && node->value != c)
  {
    node = node->next;
  }

  // Create a new Node and add it to hash_table[hash] <10e>
  if (!node)
  {
    // Create and initialize node <8b>
    node = (Node *) malloc(sizeof(Node));
    node->value = c;
    //node->parent = NULL;

    node->next = hash_table[hash];
    hash_table[hash] = node;
    // Create and initialize node <12c>
    //node->cost = SHIFT_COST + 1;
    node->cost = SHIFT_COST;
  }

  return node;
}

// The final version of find_sequence <12a>
static Node *find_sequence(int c, double limit)
{
  Node *node = lookup(c);

  if (!node->parent && node->cost < limit)
  {
    node->cost = limit;

    // Handle the positive case <9a>
    if (c > 0)
    {
      int power = 4;
      int edge = c >> 1;

      while (power < edge)
      {
      	if (c % (power - 1) == 0)
        {
          do_try(c / (power - 1), node, FACTOR_SUB);
        }
        if (c % (power + 1) == 0)
        {
          do_try(c / (power + 1), node, FACTOR_ADD);
        }
        power = power << 1;
      }
      do_try(makeOdd(c - 1), node, SHIFT_ADD);
      do_try(makeOdd(c + 1), node, SHIFT_SUB);
    }
    // Handle the negative case <9b>
    else
    {
      int power = 4;
      int edge = (-c) >> 1;

      while (power < edge)
      {
      	if (c % (1 - power) == 0)
        {
          do_try(c / (1 - power), node, FACTOR_REV);
        }
        if (c % (power + 1) == 0)
        {
          do_try(c / (power + 1), node, FACTOR_ADD);
        }
        power = power << 1;
      }
      do_try(makeOdd(1 - c), node, SHIFT_REV);
      do_try(makeOdd(c + 1), node, SHIFT_SUB);
    }
  }

  return node;
}

// The final version of try <12b>
static void do_try(int factor, Node *node, MulOp opcode)
{
  double cost = costs[opcode];
  double limit = node->cost - cost;
  Node *factor_node = find_sequence(factor, limit);

  if (factor_node->parent && factor_node->cost < limit)
  {
    node->parent = factor_node;
    node->opcode = opcode;
    node->cost = factor_node->cost + cost;
  }
}

// Function definitions <13a>
static void emit_shift(FILE *f, int target, int source)
{
  int temp = source;
  unsigned int i = 0;

  do
  {
    temp <<= 1;
    i++;
  } while (target != temp);

  dprintf(enable_debug, stdout, "Info: %d = %d << %d\n", target, source, i);
  if (enable_nac == 1 && enable_print == 1)
  {
    pfprintf(f, 2, "t%d <= shl t%d, %d;\n", count+1, count, i);   
  }
  else if (enable_cany == 1 && enable_print == 1)
  {
    pfprintf(f, 2, "t%d = t%d << %d;\n", count+1, count, i);   
  }      
  count++;
}

// Function definitions <13b>
static int emit_code(FILE *f, Node *node)
{
  int source;
  int target = node->value;
  switch (node->opcode)
  {
    // Opcode cases <13c>
    case IDENTITY:
      break;
    // Opcode cases <13d>
    case NEGATE:
      source = emit_code(f, node->parent);
      dprintf(enable_debug, stdout, "Info: %d = 0 - %d\n", target, source);
      if (enable_nac == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d <= neg t%d;\n", count+1, count);   
      }
      else if (enable_cany == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d = -t%d;\n", count+1, count);   
      }
      count++;
      break;
    // Opcode cases <14a>
    case SHIFT_ADD:
      source = emit_code(f, node->parent);
      emit_shift(f, target-1, source);
      dprintf(enable_debug, stdout, "Info: %d = %d + 1\n", target, target-1);
      if (enable_nac == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d <= add t%d, x;\n", count+1, count);   
      }
      else if (enable_cany == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d = t%d + x;\n", count+1, count);   
      }      
      count++;
      break;
    // Opcode cases <14b>
    case SHIFT_SUB:
      source = emit_code(f, node->parent);
      emit_shift(f, target+1, source);
      dprintf(enable_debug, stdout, "Info: %d = %d - 1\n", target, target+1);
      if (enable_nac == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d <= sub t%d, x;\n", count+1, count);   
      }
      else if (enable_cany == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d = t%d - x;\n", count+1, count);   
      }      
      count++;
      break;
    // Opcode cases <14c>
    case SHIFT_REV:
      source = emit_code(f, node->parent);
      emit_shift(f, 1-target, source);
      dprintf(enable_debug, stdout, "Info: %d = 1 - %d\n", target, 1-target);
      if (enable_nac == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d <= sub x, t%d;\n", count+1, count);   
      }
      else if (enable_cany == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d = x - t%d;\n", count+1, count);   
      }      
      count++;
      break;
    // Opcode cases <14d>
    case FACTOR_ADD:
      source = emit_code(f, node->parent);
      emit_shift(f, target-source, source);
      dprintf(enable_debug, stdout, "Info: %d = %d + %d\n", target, target-source, source);
      if (enable_nac == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d <= add t%d, t%d;\n", count+1, count, count-1);   
      }
      else if (enable_cany == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d <= t%d + t%d;\n", count+1, count, count-1);   
      }      
      count++;      
      break;
    // Opcode cases <14e>
    case FACTOR_SUB:
      source = emit_code(f, node->parent);
      emit_shift(f, target+source, source);
      dprintf(enable_debug, stdout, "Info: %d = %d - %d\n", target, target+source, source);
      if (enable_nac == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d <= sub t%d, t%d;\n", count+1, count, count-1);   
      }
      else if (enable_cany == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d <= t%d - t%d;\n", count+1, count, count-1);   
      }      
      count++;      
      break;
    // Opcode cases <15a>
    case FACTOR_REV:
      source = emit_code(f, node->parent);
      emit_shift(f, source-target, source);
      dprintf(enable_debug, stdout, "Info: %d = %d - %d\n", target, source, source-target);
      // FIXME: ??? Needs testing.
      if (enable_nac == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d <= sub t%d, t%d;\n", count+1, count-1, count);   
      }
      else if (enable_cany == 1 && enable_print == 1)
      {
        pfprintf(f, 2, "t%d <= t%d - t%d;\n", count+1, count-1, count);   
      }      
      count++;            
      break;
  }
  return target;
}

// Function definitions <15c>
static double estimate_cost(int c)
{
  // Multiplication cost for the multiplier unit
  return (MULT_COST);
}

// Function definitions <15d>
void multiply(int target)
{
  double multiply_cost = estimate_cost(target);

  // Handle the (straightforward) odd case <16a>
  if (IS_ODD(target))
  {
    Node *result = find_sequence(target, multiply_cost);

    if (result->parent && result->cost < multiply_cost)
    {
      emit_code(fout, result);
      const_mul_cost = result->cost;
    }
    else
    {
      const_mul_cost = multiply_cost;
    }
  }
  // Handle the (relatively complex) even case <16b>
  else
  {
    Node *result = find_sequence(makeOdd(target), multiply_cost - SHIFT_COST);

    if (result->parent && result->cost + SHIFT_COST < multiply_cost)
    {
      int source = emit_code(fout, result);
      emit_shift(fout, target, source);
      const_mul_cost = result->cost;
    }
    else
    {
      const_mul_cost = multiply_cost;
    }
  }
}

// Function definitions <16c>
void init_multiply(void)
{
  Node *node, *node1;
  unsigned int i;
  
//  dprintf(enable_debug, stdout, "Info: Initializing for constant multiplication evaluation.\n");
  for (i = 0; i < HASH_SIZE; i++)
  {
    hash_table[i] = NULL;
  }
  init_costs_for_mult_const_optimization();
  node1 = lookup(1);
  node1->parent = node1;    // must not be NULL
  node1->opcode = IDENTITY;
  node1->cost = 0;
  node = lookup(-1);
  node->parent = node1;
  node->opcode = NEGATE;
  node->cost = NEG_COST;
}

/* emit_kmul_nac:
 * Emit the NAC (generic assembly language) implementation of unsigned/signed
 * multiplication by constant. Calls "multiply" which in turn recursively calls
 * "emit_code".
 */
void emit_kmul_nac(FILE *f, int m, int s, unsigned int W)
{ 
  int i;  
  char c = ((s) ? 's' : 'u');
   
  pfprintf(f, 0, "procedure kmul_%c%d_%c_%d (in %c%d x, out %c%d y)\n", 
      c, W, ((m > 0) ? 'p' : 'm'), ABS(m), c, W, c, W);
  pfprintf(f, 0, "{\n");   
  if (m == 0)
  {
    pfprintf(f, 2, "localvar %c%d t;\n", c, W);   
  }
  else
  {
    for (i = 0; i < MAX_STEPS; i++)
    {
      pfprintf(f, 2, "localvar %c%d t%d;\n", c, W, i);   
    }
  }
  pfprintf(f, 0, "S_1:\n");
  
  enable_print = 1;
  count = 0;
  // Apply constant multiplication optimization.
  if (m == 0)
  {
    pfprintf(f, 2, "t <= ldc 0;\n");
  }
  else
  {
    pfprintf(f, 2, "t%d <= mov x;\n", count);
    if (m != 0)
    {
      init_multiply();
      multiply((int)m);  
    }
  }
  if (m == 0)
  {
    pfprintf(f, 2, "y <= mov t;\n");   
  }
  else
  {
    pfprintf(f, 2, "y <= mov t%d;\n", count);
  }
  pfprintf(f, 0, "}\n");   
}

/* set_data_width:
 * For a given data width, set the effective data width for the corresponding integer 
 * C data type.
 */
unsigned int set_data_width(unsigned int W)
{
  unsigned int effective_width = 0;

  if (W > 0 && W <= 8)
  {
    effective_width = 8;
  }
  else if (W > 8 && W <= 16)
  {
    effective_width = 16;
  }
  else if (W > 16 && W <= 32)
  {
    effective_width = 32;
  }
  else if ((W > 32 && W <= 64) && !enable_ansic)
  {
    effective_width = 64;
  }
  if ((W > 32 && enable_ansic == 1) || (W > 64))
  {
    fprintf(stderr, "Error: Data widths higher than %d bits are not supported.\n", (enable_ansic ? 32 : 64));
    exit (1);
  }

  return (effective_width);
}

/* get_c_type:
 * For the given signedness (s) and data width (W) return the corresponding 
 * C data type for either ANSI C, GNU89 (ANSI C with GNU extensions) or C99.
 */                       
char *get_c_type(int s, unsigned int W)
{
  char *c_type_str = malloc((strlen("unsigned long long int")+1) * sizeof(char));
  int offset = 0;

  if (enable_c99 == 1)
  {
    if (s == 0)
    {
      strcpy(c_type_str, "u");
      offset = 1;
    }
    sprintf(c_type_str+offset, "int%u_t", set_data_width(W));
  }
  else if (enable_ansic == 1 || enable_gnu89 == 1)
  {
    if (s == 0)
    {
      strcpy(c_type_str, "unsigned ");
    }
    if (set_data_width(W) == 8)
    {
      strcat(c_type_str, "char");
    } 
    else if (set_data_width(W) == 16)
    { 
      strcat(c_type_str, "short");
    }
    else if (set_data_width(W) == 32)
    {
      strcat(c_type_str, "long");
    }
  }
  if (set_data_width(W) == 64 && enable_gnu89 == 1)
  {
    strcat(c_type_str, "long long");
  }
  if ((W > 32 && enable_ansic == 1) || (W > 64))
  {
    fprintf(stderr, "Error: Data widths higher than %d bits are not supported.\n", (enable_ansic ? 32 : 64));
    exit (1);
  }

  return (c_type_str);
}

/* emit_kmul_cany:
 * Emit the ANSI C, GNU89 or C99 implementation of unsigned/signed multiplication by 
 * constant.
 */                       
void emit_kmul_cany(FILE *f, int m, int s, unsigned int W)
{
  int i;  
  char c = ((s) ? 's' : 'u');
  char *dt;

  if (enable_c99 == 1)
  {
    pfprintf(f, 0, "#include <stdint.h>\n");
  }
  
  dt = get_c_type(s, W);  
 
  pfprintf(f, 0, "%s kmul_%c%d_%c_%d (%s x)\n", dt, c, W, ((m > 0) ? 'p' : 'm'), ABS(m), dt);
  pfprintf(f, 0, "{\n");   
  if (m == 0)
  {
    pfprintf(f, 2, "%s t;\n", dt);   
  }
  else
  {
    for (i = 0; i < MAX_STEPS; i++)
    {
      pfprintf(f, 2, "%s t%d;\n", dt, i);   
    }
  }
  pfprintf(f, 2, "%s y;\n", dt);   
 
  enable_print = 1;
  count = 0;
  // Apply constant multiplication optimization.
  if (m == 0)
  {
    pfprintf(f, 2, "t = 0;\n");
  }
  else
  {
    pfprintf(f, 2, "t%d = x;\n", count);
    if (m != 0)
    {
      init_multiply();
      multiply((int)m);  
    }
  }
  if (m == 0)
  {
    pfprintf(f, 2, "y = t;\n");   
  }
  else
  {
    pfprintf(f, 2, "y = t%d;\n", count);
  }
  pfprintf(f, 2, "return (y);\n");
  pfprintf(f, 0, "}\n");   

  free(dt);
}

/* print_usage:
 * Print usage instructions for the "kmul" program.
 */
static void print_usage()
{
  printf("\n");
  printf("* Usage:\n");
  printf("* ./kmul.exe [options]\n");
  printf("* \n");
  printf("* Options:\n");
  printf("* \n");
  printf("*   -h:\n");
  printf("*         Print this help.\n");
  printf("*   -d:\n");
  printf("*         Enable debug/diagnostic output.\n");
  printf("*   -mul <num>:\n");
  printf("*         Set the value of the multiplier. Default: 1.\n");
  printf("*   -width <num>:\n");
  printf("*         Set the bitwidth of all operands: multiplier, multiplicand and\n");
  printf("*         product. Default: 32.\n");
  printf("*   -signed:\n");
  printf("*         Construct optimized routine for signed multiplication.\n");
  printf("*   -unsigned:\n");
  printf("*         Construct optimized routine for unsigned multiplication (default).\n");
  printf("*   -nac:\n");
  printf("*         Emit software routine in the NAC general assembly language (default).\n");
  printf("*   -ansic:\n");
  printf("*         Emit software routine in ANSI C (for widths up to 32 bits).\n");
  printf("*   -gnu89:\n");
  printf("*         Emit software routine in ANSI C with GNU extensions (for widths\n");
  printf("*         up to 64 bits).\n");
  printf("*   -c99:\n");
  printf("*         Emit software routine in C99 (for widths up to 64 bits).\n");
  printf("* \n");
  printf("* For further information, please refer to the website:\n");
  printf("* http://www.nkavvadias.com\n");
}

/* main:
 * Program entry.
 */
int main(int argc, char *argv[]) 
{
   int i;
   char *fout_name, suffix[4], ch='X', datatype[8];

   // If no arguments are passed, exit with help
   if (argc == 1)
   {
     print_usage();
     exit(1);
   }
  
  // Read input arguments
  for (i = 1; i < argc; i++)
  {
    if (strcmp("-h", argv[i]) == 0)
    {
      print_usage();
      exit(1);
    }
    else if (strcmp("-d", argv[i]) == 0)
    {
      enable_debug = 1;
    }
    else if (strcmp("-unsigned", argv[i]) == 0)
    {
      is_signed = 0;
    }
    else if (strcmp("-signed", argv[i]) == 0)
    {
      is_signed = 1;
    }
    else if (strcmp("-nac", argv[i]) == 0)
    {
      enable_nac   = 1;
      enable_ansic = 0;
      enable_c99   = 0;
    }
    else if (strcmp("-ansic", argv[i]) == 0)
    {
      enable_nac   = 0;
      enable_ansic = 1;
      enable_gnu89 = 0;
      enable_c99   = 0;
    }
    else if (strcmp("-gnu89", argv[i]) == 0)
    {
      enable_nac   = 0;
      enable_ansic = 0;
      enable_gnu89 = 1;
      enable_c99   = 0;
    }
    else if (strcmp("-c99", argv[i]) == 0)
    {
      enable_nac   = 0;
      enable_ansic = 0;
      enable_gnu89 = 0;
      enable_c99   = 1;
    }
    else if (strcmp("-mul",argv[i]) == 0)
    {
      if ((i+1) < argc)
      {
        i++;
        if (argv[i][0] == '-')
        {
          multiplier_val = -atoi(argv[i]+1);
        }
        else
        {
          multiplier_val = atoi(argv[i]);
        }
      }
    }    
    else if (strcmp("-width",argv[i]) == 0)
    {
      if ((i+1) < argc)
      {
        i++;
        width_val = atoi(argv[i]);
      }
    }    
    else
    {
      if (argv[i][0] != '-')
      {
        print_usage();
        exit(1);
      }
    }
  }
  
  if ((is_signed == 0) && (multiplier_val < 0))
  {
    fprintf(stderr, "Error: Multiplier must be positive for unsigned multiplication.\n");
    exit(1);
  }

  /* Any C standard enabled */
  enable_cany = enable_ansic || enable_gnu89 || enable_c99;

  fout_name = malloc(25 * sizeof(char));
  if (enable_nac == 1)
  {
    strcpy(suffix, "nac");
  }
  else if (enable_cany == 1)
  {
    strcpy(suffix, "c");
  }
  ch = (is_signed == 0) ? 'u' : 's';
  sprintf(datatype, "%c%d", ch, width_val);
  if (multiplier_val >= 0)
  {
    sprintf(fout_name, "kmul_%s_p_%d.%s", datatype, multiplier_val, suffix);
  }
  else
  {
    sprintf(fout_name, "kmul_%s_m_%d.%s", datatype, ABS(multiplier_val), suffix);
  }
  fout = fopen(fout_name, "w");

  if (enable_nac == 1)
  {
    emit_kmul_nac(fout, multiplier_val, is_signed, width_val);
  }
  else if (enable_cany == 1)
  {
    emit_kmul_cany(fout, multiplier_val, is_signed, width_val);
  }

  free(fout_name);
  fclose(fout);

  return 0;
}
