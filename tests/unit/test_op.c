#include "validate_testing.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jvst_macros.h"

#include "validate_op.h"
#include "validate_ir.h"

enum op_test_type {
  STOP = -1,
  NONE =  0,
  ASSEMBLE,
  ENCODE,
};

struct op_test {
  enum op_test_type type;
  struct jvst_cnode *ctree;
  struct jvst_ir_stmt *ir;
  struct jvst_op_program *prog;
  struct jvst_vm_program *vmprog;
};

static int
op_progs_equal(const char *fname, struct jvst_op_program *p1, struct jvst_op_program *p2);

static int
op_encodings_equal(const char *fname, struct jvst_vm_program *p1, struct jvst_vm_program *p2);

static int
run_test(const char *fname, const struct op_test *t)
{
  struct jvst_cnode *simplified, *canonified;
  struct jvst_ir_stmt *translated, *linearized, *flattened;
  struct jvst_op_program *assembled;
  struct jvst_vm_program *encoded;
  int ret;

  assert(t->ctree != NULL);
  assert(t->ir != NULL);
  assert(t->prog != NULL);

  assert(t->ir != NULL);

  switch (t->type) {
  case NONE:
    nskipped++;
    return 1;

  case ASSEMBLE:
    simplified = jvst_cnode_simplify(t->ctree);
    canonified = jvst_cnode_canonify(simplified);
    translated = jvst_ir_translate(canonified);
    linearized = jvst_ir_linearize(translated);
    flattened  = jvst_ir_flatten(linearized);

    // jvst_ir_print(ir);
    assembled = jvst_op_assemble(flattened);

    // fprintf(stderr, "\n");
    // jvst_op_print(result);

    ret = op_progs_equal(fname, assembled, t->prog);
    if (!ret) {
      // jvst_ir_print(ir);
    }

    return ret;

  case ENCODE:
    simplified = jvst_cnode_simplify(t->ctree);
    canonified = jvst_cnode_canonify(simplified);
    translated = jvst_ir_translate(canonified);
    linearized = jvst_ir_linearize(translated);
    flattened  = jvst_ir_flatten(linearized);

    // jvst_ir_print(ir);
    assembled = jvst_op_assemble(flattened);
    encoded   = jvst_op_encode(assembled);

    // fprintf(stderr, "\n");
    // jvst_op_print(result);

    ret = op_encodings_equal(fname, encoded, t->vmprog);
    if (!ret) {
      // jvst_ir_print(ir);
    }

    jvst_vm_program_free(encoded);

    return ret;

  case STOP:
    break;
  }

  fprintf(stderr, "%s:%d (%s) should not reach\n", __FILE__, __LINE__, __func__);
  abort();
  return 0;
}

static void report_prog_diff(const char *fname, enum op_test_type type, const char *buf1, const char *buf2, size_t n1, size_t n2)
{
  size_t n;
  size_t i, linenum, beg, off;
  const char *pname;

  switch (type) {
  case ASSEMBLE:
    pname = "op";
    break;

  case ENCODE:
    pname = "vm";
    break;

  default:
    fprintf(stderr, "%s:%d (%s) should not reach\n", 
        __FILE__, __LINE__, __func__);
    abort();
  }

  fprintf(stderr, "test %s %s programs are not equal:\n", fname, pname);
  {
    size_t i,n,l;

    fprintf(stderr, "Expected program:\n");
    print_buffer_with_lines(stderr, buf2, n2);
    fprintf(stderr, "\n");

    fprintf(stderr, "Actual program:\n");
    print_buffer_with_lines(stderr, buf1, n1);
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n\n");

  buffer_diff(stderr, buf1, buf2, n1);
}

// n1 is actual, n2 is expected
static int
op_progs_equal(const char *fname, struct jvst_op_program *p1, struct jvst_op_program *p2)
{
  size_t n;
  int ret, failed;
  static char buf1[65536];
  static char buf2[65536];
  size_t i, linenum, beg, off;

  STATIC_ASSERT(sizeof buf1 == sizeof buf2, buffer_size_not_equal);

  memset(buf1, 0, sizeof buf1);
  memset(buf2, 0, sizeof buf2);

  // kind of dumb but mostly reliable way to do deep equals...  generate
  // text dumps and compare
  // 
  // XXX - replace with an actual comparison
  if (jvst_op_dump(p1, buf1, sizeof buf1) != 0) {
    fprintf(stderr, "buffer for program 1 not large enough (currently %zu bytes)\n",
        sizeof buf1);
  }

  if (jvst_op_dump(p2, buf2, sizeof buf2) != 0) {
    fprintf(stderr, "buffer for program 2 not large enough (currently %zu bytes)\n",
        sizeof buf2);
  }

  if (strncmp(buf1, buf2, sizeof buf1) == 0) {
    // fprintf(stderr, "TREE:\n%s\n", buf1);
    return 1;
  }

  report_prog_diff(fname, ASSEMBLE, buf1, buf2, sizeof buf1, sizeof buf2);

  return 0;
}

static int
op_encodings_equal(const char *fname, struct jvst_vm_program *p1, struct jvst_vm_program *p2)
{
  size_t n;
  int ret, failed;
  static char buf1[65536];
  static char buf2[65536];
  size_t i, linenum, beg, off;

  STATIC_ASSERT(sizeof buf1 == sizeof buf2, buffer_size_not_equal);

  memset(buf1, 0, sizeof buf1);
  memset(buf2, 0, sizeof buf2);

  // kind of dumb but mostly reliable way to do deep equals...  generate
  // text dumps and compare
  // 
  // XXX - replace with an actual comparison
  if (jvst_vm_program_dump(p1, buf1, sizeof buf1) != 0) {
    fprintf(stderr, "buffer for program 1 not large enough (currently %zu bytes)\n",
        sizeof buf1);
  }

  if (jvst_vm_program_dump(p2, buf2, sizeof buf2) != 0) {
    fprintf(stderr, "buffer for program 2 not large enough (currently %zu bytes)\n",
        sizeof buf2);
  }

  if (strncmp(buf1, buf2, sizeof buf1) == 0) {
    // fprintf(stderr, "TREE:\n%s\n", buf1);
    return 1;
  }

  report_prog_diff(fname, ENCODE, buf1, buf2, sizeof buf1, sizeof buf2);

  return 0;
}

#define UNIMPLEMENTED(testlist) do{ nskipped++; (void)testlist; }while(0)
#define RUNTESTS(testlist) runtests(__func__, (testlist))
static void runtests(const char *testname, const struct op_test tests[])
{
  int i;

  (void)testname;

  for (i=0; tests[i].type != STOP; i++) {
    ntest++;

    if (!run_test(testname, &tests[i])) {
      printf("%s[%d]: failed\n", testname, i+1);
      nfail++;
    }
  }
}

static void
test_op_empty_schema(void)
{
  struct arena_info A = {0};

  const struct op_test tests[] = {
    {
      ASSEMBLE,
      newcnode_switch(&A, 1, SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_3"),

            oplabel, "false_4",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_3"),

            oplabel, "false_7",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_8",
            newop_return(&A, 0),

            oplabel, "invalid_1_3",
            newop_return(&A, 1),

            NULL
          ),

          NULL
      ),
    },

    {
      ENCODE,
      newcnode_switch(&A, 1, SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_3"),

            oplabel, "false_4",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_3"),

            oplabel, "false_7",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_8",
            newop_return(&A, 0),

            oplabel, "invalid_1_3",
            newop_return(&A, 1),

            NULL
          ),

          NULL
      ),

      newvm_program(&A,
          JVST_OP_PROC, VMLIT(0), VMLIT(0),
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_3",
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_ARRAY_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_3",
          JVST_OP_CONSUME, 0, 0,
          JVST_OP_RETURN, 0, 0,
          VM_LABEL, "invalid_1_3",
          JVST_OP_RETURN, VMLIT(1), 0,
          VM_END)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_op_type_constraints(void)
{
  struct arena_info A = {0};

  const struct op_test tests[] = {
    {
      ASSEMBLE,
      newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_seq(&A,
              newir_stmt(&A, JVST_IR_STMT_CONSUME),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

    },

    {
      ASSEMBLE,
      newcnode_switch(&A, 0, SJP_OBJECT_BEG, newcnode_valid(), SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_stmt(&A, JVST_IR_STMT_CONSUME),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

    },

    {
      ASSEMBLE,
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_valid(),
        SJP_STRING, newcnode_valid(),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_STRING),
            newir_seq(&A,
              newir_stmt(&A, JVST_IR_STMT_CONSUME),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),
            newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
            )
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_STRING)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "false_4",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "true_6"),

            oplabel, "invalid_1_8",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            oplabel, "true_6",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "valid_3"),

            NULL
          ),

          NULL
      ),

    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_op_type_integer(void)
{
  struct arena_info A = {0};
  struct ast_schema schema = {
    .types = JSON_VALUE_INTEGER,
  };

  const struct op_test tests[] = {
    {
      ASSEMBLE,
      newcnode_switch(&A, 0,
        SJP_NUMBER,
        newcnode(&A,JVST_CNODE_NUM_INTEGER),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A, newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),
              newir_invalid(&A, JVST_INVALID_NOT_INTEGER, "number is not an integer")),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_9",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_cmp(&A, JVST_OP_FINT, oparg_tnum(), oparg_none()),
            newop_br(&A, JVST_VM_BR_NE, "true_4"),

            oplabel, "invalid_2_7",
            newop_return(&A, 2),

            oplabel, "true_4",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

    },

    {
      ENCODE,
      newcnode_switch(&A, 0,
        SJP_NUMBER,
        newcnode(&A,JVST_CNODE_NUM_INTEGER),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A, newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              newir_invalid(&A, JVST_INVALID_NOT_INTEGER, "number is not an integer")),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_9",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_cmp(&A, JVST_OP_FINT, oparg_tnum(), oparg_none()),
            newop_br(&A, JVST_VM_BR_NE, "valid_5"),

            oplabel, "invalid_2_7",
            newop_return(&A, 2),

            oplabel, "true_4",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

      newvm_program(&A,
          JVST_OP_PROC, VMLIT(0), VMLIT(0),
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_NUMBER),
          JVST_OP_JMP, JVST_VM_BR_EQ, "true_2",
          JVST_OP_RETURN, VMLIT(1), 0,

          VM_LABEL, "true_2",
          JVST_OP_FINT, VMREG(JVST_VM_TNUM), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_NE, "true_4",

          JVST_OP_RETURN, VMLIT(2), 0,

          VM_LABEL, "true_4",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_5",
          JVST_OP_RETURN, 0, 0,
          VM_END)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_op_minimum(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "minimum", 1.1,
      NULL);

  const struct op_test tests[] = {
    {
      ASSEMBLE,
      newcnode_switch(&A, 0,
          SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
          SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A,
              newir_op(&A, JVST_IR_EXPR_GE, 
                newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                newir_num(&A, 1.1)),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),
              newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      ),

      newop_program(&A,
          opfloat, 1.1,

          newop_proc(&A,
            opslots, 2,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_9",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_tnum()),
            newop_load(&A, JVST_OP_FLOAD, oparg_slot(0), oparg_lit(0)),
            newop_cmp(&A, JVST_OP_FCMP, oparg_slot(1), oparg_slot(0)),
            newop_br(&A, JVST_VM_BR_GE, "true_4"),

            oplabel, "invalid_3_7",
            newop_return(&A, 3),

            oplabel, "true_4",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

    },

    {
      ENCODE,
      newcnode_switch(&A, 0,
          SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
          SJP_NONE),

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
              2, "true",
              9, "invalid_1"
            ),
            NULL
          ),

          newir_block(&A, 9, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_move(&A, newir_ftemp(&A, 1), newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
            newir_move(&A, newir_ftemp(&A, 0), newir_num(&A, 1.1)),
            newir_cbranch(&A,
              newir_op(&A, JVST_IR_EXPR_GE, 
                newir_ftemp(&A, 1),
                newir_ftemp(&A, 0)
              ),
              4, "true",
              7, "invalid_3"
            ),
            NULL
          ),

          newir_block(&A, 7, "invalid_3",
            newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid"),
            NULL
          ),

          newir_block(&A, 4, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 5, "valid"),
            NULL
          ),

          newir_block(&A, 5, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),

        NULL
      ),

      newop_program(&A,
          opfloat, 1.1,

          newop_proc(&A,
            opslots, 2,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_9",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_tnum()),
            newop_load(&A, JVST_OP_FLOAD, oparg_slot(0), oparg_lit(0)),
            newop_cmp(&A, JVST_OP_FCMP, oparg_slot(1), oparg_slot(0)),
            newop_br(&A, JVST_VM_BR_GE, "true_4"),

            oplabel, "invalid_3_7",
            newop_return(&A, 3),

            oplabel, "true_4",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

      newvm_program(&A,
          VM_FLOATS, 1, 1.1,

          JVST_OP_PROC, VMLIT(2), VMLIT(0),
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_NUMBER),
          JVST_OP_JMP, JVST_VM_BR_EQ, "true_2",
          JVST_OP_RETURN, VMLIT(1), 0,

          VM_LABEL, "true_2",
          JVST_OP_MOVE, VMSLOT(1), VMREG(JVST_VM_TNUM),
          JVST_OP_FLOAD, VMSLOT(0), VMLIT(0),
          JVST_OP_FCMP, VMSLOT(1), VMSLOT(0),
          JVST_OP_JMP, JVST_VM_BR_GE, "true_4",

          JVST_OP_RETURN, VMLIT(3), 0,

          VM_LABEL, "true_4",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_5",
          JVST_OP_RETURN, 0, 0,
          VM_END)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_op_multiple_of(void)
{
  struct arena_info A = {0};

  const struct op_test tests[] = {
    {
      ASSEMBLE,
      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_multiple_of(&A, 1.5),
          SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A, newir_multiple_of(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM), 1.5),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),
              newir_invalid(&A, JVST_INVALID_NOT_MULTIPLE, "number is not an integer multiple")
            ),
            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          opfloat, 1.5,

          newop_proc(&A,
            opslots, 1,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "false_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_15",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            oplabel, "true_2",
            newop_load(&A, JVST_OP_FLOAD, oparg_slot(0), oparg_lit(0)),
            newop_cmp(&A, JVST_OP_FINT, oparg_tnum(), oparg_slot(0)),
            newop_br(&A, JVST_VM_BR_NE, "true_4"),

            oplabel, "invalid_17_7",
            newop_return(&A, 17),

            oplabel, "true_4",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "valid_5"),

            oplabel, "invalid_1_11",
            newop_return(&A, 1),

            NULL
          ),

          NULL
      ),

    },

    {
      ENCODE,
      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_multiple_of(&A, 1.5),
          SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A, newir_multiple_of(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM), 1.5),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),
              newir_invalid(&A, JVST_INVALID_NOT_MULTIPLE, "number is not an integer multiple")
            ),
            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          opfloat, 1.5,

          newop_proc(&A,
            opslots, 1,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "false_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_15",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            oplabel, "true_2",
            newop_load(&A, JVST_OP_FLOAD, oparg_slot(0), oparg_lit(0)),
            newop_cmp(&A, JVST_OP_FINT, oparg_tnum(), oparg_slot(0)),
            newop_br(&A, JVST_VM_BR_NE, "true_4"),

            oplabel, "invalid_17_7",
            newop_return(&A, 17),

            oplabel, "true_4",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "valid_5"),

            oplabel, "invalid_1_11",
            newop_return(&A, 1),

            NULL
          ),

          NULL
      ),

      newvm_program(&A,
          VM_FLOATS, 1, 1.5,

          JVST_OP_PROC, VMLIT(1), VMLIT(0),
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_NUMBER),
          JVST_OP_JMP, JVST_VM_BR_EQ, "true_2",

          VM_LABEL, "false_8",
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_11",

          VM_LABEL, "false_12",
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_ARRAY_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_11",

          VM_LABEL, "false_15",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_5",
          JVST_OP_RETURN, 0, 0,

          VM_LABEL, "true_2",
          JVST_OP_FLOAD, VMSLOT(0), VMLIT(0),
          JVST_OP_FINT, VMREG(JVST_VM_TNUM), VMSLOT(0),
          JVST_OP_JMP, JVST_VM_BR_NE, "true_4",

          JVST_OP_RETURN, VMLIT(17), 0,

          VM_LABEL, "true_4",
          JVST_OP_CONSUME, 0, 0,
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "valid_5",

          VM_LABEL, "invalid_1_11",
          JVST_OP_RETURN, VMLIT(1), 0,

          VM_END)
    },

    {
      ASSEMBLE,
      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_multiple_of(&A, 3.0),
          SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A, newir_multiple_of(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM), 3.0),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),
              newir_invalid(&A, JVST_INVALID_NOT_MULTIPLE, "number is not an integer multiple")
            ),
            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "false_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_15",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            oplabel, "true_2",
            newop_cmp(&A, JVST_OP_FINT, oparg_tnum(), oparg_lit(3)),
            newop_br(&A, JVST_VM_BR_NE, "true_4"),

            oplabel, "invalid_17_7",
            newop_return(&A, 17),

            oplabel, "true_4",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "valid_5"),

            oplabel, "invalid_1_11",
            newop_return(&A, 1),

            NULL
          ),

          NULL
      ),

    },

    {
      ENCODE,
      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_multiple_of(&A, 3.0),
          SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A, newir_multiple_of(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM), 3.0),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),
              newir_invalid(&A, JVST_INVALID_NOT_MULTIPLE, "number is not an integer multiple")
            ),
            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "false_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_15",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            oplabel, "true_2",
            newop_load(&A, JVST_OP_FLOAD, oparg_slot(0), oparg_lit(0)),
            newop_cmp(&A, JVST_OP_FINT, oparg_tnum(), oparg_slot(0)),
            newop_br(&A, JVST_VM_BR_EQ, "true_4"),

            oplabel, "invalid_17_7",
            newop_return(&A, 17),

            oplabel, "true_4",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "valid_5"),

            oplabel, "invalid_1_11",
            newop_return(&A, 1),

            NULL
          ),

          NULL
      ),

      newvm_program(&A,
          JVST_OP_PROC, VMLIT(0), VMLIT(0),
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_NUMBER),
          JVST_OP_JMP, JVST_VM_BR_EQ, "true_2",

          VM_LABEL, "false_8",
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_11",

          VM_LABEL, "false_12",
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_ARRAY_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_11",

          VM_LABEL, "false_15",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_5",
          JVST_OP_RETURN, 0, 0,

          VM_LABEL, "true_2",
          JVST_OP_FINT, VMREG(JVST_VM_TNUM), VMLIT(3),
          JVST_OP_JMP, JVST_VM_BR_NE, "true_4",

          JVST_OP_RETURN, VMLIT(17), 0,

          VM_LABEL, "true_4",
          JVST_OP_CONSUME, 0, 0,
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "valid_5",

          VM_LABEL, "invalid_1_11",
          JVST_OP_RETURN, VMLIT(1), 0,

          VM_END)
    },
    { STOP },
  };

  RUNTESTS(tests);
}

static void test_op_properties(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct op_test tests[] = {
    {
      ASSEMBLE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_propset(&A,
                          newcnode_prop_match(&A, RE_LITERAL, "foo",
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                          newcnode_prop_match(&A, RE_LITERAL, "bar",
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                    newir_match(&A, 0,
                      // no match
                      newir_case(&A, 0, 
                        NULL,
                        newir_stmt(&A, JVST_IR_STMT_CONSUME)
                      ),

                      // match "bar"
                      newir_case(&A, 1,
                        newmatchset(&A, RE_LITERAL,  "bar", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_STRING),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                          ),
                          NULL
                        )
                      ),

                      // match "foo"
                      newir_case(&A, 2,
                        newmatchset(&A, RE_LITERAL,  "foo", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_NUMBER),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                          ),
                          NULL
                        )
                      ),

                      NULL
                    ),
                    NULL
                  )
                ),
                NULL
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          opdfa, 1,

          newop_proc(&A,
            opslots, 1,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_4"),

            oplabel, "false_16",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_19"),

            oplabel, "false_20",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_19"),

            oplabel, "false_23",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_15",
            newop_return(&A, 0),

            oplabel, "loop_4",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_15"),

            oplabel, "false_7",
            newop_match(&A, 0),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "M_next_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_13"),

            oplabel, "invalid_9_14",
            newop_return(&A, 9),

            oplabel, "M_9",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_11",
            newop_call(&A, oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_13",
            newop_call(&A, oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "invalid_1_19",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_STRING)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

    },

    {
      ENCODE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_propset(&A,
                          newcnode_prop_match(&A, RE_LITERAL, "foo",
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                          newcnode_prop_match(&A, RE_LITERAL, "bar",
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                    newir_match(&A, 0,
                      // no match
                      newir_case(&A, 0, 
                        NULL,
                        newir_stmt(&A, JVST_IR_STMT_CONSUME)
                      ),

                      // match "bar"
                      newir_case(&A, 1,
                        newmatchset(&A, RE_LITERAL,  "bar", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_STRING),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                          ),
                          NULL
                        )
                      ),

                      // match "foo"
                      newir_case(&A, 2,
                        newmatchset(&A, RE_LITERAL,  "foo", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_NUMBER),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                          ),
                          NULL
                        )
                      ),

                      NULL
                    ),
                    NULL
                  )
                ),
                NULL
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          opdfa, 1,

          newop_proc(&A,
            opslots, 1,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_4"),

            oplabel, "false_16",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_19"),

            oplabel, "false_20",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_19"),

            oplabel, "false_23",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_15",
            newop_return(&A, 0),

            oplabel, "loop_4",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_15"),

            oplabel, "false_7",
            newop_match(&A, 0),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "M_next_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_13"),

            oplabel, "invalid_9_14",
            newop_return(&A, 9),

            oplabel, "M_9",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_11",
            newop_call(&A, oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_13",
            newop_call(&A, oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "invalid_1_19",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_STRING)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

      newvm_program(&A,
          VM_DFA, 1,

          JVST_OP_PROC, VMLIT(1), VMLIT(0),
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_BEG),
          JVST_OP_JMP, JVST_VM_BR_EQ, "loop_4",

          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_19",

          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_ARRAY_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_19",

          VM_LABEL, "false_23",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_15",
          JVST_OP_RETURN, 0, 0,

          VM_LABEL, "loop_4",
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "valid_15",

          JVST_OP_MATCH, VMLIT(0), 0,
          JVST_OP_MOVE, VMSLOT(0), VMREG(JVST_VM_M),
          JVST_OP_ICMP, VMSLOT(0), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_EQ, "M_9",

          JVST_OP_ICMP, VMSLOT(0), VMLIT(1),
          JVST_OP_JMP, JVST_VM_BR_EQ, "M_11",

          JVST_OP_ICMP, VMSLOT(0), VMLIT(2),
          JVST_OP_JMP, JVST_VM_BR_EQ, "M_13",

          JVST_OP_RETURN, VMLIT(9), 0,

          VM_LABEL, "M_9",
          JVST_OP_CONSUME, 0, 0,
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_4",

          VM_LABEL, "M_11",
          JVST_OP_CALL, 2,
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_4",

          VM_LABEL, "M_13",
          JVST_OP_CALL, 3,
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_4",

          VM_LABEL, "invalid_1_19",
          JVST_OP_RETURN, VMLIT(1), 0,

          JVST_OP_PROC, VMLIT(0), VMLIT(0),

          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_STRING),
          JVST_OP_JMP, JVST_VM_BR_EQ, "true_2a",

          JVST_OP_RETURN, VMLIT(1), 0,

          VM_LABEL, "true_2a",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_3a",
          JVST_OP_RETURN, 0, 0,

          JVST_OP_PROC, VMLIT(0), VMLIT(0),

          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_NUMBER),
          JVST_OP_JMP, JVST_VM_BR_EQ, "true_2b",

          JVST_OP_RETURN, VMLIT(1), 0,

          VM_LABEL, "true_2b",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_3b",
          JVST_OP_RETURN, 0, 0,

          VM_END)
    },

    {
      ASSEMBLE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_propset(&A,
                          newcnode_prop_match(&A, RE_NATIVE, "ba.",
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                          newcnode_prop_match(&A, RE_LITERAL, "bar",
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                    newir_match(&A, 0,
                      // no match
                      newir_case(&A, 0, 
                        NULL,
                        newir_stmt(&A, JVST_IR_STMT_CONSUME)
                      ),

                      // match "bar" AND "ba."
                      newir_case(&A, 1,
                        newmatchset(&A, RE_LITERAL, "bar", RE_NATIVE,  "ba.", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),   // XXX - is this necessary?
                          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                          NULL
                        )
                      ),

                      // match /ba./
                      newir_case(&A, 2,
                        newmatchset(&A, RE_NATIVE,  "ba.", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_NUMBER),
                            newir_stmt(&A, JVST_IR_STMT_VALID),
                            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                          ),
                          NULL
                        )
                      ),

                      NULL
                    ),
                    NULL
                  )
                ),
                NULL
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_stmt(&A, JVST_IR_STMT_VALID)
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          opdfa, 1,
          newop_proc(&A,
            opslots, 1,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_4"),

            oplabel, "false_16",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_19"),

            oplabel, "false_20",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_19"),

            oplabel, "false_23",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_15",
            newop_return(&A, 0),

            oplabel, "loop_4",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_15"),

            oplabel, "false_7",
            newop_match(&A, 0),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "M_next_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_13"),

            oplabel, "invalid_9_14",
            newop_return(&A, 9),

            oplabel, "M_9",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_11",
            newop_call(&A, oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_13",
            newop_call(&A, oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "invalid_1_19",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),

            oplabel, "invalid_1_1",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_op_minmax_properties_1(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct op_test tests[] = {
    {
      ASSEMBLE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 0, false),
        SJP_NONE),

      // XXX
      // this IR is not as compact as it could be we should be able to
      // short-circuit the loop when we've encountered one property
      // instead of keeping a full count
      newir_frame(&A,
          newir_counter(&A, 0, "num_props"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_incr(&A, 0, "num_props"),
                    // XXX as mentioned above, we could short-circuit
                    //     the loop if num_props >= 1.  This would
                    //     require adding an EAT_OBJECT or similar
                    //     statement to finish the object.
                    //
                    //     The IR might look like:
                    //     IF(GE(COUNT(num_props), 1),
                    //        SEQ(EAT_OBJECT, BREAK("L_OBJ")),
                    //        NOP)
                    //
                    //     In this particular case (at least one prop),
                    //     you could even eliminate the counter and
                    //     matcher and just check if the first token in
                    //     the object is OBJECT_END or not.  Ideally
                    //     that would fall out of some more general
                    //     reasoning and wouldn't need to be
                    //     special-cased.  But if { "minProperties" : 1 }
                    //     is a common constraint, we could just
                    //     special-case it.
                    NULL
                  )
                ),
                NULL
              ),

              // Post-loop check of number of properties
              newir_if(&A,
                  newir_op(&A, JVST_IR_EXPR_GE, 
                    newir_count(&A, 0, "num_props"),
                    newir_size(&A, 1)
                  ),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  newir_invalid(&A, JVST_INVALID_TOO_FEW_PROPS, "too few properties")
              ),
              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            opslots, 4,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_4"),

            oplabel, "false_13",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_16"),

            oplabel, "false_17",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_16"),

            oplabel, "false_20",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_10",
            newop_return(&A, 0),

            oplabel, "loop_4",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_3"),

            oplabel, "false_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_incr(&A, 0),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "loop_end_3",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(3), oparg_slot(1)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_lit(1)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(3), oparg_slot(2)),
            newop_br(&A, JVST_VM_BR_GE, "valid_10"),

            oplabel, "invalid_4_12",
            newop_return(&A, 4),

            oplabel, "invalid_1_16",
            newop_return(&A, 1),

            NULL
          ),

          NULL
      ),
    },

    {
      ASSEMBLE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 0, 2, true),
        SJP_NONE),

      // XXX - comments here are largely the same as in the previous
      //       test case
      newir_frame(&A,
          newir_counter(&A, 0, "num_props"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_incr(&A, 0, "num_props"),
                    // XXX as mentioned above, we could short-circuit
                    //     the loop if num_props >= 2.
                    //
                    //     The IR might look like:
                    //     IF(GT(COUNT(num_props), 2),
                    //        INVALID,
                    //        NOP)
                    NULL
                  )
                ),
                NULL
              ),

              // Post-loop check of number of properties
              newir_if(&A,
                  newir_op(&A, JVST_IR_EXPR_LE, 
                    newir_count(&A, 0, "num_props"),
                    newir_size(&A, 2)
                  ),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  newir_invalid(&A, JVST_INVALID_TOO_MANY_PROPS, "too many properties")
              ),
              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            opslots, 4,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_4"),

            oplabel, "false_13",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_16"),

            oplabel, "false_17",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_16"),

            oplabel, "false_20",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_10",
            newop_return(&A, 0),

            oplabel, "loop_4",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_3"),

            oplabel, "false_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_incr(&A, 0),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "loop_end_3",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(3), oparg_slot(1)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_lit(2)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(3), oparg_slot(2)),
            newop_br(&A, JVST_VM_BR_LE, "valid_10"),

            oplabel, "invalid_5_12",
            newop_return(&A, 5),

            oplabel, "invalid_1_16",
            newop_return(&A, 1),

            NULL
          ),

          NULL
      ),
    },

    {
      ASSEMBLE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 2, 5, true),
        SJP_NONE),

      // XXX - comments here are largely the same as in the first
      //       test case
      newir_frame(&A,
          newir_counter(&A, 0, "num_props"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_incr(&A, 0, "num_props"),
                    NULL
                  )
                ),
                NULL
              ),

              // Post-loop check of number of properties
              newir_if(&A,
                  newir_op(&A, JVST_IR_EXPR_GE, 
                    newir_count(&A, 0, "num_props"),
                    newir_size(&A, 2)
                  ),
                  newir_if(&A,
                    newir_op(&A, JVST_IR_EXPR_LE, 
                      newir_count(&A, 0, "num_props"),
                      newir_size(&A, 5)
                    ),
                    newir_stmt(&A, JVST_IR_STMT_VALID),
                    newir_invalid(&A, JVST_INVALID_TOO_MANY_PROPS, "too many properties")
                  ),
                  newir_invalid(&A, JVST_INVALID_TOO_FEW_PROPS, "too few properties")
              ),
              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            opslots, 7,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_4"),

            oplabel, "false_17",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_20"),

            oplabel, "false_21",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_20"),

            oplabel, "false_24",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_12",
            newop_return(&A, 0),

            oplabel, "loop_4",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_3"),

            oplabel, "false_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_incr(&A, 0),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "loop_end_3",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(4), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(6), oparg_slot(4)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(5), oparg_lit(2)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(6), oparg_slot(5)),
            newop_br(&A, JVST_VM_BR_GE, "true_9"),

            oplabel, "invalid_4_16",
            newop_return(&A, 4),

            oplabel, "true_9",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(3), oparg_slot(1)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_lit(5)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(3), oparg_slot(2)),
            newop_br(&A, JVST_VM_BR_LE, "valid_12"),

            oplabel, "invalid_5_14",
            newop_return(&A, 5),

            oplabel, "invalid_1_20",
            newop_return(&A, 1),

            NULL
          ),

          NULL
      ),
    },

    {
      ENCODE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 2, 5, true),
        SJP_NONE),

      // XXX - comments here are largely the same as in the first
      //       test case
      newir_frame(&A,
          newir_counter(&A, 0, "num_props"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_incr(&A, 0, "num_props"),
                    NULL
                  )
                ),
                NULL
              ),

              // Post-loop check of number of properties
              newir_if(&A,
                  newir_op(&A, JVST_IR_EXPR_GE, 
                    newir_count(&A, 0, "num_props"),
                    newir_size(&A, 2)
                  ),
                  newir_if(&A,
                    newir_op(&A, JVST_IR_EXPR_LE, 
                      newir_count(&A, 0, "num_props"),
                      newir_size(&A, 5)
                    ),
                    newir_stmt(&A, JVST_IR_STMT_VALID),
                    newir_invalid(&A, JVST_INVALID_TOO_MANY_PROPS, "too many properties")
                  ),
                  newir_invalid(&A, JVST_INVALID_TOO_FEW_PROPS, "too few properties")
              ),
              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          newop_proc(&A,
            opslots, 7,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_4"),

            oplabel, "false_17",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_20"),

            oplabel, "false_21",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_20"),

            oplabel, "false_24",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_12",
            newop_return(&A, 0),

            oplabel, "loop_4",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_3"),

            oplabel, "false_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_incr(&A, 0),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "loop_end_3",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(4), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(6), oparg_slot(4)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(5), oparg_lit(2)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(6), oparg_slot(5)),
            newop_br(&A, JVST_VM_BR_GE, "true_9"),

            oplabel, "invalid_4_16",
            newop_return(&A, 4),

            oplabel, "true_9",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(3), oparg_slot(1)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_lit(5)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(3), oparg_slot(2)),
            newop_br(&A, JVST_VM_BR_LE, "valid_12"),

            oplabel, "invalid_5_14",
            newop_return(&A, 5),

            oplabel, "invalid_1_20",
            newop_return(&A, 1),

            NULL
          ),

          NULL
      ),

      newvm_program(&A,
          JVST_OP_PROC, VMLIT(7), VMLIT(0),
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_BEG),
          JVST_OP_JMP, JVST_VM_BR_EQ, "loop_4",

          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_20",

          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_ARRAY_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_20",

          VM_LABEL, "false_24",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_12",
          JVST_OP_RETURN, 0, 0,

          VM_LABEL, "loop_4",
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "loop_end_3",

          JVST_OP_CONSUME, 0, 0,
          JVST_OP_CONSUME, 0, 0,
          JVST_OP_INCR, VMSLOT(0), VMLIT(1),
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_4",

          VM_LABEL, "loop_end_3",
          JVST_OP_MOVE, VMSLOT(4), VMSLOT(0),
          JVST_OP_MOVE, VMSLOT(6), VMSLOT(4),
          JVST_OP_MOVE, VMSLOT(5), VMLIT(2),
          JVST_OP_ICMP, VMSLOT(6), VMSLOT(5),
          JVST_OP_JMP, JVST_VM_BR_GE, "true_9",

          JVST_OP_RETURN, VMLIT(4), 0,

          VM_LABEL, "true_9",
          JVST_OP_MOVE, VMSLOT(1), VMSLOT(0),
          JVST_OP_MOVE, VMSLOT(3), VMSLOT(1),
          JVST_OP_MOVE, VMSLOT(2), VMLIT(5),
          JVST_OP_ICMP, VMSLOT(3), VMSLOT(2),
          JVST_OP_JMP, JVST_VM_BR_LE, "valid_12",

          JVST_OP_RETURN, VMLIT(5), 0,

          VM_LABEL, "invalid_1_20",
          JVST_OP_RETURN, VMLIT(1), 0,

          VM_END)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_op_minproperties_2(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct op_test tests[] = {
    {
      ASSEMBLE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 0, false),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "foo",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                            NULL),
                          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_counter(&A, 0, "num_props"),
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                    newir_match(&A, 0,
                      // no match
                      newir_case(&A, 0, 
                        NULL,
                        newir_stmt(&A, JVST_IR_STMT_CONSUME)
                      ),

                      // match "bar"
                      newir_case(&A, 1,
                        newmatchset(&A, RE_LITERAL,  "bar", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_STRING),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                          ),
                          NULL
                        )
                      ),

                      // match "foo"
                      newir_case(&A, 2,
                        newmatchset(&A, RE_LITERAL,  "foo", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_NUMBER),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                          ),
                          NULL
                        )
                      ),

                      NULL
                    ),
                    newir_incr(&A, 0, "num_props"),
                    NULL
                  )
                ),
                NULL
              ),

              // Post-loop check of number of properties
              newir_if(&A,
                  newir_op(&A, JVST_IR_EXPR_GE, 
                    newir_count(&A, 0, "num_props"),
                    newir_size(&A, 1)
                  ),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  newir_invalid(&A, JVST_INVALID_TOO_FEW_PROPS, "too few properties")
              ),
              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          opdfa, 1,

          newop_proc(&A,
            opslots, 5,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_4"),

            oplabel, "false_20",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_23"),

            oplabel, "false_24",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_23"),

            oplabel, "false_27",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_17",
            newop_return(&A, 0),

            oplabel, "loop_4",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_3"),

            oplabel, "false_7",
            newop_match(&A, 0),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "M_next_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_13"),

            oplabel, "invalid_9_14",
            newop_return(&A, 9),

            oplabel, "M_9",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "M_join_8",
            newop_incr(&A, 0),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_11",
            newop_call(&A, oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "M_join_8"),

            oplabel, "M_13",
            newop_call(&A, oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "M_join_8"),

            oplabel, "loop_end_3",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(4), oparg_slot(2)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(3), oparg_lit(1)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(4), oparg_slot(3)),
            newop_br(&A, JVST_VM_BR_GE, "valid_17"),

            oplabel, "invalid_4_19",
            newop_return(&A, 4),

            oplabel, "invalid_1_23",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_STRING)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_op_required(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct op_test tests[] = {
    {
      ASSEMBLE,
      // schema:
      // {
      //   "properties" : {
      //     "foo" : { "type" : "number" },
      //     "foo" : { "type" : "string" }
      //   },
      //   "required" : [ "foo" ]
      // }
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_required(&A, stringset(&A, "foo", NULL)),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "foo",
                              newcnode_switch(&A, 0,
                                SJP_NUMBER, newcnode_valid(),
                                SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0,
                                SJP_STRING, newcnode_valid(),
                                SJP_NONE)),
                            NULL),
                          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_bitvec(&A, 1, "reqmask", 1),
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                    newir_match(&A, 0,
                      // no match
                      newir_case(&A, 0, 
                        NULL,
                        newir_stmt(&A, JVST_IR_STMT_CONSUME)
                      ),

                      // match "bar"
                      newir_case(&A, 1,
                        newmatchset(&A, RE_LITERAL,  "bar", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_STRING),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                          ),
                          NULL
                        )
                      ),

                      // match "foo"
                      newir_case(&A, 2,
                        newmatchset(&A, RE_LITERAL, "foo", -1),
                        newir_seq(&A,
                          newir_frame(&A,
                            newir_stmt(&A, JVST_IR_STMT_TOKEN),
                            newir_if(&A, newir_istok(&A, SJP_NUMBER),
                              newir_seq(&A,
                                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                newir_stmt(&A, JVST_IR_STMT_VALID),
                                NULL
                              ),
                              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                            ),
                            NULL
                          ),
                          newir_bitop(&A, JVST_IR_STMT_BSET, 1, "reqmask", 0),
                          NULL
                        )
                      ),

                      NULL
                    ),
                    NULL
                  )
                ),
                NULL
              ),
              newir_if(&A,
                  newir_btestall(&A, 1, "reqmask", 0,0),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
                    "missing required properties")
              ),
              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_seq(&A,
                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  NULL
                )
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          opdfa, 1,

          newop_proc(&A,
            opslots, 3,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_4"),

            oplabel, "false_20",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_23"),

            oplabel, "false_24",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_23"),

            oplabel, "false_27",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_17",
            newop_return(&A, 0),

            oplabel, "loop_4",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_3"),

            oplabel, "false_7",
            newop_match(&A, 0),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "M_next_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_13"),

            oplabel, "invalid_9_14",
            newop_return(&A, 9),

            oplabel, "M_9",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_11",
            newop_call(&A, oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_13",
            newop_call(&A, oparg_lit(2)),
            newop_bitop(&A, JVST_OP_BSET, 0, 0),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "loop_end_3",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(2), oparg_lit(1)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_17"),

            oplabel, "invalid_6_19",
            newop_return(&A, 6),

            oplabel, "invalid_1_23",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_STRING)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

    },

    {
      ENCODE,
      // schema:
      // {
      //   "properties" : {
      //     "foo" : { "type" : "number" },
      //     "foo" : { "type" : "string" }
      //   },
      //   "required" : [ "foo" ]
      // }
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_required(&A, stringset(&A, "foo", NULL)),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "foo",
                              newcnode_switch(&A, 0,
                                SJP_NUMBER, newcnode_valid(),
                                SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0,
                                SJP_STRING, newcnode_valid(),
                                SJP_NONE)),
                            NULL),
                          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_bitvec(&A, 1, "reqmask", 1),
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                    newir_match(&A, 0,
                      // no match
                      newir_case(&A, 0, 
                        NULL,
                        newir_stmt(&A, JVST_IR_STMT_CONSUME)
                      ),

                      // match "bar"
                      newir_case(&A, 1,
                        newmatchset(&A, RE_LITERAL,  "bar", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_STRING),
                            newir_stmt(&A, JVST_IR_STMT_VALID),
                            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                          ),
                          NULL
                        )
                      ),

                      // match "foo"
                      newir_case(&A, 2,
                        newmatchset(&A, RE_LITERAL,  "foo", RE_LITERAL, "foo", -1),
                        newir_seq(&A,
                          newir_frame(&A,
                            newir_stmt(&A, JVST_IR_STMT_TOKEN),
                            newir_if(&A, newir_istok(&A, SJP_NUMBER),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                            ),
                            NULL
                          ),
                          newir_bitop(&A, JVST_IR_STMT_BSET, 1, "reqmask", 0),
                          NULL
                        )
                      ),

                      NULL
                    ),
                    NULL
                  )
                ),
                NULL
              ),
              newir_if(&A,
                  newir_btestall(&A, 1, "reqmask", 0,-1),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
                    "missing required properties")
              ),
              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_stmt(&A, JVST_IR_STMT_VALID)
              )
            )
          ),
          NULL
      ),

      newop_program(&A,
          opdfa, 1,

          newop_proc(&A,
            opslots, 3,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_4"),

            oplabel, "false_20",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_23"),

            oplabel, "false_24",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_23"),

            oplabel, "false_27",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_17",
            newop_return(&A, 0),

            oplabel, "loop_4",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_3"),

            oplabel, "false_7",
            newop_match(&A, 0),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "M_next_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_13"),

            oplabel, "invalid_9_14",
            newop_return(&A, 9),

            oplabel, "M_9",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_11",
            newop_call(&A, oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "M_13",
            newop_call(&A, oparg_lit(2)),
            newop_bitop(&A, JVST_OP_BSET, 0, 0),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_4"),

            oplabel, "loop_end_3",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(2), oparg_lit(1)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_17"),

            oplabel, "invalid_6_19",
            newop_return(&A, 6),

            oplabel, "invalid_1_23",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_STRING)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          newop_proc(&A,
            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_NUMBER)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "invalid_1_5",
            newop_return(&A, 1),

            oplabel, "true_2",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_3",
            newop_return(&A, 0),

            NULL
          ),

          NULL
      ),

      newvm_program(&A,
          VM_DFA, 1,

          JVST_OP_PROC, VMLIT(3), VMLIT(0),
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_BEG),
          JVST_OP_JMP, JVST_VM_BR_EQ, "loop_4",

          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_23",

          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_ARRAY_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_23",

          VM_LABEL, "false_27",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_17",
          JVST_OP_RETURN, 0, 0,

          VM_LABEL, "loop_4",
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "loop_end_3",

          JVST_OP_MATCH, VMLIT(0), VMLIT(0),
          JVST_OP_MOVE, VMSLOT(1), VMREG(JVST_VM_M),
          JVST_OP_ICMP, VMSLOT(1), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_EQ, "M_9",

          JVST_OP_ICMP, VMSLOT(1), VMLIT(1),
          JVST_OP_JMP, JVST_VM_BR_EQ, "M_11",

          JVST_OP_ICMP, VMSLOT(1), VMLIT(2),
          JVST_OP_JMP, JVST_VM_BR_EQ, "M_13",

          JVST_OP_RETURN, VMLIT(9), 0,

          VM_LABEL, "M_9",
          JVST_OP_CONSUME, VMLIT(0), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_4",

          VM_LABEL, "M_11",
          JVST_OP_CALL, 2,
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_4",

          VM_LABEL, "M_13",
          JVST_OP_CALL, 3,
          JVST_OP_BSET, VMSLOT(0), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_4",

          VM_LABEL, "loop_end_3",
          JVST_OP_MOVE, VMSLOT(2), VMSLOT(0),
          JVST_OP_BAND, VMSLOT(2), VMLIT(1),
          JVST_OP_ICMP, VMSLOT(2), VMLIT(1),
          JVST_OP_JMP, JVST_VM_BR_EQ, "valid_17",

          JVST_OP_RETURN, VMLIT(6), 0,

          VM_LABEL, "invalid_1_23",
          JVST_OP_RETURN, VMLIT(1), 0,

          JVST_OP_PROC, VMLIT(0), VMLIT(0),

          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_STRING),
          JVST_OP_JMP, JVST_VM_BR_EQ, "true_2a",

          JVST_OP_RETURN, VMLIT(1), 0,

          VM_LABEL, "true_2a",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_3a",
          JVST_OP_RETURN, 0, 0,

          JVST_OP_PROC, VMLIT(0), VMLIT(0),

          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_NUMBER),
          JVST_OP_JMP, JVST_VM_BR_EQ, "true_2b",

          JVST_OP_RETURN, VMLIT(1), 0,

          VM_LABEL, "true_2b",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_3b",
          JVST_OP_RETURN, 0, 0,

          VM_END)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_op_dependencies(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct op_test tests[] = {
    {
      ASSEMBLE,
      // schema: { "dependencies": {"bar": ["foo"]} }
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                          newcnode_required(&A, stringset(&A, "bar", "foo", NULL)),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "bar", newcnode_invalid()),
                            NULL),
                          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_split(&A,
                    newir_frame(&A,
                      newir_matcher(&A, 0, "dfa"),
                      newir_seq(&A,
                        newir_loop(&A, "L_OBJ", 0,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                            newir_break(&A, "L_OBJ", 0),
                            newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                              newir_match(&A, 0,
                                // no match
                                newir_case(&A, 0, 
                                  NULL,
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME)
                                ),

                                // match "bar"
                                newir_case(&A, 1,
                                  newmatchset(&A, RE_LITERAL,  "bar", -1),
                                  newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                                ),

                                NULL
                              ),
                              NULL
                            )
                          ),
                          NULL
                        ),
                        newir_stmt(&A, JVST_IR_STMT_VALID),
                        NULL
                      ),
                      NULL
                    ),

                    newir_frame(&A,
                      newir_bitvec(&A, 0, "reqmask", 2),
                      newir_matcher(&A, 0, "dfa"),
                      newir_seq(&A,
                        newir_loop(&A, "L_OBJ", 0,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                            newir_break(&A, "L_OBJ", 0),
                            newir_seq(&A,
                              newir_match(&A, 0,
                                // no match
                                newir_case(&A, 0, 
                                  NULL,
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME)
                                ),

                                // match "bar"
                                newir_case(&A, 1,
                                  newmatchset(&A, RE_LITERAL,  "bar", -1),
                                  newir_seq(&A,
                                    newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
                                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                    NULL
                                  )
                                ),

                                // match "foo"
                                newir_case(&A, 2,
                                  newmatchset(&A, RE_LITERAL,  "foo", -1),
                                  newir_seq(&A,
                                    newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
                                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                    NULL
                                  )
                                ),

                                NULL
                              ),
                              NULL
                            )
                          ),
                          NULL
                        ),
                        newir_if(&A,
                            newir_btestall(&A, 0, "reqmask", 0, 1),
                            newir_stmt(&A, JVST_IR_STMT_VALID),
                            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
                              "missing required properties")
                        ),
                        NULL
                      ),
                      NULL
                    ),

                    NULL
                  ),
                  newir_size(&A, 1)
                ),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                newir_invalid(&A, JVST_INVALID_SPLIT_CONDITION, "invalid split condition")
              ),
              newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                  newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_stmt(&A, JVST_IR_STMT_VALID),
                    NULL
                  )
                )
              )
          ),
          NULL
      ),

      newop_program(&A,
          opsplit, 2, 1, 2,
          opdfa, 2,

          newop_proc(&A,
            opslots, 4,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "false_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_15",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            oplabel, "true_2",
            newop_instr2(&A, JVST_OP_SPLIT, oparg_lit(0), oparg_slot(3)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_slot(3)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_lit(1)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_slot(1)),
            newop_br(&A, JVST_VM_BR_GE, "valid_5"),

            oplabel, "invalid_7_7",
            newop_return(&A, 7),

            oplabel, "invalid_1_11",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            opslots, 1,

            oplabel, "loop_2",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_12"),

            oplabel, "false_5",
            newop_match(&A, 0),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_7"),

            oplabel, "M_next_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_8_10"),

            oplabel, "invalid_9_11",
            newop_return(&A, 9),

            oplabel, "M_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "valid_12",
            newop_return(&A, 0),

            oplabel, "invalid_8_10",
            newop_return(&A, 8),

            NULL
          ),

          newop_proc(&A,
            opslots, 3,

            oplabel, "loop_2",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_1"),

            oplabel, "false_5",
            newop_match(&A, 1),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_7"),

            oplabel, "M_next_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "invalid_9_12",
            newop_return(&A, 9),

            oplabel, "M_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_9",
            newop_bitop(&A, JVST_OP_BSET, 0, 0),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_11",
            newop_bitop(&A, JVST_OP_BSET, 0, 1),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "loop_end_1",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(2), oparg_lit(3)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_lit(3)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_15"),

            oplabel, "invalid_6_17",
            newop_return(&A, 6),

            oplabel, "valid_15",
            newop_return(&A, 0),

            NULL
          ),
              
          NULL
      ),

    },

    {
      ENCODE,
      // schema: { "dependencies": {"bar": ["foo"]} }
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                          newcnode_required(&A, stringset(&A, "bar", "foo", NULL)),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "bar", newcnode_invalid()),
                            NULL),
                          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_split(&A,
                    newir_frame(&A,
                      newir_matcher(&A, 0, "dfa"),
                      newir_seq(&A,
                        newir_loop(&A, "L_OBJ", 0,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                            newir_break(&A, "L_OBJ", 0),
                            newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                              newir_match(&A, 0,
                                // no match
                                newir_case(&A, 0, 
                                  NULL,
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME)
                                ),

                                // match "bar"
                                newir_case(&A, 1,
                                  newmatchset(&A, RE_LITERAL,  "bar", -1),
                                  newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                                ),

                                NULL
                              ),
                              NULL
                            )
                          ),
                          NULL
                        ),
                        newir_stmt(&A, JVST_IR_STMT_VALID),
                        NULL
                      ),
                      NULL
                    ),

                    newir_frame(&A,
                      newir_bitvec(&A, 0, "reqmask", 2),
                      newir_matcher(&A, 0, "dfa"),
                      newir_seq(&A,
                        newir_loop(&A, "L_OBJ", 0,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                            newir_break(&A, "L_OBJ", 0),
                            newir_seq(&A,
                              newir_match(&A, 0,
                                // no match
                                newir_case(&A, 0, 
                                  NULL,
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME)
                                ),

                                // match "foo"
                                newir_case(&A, 1,
                                  newmatchset(&A, RE_LITERAL,  "foo", -1),
                                  newir_seq(&A,
                                    newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
                                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                    NULL
                                  )
                                ),

                                // match "bar"
                                newir_case(&A, 2,
                                  newmatchset(&A, RE_LITERAL,  "bar", -1),
                                  newir_seq(&A,
                                    newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
                                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                    NULL
                                  )
                                ),

                                NULL
                              ),
                              NULL
                            )
                          ),
                          NULL
                        ),
                        newir_if(&A,
                            newir_btestall(&A, 0, "reqmask", 0, -1),
                            newir_stmt(&A, JVST_IR_STMT_VALID),
                            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
                              "missing required properties")
                        ),
                        NULL
                      ),
                      NULL
                    ),

                    NULL
                  ),
                  newir_size(&A, 1)
                ),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                newir_invalid(&A, JVST_INVALID_SPLIT_CONDITION, "invalid split condition")
              ),
              newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                  newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                  newir_stmt(&A, JVST_IR_STMT_VALID)
                )
              )
          ),
          NULL
      ),

      newop_program(&A,
          opsplit, 2, 1, 2,
          opdfa, 2,

          newop_proc(&A,
            opslots, 4,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "false_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "valid_5",
            newop_return(&A, 0),

            oplabel, "true_2",
            newop_instr2(&A, JVST_OP_SPLIT, oparg_lit(0), oparg_slot(3)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_slot(3)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_lit(1)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_slot(1)),
            newop_br(&A, JVST_VM_BR_GE, "valid_5"),

            oplabel, "invalid_7_7",
            newop_return(&A, 7),

            oplabel, "invalid_1_11",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            opslots, 1,

            oplabel, "loop_2",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_12"),

            oplabel, "false_5",
            newop_match(&A, 0),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_7"),

            oplabel, "M_next_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_8_10"),

            oplabel, "invalid_9_11",
            newop_return(&A, 9),

            oplabel, "M_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "valid_12",
            newop_return(&A, 0),

            oplabel, "invalid_8_10",
            newop_return(&A, 8),

            NULL
          ),

          newop_proc(&A,
            opslots, 3,

            oplabel, "loop_2",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_1"),

            oplabel, "false_5",
            newop_match(&A, 1),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_7"),

            oplabel, "M_next_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "invalid_9_12",
            newop_return(&A, 9),

            oplabel, "M_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_9",
            newop_bitop(&A, JVST_OP_BSET, 0, 0),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_11",
            newop_bitop(&A, JVST_OP_BSET, 0, 1),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "loop_end_1",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(2), oparg_lit(3)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_lit(3)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_15"),

            oplabel, "invalid_6_17",
            newop_return(&A, 6),

            oplabel, "valid_15",
            newop_return(&A, 0),

            NULL
          ),
              
          NULL
      ),

      newvm_program(&A,
          VM_SPLIT, 2, 2,3,
          VM_DFA, 2,

          JVST_OP_PROC, VMLIT(4), VMLIT(0),
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_BEG),
          JVST_OP_JMP, JVST_VM_BR_EQ, "true_2",

          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_11",
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_ARRAY_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_1_11",

          VM_LABEL, "false_15",
          JVST_OP_CONSUME, 0, 0,

          VM_LABEL, "valid_5",
          JVST_OP_RETURN, 0, 0,

          VM_LABEL, "true_2",
          JVST_OP_SPLIT, VMLIT(0), VMSLOT(3),
          JVST_OP_MOVE, VMSLOT(0), VMSLOT(3),
          JVST_OP_MOVE, VMSLOT(2), VMSLOT(0),
          JVST_OP_MOVE, VMSLOT(1), VMLIT(1),
          JVST_OP_ICMP, VMSLOT(2), VMSLOT(1),
          JVST_OP_JMP, JVST_VM_BR_GE, "valid_5",

          VM_LABEL, "invalid_7_7",
          JVST_OP_RETURN, VMLIT(7), 0,

          VM_LABEL, "invalid_1_11",
          JVST_OP_RETURN, VMLIT(1), 0,


          JVST_OP_PROC, VMLIT(1), VMLIT(0),
          VM_LABEL, "loop_2",
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "valid_12",

          VM_LABEL, "false_5",
          JVST_OP_MATCH, VMLIT(0), VMLIT(0),
          JVST_OP_MOVE, VMSLOT(0), VMREG(JVST_VM_M),
          JVST_OP_ICMP, VMSLOT(0), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_EQ, "M_7",

          JVST_OP_ICMP, VMSLOT(0), VMLIT(1),
          JVST_OP_JMP, JVST_VM_BR_EQ, "invalid_8_10",

          VM_LABEL, "invalid_9_11",
          JVST_OP_RETURN, VMLIT(9), 0,

          VM_LABEL, "M_7",
          JVST_OP_CONSUME, VMLIT(0), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_2",

          VM_LABEL, "valid_12",
          JVST_OP_RETURN, VMLIT(0), VMLIT(0),

          VM_LABEL, "invalid_8_10",
          JVST_OP_RETURN, VMLIT(8), 0,


          JVST_OP_PROC, VMLIT(3), VMLIT(0),
          VM_LABEL, "loop_2b",
          JVST_OP_TOKEN, 0, 0,
          JVST_OP_ICMP, VMREG(JVST_VM_TT), VMLIT(SJP_OBJECT_END),
          JVST_OP_JMP, JVST_VM_BR_EQ, "loop_end_1",

          VM_LABEL, "false_5",
          JVST_OP_MATCH, VMLIT(1), VMLIT(0),
          JVST_OP_MOVE, VMSLOT(1), VMREG(JVST_VM_M),
          JVST_OP_ICMP, VMSLOT(1), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_EQ, "M_7b",

          JVST_OP_ICMP, VMSLOT(1), VMLIT(1),
          JVST_OP_JMP, JVST_VM_BR_EQ, "M_9b",

          JVST_OP_ICMP, VMSLOT(1), VMLIT(2),
          JVST_OP_JMP, JVST_VM_BR_EQ, "M_11b",

          VM_LABEL, "invalid_9_12",
          JVST_OP_RETURN, VMLIT(9), 0,

          VM_LABEL, "M_7b",
          JVST_OP_CONSUME, VMLIT(0), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_2b",

          VM_LABEL, "M_9b",
          JVST_OP_BSET, VMSLOT(0), VMLIT(0),
          JVST_OP_CONSUME, VMLIT(0), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_2b",

          VM_LABEL, "M_11b",
          JVST_OP_BSET, VMSLOT(0), VMLIT(1),
          JVST_OP_CONSUME, VMLIT(0), VMLIT(0),
          JVST_OP_JMP, JVST_VM_BR_ALWAYS, "loop_2b",

          VM_LABEL, "loop_end_1",
          JVST_OP_MOVE, VMSLOT(2), VMSLOT(0),
          JVST_OP_BAND, VMSLOT(2), VMLIT(3),
          JVST_OP_ICMP, VMSLOT(2), VMLIT(3),
          JVST_OP_JMP, JVST_VM_BR_EQ, "valid_15",

          VM_LABEL, "invalid_6_17",
          JVST_OP_RETURN, VMLIT(6), 0,

          VM_LABEL, "valid_15",
          JVST_OP_RETURN, VMLIT(0), VMLIT(0),

          VM_END)
    },

    {
      ASSEMBLE,
      // schema: { "dependencies": {"quux": ["foo", "bar"]} }
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                          newcnode_required(&A, stringset(&A, "quux", "foo", "bar", NULL)),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "quux", newcnode_invalid()),
                            NULL),
                          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_split(&A,
                    newir_frame(&A,
                      newir_matcher(&A, 0, "dfa"),
                      newir_seq(&A,
                        newir_loop(&A, "L_OBJ", 0,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                            newir_break(&A, "L_OBJ", 0),
                            newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                              newir_match(&A, 0,
                                // no match
                                newir_case(&A, 0, 
                                  NULL,
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME)
                                ),

                                // match "bar"
                                newir_case(&A, 1,
                                  newmatchset(&A, RE_LITERAL,  "quux", -1),
                                  newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                                ),

                                NULL
                              ),
                              NULL
                            )
                          ),
                          NULL
                        ),
                        newir_stmt(&A, JVST_IR_STMT_VALID),
                        NULL
                      ),
                      NULL
                    ),

                    newir_frame(&A,
                      newir_bitvec(&A, 0, "reqmask", 3),
                      newir_matcher(&A, 0, "dfa"),
                      newir_seq(&A,
                        newir_loop(&A, "L_OBJ", 0,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                            newir_break(&A, "L_OBJ", 0),
                            newir_seq(&A,
                              newir_match(&A, 0,
                                // no match
                                newir_case(&A, 0, 
                                  NULL,
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME)
                                ),

                                // match "bar"
                                newir_case(&A, 1,
                                  newmatchset(&A, RE_LITERAL,  "bar", -1),
                                  newir_seq(&A,
                                    newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 2),
                                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                    NULL
                                  )
                                ),

                                // match "foo"
                                newir_case(&A, 2,
                                  newmatchset(&A, RE_LITERAL,  "foo", -1),
                                  newir_seq(&A,
                                    newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
                                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                    NULL
                                  )
                                ),

                                // match "quux"
                                newir_case(&A, 3,
                                  newmatchset(&A, RE_LITERAL,  "quux", -1),
                                  newir_seq(&A,
                                    newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
                                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                    NULL
                                  )
                                ),

                                NULL
                              ),
                              NULL
                            )
                          ),
                          NULL
                        ),
                        newir_if(&A,
                            newir_btestall(&A, 0, "reqmask", 0, 2),
                            newir_stmt(&A, JVST_IR_STMT_VALID),
                            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
                              "missing required properties")
                        ),
                        NULL
                      ),
                      NULL
                    ),

                    NULL
                  ),
                  newir_size(&A, 1)
                ),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                newir_invalid(&A, JVST_INVALID_SPLIT_CONDITION, "invalid split condition")
              ),
              newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                  newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_stmt(&A, JVST_IR_STMT_VALID),
                    NULL
                  )
                )
              )
          ),
          NULL
      ),

      newop_program(&A,
          opsplit, 2, 1, 2,
          opdfa, 2,

          newop_proc(&A,
            opslots, 4,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "false_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_11"),

            oplabel, "false_15",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            oplabel, "true_2",
            newop_instr2(&A, JVST_OP_SPLIT, oparg_lit(0), oparg_slot(3)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_slot(3)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_lit(1)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_slot(1)),
            newop_br(&A, JVST_VM_BR_GE, "valid_5"),

            oplabel, "invalid_7_7",
            newop_return(&A, 7),

            oplabel, "invalid_1_11",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            opslots, 1, 

            oplabel, "loop_2",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_12"),

            oplabel, "false_5",
            newop_match(&A, 0),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_7"),

            oplabel, "M_next_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_8_10"),

            oplabel, "invalid_9_11",
            newop_return(&A, 9),

            oplabel, "M_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "valid_12",
            newop_return(&A, 0),

            oplabel, "invalid_8_10",
            newop_return(&A, 8),

            NULL
          ),

          newop_proc(&A,
            opslots, 3,

            oplabel, "loop_2",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_1"),

            oplabel, "false_5",
            newop_match(&A, 1),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_7"),

            oplabel, "M_next_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "M_next_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(3)),
            newop_br(&A, JVST_VM_BR_EQ, "M_13"),

            oplabel, "invalid_9_14",
            newop_return(&A, 9),

            oplabel, "M_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_9",
            newop_bitop(&A, JVST_OP_BSET, 0, 2),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_11",
            newop_bitop(&A, JVST_OP_BSET, 0, 1),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_13",
            newop_bitop(&A, JVST_OP_BSET, 0, 0),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "loop_end_1",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(2), oparg_lit(7)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_lit(7)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_17"),

            oplabel, "invalid_6_19",
            newop_return(&A, 6),

            oplabel, "valid_17",
            newop_return(&A, 0),

            NULL
          ),
              
          NULL
      ),

    },

    {
      ASSEMBLE,
      // schema: { "dependencies": {"quux": ["foo", "bar"], "this": ["that"]} }
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_bool(&A, JVST_CNODE_OR,
                            newcnode_required(&A, stringset(&A, "quux", "foo", "bar", NULL)),
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "quux", newcnode_invalid()),
                              NULL),
                            NULL),
                          newcnode_bool(&A, JVST_CNODE_OR,
                            newcnode_required(&A, stringset(&A, "this", "that", NULL)),
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "this", newcnode_invalid()),
                              NULL),
                            NULL),
                          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_bitvec(&A, 0, "splitvec", 4),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
              newir_seq(&A,
                newir_splitvec(&A, 0, "splitvec",
                  newir_frame(&A,
                    newir_matcher(&A, 0, "dfa"),
                    newir_seq(&A,
                      newir_loop(&A, "L_OBJ", 0,
                        newir_stmt(&A, JVST_IR_STMT_TOKEN),
                        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                          newir_break(&A, "L_OBJ", 0),
                          newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                            newir_match(&A, 0,
                              // no match
                              newir_case(&A, 0, 
                                NULL,
                                newir_stmt(&A, JVST_IR_STMT_CONSUME)
                              ),

                              // match "bar"
                              newir_case(&A, 1,
                                newmatchset(&A, RE_LITERAL,  "quux", -1),
                                newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                              ),

                              NULL
                            ),
                            NULL
                          )
                        ),
                        NULL
                      ),
                      newir_stmt(&A, JVST_IR_STMT_VALID),
                      NULL
                    ),
                    NULL
                  ),

                  newir_frame(&A,
                    newir_bitvec(&A, 0, "reqmask", 3),
                    newir_matcher(&A, 0, "dfa"),
                    newir_seq(&A,
                      newir_loop(&A, "L_OBJ", 0,
                        newir_stmt(&A, JVST_IR_STMT_TOKEN),
                        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                          newir_break(&A, "L_OBJ", 0),
                          newir_seq(&A,
                            newir_match(&A, 0,
                              // no match
                              newir_case(&A, 0, 
                                NULL,
                                newir_stmt(&A, JVST_IR_STMT_CONSUME)
                              ),

                              // match "bar"
                              newir_case(&A, 1,
                                newmatchset(&A, RE_LITERAL,  "bar", -1),
                                newir_seq(&A,
                                  newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 2),
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                  NULL
                                )
                              ),

                              // match "foo"
                              newir_case(&A, 2,
                                newmatchset(&A, RE_LITERAL,  "foo", -1),
                                newir_seq(&A,
                                  newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                  NULL
                                )
                              ),

                              // match "quux"
                              newir_case(&A, 3,
                                newmatchset(&A, RE_LITERAL,  "quux", -1),
                                newir_seq(&A,
                                  newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                  NULL
                                )
                              ),

                              NULL
                            ),
                            NULL
                          )
                        ),
                        NULL
                      ),
                      newir_if(&A,
                          newir_btestall(&A, 0, "reqmask", 0, 2),
                          newir_stmt(&A, JVST_IR_STMT_VALID),
                          newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
                            "missing required properties")
                      ),
                      NULL
                    ),
                    NULL
                  ),

                  newir_frame(&A,
                    newir_matcher(&A, 0, "dfa"),
                    newir_seq(&A,
                      newir_loop(&A, "L_OBJ", 0,
                        newir_stmt(&A, JVST_IR_STMT_TOKEN),
                        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                          newir_break(&A, "L_OBJ", 0),
                          newir_seq(&A,                                 // unnecessary SEQ should be removed in the future
                            newir_match(&A, 0,
                              // no match
                              newir_case(&A, 0, 
                                NULL,
                                newir_stmt(&A, JVST_IR_STMT_CONSUME)
                              ),

                              // match "bar"
                              newir_case(&A, 1,
                                newmatchset(&A, RE_LITERAL,  "this", -1),
                                newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                              ),

                              NULL
                            ),
                            NULL
                          )
                        ),
                        NULL
                      ),
                      newir_stmt(&A, JVST_IR_STMT_VALID),
                      NULL
                    ),
                    NULL
                  ),

                  newir_frame(&A,
                    newir_bitvec(&A, 0, "reqmask", 2),
                    newir_matcher(&A, 0, "dfa"),
                    newir_seq(&A,
                      newir_loop(&A, "L_OBJ", 0,
                        newir_stmt(&A, JVST_IR_STMT_TOKEN),
                        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                          newir_break(&A, "L_OBJ", 0),
                          newir_seq(&A,
                            newir_match(&A, 0,
                              // no match
                              newir_case(&A, 0, 
                                NULL,
                                newir_stmt(&A, JVST_IR_STMT_CONSUME)
                              ),

                              // match "quux"
                              newir_case(&A, 1,
                                newmatchset(&A, RE_LITERAL,  "that", -1),
                                newir_seq(&A, 
                                  newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                  NULL
                                )
                              ),

                              // match "bar"
                              newir_case(&A, 2,
                                newmatchset(&A, RE_LITERAL,  "this", -1),
                                newir_seq(&A, 
                                  newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
                                  newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                  NULL
                                )
                              ),

                              NULL
                            ),
                            NULL
                          )
                        ),
                        NULL
                      ),
                      newir_if(&A,
                          newir_btestall(&A, 0, "reqmask", 0, 1),
                          newir_stmt(&A, JVST_IR_STMT_VALID),
                          newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
                            "missing required properties")
                      ),
                      NULL
                    ),
                    NULL
                  ),

                  NULL
                ),
                newir_if(&A,
                  newir_op(&A, JVST_IR_EXPR_AND, 
                    newir_op(&A, JVST_IR_EXPR_OR, 
                      newir_btestany(&A, 0, "splitvec", 0, 0),  // XXX - can combine the OR'd stuff...
                      newir_btest(&A, 0, "splitvec", 1)
                    ),
                    newir_op(&A, JVST_IR_EXPR_OR,               // XXX - can combine the OR'd stuff...
                      newir_btestany(&A, 0, "splitvec", 2, 2),
                      newir_btest(&A, 0, "splitvec", 3)
                    )
                  ),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  newir_invalid(&A, JVST_INVALID_SPLIT_CONDITION, "invalid split condition")
                ),

                NULL
              ),
              newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                  newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_stmt(&A, JVST_IR_STMT_VALID),
                    NULL
                  )
                )
              )
          ),
          NULL
      ),

      newop_program(&A,
          opsplit, 4, 1,2,3,4,
          opdfa, 4,

          newop_proc(&A,
            opslots, 5,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "true_2"),

            oplabel, "false_11",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_14"),

            oplabel, "false_15",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_14"),

            oplabel, "false_18",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_5",
            newop_return(&A, 0),

            oplabel, "true_2",
            newop_instr2(&A, JVST_OP_SPLITV, oparg_lit(0), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(1), oparg_lit(1)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_NE, "and_true_8"),

            oplabel, "or_false_9",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(2), oparg_lit(2)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "and_true_8"),

            oplabel, "invalid_7_7",
            newop_return(&A, 7),

            oplabel, "and_true_8",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(3), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(3), oparg_lit(4)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(3), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_NE, "valid_5"),

            oplabel, "or_false_10",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(4), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(4), oparg_lit(8)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(4), oparg_lit(8)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_5"),
            newop_br(&A, JVST_VM_BR_ALWAYS, "invalid_7_7"),

            oplabel, "invalid_1_14",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            opslots, 1,
            oplabel, "loop_2",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_12"),

            oplabel, "false_5",
            newop_match(&A, 0),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_7"),

            oplabel, "M_next_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_8_10"),

            oplabel, "invalid_9_11",
            newop_return(&A, 9),

            oplabel, "M_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "valid_12",
            newop_return(&A, 0),

            oplabel, "invalid_8_10",
            newop_return(&A, 8),

            NULL
          ),

          newop_proc(&A,
            opslots, 3,

            oplabel, "loop_2",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_1"),

            oplabel, "false_5",
            newop_match(&A, 1),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_7"),

            oplabel, "M_next_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "M_next_12",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(3)),
            newop_br(&A, JVST_VM_BR_EQ, "M_13"),

            oplabel, "invalid_9_14",
            newop_return(&A, 9),

            oplabel, "M_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_9",
            newop_bitop(&A, JVST_OP_BSET, 0, 2),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_11",
            newop_bitop(&A, JVST_OP_BSET, 0, 1),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_13",
            newop_bitop(&A, JVST_OP_BSET, 0, 0),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "loop_end_1",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(2), oparg_lit(7)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_lit(7)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_17"),

            oplabel, "invalid_6_19",
            newop_return(&A, 6),

            oplabel, "valid_17",
            newop_return(&A, 0),

            NULL
          ),
              
          newop_proc(&A,
            opslots, 1,

            oplabel, "loop_2",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_12"),

            oplabel, "false_5",
            newop_match(&A, 2),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(0), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_7"),

            oplabel, "M_next_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(0), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_8_10"),

            oplabel, "invalid_9_11",
            newop_return(&A, 9),

            oplabel, "M_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "valid_12",
            newop_return(&A, 0),

            oplabel, "invalid_8_10",
            newop_return(&A, 8),

            NULL
          ),

          newop_proc(&A,
            opslots, 3,

            oplabel, "loop_2",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_1"),

            oplabel, "false_5",
            newop_match(&A, 3),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_m()),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_EQ, "M_7"),

            oplabel, "M_next_8",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(1)),
            newop_br(&A, JVST_VM_BR_EQ, "M_9"),

            oplabel, "M_next_10",
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "M_11"),

            oplabel, "invalid_9_12",
            newop_return(&A, 9),

            oplabel, "M_7",
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_9",
            newop_bitop(&A, JVST_OP_BSET, 0, 1),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "M_11",
            newop_bitop(&A, JVST_OP_BSET, 0, 0),
            newop_instr(&A, JVST_OP_CONSUME),
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_2"),

            oplabel, "loop_end_1",
            newop_load(&A, JVST_OP_MOVE, oparg_slot(2), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(2), oparg_lit(3)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(2), oparg_lit(3)),
            newop_br(&A, JVST_VM_BR_EQ, "valid_15"),

            oplabel, "invalid_6_17",
            newop_return(&A, 6),

            oplabel, "valid_15",
            newop_return(&A, 0),

            NULL
          ),
              
          NULL
      ),

    },

    { STOP },
  };

  const struct op_test unfinished_tests[] = {
    {
      NONE,
      // schema: {
      //   "dependencies": {
      //     "bar": {
      //       "properties": {
      //         "foo": {"type": "integer"},
      //         "bar": {"type": "integer"}
      //       }
      //     }
      //   }
      // },
      newcnode_bool(&A, JVST_CNODE_AND,
          newcnode_switch(&A, 1, SJP_NONE),
          newcnode_bool(&A, JVST_CNODE_OR,
            newcnode_bool(&A, JVST_CNODE_AND,
              newcnode_switch(&A, 0, 
                SJP_OBJECT_BEG, newcnode_required(&A, stringset(&A, "bar", NULL)),
                SJP_NONE),
              newcnode_switch(&A, 1, 
                SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                                  newcnode_propset(&A,
                                    newcnode_prop_match(&A, RE_LITERAL, "foo", 
                                      newcnode_switch(&A, 0, 
                                        SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                        SJP_NONE)),
                                    newcnode_prop_match(&A, RE_LITERAL, "bar", 
                                      newcnode_switch(&A, 0, 
                                        SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                        SJP_NONE)),
                                    NULL),
                                  newcnode_valid(),
                                  NULL),
                SJP_NONE),
              NULL),
            newcnode_switch(&A, 1, 
              SJP_OBJECT_BEG, newcnode_propset(&A,
                                newcnode_prop_match(&A, RE_LITERAL, "bar", newcnode_invalid()),
                                NULL),
              SJP_NONE),
            NULL),
          NULL),

      NULL
    },

    { STOP },
  };

  (void)unfinished_tests;
  RUNTESTS(tests);
}

void test_op_unique_1(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct op_test tests[] = {
    {
      ASSEMBLE,
      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode(&A, JVST_CNODE_ARR_UNIQUE),
          SJP_NONE),

      newir_frame(&A,
        newir_bitvec(&A, 0, "uniq_contains_split", 2),
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_stmt(&A, JVST_IR_STMT_UNIQUE_INIT),
              newir_loop(&A, "ARR_OUTER", 0,
                newir_loop(&A, "ARR_INNER", 1,
                  newir_stmt(&A, JVST_IR_STMT_TOKEN),
                  newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                    newir_break(&A, "ARR_OUTER", 0),
                    newir_seq(&A,
                      newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                      newir_splitvec(&A, 0, "uniq_contains_split",
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_CONSUME),
                          newir_stmt(&A, JVST_IR_STMT_VALID),
                          NULL
                        ),

                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_UNIQUE_TOK),
                          NULL
                        ),

                        NULL
                      ),

                      newir_seq(&A,
                        newir_if(&A, newir_btest(&A, 0, "uniq_contains_split", 1),
                          newir_stmt(&A, JVST_IR_STMT_NOP),
                          newir_invalid(&A, JVST_INVALID_NOT_UNIQUE, "array elements are not unique")
                        ),

                        NULL
                      ),

                      NULL
                    )
                  ),

                  NULL
                ),

                NULL
              ),

              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_UNIQUE_FINAL),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),

              NULL
            ),

            newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              )
            )
          )
        ),
        NULL
      ),

      newop_program(&A,
          opsplit, 2, 1, 2,

          newop_proc(&A,
            opslots, 2,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_OBJECT_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_3"),

            oplabel, "false_4",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_BEG)),
            newop_br(&A, JVST_VM_BR_EQ, "true_6"),

            oplabel, "false_19",
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "invalid_1_3"),

            oplabel, "false_22",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_18",
            newop_return(&A, 0),

            oplabel, "true_6",
            newop_instr2(&A, JVST_OP_UNIQUE, oparg_lit(JVST_VM_UNIQUE_INIT), oparg_lit(0)),

            oplabel, "loop_10",
            newop_instr(&A, JVST_OP_TOKEN),
            newop_cmp(&A, JVST_OP_ICMP, oparg_tt(), oparg_tok(SJP_ARRAY_END)),
            newop_br(&A, JVST_VM_BR_EQ, "loop_end_7"),

            oplabel, "false_13",
            newop_instr2(&A, JVST_OP_TOKEN, oparg_lit(0), oparg_lit(-1)),
            newop_instr2(&A, JVST_OP_SPLITV, oparg_lit(0), oparg_slot(0)),
            newop_load(&A, JVST_OP_MOVE, oparg_slot(1), oparg_slot(0)),
            newop_instr2(&A, JVST_OP_BAND, oparg_slot(1), oparg_lit(2)),
            newop_cmp(&A, JVST_OP_ICMP, oparg_slot(1), oparg_lit(2)),
            newop_br(&A, JVST_VM_BR_EQ, "true_15"),

            oplabel, "invalid_18_17",
            newop_return(&A, 18),

            oplabel, "true_15",
            newop_br(&A, JVST_VM_BR_ALWAYS, "loop_10"),

            oplabel, "loop_end_7",
            newop_instr2(&A, JVST_OP_UNIQUE, oparg_lit(JVST_VM_UNIQUE_FINAL), oparg_lit(0)),
            newop_br(&A, JVST_VM_BR_ALWAYS, "valid_18"),

            oplabel, "invalid_1_3",
            newop_return(&A, 1),

            NULL
          ),

          newop_proc(&A,
            opslots, 0,

            oplabel, "entry_0",
            newop_instr(&A, JVST_OP_CONSUME),

            oplabel, "valid_1",
            newop_return(&A, 0),

            NULL
          ),

          newop_proc(&A,
            opslots, 0,

            oplabel, "entry_0",
            newop_instr2(&A, JVST_OP_UNIQUE, oparg_lit(JVST_VM_UNIQUE_EVAL), oparg_lit(0)),

            NULL
          ),
              
          NULL
      )
    },

    { STOP },
  };

  const struct op_test unfinished_tests[] = {
    {
      NONE,
      // schema: {
      //   "dependencies": {
      //     "bar": {
      //       "properties": {
      //         "foo": {"type": "integer"},
      //         "bar": {"type": "integer"}
      //       }
      //     }
      //   }
      // },
      newcnode_bool(&A, JVST_CNODE_AND,
          newcnode_switch(&A, 1, SJP_NONE),
          newcnode_bool(&A, JVST_CNODE_OR,
            newcnode_bool(&A, JVST_CNODE_AND,
              newcnode_switch(&A, 0, 
                SJP_OBJECT_BEG, newcnode_required(&A, stringset(&A, "bar", NULL)),
                SJP_NONE),
              newcnode_switch(&A, 1, 
                SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                                  newcnode_propset(&A,
                                    newcnode_prop_match(&A, RE_LITERAL, "foo", 
                                      newcnode_switch(&A, 0, 
                                        SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                        SJP_NONE)),
                                    newcnode_prop_match(&A, RE_LITERAL, "bar", 
                                      newcnode_switch(&A, 0, 
                                        SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                        SJP_NONE)),
                                    NULL),
                                  newcnode_valid(),
                                  NULL),
                SJP_NONE),
              NULL),
            newcnode_switch(&A, 1, 
              SJP_OBJECT_BEG, newcnode_propset(&A,
                                newcnode_prop_match(&A, RE_LITERAL, "bar", newcnode_invalid()),
                                NULL),
              SJP_NONE),
            NULL),
          NULL),

      NULL
    },

    { STOP },
  };

  (void)unfinished_tests;
  RUNTESTS(tests);
}

/* incomplete tests... placeholders for conversion from cnode tests */
static void test_op_minproperties_3(void);
static void test_op_maxproperties_1(void);
static void test_op_maxproperties_2(void);
static void test_op_minmax_properties_2(void);

static void test_op_anyof_allof_oneof_1(void);
static void test_op_anyof_2(void);

static void test_op_simplify_ands(void);
static void test_op_simplify_ored_schema(void);

int main(void)
{
  test_op_empty_schema();
  test_op_type_constraints();

  test_op_type_integer();
  test_op_minimum();
  test_op_multiple_of();

  test_op_properties();

  test_op_minmax_properties_1();
  test_op_minproperties_2();

  test_op_required();

  test_op_dependencies();

  test_op_unique_1();

  /* incomplete tests... placeholders for conversion from cnode tests */
  test_op_minproperties_3();
  test_op_maxproperties_1();
  test_op_maxproperties_2();
  test_op_minmax_properties_2();

  test_op_anyof_allof_oneof_1();
  test_op_anyof_2();

  test_op_simplify_ands();
  test_op_simplify_ored_schema();

  return report_tests();
}

/* incomplete tests... placeholders for conversion from cnode tests */
void test_op_minproperties_3(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "minProperties", 1,
      "properties", newprops(&A,
        "foo", newschema_p(&A, JSON_VALUE_OBJECT,
          "minProperties", 1,
          NULL), // XXX - JSON_VALUE_INTEGER
        "bar", newschema(&A, JSON_VALUE_STRING),
        NULL),
      NULL);

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct op_test tests[] = {
    {
      NONE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 0, false),
                          newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_switch(&A, 0,
                                  SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                                                    newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 0, false),
                                                    newcnode_valid(),
                                                    NULL),
                                  SJP_NONE)),
                              newcnode_prop_match(&A, RE_LITERAL, "bar",
                                newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                              NULL),
                            newcnode_valid(),
                            NULL),
                          NULL),
        SJP_NONE),

      NULL
    },

    { STOP },
  };

  UNIMPLEMENTED(tests);
}

void test_op_maxproperties_1(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "maxProperties", 2,
      NULL);

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct op_test tests[] = {
    {
      NONE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 0, 2, true),
                          newcnode_valid(),
                          NULL),
        SJP_NONE),

      NULL
    },

    { STOP },
  };

  UNIMPLEMENTED(tests);
}

void test_op_maxproperties_2(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "maxProperties", 1,
      "properties", newprops(&A,
        "foo", newschema_p(&A, JSON_VALUE_OBJECT,
          "maxProperties", 1,
          NULL), // XXX - JSON_VALUE_INTEGER
        "bar", newschema(&A, JSON_VALUE_STRING),
        NULL),
      NULL);

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct op_test tests[] = {
    {
      NONE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 0, 1, true),
                          newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_switch(&A, 0,
                                  SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                                                    newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 0, 1, true),
                                                    newcnode_valid(),
                                                    NULL),
                                  SJP_NONE)),
                              newcnode_prop_match(&A, RE_LITERAL, "bar",
                                newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                              NULL),
                            newcnode_valid(),
                            NULL),
                          NULL),
        SJP_NONE),

      NULL
    },

    { STOP },
  };

  UNIMPLEMENTED(tests);
}

void test_op_minmax_properties_2(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "minProperties", 1,
      "maxProperties", 1,
      "properties", newprops(&A,
        "foo", newschema_p(&A, JSON_VALUE_OBJECT,
          "minProperties", 1,
          "maxProperties", 2,
          NULL), // XXX - JSON_VALUE_INTEGER
        "bar", newschema(&A, JSON_VALUE_STRING),
        NULL),
      NULL);

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct op_test tests[] = {
    {
      NONE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 1, true),
                          newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_switch(&A, 0,
                                  SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                                                    newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 2, true),
                                                    newcnode_valid(),
                                                    NULL),
                                  SJP_NONE)),
                              newcnode_prop_match(&A, RE_LITERAL, "bar",
                                newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                              NULL),
                            newcnode_valid(),
                            NULL),
                          NULL),
        SJP_NONE),

      NULL
    },

    { STOP },
  };

  UNIMPLEMENTED(tests);
}

void test_op_anyof_allof_oneof_1(void)
{
  struct arena_info A = {0};

  const struct op_test tests[] = {
    {
      NONE,
      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_bool(&A, JVST_CNODE_OR,
          newcnode_switch(&A, 0,
            SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
            SJP_NONE),
          newcnode_switch(&A, 1,
            SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 2.0, 0.0),
                          newcnode_valid(),
                          NULL),
            SJP_NONE),
          NULL),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL),

      NULL
    },

    {
      NONE,
      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_bool(&A, JVST_CNODE_AND,
          newcnode_switch(&A, 0,
            SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
            SJP_NONE),
          newcnode_switch(&A, 1,
            SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 2.0, 0.0),
                          newcnode_valid(),
                          NULL),
            SJP_NONE),
          NULL),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL),

      NULL
    },

    {
      NONE,
      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_bool(&A, JVST_CNODE_XOR,
          newcnode_switch(&A, 0,
            SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
            SJP_NONE),
          newcnode_switch(&A, 1,
            SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 2.0, 0.0),
                          newcnode_valid(),
                          NULL),
            SJP_NONE),
          NULL),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL),

      NULL
    },

    { STOP },
  };

  UNIMPLEMENTED(tests);
}

void test_op_anyof_2(void)
{
  struct arena_info A = {0};

  const struct op_test tests[] = {
    {
      NONE,
      newcnode_bool(&A, JVST_CNODE_AND,
          newcnode_bool(&A, JVST_CNODE_OR,
            newcnode_switch(&A, 0,
              SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_propset(&A, 
                                  newcnode_prop_match(&A, RE_LITERAL, "foo",
                                    newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                                  newcnode_prop_match(&A, RE_LITERAL, "bar",
                                    newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                                  NULL),
                                newcnode_valid(),
                                NULL),
              SJP_NONE),

            newcnode_switch(&A, 0,
              SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_propset(&A,
                                  newcnode_prop_match(&A, RE_LITERAL, "foo",
                                    newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                                  newcnode_prop_match(&A, RE_LITERAL, "bar",
                                    newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                                  NULL),
                                newcnode_valid(),
                                NULL),
              SJP_NONE),
            NULL),
          newcnode_switch(&A, 1, SJP_NONE),
          NULL),
      NULL
    },

    { STOP },
  };

  UNIMPLEMENTED(tests);
}

static void test_op_simplify_ands(void)
{
  struct arena_info A = {0};

  const struct op_test tests[] = {
    // handle AND with only one level...
    {
      NONE,
      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
          SJP_NONE),

      NULL
    },

    // handle nested ANDs
    {
      NONE,
      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                        newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
                        newcnode(&A,JVST_CNODE_NUM_INTEGER),
                        NULL),
          SJP_NONE),

      NULL
    },

    // handle more complex nested ANDs
    {
      NONE,
      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                        newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
                        newcnode(&A,JVST_CNODE_NUM_INTEGER),
                        newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 3.2),
                        NULL),
          SJP_NONE),

      NULL
    },

    { STOP },
  };

  UNIMPLEMENTED(tests);
}

void test_op_simplify_ored_schema(void)
{
  struct arena_info A = {0};
  const struct op_test tests[] = {
    {
      NONE,
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                          newcnode_propset(&A, 
                            newcnode_prop_match(&A, RE_LITERAL, "foo",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                            NULL),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "foo",
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                            NULL),
                          NULL),
        SJP_NONE),

      NULL
    },

    {
      NONE,
      newcnode_switch(&A, 1, 
          SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                            newcnode_bool(&A, JVST_CNODE_AND,
                              newcnode_required(&A, stringset(&A, "bar", NULL)),
                              newcnode_propset(&A,
                                newcnode_prop_match(&A, RE_LITERAL, "foo", 
                                  newcnode_switch(&A, 0, 
                                    SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                    SJP_NONE)),
                                newcnode_prop_match(&A, RE_LITERAL, "bar", 
                                  newcnode_switch(&A, 0, 
                                    SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                    SJP_NONE)),
                                NULL),
                              NULL),
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "bar", newcnode_invalid()),
                              NULL),
                            NULL),
          SJP_NONE),

      NULL
    },

    { STOP },
  };

  UNIMPLEMENTED(tests);
}

