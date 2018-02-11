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
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
      newcnode_switch(&A, 1, SJP_NONE),

      NULL,

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
            newir_block(&A, 0, "entry",
              newir_stmt(&A, JVST_IR_STMT_TOKEN),
              newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
                3, "invalid_1",
                4, "false"
              ),
              NULL
            ),

            newir_block(&A, 4, "false",
              newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
                3, "invalid_1",
                7, "false"
              ),
              NULL
            ),

            newir_block(&A, 7, "false",
              newir_stmt(&A, JVST_IR_STMT_CONSUME),
              newir_branch(&A, 8, "valid"),
              NULL
            ),

            newir_block(&A, 8, "valid",
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),

            newir_block(&A, 3, "invalid_1",
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              NULL
            ),

            NULL
        ),

        NULL
      )
    },

    {
      FLATTEN,
      newcnode_switch(&A, 1, SJP_NONE),

      NULL,

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
            newir_block(&A, 0, "entry", NULL),
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "invalid_1",
              4, "false"
            ),

            newir_block(&A, 4, "false", NULL),
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              3, "invalid_1",
              7, "false"
            ),

            newir_block(&A, 7, "false", NULL),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),

            newir_block(&A, 8, "valid", NULL),
            newir_stmt(&A, JVST_IR_STMT_VALID),

            newir_block(&A, 3, "invalid_1", NULL),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),

            NULL
        ),

        NULL
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_ir_type_constraints(void)
{
  struct arena_info A = {0};
  struct ast_schema schema = {
    .types = JSON_VALUE_NUMBER,
  };

  const struct ir_test tests[] = {
    {
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
      newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE),

      newir_frame(&A, 
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_stmt(&A, JVST_IR_STMT_VALID),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      ),

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
              2, "true",
              5, "invalid_1"
            ),
            NULL
          ),

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 3, "valid"),
            NULL
          ),

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),
        NULL
      )
    },

    {
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
      newcnode_switch(&A, 0, SJP_OBJECT_BEG, newcnode_valid(), SJP_NONE),
      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_stmt(&A, JVST_IR_STMT_VALID),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      ),

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              2, "true",
              5, "invalid_1"
            ),
            NULL
          ),

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 3, "valid"),
            NULL
          ),

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),

        NULL
      )
    },

    {
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_valid(),
        SJP_STRING, newcnode_valid(),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_STRING),
            newir_stmt(&A, JVST_IR_STMT_VALID),
            newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
            )
          ),
          NULL
      ),

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_STRING),
              2, "true",
              4, "false"
            ),
            NULL
          ),

          newir_block(&A, 4, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              6, "true",
              8, "invalid_1"
            ),
            NULL
          ),

          newir_block(&A, 8, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 3, "valid"),
            NULL
          ),

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 6, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 3, "valid"),
            NULL
          ),

          NULL
        ),
        NULL
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_ir_type_integer(void)
{
  struct arena_info A = {0};
  struct ast_schema schema = {
    .types = JSON_VALUE_INTEGER,
  };

  const struct ir_test tests[] = {
    {
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
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
            newir_cbranch(&A, newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
              4, "true",
              7, "invalid_2"
            ),
            NULL
          ),

          newir_block(&A, 7, "invalid_2",
            newir_invalid(&A, JVST_INVALID_NOT_INTEGER, "number is not an integer"),
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
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
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
      )

    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_ir_multiple_of(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_multiple_of(&A, 3.0),
                      newcnode_valid(),
                      NULL),
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
      )
    },

    {
      LINEARIZE,
      newcnode_switch(&A, 1,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_multiple_of(&A, 3.0),
                      newcnode_valid(),
                      NULL),
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

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
              2, "true",
              8, "false"
            ),
            NULL
          ),

          newir_block(&A, 8, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              11, "invalid_1",
              12, "false"
            ),
            NULL
          ),

          newir_block(&A, 12, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              11, "invalid_1",
              15, "false"
            ),
            NULL
          ),

          newir_block(&A, 15, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 5, "valid"),
            NULL
          ),

          newir_block(&A, 5, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_cbranch(&A,
              newir_multiple_of(&A,
                newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                3.0),
              4, "true",
              7, "invalid_17"
            ),
            NULL
          ),

          newir_block(&A, 7, "invalid_17",
            newir_invalid(&A, JVST_INVALID_NOT_MULTIPLE, "number is not an integer multiple"),
            NULL
          ),

          newir_block(&A, 4, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 5, "valid"),
            NULL
          ),

          newir_block(&A, 11, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          NULL
        ),

        NULL
      )

    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_ir_properties(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct ir_test tests[] = {
    {
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_propset(&A,
                          newcnode_prop_match(&A, RE_LITERAL, "foo",
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                          newcnode_prop_match(&A, RE_LITERAL, "bar",
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                          NULL),
        SJP_NONE),

      NULL,

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A,
              newir_istok(&A, SJP_OBJECT_BEG),
              4, "loop",
              16, "false"),
            NULL
          ),

          newir_block(&A, 16, "false",
            newir_cbranch(&A,
              newir_istok(&A, SJP_OBJECT_END),
              19, "invalid_1",
              20, "false"),
            NULL
          ),

          newir_block(&A, 20, "false",
            newir_cbranch(&A,
              newir_istok(&A, SJP_ARRAY_END),
              19, "invalid_1",
              23, "false"),
            NULL
          ),

          newir_block(&A, 23, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 15, "valid"),
            NULL
          ),

          newir_block(&A, 15, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 4, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              15, "valid",
              7, "false"
            ),
            NULL
          ),

          newir_block(&A, 7, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, 
              newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              9, "M",
              10, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 10, "M_next",
              newir_cbranch(&A, 
                newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
                11, "M",
                12, "M_next"
              ),
            NULL
          ),

          newir_block(&A, 12, "M_next",
              newir_cbranch(&A, 
                newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
                13, "M",
                14, "invalid_9"
              ),
            NULL
          ),

          newir_block(&A, 14, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 9, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 4, "loop"),
            NULL
          ),

          newir_block(&A, 11, "M",
            newir_call(&A, 2),
            newir_branch(&A, 4, "loop"),
            NULL
          ),

          newir_block(&A, 13, "M",
            newir_call(&A, 3),
            newir_branch(&A, 4, "loop"),
            NULL
          ),

          newir_block(&A, 19, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_STRING),
              2, "true",
              5, "invalid_1"
            ),
            NULL
          ),

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 3, "valid"),
            NULL
          ),

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
              2, "true",
              5, "invalid_1"
            ),
            NULL
          ),

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 3, "valid"),
            NULL
          ),

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),

        NULL
      )
    },

    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "foo",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                            NULL),
                          newcnode_prop_default(&A, 
                            newcnode_switch(&A, 0,
                              SJP_TRUE, newcnode_valid(),
                              SJP_FALSE, newcnode_valid(),
                              SJP_NONE)),
                          NULL
                        ),
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
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_TRUE),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_if(&A, newir_istok(&A, SJP_FALSE),
                              newir_seq(&A,
                                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                newir_stmt(&A, JVST_IR_STMT_VALID),
                                NULL
                              ),
                              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                            )
                          ),
                          NULL
                        )
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
      )
    },

    {
      TRANSLATE,
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
      )
    },

    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_propset(&A,
                          newcnode_prop_match(&A, RE_NATIVE, "a*",
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER), SJP_NONE)),
                          newcnode_prop_match(&A, RE_NATIVE, "aaa*",
                            newcnode_switch(&A, 1,
                              SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 20.0),
                              SJP_NONE)),
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

                      // a* (but not aaa*)
                      newir_case(&A, 1,
                        newmatchset(&A, RE_NATIVE, "a*", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),   // XXX - is this necessary?
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
                        )
                      ),

                      // aaa* (also matches a*)
                      newir_case(&A, 2,
                        newmatchset(&A, RE_NATIVE, "a*", RE_NATIVE, "aaa*", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),   // XXX - is this necessary?
                          newir_if(&A, newir_istok(&A, SJP_NUMBER),
                            newir_if(&A, 
                              newir_op(&A, JVST_IR_EXPR_AND,
                                newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
                                newir_op(&A, JVST_IR_EXPR_LE, 
                                  newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                                  newir_num(&A, 20.0))
                              ),
                              newir_seq(&A,
                                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                newir_stmt(&A, JVST_IR_STMT_VALID),
                                NULL
                              ),
                              newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
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
      TRANSLATE,
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
      )

    },

    {
      LINEARIZE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 0, false),
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
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              4, "loop",
              13, "false"
            ),
            NULL
          ),

          newir_block(&A, 13, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              16, "invalid_1",
              17, "false"
            ),
            NULL
          ),

          newir_block(&A, 17, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              16, "invalid_1",
              20, "false"
            ),
            NULL
          ),

          newir_block(&A, 20, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 10, "valid"),
            NULL
          ),

          newir_block(&A, 10, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 4, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "loop_end",
              7, "false"
            ),
            NULL
          ),

          newir_block(&A, 7, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_incr(&A, 0, "num_props"),
            newir_branch(&A, 4, "loop"),
            NULL
          ),

          newir_block(&A, 3, "loop_end",
            // Post-loop check of number of properties
            newir_move(&A, newir_itemp(&A, 0), newir_count(&A, 0, "num_props")),
            newir_move(&A, newir_itemp(&A, 2), newir_itemp(&A, 0)),
            newir_move(&A, newir_itemp(&A, 1), newir_size(&A, 1)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_GE, newir_itemp(&A, 2), newir_itemp(&A,1)),
              10, "valid",
              12, "invalid_4"
            ),
            NULL
          ),

          newir_block(&A, 12, "invalid_4",
            newir_invalid(&A, JVST_INVALID_TOO_FEW_PROPS, "too few properties"),
            NULL
          ),

          newir_block(&A, 16, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          NULL
        ),

        NULL
      )

    },

    {
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 0, 2, true),
        SJP_NONE),

      NULL,

      // XXX - comments here are largely the same as in the previous
      //       test case
      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_counter(&A, 0, "num_props"),

          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              4, "loop",
              13, "false"
            ),
            NULL
          ),

          newir_block(&A, 13, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              16, "invalid_1",
              17, "false"
            ),
            NULL
          ),

          newir_block(&A, 17, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              16, "invalid_1",
              20, "false"
            ),
            NULL
          ),

          newir_block(&A, 20, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 10, "valid"),
            NULL
          ),

          newir_block(&A, 10, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 4, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "loop_end",
              7, "false"
            ),
            NULL
          ),

          newir_block(&A, 7, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_incr(&A, 0, "num_props"),
            newir_branch(&A, 4, "loop"),
            NULL
          ),

          newir_block(&A, 3, "loop_end",
            newir_move(&A, newir_itemp(&A, 0), newir_count(&A, 0, "num_props")),
            newir_move(&A, newir_itemp(&A, 2), newir_itemp(&A, 0)), 
            newir_move(&A, newir_itemp(&A, 1), newir_size(&A, 2)),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_LE, newir_itemp(&A, 2), newir_itemp(&A, 1)),
              10, "valid",
              12, "invalid_5"
            ),

            NULL
          ),

          newir_block(&A, 12, "invalid_5",
            newir_invalid(&A, JVST_INVALID_TOO_MANY_PROPS, "too many properties"),
            NULL
          ),

          newir_block(&A, 16, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          NULL
        ),
        NULL
      )
    },

    {
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 2, 5, true),
        SJP_NONE),

      NULL,

      // XXX - comments here are largely the same as in the previous
      //       test case
      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_counter(&A, 0, "num_props"),

          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              4, "loop",
              17, "false"
            ),
            NULL
          ),

          newir_block(&A, 17, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              20, "invalid_1",
              21, "false"
            ),
            NULL
          ),

          newir_block(&A, 21, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              20, "invalid_1",
              24, "false"
            ),
            NULL
          ),

          newir_block(&A, 24, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 12, "valid"),
            NULL
          ),

          newir_block(&A, 12, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 4, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "loop_end",
              7, "false"
            ),
            NULL
          ),

          newir_block(&A, 7, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_incr(&A, 0, "num_props"),
            newir_branch(&A, 4, "loop"),
            NULL
          ),

          newir_block(&A, 3, "loop_end",
            newir_move(&A, newir_itemp(&A, 3), newir_count(&A, 0, "num_props")),
            newir_move(&A, newir_itemp(&A, 5), newir_itemp(&A, 3)), 
            newir_move(&A, newir_itemp(&A, 4), newir_size(&A, 2)),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_GE, newir_itemp(&A, 5), newir_itemp(&A, 4)),
              9, "true",
              16, "invalid_4"
            ),
            NULL
          ),

          newir_block(&A, 16, "invalid_4",
            newir_invalid(&A, JVST_INVALID_TOO_FEW_PROPS, "too few properties"),
            NULL
          ),


          newir_block(&A, 9, "true",
            newir_move(&A, newir_itemp(&A, 0), newir_count(&A, 0, "num_props")),
            newir_move(&A, newir_itemp(&A, 2), newir_itemp(&A, 0)), 
            newir_move(&A, newir_itemp(&A, 1), newir_size(&A, 5)),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_LE, newir_itemp(&A, 2), newir_itemp(&A, 1)),
              12, "valid",
              14, "invalid_5"
            ),

            NULL
          ),

          newir_block(&A, 14, "invalid_5",
            newir_invalid(&A, JVST_INVALID_TOO_MANY_PROPS, "too many properties"),
            NULL
          ),

          newir_block(&A, 20, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          NULL
        ),
        NULL
      )
    },

    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 0, true),
                          newcnode_valid(),
                          NULL),
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
                    newir_size(&A, 1)
                  ),
                  newir_if(&A,
                    newir_op(&A, JVST_IR_EXPR_LE, 
                      newir_count(&A, 0, "num_props"),
                      newir_size(&A, 0)
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
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
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

      NULL,

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_counter(&A, 0, "num_props"),
          newir_matcher(&A, 0, "dfa"),

          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              4, "loop",
              20, "false"
            ),
            NULL
          ),

          newir_block(&A, 20, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              23, "invalid_1",
              24, "false"
            ),
            NULL
          ),

          newir_block(&A, 24, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              23, "invalid_1",
              27, "false"
            ),
            NULL
          ),

          newir_block(&A, 27, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 17, "valid"),
            NULL
          ),

          newir_block(&A, 17, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 4, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "loop_end",
              7, "false"
            ),
            NULL
          ),

          newir_block(&A, 7, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              9, "M",
              10, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              11, "M",
              12, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 12, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              13, "M",
              14, "invalid_9"
            ),
            NULL
          ),

          newir_block(&A, 14, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 9, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 8, "M_join"),
            NULL
          ),

          newir_block(&A, 8, "M_join",
              newir_incr(&A, 0, "num_props"),
              newir_branch(&A, 4, "loop"),
              NULL
          ),

          newir_block(&A, 11, "M",
            newir_call(&A, 2),
            newir_branch(&A, 8, "M_join"),
            NULL
          ),

          newir_block(&A, 13, "M",
            newir_call(&A, 3),
            newir_branch(&A, 8, "M_join"),
            NULL
          ),

          newir_block(&A, 3, "loop_end",
            newir_move(&A, newir_itemp(&A, 1), newir_count(&A, 0, "num_props")),
            newir_move(&A, newir_itemp(&A, 3), newir_itemp(&A, 1)),
            newir_move(&A, newir_itemp(&A, 2), newir_size(&A, 1)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_GE, newir_itemp(&A, 3), newir_itemp(&A, 2)),
              17, "valid",
              19, "invalid_4"
            ),
            NULL
          ),

          newir_block(&A, 19, "invalid_4",
            newir_invalid(&A, JVST_INVALID_TOO_FEW_PROPS, "too few properties"),
            NULL
          ),

          newir_block(&A, 23, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_STRING),
              2, "true",
              5, "invalid_1"
            ),
            NULL
          ),

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 3, "valid"),
            NULL
          ),

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
              2, "true",
              5, "invalid_1"
            ),
            NULL
          ),

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 3, "valid"),
            NULL
          ),

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

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
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
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
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              4, "loop",
              20, "false"
            ),
            NULL
          ),

          newir_block(&A, 20, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              23, "invalid_1",
              24, "false"
            ),
            NULL
          ),

          newir_block(&A, 24, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              23, "invalid_1",
              27, "false"
            ),
            NULL
          ),

          newir_block(&A, 27, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 17, "valid"),
            NULL
          ),

          newir_block(&A, 17, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 4, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              3, "loop_end",
              7, "false"
            ),
            NULL
          ),

          newir_block(&A, 7, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              9, "M",
              10, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              11, "M",
              12, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 12, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              13, "M",
              14, "invalid_9"
            ),
            NULL
          ),

          newir_block(&A, 14, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 9, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 4, "loop"),
            NULL
          ),

          newir_block(&A, 11, "M",
            newir_call(&A, 2),
            newir_branch(&A, 4, "loop"),
            NULL
          ),

          newir_block(&A, 13, "M",
            newir_call(&A, 3),
            newir_bitop(&A, JVST_IR_STMT_BSET, 1, "reqmask", 0),
            newir_branch(&A, 4, "loop"),
            NULL
          ),

          newir_block(&A, 3, "loop_end",
            newir_cbranch(&A, newir_btestall(&A, 1, "reqmask", 0,0),
              17, "valid",
              19, "invalid_6"
            ),
            NULL
          ),

          newir_block(&A, 19, "invalid_6",
            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
              "missing required properties"),
            NULL
          ),

          newir_block(&A, 23, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_STRING),
              2, "true",
              5, "invalid_1"
            ),
            NULL
          ),

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 3, "valid"),
            NULL
          ),

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
              2, "true",
              5, "invalid_1"
            ),
            NULL
          ),

          newir_block(&A, 5, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 3, "valid"),
            NULL
          ),

          newir_block(&A, 3, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),

        NULL
      )
    },

    {
      FLATTEN,
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

          newir_block(&A, 0, "entry", NULL),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
            4, "loop",
            20, "false"
          ),

          newir_block(&A, 20, "false", NULL),
          newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
            23, "invalid_1",
            24, "false"
          ),

          newir_block(&A, 24, "false", NULL),
          newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
            23, "invalid_1",
            27, "false"
          ),

          newir_block(&A, 27, "false", NULL),
          newir_stmt(&A, JVST_IR_STMT_CONSUME),

          newir_block(&A, 17, "valid", NULL),
          newir_stmt(&A, JVST_IR_STMT_VALID),

          newir_block(&A, 4, "loop", NULL),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
            3, "loop_end",
            7, "false"
          ),

          newir_block(&A, 7, "false", NULL),
          newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
          newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
            9, "M",
            10, "M_next"
          ),

          newir_block(&A, 10, "M_next", NULL),
          newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
            11, "M",
            12, "M_next"
          ),

          newir_block(&A, 12, "M_next", NULL),
          newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
            13, "M",
            14, "invalid_9"
          ),

          newir_block(&A, 14, "invalid_9", NULL),
          newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),

          newir_block(&A, 9, "M", NULL),
          newir_stmt(&A, JVST_IR_STMT_CONSUME),
          newir_branch(&A, 4, "loop"),

          newir_block(&A, 11, "M", NULL),
          newir_call(&A, 2),
          newir_branch(&A, 4, "loop"),

          newir_block(&A, 13, "M", NULL),
          newir_call(&A, 3),
          newir_bitop(&A, JVST_IR_STMT_BSET, 1, "reqmask", 0),
          newir_branch(&A, 4, "loop"),

          newir_block(&A, 3, "loop_end", NULL),
          newir_cbranch(&A, newir_btestall(&A, 1, "reqmask", 0,0),
            17, "valid",
            19, "invalid_6"
          ),

          newir_block(&A, 19, "invalid_6", NULL),
          newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
            "missing required properties"),

          newir_block(&A, 23, "invalid_1", NULL),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_block(&A, 0, "entry", NULL),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_cbranch(&A, newir_istok(&A, SJP_STRING),
            2, "true",
            5, "invalid_1"
          ),

          newir_block(&A, 5, "invalid_1", NULL),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),

          newir_block(&A, 2, "true", NULL),
          newir_stmt(&A, JVST_IR_STMT_CONSUME),

          newir_block(&A, 3, "valid", NULL),
          newir_stmt(&A, JVST_IR_STMT_VALID),

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_block(&A, 0, "entry", NULL),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
            2, "true",
            5, "invalid_1"
          ),

          newir_block(&A, 5, "invalid_1", NULL),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),

          newir_block(&A, 2, "true", NULL),
          newir_stmt(&A, JVST_IR_STMT_CONSUME),

          newir_block(&A, 3, "valid", NULL),
          newir_stmt(&A, JVST_IR_STMT_VALID),

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
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
      // schema: { "dependencies": {"bar": ["foo"]} }
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                          newcnode_required(&A, stringset(&A, "bar", "foo", NULL)),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "bar", newcnode_invalid()),
                            NULL),
                          NULL),
        SJP_NONE),

      NULL,

      newir_program(&A,
        newir_frame(&A, frameindex, 1,
          newir_splitlist(&A, 0, 2, 2, 3),
          newir_block(&A, 0, "entry",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              2, "true",
              8, "false"
            ),
            NULL
          ),

          newir_block(&A, 8, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              11, "invalid_1",
              12, "false"
            ),
            NULL
          ),

          newir_block(&A, 12, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              11, "invalid_1",
              15, "false"
            ),
            NULL
          ),

          newir_block(&A, 15, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 5, "valid"),
            NULL
          ),

          newir_block(&A, 5, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_move(&A, newir_itemp(&A, 0), newir_split(&A, splitlist, 0)),
            newir_move(&A, newir_itemp(&A, 2), newir_itemp(&A, 0)),
            newir_move(&A, newir_itemp(&A, 1), newir_size(&A, 1)),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_GE, newir_itemp(&A, 2), newir_itemp(&A, 1)),
              5, "valid",
              7, "invalid_7"
            ),
            NULL
          ),

          newir_block(&A, 7, "invalid_7",
            newir_invalid(&A, JVST_INVALID_SPLIT_CONDITION, "invalid split condition"),
            NULL
          ),

          newir_block(&A, 11, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              12, "valid",
              5, "false"
            ),
            NULL
          ),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              10, "invalid_8",
              11, "invalid_9"
            ),
            NULL
          ),

          newir_block(&A, 11, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 12, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 10, "invalid_8",
            newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name"),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_bitvec(&A, 0, "reqmask", 2),
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              1, "loop_end",
              5, "false"
            ),
            NULL
          ),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              9, "M",
              10, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              11, "M",
              12, "invalid_9"
            ),
            NULL
          ),

          newir_block(&A, 12, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 9, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 11, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 1, "loop_end",
            newir_cbranch(&A, newir_btestall(&A, 0, "reqmask", 0, 1),
              15, "valid",
              17, "invalid_6"
            ),
            NULL
          ),

          newir_block(&A, 17, "invalid_6",
            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
              "missing required properties"),
            NULL
          ),

          newir_block(&A, 15, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),

        NULL
      )
    },

    {
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
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
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              2, "true",
              8, "false"
            ),
            NULL
          ),

          newir_block(&A, 8, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              11, "invalid_1",
              12, "false"
            ),
            NULL
          ),

          newir_block(&A, 12, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              11, "invalid_1",
              15, "false"
            ),
            NULL
          ),

          newir_block(&A, 15, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 5, "valid"),
            NULL
          ),

          newir_block(&A, 5, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_move(&A, newir_itemp(&A, 0), newir_split(&A, splitlist, 0)),
            newir_move(&A, newir_itemp(&A, 2), newir_itemp(&A, 0)),
            newir_move(&A, newir_itemp(&A, 1), newir_size(&A, 1)),

            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_GE, newir_itemp(&A, 2), newir_itemp(&A, 1)),
              5, "valid",
              7, "invalid_7"
            ),
            NULL
          ),

          newir_block(&A, 7, "invalid_7",
            newir_invalid(&A, JVST_INVALID_SPLIT_CONDITION, "invalid split condition"),
            NULL
          ),

          newir_block(&A, 11, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              12, "valid",
              5, "false"
            ),
            NULL
          ),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              10, "invalid_8",
              11, "invalid_9"
            ),
            NULL
          ),

          newir_block(&A, 11, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 12, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 10, "invalid_8",
            newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name"),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_bitvec(&A, 0, "reqmask", 3),
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              1, "loop_end",
              5, "false"
            ),
            NULL
          ),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              9, "M",
              10, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              11, "M",
              12, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 12, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 3)),
              13, "M",
              14, "invalid_9"
            ),
            NULL
          ),

          newir_block(&A, 14, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 9, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 2),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 11, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 13, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 1, "loop_end",
            newir_cbranch(&A, newir_btestall(&A, 0, "reqmask", 0, 2),
              17, "valid",
              19, "invalid_6"
            ),
            NULL
          ),

          newir_block(&A, 19, "invalid_6",
            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
              "missing required properties"),
            NULL
          ),

          newir_block(&A, 17, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),

        NULL
      )
    },

    {
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
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
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_BEG),
              2, "true",
              11, "false"
            ),
            NULL
          ),

          newir_block(&A, 11, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              14, "invalid_1",
              15, "false"
            ),
            NULL
          ),

          newir_block(&A, 15, "false",
            newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
              14, "invalid_1",
              18, "false"
            ),
            NULL
          ),

          newir_block(&A, 18, "false",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 5, "valid"),
            NULL
          ),

          newir_block(&A, 5, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 2, "true",
            newir_splitvec(&A, 0, "splitvec", splitlist, 0),

            newir_cbranch(&A, newir_btestany(&A, 0, "splitvec", 0, 0),  // XXX - can combine the OR'd stuff...
              8, "and_true",
              9, "or_false"
            ),
            NULL
          ),

          newir_block(&A, 9, "or_false",
            newir_cbranch(&A, newir_btest(&A, 0, "splitvec", 1),  // XXX - can combine the OR'd stuff...
              8, "and_true",
              7, "invalid_7"
            ),
            NULL
          ),

          newir_block(&A, 7, "invalid_7",
            newir_invalid(&A, JVST_INVALID_SPLIT_CONDITION, "invalid split condition"),
            NULL
          ),

          newir_block(&A, 8, "and_true",
            newir_cbranch(&A, newir_btestany(&A, 0, "splitvec", 2,2),  // XXX - can combine the OR'd stuff...
              5, "valid",
              10, "or_false"
            ),
            NULL
          ),

          newir_block(&A, 10, "or_false",
            newir_cbranch(&A, newir_btest(&A, 0, "splitvec", 3),  // XXX - can combine the OR'd stuff...
              5, "valid",
              7, "invalid_7"
            ),
            NULL
          ),

          newir_block(&A, 14, "invalid_1",
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 2,
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              12, "valid",
              5, "false"
            ),
            NULL
          ),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              10, "invalid_8",
              11, "invalid_9"
            ),
            NULL
          ),

          newir_block(&A, 11, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 12, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 10, "invalid_8",
            newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name"),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 3,
          newir_bitvec(&A, 0, "reqmask", 3),
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              1, "loop_end",
              5, "false"
            ),
            NULL
          ),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              9, "M",
              10, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              11, "M",
              12, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 12, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 3)),
              13, "M",
              14, "invalid_9"
            ),
            NULL
          ),

          newir_block(&A, 14, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 9, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 2),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 11, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 13, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 1, "loop_end",
            newir_cbranch(&A, newir_btestall(&A, 0, "reqmask", 0, 2),
              17, "valid",
              19, "invalid_6"
            ),
            NULL
          ),

          newir_block(&A, 19, "invalid_6",
            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
              "missing required properties"),
            NULL
          ),

          newir_block(&A, 17, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 4,
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              12, "valid",
              5, "false"
            ),
            NULL
          ),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              10, "invalid_8",
              11, "invalid_9"
            ),
            NULL
          ),

          newir_block(&A, 11, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 12, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

          newir_block(&A, 10, "invalid_8",
            newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name"),
            NULL
          ),

          NULL
        ),

        newir_frame(&A, frameindex, 5,
          newir_bitvec(&A, 0, "reqmask", 2),
          newir_matcher(&A, 0, "dfa"),
          newir_block(&A, 0, "entry",
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 2, "loop",
            newir_stmt(&A, JVST_IR_STMT_TOKEN),
            newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
              1, "loop_end",
              5, "false"
            ),
            NULL
          ),

          newir_block(&A, 5, "false",
            newir_move(&A, newir_itemp(&A, 0), newir_ematch(&A, 0)),
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 0)),
              7, "M",
              8, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 8, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 1)),
              9, "M",
              10, "M_next"
            ),
            NULL
          ),

          newir_block(&A, 10, "M_next",
            newir_cbranch(&A, newir_op(&A, JVST_IR_EXPR_EQ, newir_itemp(&A, 0), newir_size(&A, 2)),
              11, "M",
              12, "invalid_9"
            ),
            NULL
          ),

          newir_block(&A, 12, "invalid_9",
            newir_invalid(&A, JVST_INVALID_MATCH_CASE, "invalid match case (internal error)"),
            NULL
          ),

          newir_block(&A, 7, "M",
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 9, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 1),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 11, "M",
            newir_bitop(&A, JVST_IR_STMT_BSET, 0, "reqmask", 0),
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_branch(&A, 2, "loop"),
            NULL
          ),

          newir_block(&A, 1, "loop_end",
            newir_cbranch(&A, newir_btestall(&A, 0, "reqmask", 0, 1),
              15, "valid",
              17, "invalid_6"
            ),
            NULL
          ),

          newir_block(&A, 17, "invalid_6",
            newir_invalid(&A, JVST_INVALID_MISSING_REQUIRED_PROPERTIES,
              "missing required properties"),
            NULL
          ),

          newir_block(&A, 15, "valid",
            newir_stmt(&A, JVST_IR_STMT_VALID),
            NULL
          ),

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

void test_ir_patterns(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct ir_test tests[] = {
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "a+b.d"),
        SJP_NONE),

      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_STRING),
            newir_match(&A, 0,
              newir_case(&A, 0, 
                NULL,
                newir_invalid(&A, JVST_INVALID_STRING, "invalid string")
              ),

              // match "bar"
              newir_case(&A, 1,
                newmatchset(&A, RE_NATIVE,  "a+b.d", -1),
                newir_stmt(&A, JVST_IR_STMT_VALID)
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
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_ir_minmax_length_1(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 12, 0, false),
                      newcnode_valid(),
                      NULL),
        SJP_NONE),

      newir_frame(&A,
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_STRING),
          newir_seq(&A,
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_if(&A,
              newir_op(&A, JVST_IR_EXPR_GE, 
                newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                newir_size(&A, 12)
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              newir_invalid(&A, JVST_INVALID_LENGTH_TOO_SHORT, "length is too short")
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
      )
    },

    {
      TRANSLATE,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 0, 36, true),
                      newcnode_valid(),
                      NULL),
        SJP_NONE),

      newir_frame(&A,
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_STRING),
          newir_seq(&A,
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_if(&A,
              newir_op(&A, JVST_IR_EXPR_LE, 
                newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                newir_size(&A, 36)
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              newir_invalid(&A, JVST_INVALID_LENGTH_TOO_LONG, "length is too long")
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
      )
    },

    {
      TRANSLATE,
      // newschema_p(&A, 0, "minLength", 23, "maxLength", 50, NULL),

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 23, 50, true),
                      newcnode_valid(),
                      NULL),
        SJP_NONE),

      newir_frame(&A,
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_STRING),
          newir_seq(&A,
            newir_stmt(&A, JVST_IR_STMT_CONSUME),
            newir_if(&A,
              newir_op(&A, JVST_IR_EXPR_GE, 
                newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                newir_size(&A, 23)
              ),
              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_LE, 
                  newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                  newir_size(&A, 50)
                ),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                newir_invalid(&A, JVST_INVALID_LENGTH_TOO_LONG, "length is too long")
              ),
              newir_invalid(&A, JVST_INVALID_LENGTH_TOO_SHORT, "length is too short")
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
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_ir_str_constraints(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_strmatch(&A, RE_NATIVE, "a+b.d"),
                      newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 12, 0, false),
                      newcnode_valid(),
                      NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_STRING),
            newir_match(&A, 0,
              newir_case(&A, 0, 
                NULL,
                newir_invalid(&A, JVST_INVALID_STRING, "invalid string")
              ),

              // match "bar"
              newir_case(&A, 1,
                newmatchset(&A, RE_NATIVE,  "a+b.d", -1),
                newir_if(&A,
                  newir_op(&A, JVST_IR_EXPR_GE, 
                    newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                    newir_size(&A, 12)
                  ),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  newir_invalid(&A, JVST_INVALID_LENGTH_TOO_SHORT, "length is too short")
                )
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
      )

    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_ir_propertynames()
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,
      /*
      newschema_p(&A, 0,
        "properties", newprops(&A, "foo", newschema(&A, JSON_VALUE_NUMBER), NULL),
        "propertyNames", "f.*",
        NULL
      ),
      */
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_propnames(&A,
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "f.*"), SJP_NONE)),
                          newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                              NULL
                            ),
                            newcnode_valid(),
                            NULL
                          ),
                          NULL
                        ),
        SJP_NONE),

      /*
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_invalid(),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_LITERAL, "foo", RE_NATIVE, "f.*", -1),
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_NATIVE, "f.*", -1),
                            newcnode_valid()),
                          NULL),
        SJP_NONE)
      */

      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,
                    newir_match(&A, 0,
                      newir_case(&A, 0, 
                        NULL,
                        newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                      ),

                      newir_case(&A, 1,
                        newmatchset(&A, RE_LITERAL, "foo", RE_NATIVE,  "f.*", -1),
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

                      newir_case(&A, 2,
                        newmatchset(&A, RE_NATIVE,  "f.*", -1),
                        newir_stmt(&A, JVST_IR_STMT_CONSUME)
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
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    {
      TRANSLATE,
      /*
      newschema_p(&A, 0,
          "properties", newprops(&A,
                          "foo", newschema(&A, JSON_VALUE_NUMBER), // XXX - JSON_VALUE_INTEGER
                          "bar", newschema(&A, JSON_VALUE_STRING),
                          NULL),
          "additionalProperties", newschema(&A, JSON_VALUE_BOOL),
          "propertyNames", "f.*",
          NULL),
          */

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_propnames(&A,
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "f.*"), SJP_NONE)),
                          newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_prop_default(&A, 
                              newcnode_switch(&A, 0,
                                SJP_TRUE, newcnode_valid(),
                                SJP_FALSE, newcnode_valid(),
                                SJP_NONE)),
                            newcnode_bool(&A,JVST_CNODE_AND,
                              newcnode_propset(&A,
                                newcnode_prop_match(&A, RE_LITERAL, "foo",
                                  newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                                newcnode_prop_match(&A, RE_LITERAL, "bar",
                                  newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                                NULL),
                              newcnode_valid(),
                              NULL),
                            NULL),
                          NULL),
        SJP_NONE),

      /*
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_invalid(),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_LITERAL, "foo", RE_NATIVE, "f.*", -1),
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_NATIVE, "f.*", -1),
                            newcnode_switch(&A, 0,
                              SJP_TRUE, newcnode_valid(),
                              SJP_FALSE, newcnode_valid(),
                              SJP_NONE)),
                          NULL),
        SJP_NONE)
      */

      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,
                    newir_match(&A, 0,
                      newir_case(&A, 0, 
                        NULL,
                        newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                      ),

                      newir_case(&A, 1, 
                        newmatchset(&A, RE_LITERAL, "bar", -1),
                        newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                      ),

                      newir_case(&A, 2,
                        newmatchset(&A, RE_LITERAL, "foo", RE_NATIVE,  "f.*", -1),
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

                      newir_case(&A, 3,
                        newmatchset(&A, RE_NATIVE,  "f.*", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_TRUE),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_if(&A, newir_istok(&A, SJP_FALSE),
                              newir_seq(&A,
                                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                newir_stmt(&A, JVST_IR_STMT_VALID),
                                NULL
                              ),
                              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                            )
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
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    {
      TRANSLATE,
      /*
      newschema_p(&A, 0,
          "additionalProperties", newschema(&A, JSON_VALUE_BOOL),
          "propertyNames", "f.*",
          NULL),
          */
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_propnames(&A,
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "f.*"), SJP_NONE)),
                          newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_prop_default(&A, 
                              newcnode_switch(&A, 0,
                                SJP_TRUE, newcnode_valid(),
                                SJP_FALSE, newcnode_valid(),
                                SJP_NONE)),
                            NULL),
                          NULL),
        SJP_NONE),

      /*
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_invalid(),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_NATIVE, "f.*", -1),
                            newcnode_switch(&A, 0,
                              SJP_TRUE, newcnode_valid(),
                              SJP_FALSE, newcnode_valid(),
                              SJP_NONE)),
                          NULL),
        SJP_NONE),
      */

      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,
                    newir_match(&A, 0,
                      newir_case(&A, 0, 
                        NULL,
                        newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                      ),

                      newir_case(&A, 1,
                        newmatchset(&A, RE_NATIVE,  "f.*", -1),
                        newir_frame(&A,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_TRUE),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_if(&A, newir_istok(&A, SJP_FALSE),
                              newir_seq(&A,
                                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                newir_stmt(&A, JVST_IR_STMT_VALID),
                                NULL
                              ),
                              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
                            )
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
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    {
      TRANSLATE,
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propnames(&A,
                          newcnode_switch(&A, 0,
                            SJP_STRING, newcnode_bool(&A,JVST_CNODE_AND,
                                          newcnode_strmatch(&A, RE_NATIVE, "f.*"),
                                          newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 3, 16, true),
                                          NULL),
                            SJP_NONE)),
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
                  newir_seq(&A,
                    newir_match(&A, 0,
                      newir_case(&A, 0, 
                        NULL,
                        newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                      ),

                      newir_case(&A, 1,
                        newmatchset(&A, RE_NATIVE,  "f.*", -1),
                        newir_if(&A,
                          newir_op(&A, JVST_IR_EXPR_GE, 
                            newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                            newir_size(&A, 3)
                          ),
                          newir_if(&A,
                            newir_op(&A, JVST_IR_EXPR_LE,
                              newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                              newir_size(&A, 16)
                            ),
                            newir_stmt(&A, JVST_IR_STMT_CONSUME),
                            newir_invalid(&A, JVST_INVALID_LENGTH_TOO_LONG, "length is too long")
                          ),
                          newir_invalid(&A, JVST_INVALID_LENGTH_TOO_SHORT, "length is too short")
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
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    {
      TRANSLATE,
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propnames(&A,
                          newcnode_switch(&A, 0,
                            SJP_STRING, newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 3, 16, true),
                            SJP_NONE)),
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
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_if(&A,
                      newir_op(&A, JVST_IR_EXPR_GE, 
                        newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                        newir_size(&A, 3)
                      ),
                      newir_if(&A,
                        newir_op(&A, JVST_IR_EXPR_LE,
                          newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                          newir_size(&A, 16)
                        ),
                        newir_stmt(&A, JVST_IR_STMT_CONSUME),
                        newir_invalid(&A, JVST_INVALID_LENGTH_TOO_LONG, "length is too long")
                      ),
                      newir_invalid(&A, JVST_INVALID_LENGTH_TOO_SHORT, "length is too short")
                    ),
                    NULL
                  )
                ),
                NULL
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    {
      TRANSLATE,
      /*
      newschema_p(&A, 0,
          "propertyNames", "f.*",
          NULL),
          */

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propnames(&A,
                          newcnode_switch(&A, 0, SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "f.*"), SJP_NONE)),
        SJP_NONE),

      /*
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_invalid(),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_NATIVE, "f.*", -1),
                            newcnode_valid()),
                          NULL),
        SJP_NONE)
      */

      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,
                    newir_match(&A, 0,
                      newir_case(&A, 0, 
                        NULL,
                        newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name")
                      ),

                      newir_case(&A, 1,
                        newmatchset(&A, RE_NATIVE,  "f.*", -1),
                        newir_stmt(&A, JVST_IR_STMT_CONSUME)
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
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    // binary schema
    {
      TRANSLATE,
      /*
      newschema_p(&A, 0, "propertyNames", true_schema(), NULL),
      */

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propnames(&A,
                          newcnode_switch(&A, 1, SJP_NONE)),
        SJP_NONE),

      /*
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_valid(),
                          NULL),
        SJP_NONE)
        */

      /* TODO: we can improve this (maybe at the cnode level?
       * the entire loop should become a CONSUME
       */
      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    NULL
                  )
                ),
                NULL
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    {
      TRANSLATE,
      /*
      newschema_p(&A, 0, "propertyNames", false_schema(), NULL),
      */

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propnames(&A,
                          newcnode_switch(&A, 0, SJP_NONE)),
        SJP_NONE),

      /*
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_invalid(),
                          NULL),
        SJP_NONE)
      */

      // TODO: we can improve this, as well:
      //   SEQ(CONSUME, INVALID(BAD_PROPERTY_NAME))
      // can be replaced with:
      //   INVALID(BAD_PROPERTY_NAME)
      newir_frame(&A,
          newir_matcher(&A, 0, "dfa"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_OBJECT_BEG),
            newir_seq(&A,
              newir_loop(&A, "L_OBJ", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
                  newir_break(&A, "L_OBJ", 0),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_CONSUME),
                    newir_invalid(&A, JVST_INVALID_BAD_PROPERTY_NAME, "bad property name"),
                    NULL
                  )
                ),
                NULL
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_ir_items_1(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_items(&A,
                             newcnode_switch(&A, 0,
                               SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                               SJP_NONE),
                             NULL),
                           newcnode_valid(),
                           NULL),
            SJP_NONE),

      newir_frame(&A,
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_loop(&A, "ARR_OUTER", 0,
                newir_loop(&A, "ARR_INNER", 1,
                  newir_stmt(&A, JVST_IR_STMT_TOKEN),
                  newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                    newir_break(&A, "ARR_OUTER", 0),
                    newir_seq(&A,
                      newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
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
                      NULL
                    )
                  ),
                  NULL
                ),
                NULL
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
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
      )
    },

    {
      TRANSLATE,

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_items(&A,
                             NULL,
                             newcnode_switch(&A, 0,
                               SJP_STRING, newcnode_valid(),
                               SJP_NONE),
                             newcnode_switch(&A, 0,
                               SJP_STRING, newcnode_valid(),
                               SJP_NONE),
                             NULL),
                           newcnode_valid(),
                           NULL),
            SJP_NONE),

      newir_frame(&A,
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_loop(&A, "ARR_OUTER", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                  newir_break(&A, "ARR_OUTER", 0),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
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
                    ),
                    newir_stmt(&A, JVST_IR_STMT_TOKEN),
                    newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                      newir_break(&A, "ARR_OUTER", 0),
                      newir_seq(&A,
                        newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
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
                        ),
                        newir_loop(&A, "ARR_INNER", 1,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                            newir_break(&A, "ARR_OUTER", 0),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
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
                  )
                ),
                NULL
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
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
      )
    },

    {
      TRANSLATE,

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_items(&A,
                           newcnode_switch(&A, 0,
                             SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                             SJP_NONE),
                           newcnode_switch(&A, 0,
                             SJP_STRING, newcnode_valid(),
                             SJP_NONE),
                           newcnode_switch(&A, 0,
                             SJP_STRING, newcnode_valid(),
                             SJP_NONE),
                           NULL),
            SJP_NONE),

      newir_frame(&A,
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_loop(&A, "ARR_OUTER", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                  newir_break(&A, "ARR_OUTER", 0),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
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
                    ),
                    newir_stmt(&A, JVST_IR_STMT_TOKEN),
                    newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                      newir_break(&A, "ARR_OUTER", 0),
                      newir_seq(&A,
                        newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
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
                        ),
                        newir_loop(&A, "ARR_INNER", 1,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                            newir_break(&A, "ARR_OUTER", 0),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
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
                              NULL
                            )
                          ),
                          NULL
                        ),
                        NULL
                      )
                    ),
                    NULL
                  )
                ),
                NULL
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
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
      )

    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_ir_minmax_items(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,
      // newschema_p(&A, 0, "minItems", 3, NULL),
      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 0, false),
                           newcnode_valid(),
                           NULL),
            SJP_NONE),

      newir_frame(&A,
        newir_counter(&A, 0, "num_items"),
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_loop(&A, "ARR_OUTER", 0,
                newir_loop(&A, "ARR_INNER", 1,
                  newir_stmt(&A, JVST_IR_STMT_TOKEN),
                  newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                    newir_break(&A, "ARR_OUTER", 0),
                    newir_seq(&A,
                      newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                      newir_incr(&A, 0, "num_items"),
                      newir_stmt(&A, JVST_IR_STMT_CONSUME),
                      NULL
                    )
                  ),
                  NULL
                ),
                NULL
              ),

              // Post-loop check of number of items
              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_count(&A, 0, "num_items"),
                  newir_size(&A, 3)
                  ),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                newir_invalid(&A, JVST_INVALID_TOO_FEW_ITEMS, "array has too few items")
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
      )
    },

    {
      TRANSLATE,
      // newschema_p(&A, 0, "maxItems", 3, NULL),
      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                           newcnode_valid(),
                           NULL),
            SJP_NONE),

      newir_frame(&A,
        newir_counter(&A, 0, "num_items"),
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_loop(&A, "ARR_OUTER", 0,
                newir_loop(&A, "ARR_INNER", 1,
                  newir_stmt(&A, JVST_IR_STMT_TOKEN),
                  newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                    newir_break(&A, "ARR_OUTER", 0),
                    newir_seq(&A,
                      newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                      newir_incr(&A, 0, "num_items"),
                      newir_stmt(&A, JVST_IR_STMT_CONSUME),
                      NULL
                    )
                  ),
                  NULL
                ),
                NULL
              ),

              // Post-loop check of number of items
              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_LE, 
                  newir_count(&A, 0, "num_items"),
                  newir_size(&A, 3)
                  ),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                newir_invalid(&A, JVST_INVALID_TOO_MANY_ITEMS, "array has too many items")
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
      )
    },

    {
      TRANSLATE,
      /*
      newschema_p(&A, 0,
          "minItems", 1,
          "maxItems", 5,
          NULL),
      */
      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 1, 5, true),
                           newcnode_valid(),
                           NULL),
            SJP_NONE),

      newir_frame(&A,
        newir_counter(&A, 0, "num_items"),
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_loop(&A, "ARR_OUTER", 0,
                newir_loop(&A, "ARR_INNER", 1,
                  newir_stmt(&A, JVST_IR_STMT_TOKEN),
                  newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                    newir_break(&A, "ARR_OUTER", 0),
                    newir_seq(&A,
                      newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                      newir_incr(&A, 0, "num_items"),
                      newir_stmt(&A, JVST_IR_STMT_CONSUME),
                      NULL
                    )
                  ),
                  NULL
                ),
                NULL
              ),

              // Post-loop check of number of items
              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_count(&A, 0, "num_items"),
                  newir_size(&A, 1)
                ),
                newir_if(&A,
                  newir_op(&A, JVST_IR_EXPR_LE, 
                    newir_count(&A, 0, "num_items"),
                    newir_size(&A, 5)
                  ),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  newir_invalid(&A, JVST_INVALID_TOO_MANY_ITEMS, "array has too many items")
                ),
                newir_invalid(&A, JVST_INVALID_TOO_FEW_ITEMS, "array has too few items")
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
      )
    },

    {
      TRANSLATE,
      /*
      newschema_p(&A, 0,
          "items_single", newschema_p(&A, JSON_VALUE_INTEGER, NULL),
          "maxItems", 5,
          NULL),
      */
      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 5, true),
                           newcnode_bool(&A, JVST_CNODE_AND, 
                             newcnode_items(&A,
                               newcnode_switch(&A, 0,
                                 SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                 SJP_NONE),
                               NULL),
                             newcnode_valid(),
                             NULL),
                           NULL),
            SJP_NONE),

      newir_frame(&A,
        newir_counter(&A, 0, "num_items"),
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_loop(&A, "ARR_OUTER", 0,
                newir_loop(&A, "ARR_INNER", 1,
                  newir_stmt(&A, JVST_IR_STMT_TOKEN),
                  newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                    newir_break(&A, "ARR_OUTER", 0),
                    newir_seq(&A,
                      newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                      newir_incr(&A, 0, "num_items"),
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
                      NULL
                    )
                  ),
                  NULL
                ),
                NULL
              ),

              // Post-loop check of number of items
              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_LE, 
                  newir_count(&A, 0, "num_items"),
                  newir_size(&A, 5)
                ),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                newir_invalid(&A, JVST_INVALID_TOO_MANY_ITEMS, "array has too many items")
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
      )
    },

    {
      TRANSLATE,
      /*
      newschema_p(&A, 0,
          "items", schema_set(&A, 
             newschema_p(&A, JSON_VALUE_STRING, NULL),
             newschema_p(&A, JSON_VALUE_STRING, NULL),
             NULL),
          "additionalItems", newschema_p(&A, JSON_VALUE_INTEGER, NULL),
          "minItems", 1,
          "maxItems", 7,
          NULL),
      */

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 1, 7, true),
                           newcnode_items(&A,
                             newcnode_switch(&A, 0,
                               SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                               SJP_NONE),
                             newcnode_switch(&A, 0,
                               SJP_STRING, newcnode_valid(),
                               SJP_NONE),
                             newcnode_switch(&A, 0,
                               SJP_STRING, newcnode_valid(),
                               SJP_NONE),
                             NULL),
                           NULL),
            SJP_NONE),

      newir_frame(&A,
        newir_counter(&A, 0, "num_items"),
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_loop(&A, "ARR_OUTER", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                  newir_break(&A, "ARR_OUTER", 0),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                    newir_incr(&A, 0, "num_items"),
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
                    ),
                    newir_stmt(&A, JVST_IR_STMT_TOKEN),
                    newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                      newir_break(&A, "ARR_OUTER", 0),
                      newir_seq(&A,
                        newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                        newir_incr(&A, 0, "num_items"),
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
                        ),
                        newir_loop(&A, "ARR_INNER", 1,
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                            newir_break(&A, "ARR_OUTER", 0),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                              newir_incr(&A, 0, "num_items"),
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
                              NULL
                            )
                          ),
                          NULL
                        ),
                        NULL
                      )
                    ),
                    NULL
                  )
                ),
                NULL
              ),

              // Post-loop check of number of items
              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_count(&A, 0, "num_items"),
                  newir_size(&A, 1)
                ),
                newir_if(&A,
                  newir_op(&A, JVST_IR_EXPR_LE, 
                    newir_count(&A, 0, "num_items"),
                    newir_size(&A, 7)
                  ),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  newir_invalid(&A, JVST_INVALID_TOO_MANY_ITEMS, "array has too many items")
                ),
                newir_invalid(&A, JVST_INVALID_TOO_FEW_ITEMS, "array has too few items")
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
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_ir_contains_1(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_contains(&A,
                           newcnode_switch(&A, 1,
                             SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                                           newcnode_range(&A, JVST_CNODE_RANGE_MIN, 5.0, 0.0),
                                           newcnode_valid(),
                                           NULL),
                             SJP_NONE)
                         ),
          SJP_NONE),

      newir_frame(&A,
        newir_bitvec(&A, 1, "contains", 1),
        newir_bitvec(&A, 0, "uniq_contains_split", 2),
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
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
                          newir_stmt(&A, JVST_IR_STMT_TOKEN),
                          newir_if(&A, newir_istok(&A, SJP_NUMBER),
                            newir_if(&A, 
                              newir_op(&A, JVST_IR_EXPR_GE, 
                                newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                                newir_num(&A, 5.0)),
                              newir_seq(&A,
                                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                newir_stmt(&A, JVST_IR_STMT_VALID),
                                NULL
                              ),
                              newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
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

                        NULL
                      ),

                      newir_seq(&A,
                        newir_if(&A,
                          newir_btest(&A, 0, "uniq_contains_split", 1),
                          newir_bitop(&A, JVST_IR_STMT_BSET, 1, "contains", 0),
                          newir_stmt(&A, JVST_IR_STMT_NOP)
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

              newir_if(&A, newir_btest(&A, 1, "contains", 0),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                newir_invalid(&A, JVST_INVALID_UNSATISFIED_CONTAINS, "contains constraint is not satisfied")
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
      )
    },

    {
      TRANSLATE,
      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_contains(&A,
                             newcnode_switch(&A, 1,
                               SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                                             newcnode_range(&A, JVST_CNODE_RANGE_MIN, 5.0, 0.0),
                                             newcnode_valid(),
                                             NULL),
                               SJP_NONE
                             )
                           ),

                           newcnode_items(&A,
                             NULL,
                             newcnode_switch(&A, 0,
                               SJP_STRING, newcnode_valid(),
                               SJP_NONE),
                             NULL),

                           NULL),
          SJP_NONE),

      newir_frame(&A,
        newir_bitvec(&A, 1, "contains", 1),
        newir_bitvec(&A, 0, "uniq_contains_split", 2),
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_loop(&A, "ARR_OUTER", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                  newir_break(&A, "ARR_OUTER", 0),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                    newir_splitvec(&A, 0, "uniq_contains_split",

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
                      ),

                      newir_frame(&A,
                        newir_stmt(&A, JVST_IR_STMT_TOKEN),
                        newir_if(&A, newir_istok(&A, SJP_NUMBER),
                          newir_if(&A, 
                            newir_op(&A, JVST_IR_EXPR_GE, 
                              newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                              newir_num(&A, 5.0)),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
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

                      NULL
                    ),

                    newir_seq(&A,
                      newir_if(&A,
                        newir_btest(&A, 0, "uniq_contains_split", 0),
                        newir_stmt(&A, JVST_IR_STMT_NOP),
                        newir_invalid(&A, JVST_INVALID_ARRAY, "array is invalid")
                      ),

                      newir_if(&A,
                        newir_btest(&A, 0, "uniq_contains_split", 1),
                        newir_bitop(&A, JVST_IR_STMT_BSET, 1, "contains", 0),
                        newir_stmt(&A, JVST_IR_STMT_NOP)
                      ),

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
                                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                                newir_if(&A, newir_istok(&A, SJP_NUMBER),
                                  newir_if(&A, 
                                    newir_op(&A, JVST_IR_EXPR_GE, 
                                      newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                                      newir_num(&A, 5.0)),
                                    newir_seq(&A,
                                      newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                      newir_stmt(&A, JVST_IR_STMT_VALID),
                                      NULL
                                    ),
                                    newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
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

                              NULL
                            ),

                            newir_seq(&A,
                              newir_if(&A,
                                newir_btest(&A, 0, "uniq_contains_split", 1),
                                newir_bitop(&A, JVST_IR_STMT_BSET, 1, "contains", 0),
                                newir_stmt(&A, JVST_IR_STMT_NOP)
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

                    NULL
                  )
                ),

                NULL
              ),

              newir_if(&A, newir_btest(&A, 1, "contains", 0),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                newir_invalid(&A, JVST_INVALID_UNSATISFIED_CONTAINS, "contains constraint is not satisfied")
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
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_ir_unique_1(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,
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
      )
    },

    {
      LINEARIZE,
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

      newir_program(&A,
          newir_frame(&A, frameindex, 1,
            newir_bitvec(&A, 0, "uniq_contains_split", 2),
            newir_splitlist(&A, 0, 2, 2, 3),
            newir_block(&A, 0, "entry",
              newir_stmt(&A, JVST_IR_STMT_TOKEN),
              newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
                3, "invalid_1",
                4, "false"
              ),
              NULL
            ),

            newir_block(&A, 4, "false",
              newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_BEG),
                6, "true",
                19, "false"
              ),
              NULL
            ),

            newir_block(&A, 19, "false",
              newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
                3, "invalid_1",
                22, "false"
              ),
              NULL
            ),

            newir_block(&A, 22, "false",
              newir_stmt(&A, JVST_IR_STMT_CONSUME),
              newir_branch(&A, 18, "valid"),
              NULL
            ),

            newir_block(&A, 18, "valid",
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),

            newir_block(&A, 6, "true",
              newir_stmt(&A, JVST_IR_STMT_UNIQUE_INIT),
              newir_branch(&A, 10, "loop"),
              NULL
            ),

            newir_block(&A, 10, "loop",
              newir_stmt(&A, JVST_IR_STMT_TOKEN),
              newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
                7, "loop_end",
                13, "false"
              ),
              NULL
            ),

            newir_block(&A, 13, "false",
              newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
              newir_splitvec(&A, 0, "uniq_contains_split", splitlist, 0),
              newir_cbranch(&A, newir_btest(&A, 0, "uniq_contains_split", 1),
                15, "true",
                17, "invalid_18"
              ),
              NULL
            ),

            newir_block(&A, 17, "invalid_18",
              newir_invalid(&A, JVST_INVALID_NOT_UNIQUE, "array elements are not unique"),
              NULL
            ),

            newir_block(&A, 15, "true",
              newir_stmt(&A, JVST_IR_STMT_NOP),
              newir_branch(&A, 10, "loop"),
              NULL
            ),

            newir_block(&A, 7, "loop_end",
              newir_stmt(&A, JVST_IR_STMT_UNIQUE_FINAL),
              newir_branch(&A, 18, "valid"),
              NULL
            ),

            newir_block(&A, 3, "invalid_1",
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              NULL
            ),

            NULL
          ),

          newir_frame(&A, frameindex, 2,
            newir_block(&A, 0, "entry",
              newir_stmt(&A, JVST_IR_STMT_CONSUME),
              newir_branch(&A, 1, "valid"),
              NULL
            ),

            newir_block(&A, 1, "valid",
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),

            NULL
          ),

          newir_frame(&A, frameindex, 3,
            newir_block(&A, 0, "entry",
              newir_stmt(&A, JVST_IR_STMT_UNIQUE_TOK),
              NULL
            ),

            NULL
          ),


          NULL
      )
    },

    {
      TRANSLATE,
      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,

                           newcnode_items(&A,
                             NULL,
                             newcnode_switch(&A, 0,
                               SJP_STRING, newcnode_valid(),
                               SJP_NONE),
                             NULL),

                           newcnode(&A, JVST_CNODE_ARR_UNIQUE),

                           NULL),
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
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                  newir_break(&A, "ARR_OUTER", 0),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                    newir_splitvec(&A, 0, "uniq_contains_split",
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
                      ),

                      newir_frame(&A,
                        newir_stmt(&A, JVST_IR_STMT_UNIQUE_TOK),
                        NULL
                      ),

                      NULL
                    ),

                    newir_seq(&A,
                      newir_if(&A,
                        newir_btest(&A, 0, "uniq_contains_split", 0),
                        newir_stmt(&A, JVST_IR_STMT_NOP),
                        newir_invalid(&A, JVST_INVALID_ARRAY, "array is invalid")
                      ),
                      newir_if(&A,
                        newir_btest(&A, 0, "uniq_contains_split", 1),
                        newir_stmt(&A, JVST_IR_STMT_NOP),
                        newir_invalid(&A, JVST_INVALID_NOT_UNIQUE, "array elements are not unique")
                      ),

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
                              newir_if(&A,
                                newir_btest(&A, 0, "uniq_contains_split", 1),
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

                    NULL
                  )
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
      )
    },

    {
      TRANSLATE,
      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_contains(&A,
                             newcnode_switch(&A, 1,
                               SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                                             newcnode_range(&A, JVST_CNODE_RANGE_MIN, 5.0, 0.0),
                                             newcnode_valid(),
                                             NULL),
                               SJP_NONE
                             )
                           ),

                           newcnode(&A, JVST_CNODE_ARR_UNIQUE),

                           newcnode_items(&A,
                             NULL,
                             newcnode_switch(&A, 0,
                               SJP_STRING, newcnode_valid(),
                               SJP_NONE),
                             NULL),

                           NULL),
          SJP_NONE),

      newir_frame(&A,
        newir_bitvec(&A, 1, "contains", 1),
        newir_bitvec(&A, 0, "uniq_contains_split", 3),
        newir_stmt(&A, JVST_IR_STMT_TOKEN),
        newir_if(&A, newir_istok(&A, SJP_OBJECT_END),
          newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
          newir_if(&A, newir_istok(&A, SJP_ARRAY_BEG),
            newir_seq(&A,
              newir_stmt(&A, JVST_IR_STMT_UNIQUE_INIT),
              newir_loop(&A, "ARR_OUTER", 0,
                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                newir_if(&A, newir_istok(&A, SJP_ARRAY_END),
                  newir_break(&A, "ARR_OUTER", 0),
                  newir_seq(&A,
                    newir_stmt(&A, JVST_IR_STMT_UNTOKEN),
                    newir_splitvec(&A, 0, "uniq_contains_split",
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
                      ),

                      newir_frame(&A,
                        newir_stmt(&A, JVST_IR_STMT_UNIQUE_TOK),
                        NULL
                      ),

                      newir_frame(&A,
                        newir_stmt(&A, JVST_IR_STMT_TOKEN),
                        newir_if(&A, newir_istok(&A, SJP_NUMBER),
                          newir_if(&A, 
                            newir_op(&A, JVST_IR_EXPR_GE, 
                              newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                              newir_num(&A, 5.0)),
                            newir_seq(&A,
                              newir_stmt(&A, JVST_IR_STMT_CONSUME),
                              newir_stmt(&A, JVST_IR_STMT_VALID),
                              NULL
                            ),
                            newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
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

                      NULL
                    ),

                    newir_seq(&A,
                      newir_if(&A,
                        newir_btest(&A, 0, "uniq_contains_split", 0),
                        newir_stmt(&A, JVST_IR_STMT_NOP),
                        newir_invalid(&A, JVST_INVALID_ARRAY, "array is invalid")
                      ),
                      newir_if(&A,
                        newir_btest(&A, 0, "uniq_contains_split", 1),
                        newir_stmt(&A, JVST_IR_STMT_NOP),
                        newir_invalid(&A, JVST_INVALID_NOT_UNIQUE, "array elements are not unique")
                      ),
                      newir_if(&A,
                        newir_btest(&A, 0, "uniq_contains_split", 2),
                        newir_bitop(&A, JVST_IR_STMT_BSET, 1, "contains", 0),
                        newir_stmt(&A, JVST_IR_STMT_NOP)
                      ),

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

                              newir_frame(&A,
                                newir_stmt(&A, JVST_IR_STMT_TOKEN),
                                newir_if(&A, newir_istok(&A, SJP_NUMBER),
                                  newir_if(&A, 
                                    newir_op(&A, JVST_IR_EXPR_GE, 
                                      newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                                      newir_num(&A, 5.0)),
                                    newir_seq(&A,
                                      newir_stmt(&A, JVST_IR_STMT_CONSUME),
                                      newir_stmt(&A, JVST_IR_STMT_VALID),
                                      NULL
                                    ),
                                    newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
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

                              NULL
                            ),

                            newir_seq(&A,
                                newir_if(&A,
                                  newir_btest(&A, 0, "uniq_contains_split", 1),
                                  newir_stmt(&A, JVST_IR_STMT_NOP),
                                  newir_invalid(&A, JVST_INVALID_NOT_UNIQUE, "array elements are not unique")
                                ),
                                newir_if(&A,
                                  newir_btest(&A, 0, "uniq_contains_split", 2),
                                  newir_bitop(&A, JVST_IR_STMT_BSET, 1, "contains", 0),
                                  newir_stmt(&A, JVST_IR_STMT_NOP)
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

                    NULL
                  )
                ),

                NULL
              ),

              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_UNIQUE_FINAL),
                newir_if(&A, newir_btest(&A, 1, "contains", 0),
                  newir_stmt(&A, JVST_IR_STMT_VALID),
                  newir_invalid(&A, JVST_INVALID_UNSATISFIED_CONTAINS, "contains constraint is not satisfied")
                ),
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
      )
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

static void test_ir_xor_numbers(void);

static void test_ir_simplify_ands(void);
static void test_ir_simplify_ored_schema(void);

int main(void)
{
  test_ir_empty_schema();

  test_ir_type_constraints();

  test_ir_type_integer();
  test_ir_minimum();
  test_ir_multiple_of();

  test_ir_properties();

  test_ir_minmax_properties_1();
  test_ir_minproperties_2();

  test_ir_required();

  test_ir_dependencies();

  test_ir_anyof_allof_oneof_1();

  test_ir_xor_numbers();

  test_ir_patterns();
  test_ir_minmax_length_1();
  test_ir_str_constraints();

  test_ir_propertynames();

  test_ir_items_1();
  test_ir_contains_1();
  test_ir_unique_1();
  test_ir_minmax_items();

  /* incomplete tests... placeholders for conversion from cnode tests */
  test_ir_minproperties_3();
  test_ir_maxproperties_1();
  test_ir_maxproperties_2();
  test_ir_minmax_properties_2();

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

void test_ir_anyof_allof_oneof_1(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_OR,
          newcnode(&A,JVST_CNODE_NUM_INTEGER),
          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 2.0, 0.0),
          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A,
              newir_op(&A, JVST_IR_EXPR_OR, 
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                  newir_num(&A, 2.0)
                ),
                newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM))
              ),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),
              newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
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
      )
    },

    {
      LINEARIZE,
      newcnode_switch(&A, 1,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_OR,
          newcnode(&A,JVST_CNODE_NUM_INTEGER),
          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 2.0, 0.0),
          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A,
              newir_op(&A, JVST_IR_EXPR_OR, 
                newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                  newir_num(&A, 2.0)
                )
              ),
              newir_stmt(&A, JVST_IR_STMT_VALID),
              newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
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

      newir_program(&A,
          newir_frame(&A, frameindex, 1,
            newir_block(&A, 0, "entry",
              newir_stmt(&A, JVST_IR_STMT_TOKEN),
              newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
                2, "true",
                9, "false"
              ),
              NULL
            ),

            newir_block(&A, 9, "false",
              newir_cbranch(&A, newir_istok(&A, SJP_OBJECT_END),
                12, "invalid_1",
                13, "false"
              ),
              NULL
            ),

            newir_block(&A, 13, "false",
              newir_cbranch(&A, newir_istok(&A, SJP_ARRAY_END),
                12, "invalid_1",
                16, "false"
              ),
              NULL
            ),

            newir_block(&A, 16, "false",
              newir_stmt(&A, JVST_IR_STMT_CONSUME),
              newir_branch(&A, 5, "valid"),
              NULL
            ),

            newir_block(&A, 5, "valid",
              newir_stmt(&A, JVST_IR_STMT_VALID),
              NULL
            ),

            newir_block(&A, 2, "true",
              newir_move(&A, newir_ftemp(&A, 1), newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
              newir_move(&A, newir_ftemp(&A, 0), newir_num(&A, 2.0)),
              newir_cbranch(&A,
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_ftemp(&A, 1),
                  newir_ftemp(&A, 0)
                ),
                4, "true",
                8, "or_false"
              ),
              NULL
            ),

            newir_block(&A, 8, "or_false",
              newir_cbranch(&A, newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
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

            newir_block(&A, 12, "invalid_1",
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              NULL
            ),

            NULL
          ),

          NULL
      )
    },

    {
      TRANSLATE,
      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
          newcnode(&A,JVST_CNODE_NUM_INTEGER),
          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 2.0, 0.0),
          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A,
              newir_op(&A, JVST_IR_EXPR_AND, 
                newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                  newir_num(&A, 2.0)
                )
              ),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),
              newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    {
      LINEARIZE,
      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
          newcnode(&A,JVST_CNODE_NUM_INTEGER),
          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 2.0, 0.0),
          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A,
              newir_op(&A, JVST_IR_EXPR_OR, 
                newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                  newir_num(&A, 2.0)
                )
              ),
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
              newir_cbranch(&A, newir_istok(&A, SJP_NUMBER),
                2, "true",
                10, "invalid_1"
              ),
              NULL
            ),

            newir_block(&A, 10, "invalid_1",
              newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token"),
              NULL
            ),

            newir_block(&A, 2, "true",
              newir_cbranch(&A, newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
                8, "and_true",
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

            newir_block(&A, 8, "and_true",
              newir_move(&A, newir_ftemp(&A, 1), newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
              newir_move(&A, newir_ftemp(&A, 0), newir_num(&A, 2.0)),
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

            NULL
          ),

          NULL
      )
    },

    {
      TRANSLATE,

      // simplified tree
      newcnode_switch(&A, 0, 
          SJP_STRING, newcnode_bool(&A, JVST_CNODE_OR,
                        newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 0, 2, true),
                        newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 4, 0, false),
                        NULL),
          SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_STRING),
            newir_if(&A,
              newir_op(&A, JVST_IR_EXPR_GE, 
                newir_split(&A,
                  newir_frame(&A,
                    newir_seq(&A,
                      newir_stmt(&A, JVST_IR_STMT_CONSUME),
                      newir_if(&A,
                        newir_op(&A, JVST_IR_EXPR_LE, 
                          newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                          newir_size(&A, 2)
                        ),
                        newir_stmt(&A, JVST_IR_STMT_VALID),
                        newir_invalid(&A, JVST_INVALID_LENGTH_TOO_LONG, "length is too long")
                      ),
                      NULL
                    ),
                    NULL
                  ),
                  newir_frame(&A,
                    newir_seq(&A,
                      newir_stmt(&A, JVST_IR_STMT_CONSUME),
                      newir_if(&A,
                        newir_op(&A, JVST_IR_EXPR_GE, 
                          newir_expr(&A, JVST_IR_EXPR_TOK_LEN),
                          newir_size(&A, 4)
                        ),
                        newir_stmt(&A, JVST_IR_STMT_VALID),
                        newir_invalid(&A, JVST_INVALID_LENGTH_TOO_SHORT, "length is too short")
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
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    {
      TRANSLATE,

      // simplified tree
      newcnode_switch(&A, 0, 
          SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_OR,
                        newcnode_range(&A, JVST_CNODE_RANGE_MIN, 3.0, 0.0),
                        newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 2.5),
                        NULL),
          SJP_NONE),

      newir_frame(&A,
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            newir_if(&A,
              newir_op(&A, JVST_IR_EXPR_OR, 
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                  newir_num(&A, 3.0)
                ),
                newir_op(&A, JVST_IR_EXPR_LE, 
                  newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                  newir_num(&A, 2.5)
                )
              ),
              newir_seq(&A,
                newir_stmt(&A, JVST_IR_STMT_CONSUME),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                NULL
              ),
              newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")
            ),
            newir_invalid(&A, JVST_INVALID_UNEXPECTED_TOKEN, "unexpected token")
          ),
          NULL
      )
    },

    { STOP },
  };

  const struct ir_test unimplemented_tests[] = {
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

    { STOP }
  };

  RUNTESTS(tests);
  UNIMPLEMENTED(unimplemented_tests);
}

static void test_ir_xor_numbers(void)
{
  struct arena_info A = {0};

  const struct ir_test tests[] = {
    {
      TRANSLATE,
      newcnode_switch(&A, 1,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_XOR,
          newcnode(&A,JVST_CNODE_NUM_INTEGER),
          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 2.0, 0.0),
          NULL),
        SJP_NONE),

      newir_frame(&A,
          newir_counter(&A, 0, "xor_num"),
          newir_stmt(&A, JVST_IR_STMT_TOKEN),
          newir_if(&A, newir_istok(&A, SJP_NUMBER),
            // NB: this is a slightly indirect way to translate XOR'd
            // constraints.  Should we build the XOR sum into a
            // expression node?  This might translate better into C.
            //
            // nbjoerg suggested !expr_a != !expr_b for two-node XOR in
            // C.  Would this make sense here, as well?  We'd need to
            // coerce the booleans into integers.
            newir_seq(&A,
              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_GE, 
                  newir_expr(&A, JVST_IR_EXPR_TOK_NUM),
                  newir_num(&A, 2.0)
                ),
                newir_incr(&A, 0, "xor_num"),
                newir_stmt(&A, JVST_IR_STMT_NOP)   // <--- XXX: there must be a better way!
              ),

              newir_if(&A,
                newir_isint(&A, newir_expr(&A, JVST_IR_EXPR_TOK_NUM)),
                newir_incr(&A, 0, "xor_num"),
                newir_stmt(&A, JVST_IR_STMT_NOP)   // <--- XXX: there must be a better way!
              ),

              newir_if(&A,
                newir_op(&A, JVST_IR_EXPR_EQ,
                  newir_count(&A, 0, "xor_num"),
                  newir_size(&A, 1)
                ),
                newir_stmt(&A, JVST_IR_STMT_VALID),
                newir_invalid(&A, JVST_INVALID_NUMBER, "number not valid")
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
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
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

