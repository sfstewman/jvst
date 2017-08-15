#include "ir_testing.h"
#include "validate_testing.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jvst_macros.h"

#include "validate_ir.h"

static void test_ir_empty_schema(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      LIVE_ANNO,
      newcnode_switch(&A, 1, SJP_NONE),

      NULL,

      newir_program(&A,
        newir_frame(&A, frameindex, 1,

          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in, 0,
            live_out, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "invalid_1",
              4, "false"
            ),
            live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_in, 0,
          live_out, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_gen, 0,
          // live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 4, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              3, "invalid_1",
              8, "valid"
            ),
            live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out, 0,
            NULL
          ),
          live_in, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out, 0,

          newir_block(&A, 8, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_gen, 0,
            live_kill, 0,
            NULL
          ),
          live_in, 0,
          live_out, 0,

          newir_block(&A, 3, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_gen, 0,
            live_kill, 0,
            NULL
          ),
          live_in, 0,
          live_out, 0,

          NULL
        ),

        NULL
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
  cleanup_liveanno(&A);
}

static void test_ir_type_integer(void)
{
  struct arena_info A = {0};
  struct ast_schema schema = {
    .types = JSON_VALUE_INTEGER,
  };

  const struct ir_test tests[] = {
    {
      LIVE_ANNO,
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

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_gen,  0,
            live_in,   0,
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
              2, "true",
              9, "invalid_1"
            ),
            live_kill, 0,
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_gen,  0,
          live_in,   0,
          live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 9, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_in,   0,
            NULL
          ),
          live_in,   0,

          newir_block(&A, 2, "true",
            newir_cbranch(&A, newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
              5, "valid",
              7, "invalid_2"
            ),
            live_kill, 0,
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  0,

            NULL
          ),
          live_kill, 0,
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  0,

          newir_block(&A, 7, "invalid_2",
            newir_invalid(&A, JVST_INVALID_NOT_INTEGER, "number is not an integer"),
            live_in,   0,
            NULL
          ),
          live_in,   0,

          newir_block(&A, 5, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in,   0,
            NULL
          ),
          live_in,   0,

          NULL
        ),

        NULL
      )

    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_ir_minimum(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "minimum", 1.1,
      NULL);

  const struct ir_test tests[] = {
    {
      LIVE_ANNO,
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
              newir_stmt(&A, JVST_IR_STMT_VALID),
              newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      ),

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_gen,  0,
            live_in,   0,
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
              2, "true",
              9, "invalid_1"
            ),
            live_kill, 0,
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_gen,  0,
          live_in,   0,
          live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 9, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_in,   0,

            NULL
          ),
          live_in,   0,

          newir_block(&A, 2, "true",
            newir_move(&A, newir_ftemp(&A, 1), newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
            live_kill, 1, newir_ftemp(&A, 1),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_ftemp(&A, 1),

            newir_move(&A, newir_ftemp(&A, 0), newir_num(&A, 1.1)),
            live_kill, 1, newir_ftemp(&A, 0),
            live_gen,  0,
            live_in,   1, newir_ftemp(&A, 1),
            live_out,  2, newir_ftemp(&A, 1), newir_ftemp(&A, 0),

            newir_cbranch(&A,
              newir_op(&A, JVST_IR_EXPR_GE, 
                newir_ftemp(&A, 1),
                newir_ftemp(&A, 0)
              ),
              5, "valid",
              7, "invalid_3"
            ),
            live_kill, 0,
            live_gen,  2, newir_ftemp(&A, 1), newir_ftemp(&A, 0),
            live_in,   2, newir_ftemp(&A, 1), newir_ftemp(&A, 0),
            live_out,  0,

            NULL
          ),
          live_kill, 2, newir_ftemp(&A, 1), newir_ftemp(&A, 0),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  0,


          newir_block(&A, 7, "invalid_3",
            newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid"),
            live_out,  0,
            NULL
          ),
          live_out,  0,

          newir_block(&A, 5, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_out,  0,
            NULL
          ),
          live_out,  0,

          NULL
        ),

        NULL
      )

    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_ir_minmax_properties_1(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct ir_test tests[] = {
    {
      LIVE_ANNO,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_counts(&A, 1, 0),
        SJP_NONE),

      NULL,

      // XXX
      // this IR is not as compact as it could be we should be able to
      // short-circuit the loop when we've encountered one property
      // instead of keeping a full count
      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_counter(&A, 0, "num_props"),

          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              4, "loop",
              16, "false"
            ),
            live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 16, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              19, "invalid_1",
              20, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 20, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              19, "invalid_1",
              13, "valid"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  0,

          newir_block(&A, 13, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_gen,  0, 

            NULL
          ),
          live_gen,  0, 

          newir_block(&A, 4, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_gen,  0,
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "loop_end",
              7, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_kill, 0,
            live_in,   2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 7, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            live_gen,  0,
            live_kill, 1, newir_itemp(&A, 0),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            newir_cbranch(&A, 
              newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              9, "M",
              10, "invalid_9"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_kill, 0,
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  1, newir_slot(&A, 2),
            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_itemp(&A, 0),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 10, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            live_gen,  0, 

            NULL
          ),
          live_gen,  0, 

          newir_block(&A, 9, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_gen,  0,
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 8, "M_join"),
            live_gen,  0,
            live_kill, 0,
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 8, "M_join",
            newir_incr(&A, 0, "num_props"),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 4, "loop"),
            live_gen,  0,
            live_kill, 0,
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 3, "loop_end",
            // Post-loop check of number of properties
            newir_move(&A, newir_itemp(&A, 1), newir_count(&A, 0, "num_props")),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_itemp(&A, 1),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_itemp(&A, 1),

            newir_move(&A, newir_itemp(&A, 3), newir_itemp(&A, 1)),
            live_gen,  1, newir_itemp(&A, 1),
            live_kill, 1, newir_itemp(&A, 3),
            live_in,   1, newir_itemp(&A, 1),
            live_out,  1, newir_itemp(&A, 3),

            newir_move(&A, newir_itemp(&A, 2), newir_size(&A, 1)),
            live_gen,  0,
            live_kill, 1, newir_itemp(&A, 2),
            live_in,   1, newir_itemp(&A, 3),
            live_out,  2, newir_itemp(&A, 3), newir_itemp(&A, 2),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_GE, newir_itemp(&A, 3), newir_itemp(&A,2)),
              13, "valid",
              15, "invalid_4"
            ),
            live_gen,  2, newir_itemp(&A, 3), newir_itemp(&A, 2),
            live_kill, 0,
            live_in,   2, newir_itemp(&A, 3), newir_itemp(&A, 2),
            live_out,  0, 

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 3, newir_itemp(&A, 1), newir_itemp(&A, 3), newir_itemp(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  0,

          newir_block(&A, 15, "invalid_4",
            newir_invalid(&A, JVST_INVALID_TOO_FEW_PROPS, "too few properties"),
            live_gen,  0, 

            NULL
          ),
          live_gen,  0, 

          newir_block(&A, 19, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_gen,  0, 

            NULL
          ),
          live_gen,  0, 

          NULL
        ),

        NULL
      )

    },

    {
      LIVE_ANNO,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_counts(&A, 2, 5),
        SJP_NONE),

      NULL,

      // XXX - comments here are largely the same as in the previous
      //       test case
      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_counter(&A, 0, "num_props"),

          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              4, "loop",
              20, "false"
            ),
            live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 20, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              23, "invalid_1",
              24, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 24, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              23, "invalid_1",
              15, "valid"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  0,

          newir_block(&A, 15, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_gen,  0, 

            NULL
          ),
          live_gen,  0, 

          newir_block(&A, 4, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_gen,  0,
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "loop_end",
              7, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_kill, 0,
            live_in,   2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 7, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            live_gen,  0,
            live_kill, 1, newir_itemp(&A, 0),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              9, "M",
              10, "invalid_9"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_kill, 0,
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_itemp(&A, 0),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 10, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            live_gen,  0, 

            NULL
          ),
          live_gen,  0, 

          newir_block(&A, 9, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_gen,  0,
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 8, "M_join"),
            live_gen,  0,
            live_kill, 0,
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 8, "M_join",
            newir_incr(&A, 0, "num_props"),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 4, "loop"),
            live_gen,  0,
            live_kill, 0,
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 3, "loop_end",
            newir_move(&A, newir_itemp(&A, 4), newir_count(&A, 0, "num_props")),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_itemp(&A, 4),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 4),

            newir_move(&A, newir_itemp(&A, 6), newir_itemp(&A, 4)), 
            live_gen,  1, newir_itemp(&A, 4),
            live_kill, 1, newir_itemp(&A, 6),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 4),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 6),

            newir_move(&A, newir_itemp(&A, 5), newir_size(&A, 2)),
            live_gen,  0,
            live_kill, 1, newir_itemp(&A, 5),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 6),
            live_out,  3, newir_slot(&A, 2), newir_itemp(&A, 6), newir_itemp(&A, 5),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_GE, newir_itemp(&A, 6), newir_itemp(&A, 5)),
              12, "true",
              19, "invalid_4"
            ),
            live_gen,  2, newir_itemp(&A, 6), newir_itemp(&A, 5),
            live_kill, 0,
            live_in,   3, newir_slot(&A, 2), newir_itemp(&A, 6), newir_itemp(&A, 5),
            live_out,  1, newir_slot(&A, 2),  

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 3, newir_itemp(&A, 4), newir_itemp(&A, 6), newir_itemp(&A, 5),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 19, "invalid_4",
            newir_invalid(&A, JVST_INVALID_TOO_FEW_PROPS, "too few properties"),
            live_out,  0,
            NULL
          ),
          live_out,  0,

          newir_block(&A, 12, "true",
            newir_move(&A, newir_itemp(&A, 1), newir_count(&A, 0, "num_props")),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_itemp(&A, 1),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_itemp(&A, 1),

            newir_move(&A, newir_itemp(&A, 3), newir_itemp(&A, 1)), 
            live_gen,  1, newir_itemp(&A, 1),
            live_kill, 1, newir_itemp(&A, 3),
            live_in,   1, newir_itemp(&A, 1),
            live_out,  1, newir_itemp(&A, 3),

            newir_move(&A, newir_itemp(&A, 2), newir_size(&A, 5)),
            live_gen,  0,
            live_kill, 1, newir_itemp(&A, 2),
            live_in,   1, newir_itemp(&A, 3),
            live_out,  2, newir_itemp(&A, 3), newir_itemp(&A, 2),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_LE, newir_itemp(&A, 3), newir_itemp(&A, 2)),
              15, "valid",
              17, "invalid_5"
            ),
            live_gen,  2, newir_itemp(&A, 3), newir_itemp(&A, 2),
            live_kill, 0,
            live_in,   2, newir_itemp(&A, 3), newir_itemp(&A, 2),
            live_out,  0, 

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 3, newir_itemp(&A, 1), newir_itemp(&A, 3), newir_itemp(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  0,


          newir_block(&A, 17, "invalid_5",
            newir_invalid(&A, JVST_INVALID_TOO_MANY_PROPS, "too many properties"),
            live_out,  0,

            NULL
          ),
          live_out,  0,

          newir_block(&A, 23, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_out,  0,

            NULL
          ),
          live_out,  0,

          NULL
        ),
        NULL
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_ir_minproperties_2(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct ir_test tests[] = {
    {
      LIVE_ANNO,
      newcnode_switch(&A, 1,
          SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, 1, 0),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "foo",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                            NULL),
                          NULL),
          SJP_NONE),

      NULL,

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_counter(&A, 0, "num_props"),
          newir_matcher(&A, 0, "dfa"),

          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              4, "loop",
              20, "false"
            ),
            live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 20, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              23, "invalid_1",
              24, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 24, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              23, "invalid_1",
              17, "valid"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  0,

          newir_block(&A, 17, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_gen,  0, 

            NULL
          ),
          live_gen,  0, 

          newir_block(&A, 4, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_gen,  0,
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "loop_end",
              7, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_kill, 0,
            live_in,   2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 7, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            live_gen,  0,
            live_kill, 1, newir_itemp(&A, 0),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              9, "M",
              10, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_kill, 0,
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_itemp(&A, 0),
          live_in,   1, newir_slot(&A, 2),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              11, "M",
              12, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_kill, 0,
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_kill, 0,
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 12, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              13, "M",
              14, "invalid_9"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_kill, 0,
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_kill, 0,
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 14, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 9, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_gen,  0,
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 8, "M_join"),
            live_gen,  0,
            live_kill, 0,
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 8, "M_join",
              newir_incr(&A, 0, "num_props"),
              live_gen,  1, newir_slot(&A, 2),
              live_kill, 1, newir_slot(&A, 2),
              live_in,   1, newir_slot(&A, 2),
              live_out,  1, newir_slot(&A, 2),

              newir_branch(&A, 4, "loop"),
              live_gen,  0,
              live_kill, 0,
              live_in,   1, newir_slot(&A, 2),
              live_out,  1, newir_slot(&A, 2),

              NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 11, "M",
            newir_call(&A, 2),
            live_gen,  0,
            live_kill, 0,
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 8, "M_join"),
            live_gen,  0,
            live_kill, 0,
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  0,
          live_kill, 0,
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 13, "M",
            newir_call(&A, 3),
            live_gen,  0,
            live_kill, 0,
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 8, "M_join"),
            live_gen,  0,
            live_kill, 0,
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  0,
          live_kill, 0,
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 3, "loop_end",
            newir_move(&A, newir_itemp(&A, 1), newir_count(&A, 0, "num_props")),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_itemp(&A, 1),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_itemp(&A, 1),

            newir_move(&A, newir_itemp(&A, 3), newir_itemp(&A, 1)),
            live_gen,  1, newir_itemp(&A, 1),
            live_kill, 1, newir_itemp(&A, 3),
            live_in,   1, newir_itemp(&A, 1),
            live_out,  1, newir_itemp(&A, 3),

            newir_move(&A, newir_itemp(&A, 2), newir_size(&A, 1)),
            live_gen,  0,
            live_kill, 1, newir_itemp(&A, 2),
            live_in,   1, newir_itemp(&A, 3),
            live_out,  2, newir_itemp(&A, 3), newir_itemp(&A, 2),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_GE, newir_itemp(&A, 3), newir_itemp(&A, 2)),
              17, "valid",
              19, "invalid_4"
            ),
            live_gen,  2, newir_itemp(&A, 3), newir_itemp(&A, 2),
            live_kill, 0,
            live_in,   2, newir_itemp(&A, 3), newir_itemp(&A, 2),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 3, newir_itemp(&A, 1), newir_itemp(&A, 3), newir_itemp(&A, 2), 
          live_in,   1, newir_slot(&A, 2),
          live_out,  0,

          newir_block(&A, 19, "invalid_4",
            newir_invalid(&A, JVST_INVALID_TOO_FEW_PROPS, "too few properties"),
            live_out,  0,

            NULL
          ),
          live_out,  0,

          newir_block(&A, 23, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_out,  0,

            NULL
          ),
          live_out,  0,

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_gen,  0,
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   0,
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_STRING),
              3, "valid",
              5, "invalid_1"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_kill, 0,
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  0,

            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   0,
          live_out,  0,

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_in,   0,
            NULL
          ),
          live_in,   0,

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in,   0,
            NULL
          ),
          live_in,   0,

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_gen,  0,
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   0,
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
              3, "valid",
              5, "invalid_1"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_kill, 0,
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  0,

            NULL
          ),
          live_gen,  0,
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   0,
          live_out,  0,

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_out,  0,

            NULL
          ),
          live_out,  0,

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_out,  0,

            NULL
          ),
          live_out,  0,

          NULL
        ),

        NULL
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_ir_required(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct ir_test tests[] = {
    {
      LIVE_ANNO,
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

      NULL,

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_bitvec(&A, 1, "reqmask", 1),
          newir_matcher(&A, 0, "dfa"),

          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              4, "loop",
              20, "false"
            ),
            live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 20, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              23, "invalid_1",
              24, "false"
            ),
            live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 24, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              23, "invalid_1",
              17, "valid"
            ),
            live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out, 0,

            NULL
          ),
          live_gen, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out, 0,

          newir_block(&A, 17, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_out, 0,
            NULL
          ),
          live_out, 0,

          newir_block(&A, 4, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "loop_end",
              7, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 7, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            live_kill, 1, newir_itemp(&A, 0),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              9, "M",
              10, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_kill, 1, newir_itemp(&A, 0),
          live_in,   1, newir_slot(&A, 2),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              11, "M",
              12, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 12, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              13, "M",
              14, "invalid_9"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 14, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            live_out, 0,
            NULL
          ),
          live_out, 0,

          newir_block(&A, 9, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 4, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 11, "M",
            newir_call(&A, 2),
            live_kill, 0,
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 4, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),
            NULL
          ),
          live_kill, 0,
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 13, "M",
            newir_call(&A, 3),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_bitop(&A, JVST_IR_STMT_BSET, 1, "reqmask", 0),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 4, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 3, "loop_end",
            newir_cbranch(&A, newir_btestall(&A, 1, "reqmask", 0,0),
              17, "valid",
              19, "invalid_6"
            ),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),

          newir_block(&A, 19, "invalid_6",
            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
              "missing required properties"),
            live_out, 0,

            NULL
          ),
          live_out, 0,

          newir_block(&A, 23, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_out, 0,

            NULL
          ),
          live_out, 0,

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_STRING),
              3, "valid",
              5, "invalid_1"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
              3, "valid",
              5, "invalid_1"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          NULL
        ),

        NULL
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_ir_dependencies(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct ir_test tests[] = {
    {
      LIVE_ANNO,
      // schema: { "dependencies": {"quux": ["foo", "bar"]} }
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                          newcnode_required(&A, stringset(&A, "quux", "foo", "bar", NULL)),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "quux", newcnode_invalid()),
                            NULL),
                          NULL),
        SJP_NONE),

      NULL,

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_splitlist(&A, 0, 2, 2, 3),

          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              2, "true",
              8, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 8, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              11, "invalid_1",
              12, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 12, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              11, "invalid_1",
              5, "valid"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 5, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in, 0,
            NULL
          ),
          live_in, 0,

          newir_block(&A, 2, "true",
            newir_move(&A, newir_itemp(&A, 0), newir_split(&A, splitlist, 0)),
            live_kill, 1, newir_itemp(&A, 0),
            live_out,  1, newir_itemp(&A, 0),

            newir_move(&A, newir_itemp(&A, 2), newir_itemp(&A, 0)),
            live_kill, 1, newir_itemp(&A, 2),
            live_in,   1, newir_itemp(&A, 0),
            live_gen,  1, newir_itemp(&A, 0),
            live_out,  1, newir_itemp(&A, 2),

            newir_move(&A, newir_itemp(&A, 1), newir_size(&A, 1)),
            live_kill, 1, newir_itemp(&A, 1),
            live_in,   1, newir_itemp(&A, 2),
            live_gen,  0,
            live_out,  2, newir_itemp(&A, 2), newir_itemp(&A, 1),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_GE, newir_itemp(&A, 2), newir_itemp(&A, 1)),
              5, "valid",
              7, "invalid_7"
            ),
            live_in,   2, newir_itemp(&A, 2), newir_itemp(&A, 1),
            live_gen,  2, newir_itemp(&A, 2), newir_itemp(&A, 1),

            NULL
          ),
          live_kill, 3, newir_itemp(&A, 0), newir_itemp(&A, 2), newir_itemp(&A, 1),
          live_in,   0,
          live_gen,  0,
          live_out,  0,

          newir_block(&A, 7, "invalid_7",
            newir_invalid(&A, JVST_INVALID_SPLIT_CONDITION, "invalid split condition"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 11, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            live_in,   0,
            live_out,  0,

            NULL
          ),
          live_in,   0,
          live_out,  0,

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   0,
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              12, "valid",
              5, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  0,

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   0,
          live_out,  0,

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            live_kill, 1, newir_itemp(&A, 0),
            live_in,   0,
            live_out,  1, newir_itemp(&A, 0),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   1, newir_itemp(&A, 0),
            live_out,  1, newir_itemp(&A, 0),

            NULL
          ),
          live_kill, 1, newir_itemp(&A, 0),
          live_in,   0,
          live_out,  1, newir_itemp(&A, 0),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              10, "invalid_8",
              11, "invalid_9"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   1, newir_itemp(&A, 0),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   1, newir_itemp(&A, 0),
          live_out,  0,

          newir_block(&A, 11, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            live_out,  0,

            NULL
          ),
          live_out,  0,

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   0,
            live_out,  0,

            newir_branch(&A, 2, "loop"),
            live_in,   0,
            live_out,  0,

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   0,
          live_out,  0,

          newir_block(&A, 12, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in,   0,

            NULL
          ),
          live_in,   0,

          newir_block(&A, 10, "invalid_8",
            newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name"),
            live_in,   0,

            NULL
          ),
          live_in,   0,

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_bitvec(&A, 0, "reqmask", 3),
          newir_matcher(&A, 0, "dfa"),

          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              1, "loop_end",
              5, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            live_kill, 1, newir_itemp(&A, 0),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_kill, 1, newir_itemp(&A, 0),
          live_in,   1, newir_slot(&A, 2),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              9, "M",
              10, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),


          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              11, "M",
              12, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 12, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 3)),
              13, "M",
              14, "invalid_9"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 14, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 9, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 2),
            live_kill, 1, newir_slot(&A, 2),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_gen,  1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 11, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
            live_kill, 1, newir_slot(&A, 2),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_gen,  1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 13, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
            live_kill, 1, newir_slot(&A, 2),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_gen,  1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 1, "loop_end",
            newir_cbranch(&A, newir_btestall(&A, 0, "reqmask", 0, 2),
              17, "valid",
              19, "invalid_6"
            ),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  0,

          newir_block(&A, 19, "invalid_6",
            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
              "missing required properties"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 17, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          NULL
        ),

        NULL
      )
    },

    {
      LIVE_ANNO,
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

      NULL,

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_splitlist(&A, 0, 4, 2, 3, 4, 5),
          newir_bitvec(&A, 0, "splitvec", 4),

          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   0,
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              2, "true",
              11, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   0,
          live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 11, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              14, "invalid_1",
              15, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            NULL
          ),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

          newir_block(&A, 15, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              14, "invalid_1",
              5, "valid"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_out,  0,

          newir_block(&A, 5, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 2, "true",
            newir_splitvec(&A, 0, "splitvec", splitlist, 0),
            live_kill, 1, newir_slot(&A, 2),
            live_gen,  0,
            live_in,   0,
            live_out,  1, newir_slot(&A, 2),

            newir_cbranch(&A, newir_btestany(&A, 0, "splitvec", 0, 0),  // XXX - can combine the OR'd stuff...
              8, "and_true",
              9, "or_false"
            ),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 1, newir_slot(&A, 2),
          live_gen,  0,
          live_in,   0,
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 9, "or_false",
            newir_cbranch(&A, newir_btest(&A, 0, "splitvec", 1),  // XXX - can combine the OR'd stuff...
              8, "and_true",
              7, "invalid_7"
            ),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 7, "invalid_7",
            newir_invalid(&A, JVST_INVALID_SPLIT_CONDITION, "invalid split condition"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 8, "and_true",
            newir_cbranch(&A, newir_btestany(&A, 0, "splitvec", 2,2),  // XXX - can combine the OR'd stuff...
              5, "valid",
              10, "or_false"
            ),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),


          newir_block(&A, 10, "or_false",
            newir_cbranch(&A, newir_btest(&A, 0, "splitvec", 3),  // XXX - can combine the OR'd stuff...
              5, "valid",
              7, "invalid_7"
            ),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  0,

          newir_block(&A, 14, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_matcher(&A, 0, "dfa"),

          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            live_in,   0,
            NULL
          ),
          live_in,   0,

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   0,
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              12, "valid",
              5, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  0,

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   0,
          live_out,  0,

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            live_kill, 1, newir_itemp(&A, 0),
            live_in,   0,
            live_out,  1, newir_itemp(&A, 0),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
              ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   1, newir_itemp(&A, 0),
            live_out,  1, newir_itemp(&A, 0),

            NULL
          ),
          live_kill, 1, newir_itemp(&A, 0),
          live_in,   0,
          live_out,  1, newir_itemp(&A, 0),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              10, "invalid_8",
              11, "invalid_9"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   1, newir_itemp(&A, 0),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   1, newir_itemp(&A, 0),
          live_out,  0,

          newir_block(&A, 11, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   0,
            live_out,  0,

            newir_branch(&A, 2, "loop"),
            live_in,   0,
            live_out,  0,

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   0,
          live_out,  0,

          newir_block(&A, 12, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in,   0,

            NULL
          ),
          live_in,   0,

          newir_block(&A, 10, "invalid_8",
            newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name"),
            live_in,   0,

            NULL
          ),
          live_in,   0,

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_bitvec(&A, 0, "reqmask", 3),
          newir_matcher(&A, 0, "dfa"),

          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),
            NULL
          ),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              1, "loop_end",
              5, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            live_kill, 1, newir_itemp(&A, 0),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_kill, 1, newir_itemp(&A, 0),
          live_in,   1, newir_slot(&A, 2),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              9, "M",
              10, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              11, "M",
              12, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 12, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 3)),
              13, "M",
              14, "invalid_9"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 14, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),
            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 9, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 2),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 11, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 13, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 1, "loop_end",
            newir_cbranch(&A, newir_btestall(&A, 0, "reqmask", 0, 2),
              17, "valid",
              19, "invalid_6"
            ),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  0,

          newir_block(&A, 19, "invalid_6",
            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
              "missing required properties"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 17, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          NULL
        ),

        newir_frame(&A, frameindex, 4,
          newir_matcher(&A, 0, "dfa"),

          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            live_in,   0,

            NULL
          ),
          live_in,   0,

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   0,
            live_out,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              12, "valid",
              5, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  0,

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   0,
          live_out,  0,

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            live_kill, 1, newir_itemp(&A, 0),
            live_in,   0,
            live_out,  1, newir_itemp(&A, 0),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   1, newir_itemp(&A, 0),
            live_out,  1, newir_itemp(&A, 0),

            NULL
          ),
          live_kill, 1, newir_itemp(&A, 0),
          live_in,   0,
          live_out,  1, newir_itemp(&A, 0),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              10, "invalid_8",
              11, "invalid_9"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   1, newir_itemp(&A, 0),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   1, newir_itemp(&A, 0),
          live_out,  0,

          newir_block(&A, 11, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   0,
            live_out,  0,

            newir_branch(&A, 2, "loop"),
            live_in,   0,
            live_out,  0,

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   0,
          live_out,  0,

          newir_block(&A, 12, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in,   0,

            NULL
          ),
          live_in,   0,

          newir_block(&A, 10, "invalid_8",
            newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name"),
            live_in,   0,

            NULL
          ),
          live_in,   0,

          NULL
        ),

        newir_frame(&A, frameindex, 5,
          newir_bitvec(&A, 0, "reqmask", 2),
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),

            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              1, "loop_end",
              5, "false"
            ),
            live_gen,  1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            live_kill, 1, newir_itemp(&A, 0),
            live_in,   1, newir_slot(&A, 2),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_kill, 1, newir_itemp(&A, 0),
          live_in,   1, newir_slot(&A, 2),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              9, "M",
              10, "M_next"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  2, newir_slot(&A, 2), newir_itemp(&A, 0),

          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              11, "M",
              12, "invalid_9"
            ),
            live_gen,  1, newir_itemp(&A, 0),
            live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_itemp(&A, 0),
          live_in,   2, newir_slot(&A, 2), newir_itemp(&A, 0),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 12, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 9, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 11, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
            live_gen,  1, newir_slot(&A, 2),
            live_kill, 1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            live_kill, 1, newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            newir_branch(&A, 2, "loop"),
            live_in,   1, newir_slot(&A, 2),
            live_out,  1, newir_slot(&A, 2),

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_kill, 2, newir_slot(&A, 2), newir_expr(&A, JVST_IR_EXPR_TOK_TYPE),
          live_in,   1, newir_slot(&A, 2),
          live_out,  1, newir_slot(&A, 2),

          newir_block(&A, 1, "loop_end",
            newir_cbranch(&A, newir_btestall(&A, 0, "reqmask", 0, 1),
              15, "valid",
              17, "invalid_6"
            ),
            live_gen,  1, newir_slot(&A, 2),
            live_in,   1, newir_slot(&A, 2),
            live_out,  0,

            NULL
          ),
          live_gen,  1, newir_slot(&A, 2),
          live_in,   1, newir_slot(&A, 2),
          live_out,  0,

          newir_block(&A, 17, "invalid_6",
            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
              "missing required properties"),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          newir_block(&A, 15, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            live_in, 0,

            NULL
          ),
          live_in, 0,

          NULL
        ),

        NULL
      )
    },

    { STOP },
  };

  const struct ir_test unfinished_tests[] = {
    {
      TRANSLATE,
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

  RUNTESTS(tests);
}

/* incomplete tests... placeholders for conversion from cnode tests */
static void test_ir_minproperties_3(void);
static void test_ir_maxproperties_1(void);
static void test_ir_maxproperties_2(void);
static void test_ir_minmax_properties_2(void);

static void test_ir_anyof_allof_oneof_1(void);
static void test_ir_anyof_2(void);

static void test_ir_simplify_ands(void);
static void test_ir_simplify_ored_schema(void);

int main(void)
{
  test_ir_empty_schema();

  test_ir_type_integer();
  test_ir_minimum();

  test_ir_minmax_properties_1();
  test_ir_minproperties_2();

  test_ir_required();

  test_ir_dependencies();

  /* incomplete tests... placeholders for conversion from cnode tests */
  test_ir_minproperties_3();
  test_ir_maxproperties_1();
  test_ir_maxproperties_2();
  test_ir_minmax_properties_2();

  test_ir_anyof_allof_oneof_1();
  test_ir_anyof_2();

  test_ir_simplify_ands();
  test_ir_simplify_ored_schema();

  return report_tests();
}

/* incomplete tests... placeholders for conversion from cnode tests */
void test_ir_minproperties_3(void)
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
  const struct ir_test tests[] = {
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, 1, 0),
                          newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_switch(&A, 0,
                                  SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                                                    newcnode_counts(&A, 1, 0),
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

void test_ir_maxproperties_1(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "maxProperties", 2,
      NULL);

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct ir_test tests[] = {
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, 0, 2),
                          newcnode_valid(),
                          NULL),
        SJP_NONE),

      NULL
    },

    { STOP },
  };

  UNIMPLEMENTED(tests);
}

void test_ir_maxproperties_2(void)
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
  const struct ir_test tests[] = {
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, 0, 1),
                          newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_switch(&A, 0,
                                  SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                                                    newcnode_counts(&A, 0, 1),
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

void test_ir_minmax_properties_2(void)
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
  const struct ir_test tests[] = {
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, 1, 1),
                          newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_switch(&A, 0,
                                  SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                                                    newcnode_counts(&A, 1, 2),
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

void test_ir_anyof_allof_oneof_1(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,
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
      TRANSLATE,
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
      TRANSLATE,
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

void test_ir_anyof_2(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,
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

static void test_ir_simplify_ands(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    // handle AND with only one level...
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
          SJP_NONE),

      NULL
    },

    // handle nested ANDs
    {
      TRANSLATE,
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
      TRANSLATE,
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

void test_ir_simplify_ored_schema(void)
{
  struct arena_info A = {0};
  const struct ir_test tests[] = {
    {
      TRANSLATE,
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
      TRANSLATE,
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


