#include "validate_testing.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jvst_macros.h"

#define BASE_URI "http://example.com"

enum TEST_OP {
  STOP = 0,
  TRANSLATE,
  SIMPLIFY,
  CANONIFY,
  ALL,

  // add specific optimization stages...
  //
};

struct cnode_test {
  enum TEST_OP op;
  struct ast_schema *ast;
  struct jvst_cnode *cnode;
  struct jvst_cnode *expected;
};

static int run_test(const char *fname, const struct cnode_test *t)
{
  static const struct ast_string_set zero;

  struct jvst_cnode *result;
  struct json_string base_uri;
  struct ast_string_set sset;

  base_uri.s = BASE_URI;
  base_uri.len = strlen(base_uri.s);

  sset = zero;
  sset.str = base_uri;

  assert(t->expected != NULL);

  switch (t->op) {
    default:
    case STOP:
      fprintf(stderr, "invalid to call run_test(t) with t->op == %d\n", t->op);
      abort();
      break;

    case TRANSLATE:
      assert(t->ast != NULL);
      assert(t->expected != NULL);
      if (t->ast->all_ids == NULL) {
        t->ast->all_ids = &sset;
      }

      result = jvst_cnode_translate_ast(t->ast);
      break;

    case SIMPLIFY:
      assert(t->cnode != NULL);
      assert(t->expected != NULL);
      result = jvst_cnode_simplify(t->cnode);
      break;

    case CANONIFY:
      assert(t->cnode != NULL);
      assert(t->expected != NULL);
      result = jvst_cnode_simplify(t->cnode);
      result = jvst_cnode_canonify(result);
      break;

    case ALL:
      assert(t->ast != NULL);
      assert(t->expected != NULL);
      // result = jvst_cnode_from_ast(t->ast);
      fprintf(stderr, "ALL passes not implemented\n");
      abort();
      break;
  }

  // compare result with expected output
  return cnode_trees_equal(fname, result, t->expected);
}

#define RUNTESTS(testlist) runtests(__func__, (testlist))
static void runtests(const char *testname, const struct cnode_test tests[])
{
  int i;

  for (i=0; tests[i].op != STOP; i++) {
    ntest++;

    if (!run_test(testname, &tests[i])) {
      printf("%s_%d: failed\n", testname, i+1);
      nfail++;
    }
  }
}

static void test_xlate_empty_schema(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    { TRANSLATE, empty_schema(&A), NULL, newcnode_switch(&A, 1, SJP_NONE) },
    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_true_false_schemas(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    { TRANSLATE, true_schema(&A), NULL, newcnode_switch(&A, 1, SJP_NONE) },
    { TRANSLATE, false_schema(&A), NULL, newcnode_switch(&A, 0, SJP_NONE) },
    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_type_number(void)
{
  struct arena_info A = {0};
  struct ast_schema schema = {
    .types = JSON_VALUE_NUMBER,
  };

  const struct cnode_test tests[] = {
    { TRANSLATE, &schema, NULL, newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE) },
    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_type_object(void)
{
  struct arena_info A = {0};
  struct ast_schema schema = {
    .types = JSON_VALUE_OBJECT,
  };

  const struct cnode_test tests[] = {
    { TRANSLATE, &schema, NULL, newcnode_switch(&A, 0, SJP_OBJECT_BEG, newcnode_valid(), SJP_NONE) },
    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_type_several(void)
{
  struct arena_info A = {0};
  struct ast_schema schema = {
    .types = JSON_VALUE_OBJECT | JSON_VALUE_STRING,
  };

  const struct cnode_test tests[] = {
    { TRANSLATE, &schema, NULL, newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_valid(),
        SJP_STRING, newcnode_valid(),
        SJP_NONE) },
    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_type_integer(void)
{
  struct arena_info A = {0};
  struct ast_schema schema = {
    .types = JSON_VALUE_INTEGER,
  };

  const struct cnode_test tests[] = {
    {
      TRANSLATE, &schema, NULL, newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
        SJP_NONE)
    },
    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_minimum(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "minimum", 1.1,
      NULL);

  const struct cnode_test tests[] = {
    {
      TRANSLATE, schema, NULL, newcnode_switch(&A, 1,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
                      newcnode_valid(),
                      NULL),
        SJP_NONE),
    },
    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_multiple_of(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0,
          "multipleOf", 3.0,
          NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_multiple_of(&A, 3.0),
                      newcnode_valid(),
                      NULL),
        SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_properties(void)
{
  struct arena_info A = {0};

  struct ast_schema schema = {
    .properties = {
      .set = newprops(&A,
          "foo", newschema(&A, JSON_VALUE_NUMBER), // XXX - JSON_VALUE_INTEGER
          "bar", newschema(&A, JSON_VALUE_STRING),
          NULL)
    }
  };

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      TRANSLATE, &schema, NULL,
        newcnode_switch(&A, 1,
          SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                              newcnode_prop_match(&A, RE_LITERAL, "bar",
                                newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                              NULL),
                            newcnode_valid(),
                            NULL),
          SJP_NONE),
    },

    {
      TRANSLATE, 
      newschema_p(&A, 0,
        "properties", newpatternprops(&A, "a*", newschema(&A, JSON_VALUE_INTEGER), NULL),
        "properties", newpatternprops(&A, "aaa*", newschema_p(&A, 0, "maximum", 20.0, NULL), NULL),
        NULL
      ),

      NULL,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_NATIVE, "a*",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER), SJP_NONE)),
                            newcnode_prop_match(&A, RE_NATIVE, "aaa*",
                              newcnode_switch(&A, 1,
                                SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                                              newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 20.0),
                                              newcnode_valid(),
                                              NULL),
                                SJP_NONE)),
                            NULL),
                          newcnode_valid(),
                          NULL),
        SJP_NONE)
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "properties", newprops(&A,
                          "foo", newschema(&A, JSON_VALUE_NUMBER), // XXX - JSON_VALUE_INTEGER
                          "bar", newschema(&A, JSON_VALUE_STRING),
                          NULL),
          "additionalProperties", newschema(&A, JSON_VALUE_BOOL),
          NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
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
        SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_propertynames(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0,
        "properties", newprops(&A, "foo", newschema(&A, JSON_VALUE_NUMBER), NULL),
        "propertyNames", newschema_p(&A, 0, "pattern", "f.*", NULL),
        NULL
      ),

      NULL,

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_propnames(&A, 
                            newcnode_switch(&A, 1,
                              SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                                            newcnode_strmatch(&A, RE_NATIVE, "f.*"),
                                            newcnode_valid(),
                                            NULL),
                              SJP_NONE)),
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
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "properties", newprops(&A,
                          "foo", newschema(&A, JSON_VALUE_NUMBER), // XXX - JSON_VALUE_INTEGER
                          "bar", newschema(&A, JSON_VALUE_STRING),
                          NULL),
          "additionalProperties", newschema(&A, JSON_VALUE_BOOL),
          "propertyNames", newschema_p(&A, 0, "pattern", "f.*", NULL),
          NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_propnames(&A,
                            newcnode_switch(&A, 1,
                              SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                                            newcnode_strmatch(&A, RE_NATIVE, "f.*"),
                                            newcnode_valid(),
                                            NULL),
                              SJP_NONE)),
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
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_minproperties_1(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "minProperties", 1,
      NULL);

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      TRANSLATE, schema, NULL,
        newcnode_switch(&A, 1,
          SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 0, false),
                            newcnode_valid(),
                            NULL),
          SJP_NONE),
    },
    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_minproperties_2(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "minProperties", 1,
      "properties", newprops(&A,
        "foo", newschema(&A, JSON_VALUE_NUMBER), // XXX - JSON_VALUE_INTEGER
        "bar", newschema(&A, JSON_VALUE_STRING),
        NULL),
      NULL);

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      TRANSLATE, schema, NULL,
        newcnode_switch(&A, 1,
          SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 0, false),
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
          SJP_NONE),
    },
    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_minproperties_3(void)
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
  const struct cnode_test tests[] = {
    {
      TRANSLATE, schema, NULL,
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
    },
    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_maxproperties_1(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "maxProperties", 2,
      NULL);

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      TRANSLATE, schema, NULL,
        newcnode_switch(&A, 1,
          SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 0, 2, true),
                            newcnode_valid(),
                            NULL),
          SJP_NONE),
    },
    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_maxproperties_2(void)
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
  const struct cnode_test tests[] = {
    {
      TRANSLATE, schema, NULL,
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
    },
    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_minmaxproperties_1(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      TRANSLATE, 

      newschema_p(&A, 0,
      "minProperties", 1,
      "maxProperties", 1,
      "properties", newprops(&A,
        "foo", newschema_p(&A, JSON_VALUE_OBJECT,
          "minProperties", 1,
          "maxProperties", 2,
          NULL), // XXX - JSON_VALUE_INTEGER
        "bar", newschema(&A, JSON_VALUE_STRING),
        NULL),
      NULL),

      NULL,

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
    },

    {
      TRANSLATE, 

      newschema_p(&A, 0,
      "minProperties", 1,
      "maxProperties", 0,
      NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_counts(&A, JVST_CNODE_PROP_RANGE, 1, 0, true),
                          newcnode_valid(),
                          NULL),
        SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_required(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "properties", newprops(&A,
        "foo", empty_schema(&A),
        "bar", empty_schema(&A),
        NULL),
      "required", stringset(&A, "foo", NULL),
      NULL);

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      TRANSLATE, schema, NULL,
        newcnode_switch(&A, 1,
          SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_required(&A, stringset(&A, "foo", NULL)),
                            newcnode_bool(&A,JVST_CNODE_AND,
                              newcnode_propset(&A,
                                newcnode_prop_match(&A, RE_LITERAL, "foo", newcnode_switch(&A, 1, SJP_NONE)),
                                newcnode_prop_match(&A, RE_LITERAL, "bar", newcnode_switch(&A, 1, SJP_NONE)),
                                NULL),
                              newcnode_valid(),
                              NULL),
                            NULL),
          SJP_NONE),
    },

    {
      TRANSLATE, 
      newschema_p(&A, 0,
          "properties", newprops(&A,
            "foo", newschema(&A, JSON_VALUE_NUMBER),
            "bar", newschema(&A, JSON_VALUE_STRING),
            NULL),
          "required", stringset(&A, "foo", NULL),
          NULL),

        NULL,

        newcnode_switch(&A, 1,
          SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_required(&A, stringset(&A, "foo", NULL)),
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
          SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_dependencies(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      TRANSLATE, 
      // schema: { "dependencies": {"bar": ["foo"]} }
      newschema_p(&A, 0,
          "dep_strings", newpropnames(&A, "bar", stringset(&A, "foo", NULL), NULL),
          NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_bool(&A, JVST_CNODE_OR,
                            newcnode_required(&A, stringset(&A, "bar", "foo", NULL)),
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "bar", newcnode_invalid()),
                              NULL),
                            NULL),
                          newcnode_valid(),
                          NULL),
        SJP_NONE),
    },

    {
      TRANSLATE, 
      // schema: { "dependencies": {"quux": ["foo", "bar"]} }
      newschema_p(&A, 0,
          "dep_strings", newpropnames(&A, "quux", stringset(&A, "foo", "bar", NULL), NULL),
          NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_bool(&A, JVST_CNODE_OR,
                            newcnode_required(&A, stringset(&A, "quux", "foo", "bar", NULL)),
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "quux", newcnode_invalid()),
                              NULL),
                            NULL),
                          newcnode_valid(),
                          NULL),
        SJP_NONE),
    },

    {
      TRANSLATE, 
      // schema: { "dependencies": {"quux": ["foo", "bar"], "this": ["that"]} }
      newschema_p(&A, 0,
          "dep_strings", newpropnames(&A,
                           "quux", stringset(&A, "foo", "bar", NULL),
                           "this", stringset(&A, "that", NULL),
                           NULL),
          NULL),

      NULL,

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
                          newcnode_valid(),
                          NULL),
        SJP_NONE),
    },

    {
      TRANSLATE, 
      // schema: { "dependencies": {"quux": ["foo", "bar"]} }
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
      newschema_p(&A, 0,
          "dep_schema",
          newprops(&A, "bar", newschema_p(&A, 0,
                                "properties", newprops(&A,
                                  "foo", newschema(&A, JSON_VALUE_INTEGER),
                                  "bar", newschema(&A, JSON_VALUE_INTEGER),
                                  NULL),
                                NULL),
            NULL),
          NULL),

      NULL,

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
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_anyof_allof_oneof_1(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0,
          "anyOf", schema_set(&A, 
            newschema_p(&A, JSON_VALUE_INTEGER, NULL),
            newschema_p(&A, 0, "minimum", 2.0, NULL),
            NULL),
          NULL),

      NULL,

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
    },

    {
      SIMPLIFY,
      newschema_p(&A, 0,
          "anyOf", schema_set(&A, 
            newschema_p(&A, JSON_VALUE_INTEGER, NULL),
            newschema_p(&A, 0, "minimum", 2.0, NULL),
            NULL),
          NULL),

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

      newcnode_switch(&A, 1,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_OR,
          newcnode(&A,JVST_CNODE_NUM_INTEGER),
          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 2.0, 0.0),
          NULL),
        SJP_NONE),
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "allOf", schema_set(&A, 
            newschema_p(&A, JSON_VALUE_INTEGER, NULL),
            newschema_p(&A, 0, "minimum", 2.0, NULL),
            NULL),
          NULL),

      NULL,

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
    },

    {
      SIMPLIFY,
      newschema_p(&A, 0,
          "allOf", schema_set(&A, 
            newschema_p(&A, JSON_VALUE_INTEGER, NULL),
            newschema_p(&A, 0, "minimum", 2.0, NULL),
            NULL),
          NULL),

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

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
          newcnode(&A,JVST_CNODE_NUM_INTEGER),
          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 2.0, 0.0),
          NULL),
        SJP_NONE),
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "oneOf", schema_set(&A, 
            newschema_p(&A, JSON_VALUE_INTEGER, NULL),
            newschema_p(&A, 0, "minimum", 2.0, NULL),
            NULL),
          NULL),

      NULL,

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
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_anyof_2(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "anyOf", schema_set(&A, 
        newschema_p(&A, JSON_VALUE_OBJECT,
          "properties", newprops(&A,
            "foo", newschema_p(&A, JSON_VALUE_NUMBER, NULL),
            "bar", newschema_p(&A, JSON_VALUE_STRING, NULL),
            NULL),
          NULL),
        newschema_p(&A, JSON_VALUE_OBJECT,
          "properties", newprops(&A,
            "foo", newschema_p(&A, JSON_VALUE_STRING, NULL),
            "bar", newschema_p(&A, JSON_VALUE_NUMBER, NULL),
            NULL),
          NULL),
        NULL),
      NULL);

  const struct cnode_test tests[] = {
    {
      TRANSLATE, schema, NULL, newcnode_bool(&A, JVST_CNODE_AND,
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
    },
    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_allof_2(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0,
        "properties", newprops(&A, "bar", newschema(&A, JSON_VALUE_INTEGER), NULL),
        "required", stringset(&A, "bar", NULL),
        "allOf", schema_set(&A, 
          newschema_p(&A, 0,
            "properties", newprops(&A, "foo", newschema_p(&A, JSON_VALUE_STRING, NULL), NULL),
            "required", stringset(&A, "foo", NULL),
            NULL),
          newschema_p(&A, 0,
            "properties", newprops(&A, "baz", newschema_p(&A, JSON_VALUE_NULL, NULL), NULL),
            "required", stringset(&A, "baz", NULL),
            NULL),
          NULL),
        NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
          newcnode_bool(&A, JVST_CNODE_AND,
            newcnode_switch(&A, 1,
              SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_required(&A, stringset(&A, "foo", NULL)),
                                newcnode_bool(&A, JVST_CNODE_AND,
                                  newcnode_propset(&A, 
                                    newcnode_prop_match(&A, RE_LITERAL, "foo",
                                      newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                                    NULL),
                                newcnode_valid(),
                                NULL),
                              NULL),
              SJP_NONE),
            newcnode_switch(&A, 1,
              SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_required(&A, stringset(&A, "baz", NULL)),
                                newcnode_bool(&A, JVST_CNODE_AND,
                                  newcnode_propset(&A, 
                                    newcnode_prop_match(&A, RE_LITERAL, "baz",
                                      newcnode_switch(&A, 0, SJP_NULL, newcnode_valid(), SJP_NONE)),
                                    NULL),
                                newcnode_valid(),
                                NULL),
                              NULL),
              SJP_NONE),

            NULL),

          newcnode_switch(&A, 1,
            SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                              newcnode_required(&A, stringset(&A, "bar", NULL)),
                              newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_propset(&A, 
                                  newcnode_prop_match(&A, RE_LITERAL, "bar",
                                    newcnode_switch(&A, 0, SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER), SJP_NONE)),
                                  NULL),
                              newcnode_valid(),
                              NULL),
                            NULL),
            SJP_NONE),

          NULL)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_xlate_oneof_2(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0,
        "oneOf", schema_set(&A, 
          newschema(&A, JSON_VALUE_INTEGER),
          newschema(&A, JSON_VALUE_STRING),
          NULL),
        NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_bool(&A, JVST_CNODE_XOR,
          newcnode_switch(&A, 0,
            SJP_NUMBER, newcnode(&A, JVST_CNODE_NUM_INTEGER),
            SJP_NONE),
          newcnode_switch(&A, 0,
            SJP_STRING, newcnode_valid(),
            SJP_NONE),

          NULL),

        newcnode_switch(&A, 1, SJP_NONE),
        NULL)
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
        "oneOf", schema_set(&A, 
          newschema_p(&A, 0,
            "properties", newprops(&A, "foo", newschema_p(&A, JSON_VALUE_STRING, NULL), NULL),
            "required", stringset(&A, "foo", NULL),
            NULL),
          newschema_p(&A, 0,
            "properties", newprops(&A, "baz", newschema_p(&A, JSON_VALUE_NULL, NULL), NULL),
            "required", stringset(&A, "baz", NULL),
            NULL),
          NULL),
        NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
          newcnode_bool(&A, JVST_CNODE_XOR,
            newcnode_switch(&A, 1,
              SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_required(&A, stringset(&A, "foo", NULL)),
                                newcnode_bool(&A, JVST_CNODE_AND,
                                  newcnode_propset(&A, 
                                    newcnode_prop_match(&A, RE_LITERAL, "foo",
                                      newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                                    NULL),
                                newcnode_valid(),
                                NULL),
                              NULL),
              SJP_NONE),
            newcnode_switch(&A, 1,
              SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_required(&A, stringset(&A, "baz", NULL)),
                                newcnode_bool(&A, JVST_CNODE_AND,
                                  newcnode_propset(&A, 
                                    newcnode_prop_match(&A, RE_LITERAL, "baz",
                                      newcnode_switch(&A, 0, SJP_NULL, newcnode_valid(), SJP_NONE)),
                                    NULL),
                                newcnode_valid(),
                                NULL),
                              NULL),
              SJP_NONE),

            NULL),

          newcnode_switch(&A, 1, SJP_NONE),

          NULL)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_pattern_1(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0, "pattern", "a+", NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_strmatch(&A, RE_NATIVE, "a+"),
                      newcnode_valid(),
                      NULL),
        SJP_NONE)
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "pattern", "a+b.d",
          "minLength", 12,
          NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 12, 0, false),
                      newcnode_bool(&A, JVST_CNODE_AND,
                        newcnode_strmatch(&A, RE_NATIVE, "a+b.d"),
                        newcnode_valid(),
                        NULL),
                      NULL),
        SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_minmax_length_1(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0, "minLength", 12, NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 12, 0, false),
                      newcnode_valid(),
                      NULL),
        SJP_NONE)
    },

    {
      TRANSLATE,
      newschema_p(&A, 0, "maxLength", 36, NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 0, 36, true),
                      newcnode_valid(),
                      NULL),
        SJP_NONE)
    },

    {
      TRANSLATE,
      newschema_p(&A, 0, "minLength", 23, "maxLength", 50, NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 23, 50, true),
                      newcnode_valid(),
                      NULL),
        SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_items_1(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0, "items_single",
          newschema_p(&A, JSON_VALUE_INTEGER, NULL),
          NULL),

      NULL,

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_items(&A,
                             newcnode_switch(&A, 0,
                               SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                               SJP_NONE),
                             NULL),
                           newcnode_valid(),
                           NULL),
            SJP_NONE)
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "items", schema_set(&A, 
             newschema_p(&A, JSON_VALUE_STRING, NULL),
             newschema_p(&A, JSON_VALUE_STRING, NULL),
             NULL),
          "additionalItems", newschema_p(&A, JSON_VALUE_INTEGER, NULL),
          NULL),

      NULL,

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
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
                           newcnode_valid(),
                           NULL),
            SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_simplify_ands(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    // handle AND with only one level...
    {
      SIMPLIFY,

      NULL,

      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                        newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
                        newcnode_valid(),
                        NULL),
          SJP_NONE),

      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
          SJP_NONE),
    },

    // handle nested ANDs
    {
      SIMPLIFY,

      NULL,

      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                        newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
                          newcnode_valid(),
                          NULL),
                        newcnode(&A,JVST_CNODE_NUM_INTEGER),
                        NULL),
          SJP_NONE),

      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                        newcnode(&A,JVST_CNODE_NUM_INTEGER),
                        newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
                        NULL),
          SJP_NONE),
    },

    // handle more complex nested ANDs
    {
      SIMPLIFY,
      
      NULL,

      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                        newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.1, 0.0),
                          newcnode_bool(&A, JVST_CNODE_AND,
                            newcnode(&A,JVST_CNODE_NUM_INTEGER),
                            newcnode_valid(),
                            NULL),
                          NULL),
                        newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 3.2),
                        NULL),
          SJP_NONE),

      newcnode_switch(&A, 1,
          SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                        newcnode(&A,JVST_CNODE_NUM_INTEGER),
                        newcnode_range(&A,
                          JVST_CNODE_RANGE_MIN | JVST_CNODE_RANGE_MAX, 1.1, 3.2),
                        NULL),
          SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_simplify_propsets(void)
{
  struct arena_info A = {0};
  const struct cnode_test tests[] = {
    {
      SIMPLIFY, NULL, 

      // initial tree
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_propset(&A, 
                            newcnode_prop_match(&A, RE_NATIVE, "foo",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),

                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                            NULL),

                          newcnode_propset(&A, 
                            newcnode_prop_match(&A, RE_NATIVE, "this",
                              newcnode_switch(&A, 0,
                                SJP_TRUE, newcnode_valid(),
                                SJP_FALSE, newcnode_valid(),
                                SJP_NONE)),

                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                            NULL),
                          NULL
                        ),
        SJP_NONE),

      // optimized
      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propset(&A, 
                          newcnode_prop_match(&A, RE_NATIVE, "foo",
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),

                          newcnode_prop_match(&A, RE_LITERAL, "bar",
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),

                          newcnode_prop_match(&A, RE_NATIVE, "this",
                            newcnode_switch(&A, 0,
                              SJP_TRUE, newcnode_valid(),
                              SJP_FALSE, newcnode_valid(),
                              SJP_NONE)),

                          newcnode_prop_match(&A, RE_LITERAL, "bar",
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                          NULL),
        SJP_NONE),

    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
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
        SJP_NONE),

        newcnode_switch(&A, 0,
          SJP_OBJECT_BEG, newcnode_propset(&A,
                            newcnode_prop_default(&A, 
                              newcnode_switch(&A, 0,
                                SJP_TRUE, newcnode_valid(),
                                SJP_FALSE, newcnode_valid(),
                                SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "foo",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                            NULL),
          SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_prop_default(&A, 
                            newcnode_switch(&A, 0,
                              SJP_TRUE, newcnode_valid(),
                              SJP_FALSE, newcnode_valid(),
                              SJP_NONE)),
                          newcnode_valid(),
                          NULL),
        SJP_NONE),

        newcnode_switch(&A, 0,
          SJP_OBJECT_BEG, newcnode_propset(&A,
                            newcnode_prop_default(&A, 
                              newcnode_switch(&A, 0,
                                SJP_TRUE, newcnode_valid(),
                                SJP_FALSE, newcnode_valid(),
                                SJP_NONE)),
                            NULL),
          SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_simplify_propertynames(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      SIMPLIFY,
      /*
      newschema_p(&A, 0,
        "properties", newprops(&A, "foo", newschema(&A, JSON_VALUE_NUMBER), NULL),
        "propertyNames", "f.*",
        NULL
      ),
      */
      NULL,

      newcnode_switch(&A, 1,
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

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_propset(&A,
                          newcnode_propnames(&A,
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "f.*"), SJP_NONE)),
                          newcnode_prop_match(&A, RE_LITERAL, "foo",
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                          NULL
                        ),
        SJP_NONE),
    },

    {
      SIMPLIFY,
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

      NULL,

      newcnode_switch(&A, 1,
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

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_propset(&A,
                          newcnode_propnames(&A,
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "f.*"), SJP_NONE)),
                          newcnode_prop_default(&A, 
                            newcnode_switch(&A, 0,
                              SJP_TRUE, newcnode_valid(),
                              SJP_FALSE, newcnode_valid(),
                              SJP_NONE)),
                          newcnode_prop_match(&A, RE_LITERAL, "foo",
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                          newcnode_prop_match(&A, RE_LITERAL, "bar",
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                          NULL),
        SJP_NONE),
    },

    {
      SIMPLIFY,
      /*
      newschema_p(&A, 0,
          "propertyNames", "f.*",
          NULL),
          */

      NULL,

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propnames(&A,
                          newcnode_switch(&A, 0, SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "f.*"), SJP_NONE)),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propset(&A, 
                          newcnode_propnames(&A,
                            newcnode_switch(&A, 0, SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "f.*"), SJP_NONE)),
                          NULL),
        SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_simplify_required(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      SIMPLIFY, NULL,

      newcnode_bool(&A,JVST_CNODE_AND,
        newcnode_switch(&A, 1,
          SJP_OBJECT_BEG, newcnode_required(&A, stringset(&A, "foo", NULL)),
          SJP_NONE),
        newcnode_switch(&A, 1,
          SJP_OBJECT_BEG, newcnode_required(&A, stringset(&A, "bar", NULL)),
          SJP_NONE),
        NULL),

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_required(&A, stringset(&A, "foo", "bar", NULL)),
        SJP_NONE),

    },

    {
      SIMPLIFY, NULL,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_required(&A, stringset(&A, "foo", NULL)),
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
        SJP_NONE),

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_required(&A, stringset(&A, "foo", NULL)),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "foo",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                            NULL),
                          NULL),
        SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_simplify_dependencies(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      SIMPLIFY, NULL,
      // schema: { "dependencies": {"bar": ["foo"]} }

      // initial cnode tree
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_bool(&A, JVST_CNODE_OR,
                            newcnode_required(&A, stringset(&A, "bar", "foo", NULL)),
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "bar", newcnode_invalid()),
                              NULL),
                            NULL),
                          newcnode_valid(),
                          NULL),
        SJP_NONE),

      // simplified cnode tree
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                          newcnode_required(&A, stringset(&A, "bar", "foo", NULL)),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "bar", newcnode_invalid()),
                            NULL),
                          NULL),
        SJP_NONE),
    },

    {
      SIMPLIFY, NULL,
      // schema: { "dependencies": {"quux": ["foo", "bar"]} }

      // original cnode tree
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_bool(&A, JVST_CNODE_OR,
                            newcnode_required(&A, stringset(&A, "quux", "foo", "bar", NULL)),
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "quux", newcnode_invalid()),
                              NULL),
                            NULL),
                          newcnode_valid(),
                          NULL),
        SJP_NONE),

      // simplified cnode tree
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                          newcnode_required(&A, stringset(&A, "quux", "foo", "bar", NULL)),
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "quux", newcnode_invalid()),
                            NULL),
                          NULL),
        SJP_NONE),

    },

    {
      SIMPLIFY, NULL, 
      // schema: { "dependencies": {"quux": ["foo", "bar"], "this": ["that"]} }

      // original cnode tree
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
                          newcnode_valid(),
                          NULL),
        SJP_NONE),

      // simplified cnode tree
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

    },

    {
      SIMPLIFY, NULL,
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

      // original cnode tree
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

      // simplified cnode tree
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

    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_simplify_ored_schema(void)
{
  struct arena_info A = {0};
  const struct cnode_test tests[] = {
    {
      SIMPLIFY, NULL, 

        // initial tree
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

        // optimized

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

                            NULL
                          ),
          SJP_NONE),
    },

    {
      SIMPLIFY, 
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
      NULL,

      // original cnode tree...
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

      // simplified cnode tree
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
                              NULL
                            ),
                            
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "bar",
                                newcnode_invalid()),
                              NULL),

                            NULL
                          ),
          SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_simplify_patterns(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      SIMPLIFY,
      /*
      newschema_p(&A, 0, "pattern", "a+", NULL),
      */
      NULL,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_strmatch(&A, RE_NATIVE, "a+"),
                      newcnode_valid(),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "a+"),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      /*
      newschema_p(&A, 0, "pattern", "a+", NULL),
      */
      NULL,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_strmatch(&A, RE_NATIVE, "a+"),
                      newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 12, 0, false),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_strmatch(&A, RE_NATIVE, "a+"),
                      newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 12, 0, false),
                      NULL),
        SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_simplify_allof_2(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      SIMPLIFY,
      NULL,
      /*
      newschema_p(&A, 0,
        "properties", newprops(&A, "bar", newschema(&A, JSON_VALUE_INTEGER), NULL),
        "required", stringset(&A, "bar", NULL),
        "allOf", schema_set(&A, 
          newschema_p(&A, 0,
            "properties", newprops(&A, "foo", newschema_p(&A, JSON_VALUE_STRING, NULL), NULL),
            "required", stringset(&A, "foo", NULL),
            NULL),
          newschema_p(&A, 0,
            "properties", newprops(&A, "baz", newschema_p(&A, JSON_VALUE_NULL, NULL), NULL),
            "required", stringset(&A, "baz", NULL),
            NULL),
          NULL),
        NULL),
      */

      newcnode_bool(&A, JVST_CNODE_AND,
          newcnode_bool(&A, JVST_CNODE_AND,
            newcnode_switch(&A, 1,
              SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_required(&A, stringset(&A, "foo", NULL)),
                                newcnode_bool(&A, JVST_CNODE_AND,
                                  newcnode_propset(&A, 
                                    newcnode_prop_match(&A, RE_LITERAL, "foo",
                                      newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                                    NULL),
                                newcnode_valid(),
                                NULL),
                              NULL),
              SJP_NONE),
            newcnode_switch(&A, 1,
              SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_required(&A, stringset(&A, "baz", NULL)),
                                newcnode_bool(&A, JVST_CNODE_AND,
                                  newcnode_propset(&A, 
                                    newcnode_prop_match(&A, RE_LITERAL, "baz",
                                      newcnode_switch(&A, 0, SJP_NULL, newcnode_valid(), SJP_NONE)),
                                    NULL),
                                newcnode_valid(),
                                NULL),
                              NULL),
              SJP_NONE),

            NULL),

          newcnode_switch(&A, 1,
            SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                              newcnode_required(&A, stringset(&A, "bar", NULL)),
                              newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_propset(&A, 
                                  newcnode_prop_match(&A, RE_LITERAL, "bar",
                                    newcnode_switch(&A, 0, SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER), SJP_NONE)),
                                  NULL),
                              newcnode_valid(),
                              NULL),
                            NULL),
            SJP_NONE),

          NULL),

        newcnode_switch(&A, 1,
          SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                            newcnode_propset(&A, 
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),

                              newcnode_prop_match(&A, RE_LITERAL, "baz",
                                newcnode_switch(&A, 0, SJP_NULL, newcnode_valid(), SJP_NONE)),

                              newcnode_prop_match(&A, RE_LITERAL, "bar",
                                newcnode_switch(&A, 0, SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER), SJP_NONE)),
                              NULL),

                            newcnode_required(&A, stringset(&A, "foo", "baz", "bar", NULL)),

                          NULL),
          SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_simplify_anded_counts(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 5, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 5, 0, false),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_invalid(),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 0, false),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 5, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 5, true),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 1, 0, false),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 2, 0, false),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 2, 0, false),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 6, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 1, 4, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 4, true),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 6, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 1, 4, true),
                         NULL),
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 1, 20, true),
                         newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 0, 12, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 4, true),
        SJP_STRING, newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 1, 12, true),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 6, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 1, 4, true),
                         NULL),
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 1, 20, true),
                         newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 0, 12, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 4, true),
        SJP_STRING, newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 1, 12, true),
        SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_simplify_ored_counts(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {

    // one interval includes the other (1)
    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 5, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 5, true),
        SJP_NONE)
    },

    // one interval includes the other (2)
    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 20, 0, false),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 12, 0, false),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 12, 0, false),
        SJP_NONE)
    },

    // one interval includes the other (3)
    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 20, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 6, 12, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 20, true),
        SJP_NONE)
    },

    // intervals have no overlap (irreducible OR case)
    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 5, 0, false),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                         NULL),
        SJP_NONE),

      // intervals are sorted, though.
      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 5, 0, false),
                         NULL),
        SJP_NONE)
    },

    // intervals overlap but one is not a subset of the other
    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 12, 20, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 6, 16, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 6, 20, true),
        SJP_NONE)
    },

    // several intervals, some have overlap, so do not
    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 5, 8, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 16, 0, false),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 5, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 10, 12, true),
                         NULL),
        SJP_NONE),

      // intervals are sorted, though.
      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 8, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 10, 12, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 16, 0, false),
                         NULL),
        SJP_NONE)
    },

    // Multiple constraints; ensure that we don't mix them up
    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 6, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 1, 4, true),
                         NULL),
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 1, 8, true),
                         newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 6, 12, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 1, 6, true),
        SJP_STRING, newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 1, 12, true),
        SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_simplify_anded_ored_counts(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 6, 0, false),
                         newcnode_bool(&A, JVST_CNODE_OR,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 4, 8, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 2, true),
                           NULL),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 6, 8, true),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_bool(&A, JVST_CNODE_OR,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 6, 0, false),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                           NULL
                         ),
                         newcnode_bool(&A, JVST_CNODE_OR,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 4, 8, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 2, true),
                           NULL),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 2, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 6, 8, true),
                         NULL),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_bool(&A, JVST_CNODE_OR,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 20, 0, false),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 10, 15, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                           NULL
                         ),
                         newcnode_bool(&A, JVST_CNODE_OR,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 8, 12, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 5, true),
                           NULL),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 10, 12, true),
                         NULL),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_bool(&A, JVST_CNODE_OR,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 20, 0, false),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 10, 15, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                           NULL
                         ),
                         newcnode_bool(&A, JVST_CNODE_OR,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 25, 103, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 8, 12, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 5, true),
                           NULL),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 10, 12, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 25, 103, true),
                         NULL),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                         newcnode_bool(&A, JVST_CNODE_OR,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 18, 24, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 8, 12, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                           NULL
                         ),
                         newcnode_bool(&A, JVST_CNODE_OR,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 4, 7, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 14, 17, true),
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 25, 0, false),
                           NULL),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_invalid(),
        SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_simplify_not_counts(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_NOT,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 6, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 7, 0, false),
        SJP_NONE),
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_NOT,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 10, 0, false),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 9, true),
        SJP_NONE),
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_NOT,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 10, 20, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 9, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 21, 0, false),
                         NULL
                       ),
        SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_simplify_xored_counts(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_XOR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 6, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 4, 8, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 7, 8, true),
                         NULL),
        SJP_NONE),
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_XOR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 2, 6, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 8, 12, true),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 2, 6, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 8, 12, true),
                         NULL),
        SJP_NONE),
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_XOR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 8, 12, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 10, 0, false),
                         NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 8, 9, true),
                         newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 13, 0, false),
                         NULL),
        SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_simplify_anded_items(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      SIMPLIFY,
      /*
      newschema_p(&A, 0,
        "oneOf", schema_set(&A, 
          newschema(&A, JSON_VALUE_INTEGER),
          newschema(&A, JSON_VALUE_STRING),
          NULL),
        NULL),
      */

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_switch(&A, 0,
	  SJP_ARRAY_BEG, newcnode_items(&A,
	  		   NULL,
	  		   newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE),
			   newcnode_switch(&A, 0, SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER), SJP_NONE),
			   NULL),
	  SJP_NONE),
        newcnode_switch(&A, 0,
	  SJP_ARRAY_BEG, newcnode_items(&A,
	  		   NULL,
		           newcnode_switch(&A, 0,
			           SJP_STRING, newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 12, 0, false),
			           SJP_NONE),
			   newcnode_switch(&A, 0,
			           SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.0, 0.0),
			           SJP_NONE),
			   NULL),
	  SJP_NONE),
        newcnode_switch(&A, 0,
	  SJP_ARRAY_BEG, newcnode_items(&A,
	  		   NULL,
		           newcnode_switch(&A, 0,
			           SJP_STRING, newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 0, 24, true),
			           SJP_NONE),
			   newcnode_switch(&A, 0,
			           SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 10.0),
			           SJP_NONE),
			   NULL),
	  SJP_NONE),
	NULL),

      newcnode_switch(&A, 0,
	SJP_ARRAY_BEG, newcnode_items(&A,
	  		   NULL,
	  		   newcnode_switch(&A, 0,
			     SJP_STRING, newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 12, 24, true),
			     SJP_NONE),
	  		   newcnode_switch(&A, 0,
			     SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
			                   newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                           newcnode_range(&A,
                                             JVST_CNODE_RANGE_MIN | JVST_CNODE_RANGE_MAX, 1.0, 10.0),
                                           NULL),
			     SJP_NONE),
			   NULL),
	SJP_NONE),
    },

    // ANDs ARR_ITEM nodes with different numbers of item constraints,
    // checks that additional items are added to the ones with fewer
    // item constraints
    {
      SIMPLIFY,
      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_switch(&A, 0,
	  SJP_ARRAY_BEG, newcnode_items(&A,
	  		   newcnode_switch(&A, 0, SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER), SJP_NONE),
	  		   newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE),
			   NULL),
	  SJP_NONE),
        newcnode_switch(&A, 0,
	  SJP_ARRAY_BEG, newcnode_items(&A,
	  		   newcnode_switch(&A, 0, SJP_NONE),  // no additional items
		           newcnode_switch(&A, 0,
			           SJP_STRING, newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 12, 0, false),
			           SJP_NONE),
			   newcnode_switch(&A, 0,
			           SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.0, 0.0),
			           SJP_NONE),
			   NULL),
	  SJP_NONE),
        newcnode_switch(&A, 0,
	  SJP_ARRAY_BEG, newcnode_items(&A,
			   newcnode_switch(&A, 0,
			           SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 10.0),
			           SJP_NONE),
		           newcnode_switch(&A, 0,
			           SJP_STRING, newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 0, 24, true),
			           SJP_NONE),
			   NULL),
	  SJP_NONE),
	NULL),

      newcnode_switch(&A, 0,
	SJP_ARRAY_BEG, newcnode_items(&A,
	  		   newcnode_switch(&A, 0, SJP_NONE),  // no additional items
	  		   newcnode_switch(&A, 0,
			     SJP_STRING, newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 12, 24, true),
			     SJP_NONE),
	  		   newcnode_switch(&A, 0,
			     SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
			                   newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                           newcnode_range(&A,
                                             JVST_CNODE_RANGE_MIN | JVST_CNODE_RANGE_MAX, 1.0, 10.0),
                                           NULL),
			     SJP_NONE),
			   NULL),
	SJP_NONE),


    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_simplify_anded_number_constraints(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.0, 0.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 1.5),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN | JVST_CNODE_RANGE_MAX, 1.0, 1.5),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN | JVST_CNODE_RANGE_MAX, 1.0, 3.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 1.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN | JVST_CNODE_RANGE_MAX, 1.0, 1.0),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN, 3.0, 0.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN, 1.0, 0.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN, 3.0, 0.0),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 3.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 1.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 1.0),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN, 3.1, 0.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_EXCL_MIN, 3.1, 0.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_EXCL_MIN, 3.1, 0.0),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN, 3.2, 0.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_EXCL_MIN, 3.1, 0.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN, 3.2, 0.0),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 3.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_EXCL_MAX, 0.0, 3.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_EXCL_MAX, 0.0, 3.0),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 3.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_EXCL_MAX, 0.0, 3.5),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 3.0),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 3.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_EXCL_MIN, 3.0, 0.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_invalid(),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX, 1.0, 3.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX, 5.0, 6.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_invalid(),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX, 1.0, 3.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX, 2.0, 6.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX, 2.0, 3.0),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_EXCL_MAX, 1.0, 3.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_EXCL_MIN|JVST_CNODE_RANGE_MAX, 2.0, 6.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_EXCL_MIN|JVST_CNODE_RANGE_EXCL_MAX, 2.0, 3.0),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_range(&A, JVST_CNODE_RANGE_EXCL_MIN|JVST_CNODE_RANGE_MAX, 1.0, 3.0),
                      newcnode_range(&A, JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_EXCL_MAX, 2.0, 6.0),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode_range(&A, JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX, 2.0, 3.0),
        SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_simplify_oneof_2(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      SIMPLIFY,
      /*
      newschema_p(&A, 0,
        "oneOf", schema_set(&A, 
          newschema(&A, JSON_VALUE_INTEGER),
          newschema(&A, JSON_VALUE_STRING),
          NULL),
        NULL),
      */

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_bool(&A, JVST_CNODE_XOR,
          newcnode_switch(&A, 0,
            SJP_NUMBER, newcnode(&A, JVST_CNODE_NUM_INTEGER),
            SJP_NONE),
          newcnode_switch(&A, 0,
            SJP_STRING, newcnode_valid(),
            SJP_NONE),
          NULL),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL),

      newcnode_switch(&A, 0,
        SJP_NUMBER, newcnode(&A, JVST_CNODE_NUM_INTEGER),
        SJP_STRING, newcnode_valid(),
        SJP_NONE)
    },

    {
      SIMPLIFY,
      /*
      newschema_p(&A, 0,
        "oneOf", schema_set(&A, 
          newschema_p(&A, 0,
            "properties", newprops(&A, "foo", newschema_p(&A, JSON_VALUE_STRING, NULL), NULL),
            "required", stringset(&A, "foo", NULL),
            NULL),
          newschema_p(&A, 0,
            "properties", newprops(&A, "baz", newschema_p(&A, JSON_VALUE_NULL, NULL), NULL),
            "required", stringset(&A, "baz", NULL),
            NULL),
          NULL),
        NULL),
        */
      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_bool(&A, JVST_CNODE_XOR,
          newcnode_switch(&A, 1,
            SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                              newcnode_required(&A, stringset(&A, "foo", NULL)),
                              newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_propset(&A, 
                                  newcnode_prop_match(&A, RE_LITERAL, "foo",
                                    newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                                  NULL),
                              newcnode_valid(),
                              NULL),
                            NULL),
            SJP_NONE),
          newcnode_switch(&A, 1,
            SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                              newcnode_required(&A, stringset(&A, "baz", NULL)),
                              newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode_propset(&A, 
                                  newcnode_prop_match(&A, RE_LITERAL, "baz",
                                    newcnode_switch(&A, 0, SJP_NULL, newcnode_valid(), SJP_NONE)),
                                  NULL),
                              newcnode_valid(),
                              NULL),
                            NULL),
            SJP_NONE),

          NULL),

        newcnode_switch(&A, 1, SJP_NONE),

        NULL),

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_XOR,
                          newcnode_bool(&A, JVST_CNODE_AND,
                            newcnode_required(&A, stringset(&A, "foo", NULL)),
                            newcnode_propset(&A, 
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                              NULL),
                          NULL),

                          newcnode_bool(&A, JVST_CNODE_AND,
                            newcnode_required(&A, stringset(&A, "baz", NULL)),
                            newcnode_propset(&A, 
                              newcnode_prop_match(&A, RE_LITERAL, "baz",
                                newcnode_switch(&A, 0, SJP_NULL, newcnode_valid(), SJP_NONE)),
                              NULL),
                            NULL),
                          NULL),
          SJP_NONE),

    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_simplify_uniqueItems(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      SIMPLIFY,
      NULL,

      /* make sure ANDed uniques are combined into one UNIQUE node */
      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode(&A, JVST_CNODE_ARR_UNIQUE),
                           newcnode_valid(),
                           NULL
                         ),
          SJP_NONE),
        newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode(&A, JVST_CNODE_ARR_UNIQUE),
                           newcnode_valid(),
                           NULL
                         ),
          SJP_NONE),
        NULL),

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode(&A, JVST_CNODE_ARR_UNIQUE),
          SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_canonify_ored_schema(void)
{
  struct arena_info A = {0};
  const struct cnode_test tests[] = {
    {
      CANONIFY, NULL, 

      // simplified tree
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
                              newcnode_switch(&A, 0, SJP_TRUE, newcnode_valid(), SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),
                            NULL),

                          NULL
                        ),
        SJP_NONE),

        // canonified tree
        newcnode_switch(&A, 0,
          SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                            newcnode_mswitch(&A, 
                              // default case
                              newcnode_valid(),

                              newcnode_mcase(&A,
                                newmatchset(&A, RE_LITERAL, "bar", -1),
                                newcnode_switch(&A, 0,
                                  SJP_STRING, newcnode_valid(),
                                  SJP_NONE)
                              ),

                              newcnode_mcase(&A,
                                newmatchset(&A, RE_LITERAL, "foo", -1),
                                newcnode_switch(&A, 0,
                                  SJP_NUMBER, newcnode_valid(),
                                  SJP_NONE)
                              ),

                              NULL
                            ),

                            newcnode_mswitch(&A, 
                              // default case
                              newcnode_valid(),

                              newcnode_mcase(&A,
                                newmatchset(&A, RE_LITERAL, "bar", -1),
                                newcnode_switch(&A, 0,
                                  SJP_NUMBER, newcnode_valid(),
                                  SJP_NONE)
                              ),

                              newcnode_mcase(&A,
                                newmatchset(&A, RE_LITERAL, "foo", -1),
                                newcnode_switch(&A, 0,
                                  SJP_TRUE, newcnode_valid(),
                                  SJP_NONE)
                              ),

                              NULL
                            ),
                            NULL
                          ),
          SJP_NONE),
    },

    {
      CANONIFY, 
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
      NULL,

      // simplified tree
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
                              NULL
                            ),
                            
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "bar",
                                newcnode_invalid()),
                              NULL),

                            NULL
                          ),
          SJP_NONE),

      // canonified tree
      newcnode_switch(&A, 1, 
          SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_OR,
                            newcnode_bool(&A, JVST_CNODE_AND,
                              newcnode_reqmask(&A, 1),
                              newcnode_mswitch(&A,
                                // default case
                                newcnode_valid(),

                                newcnode_mcase(&A,
                                  newmatchset(&A, RE_LITERAL, "bar", -1),
                                  newcnode_bool(&A, JVST_CNODE_AND,
                                    newcnode_switch(&A, 0, 
                                      SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                      SJP_NONE),
                                    newcnode_reqbit(&A, 0),
                                    NULL)
                                ),

                                newcnode_mcase(&A,
                                  newmatchset(&A, RE_LITERAL, "foo", -1),
                                    newcnode_switch(&A, 0, 
                                      SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                      SJP_NONE)
                                ),

                                NULL
                              ),
                              NULL
                            ),

                            newcnode_mswitch(&A,
                              // default case
                              newcnode_valid(),

                              newcnode_mcase(&A,
                                newmatchset(&A, RE_LITERAL, "bar", -1),
                                newcnode_invalid()
                              ),

                              NULL
                            ),

                            NULL
                          ),
          SJP_NONE),
    },

    {
      CANONIFY, 
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
      NULL,

      // simplified tree
      newcnode_switch(&A, 0, 
          SJP_STRING, newcnode_bool(&A, JVST_CNODE_OR,
                        newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 0, 2, true),
                        newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 4, 0, false),
                        NULL),
          SJP_NONE),

      // canonified tree
      newcnode_switch(&A, 0, 
          SJP_STRING, newcnode_bool(&A, JVST_CNODE_OR,
                        newcnode_mswitch(&A,
                          // default case
                          newcnode_mcase(&A,
                            NULL,
                            newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 0, 2, true)
                          ),

                          NULL
                        ),

                        newcnode_mswitch(&A,
                          // default case
                          newcnode_mcase(&A,
                            NULL,
                            newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 4, 0, false)
                          ),

                          NULL
                        ),

                        NULL
                      ),
          SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_canonify_propsets(void)
{
  struct arena_info A = {0};
  const struct cnode_test tests[] = {
    {
      CANONIFY, NULL, 

        // initial tree
        newcnode_switch(&A, 0,
          SJP_OBJECT_BEG, newcnode_propset(&A, 
                            newcnode_prop_match(&A, RE_NATIVE, "ba.",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),

                            newcnode_prop_match(&A, RE_LITERAL, "bar",
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)),
                            NULL),
          SJP_NONE),

        // optimized
        newcnode_switch(&A, 0,
          SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                            // default case
                            newcnode_valid(),

                            newcnode_mcase(&A,
                              newmatchset(&A,
                                RE_LITERAL, "bar",
                                RE_NATIVE,  "ba.",
                                -1),
                              newcnode_switch(&A, 0, SJP_NONE)
                            ),

                            newcnode_mcase(&A,
                              newmatchset(&A,
                                RE_NATIVE,  "ba.",
                                -1),
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)
                            ),

                            NULL),
          SJP_NONE),
    },

    {
      CANONIFY, NULL, 
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_NATIVE, "a*",
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER), SJP_NONE)),
                            newcnode_prop_match(&A, RE_NATIVE, "aaa*",
                              newcnode_switch(&A, 1,
                                SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                                              newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 20.0),
                                              newcnode_valid(),
                                              NULL),
                                SJP_NONE)),
                            NULL),
                          newcnode_valid(),
                          NULL),
        SJP_NONE),

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_mswitch(&A,
                          newcnode_valid(),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_NATIVE, "a*", -1),
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode(&A,JVST_CNODE_NUM_INTEGER), SJP_NONE)
                          ),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_NATIVE, "a*", RE_NATIVE, "aaa*", -1),
                            newcnode_switch(&A, 0,
                              SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                                newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                newcnode_range(&A, JVST_CNODE_RANGE_MAX, 0.0, 20.0),
                                NULL),
                              SJP_NONE)
                            ),
                          NULL
                        ),
        SJP_NONE)
    },

    {
      CANONIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
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
        SJP_NONE),

        newcnode_switch(&A, 0,
          SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                            // default case
                            newcnode_switch(&A, 0,
                              SJP_TRUE, newcnode_valid(),
                              SJP_FALSE, newcnode_valid(),
                              SJP_NONE),

                            newcnode_mcase(&A,
                              newmatchset(&A, RE_LITERAL,  "bar", -1),
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)
                            ),

                            newcnode_mcase(&A,
                              newmatchset(&A, RE_LITERAL, "foo", -1),
                              newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)
                            ),

                            NULL),
          SJP_NONE)
    },

    {
      CANONIFY,
      NULL,

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_prop_default(&A, 
                            newcnode_switch(&A, 0,
                              SJP_TRUE, newcnode_valid(),
                              SJP_FALSE, newcnode_valid(),
                              SJP_NONE)),
                          newcnode_valid(),
                          NULL),
        SJP_NONE),

        newcnode_switch(&A, 0,
          SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                            // default case
                            newcnode_switch(&A, 0,
                              SJP_TRUE, newcnode_valid(),
                              SJP_FALSE, newcnode_valid(),
                              SJP_NONE),

                            NULL),
          SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_canonify_propertynames(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      CANONIFY,
      /*
      newschema_p(&A, 0,
        "properties", newprops(&A, "foo", newschema(&A, JSON_VALUE_NUMBER), NULL),
        "propertyNames", "f.*",
        NULL
      ),
      */
      NULL,

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

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_invalid(),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_LITERAL, "foo", RE_NATIVE, "f.*", -1),
                            newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE)),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_NATIVE, "f.*", -1),
                            // newcnode_switch(&A, 0, SJP_NONE)),
                            newcnode_valid()),
                          NULL),
        SJP_NONE)
    },

    {
      CANONIFY,
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

      NULL,

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

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_invalid(),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_LITERAL, "bar", -1),
                            newcnode_invalid()),

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
    },

    {
      CANONIFY,
      /*
      newschema_p(&A, 0,
          "additionalProperties", newschema(&A, JSON_VALUE_BOOL),
          "propertyNames", "f.*",
          NULL),
          */

      NULL,

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
        SJP_NONE)
    },

    {
      CANONIFY,
      /*
      newschema_p(&A, 0,
          "propertyNames", "f.*",
          NULL),
          */

      NULL,

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propnames(&A,
                          newcnode_switch(&A, 0,
                            SJP_STRING, newcnode_bool(&A,JVST_CNODE_AND,
                                          newcnode_strmatch(&A, RE_NATIVE, "f.*"),
                                          newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 3, 16, true),
                                          NULL),
                            SJP_NONE)),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_invalid(),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_NATIVE, "f.*", -1),
                            newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 3, 16, true)),
                          NULL),
        SJP_NONE)
    },

    {
      CANONIFY,
      /*
      newschema_p(&A, 0,
          "propertyNames", "f.*",
          NULL),
          */

      NULL,

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propnames(&A,
                          newcnode_switch(&A, 0, SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "f.*"), SJP_NONE)),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_invalid(),

                          newcnode_mcase(&A,
                            newmatchset(&A, RE_NATIVE, "f.*", -1),
                            newcnode_valid()),
                          NULL),
        SJP_NONE)
    },

    // binary schema
    {
      CANONIFY,
      /*
      newschema_p(&A, 0, "propertyNames", true_schema(), NULL),
      */

      NULL,

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propnames(&A,
                          newcnode_switch(&A, 1, SJP_NONE)),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_valid(),
                          NULL),
        SJP_NONE)

    },

    {
      CANONIFY,
      /*
      newschema_p(&A, 0, "propertyNames", false_schema(), NULL),
      */

      NULL,

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_propnames(&A,
                          newcnode_switch(&A, 0, SJP_NONE)),
        SJP_NONE),

      newcnode_switch(&A, 0,
        SJP_OBJECT_BEG, newcnode_mswitch(&A, 
                          // default case
                          newcnode_invalid(),
                          NULL),
        SJP_NONE)

    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_canonify_length_constraints(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      CANONIFY, NULL,
        newcnode_switch(&A, 0,
          SJP_OBJECT_BEG, newcnode_propset(&A,
                            newcnode_prop_match(&A, RE_LITERAL, "description",
                              newcnode_switch(&A, 0,
                                SJP_STRING, newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 3, 30, true),
                                SJP_NONE)),
                            newcnode_prop_match(&A, RE_LITERAL, "quantity",
                              newcnode_switch(&A, 0,
                                SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                                              newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                              newcnode_range(&A, JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX, 0.0, 100.0),
                                              NULL),
                                SJP_NONE)),
                            NULL),
          SJP_NONE),

        newcnode_switch(&A, 0,
          SJP_OBJECT_BEG, newcnode_mswitch(&A,
                            // default case
                            newcnode_valid(),

                            newcnode_mcase(&A,
                              newmatchset(&A, RE_LITERAL, "description", -1),
                              newcnode_switch(&A, 0,
                                SJP_STRING, newcnode_mswitch(&A,
                                              // default case
                                              newcnode_mcase(&A,
                                                NULL,
                                                newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 3, 30, true)),
                                              NULL),
                                SJP_NONE)),

                            newcnode_mcase(&A,
                              newmatchset(&A, RE_LITERAL, "quantity", -1),
                              newcnode_switch(&A, 0,
                                SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                                              newcnode(&A,JVST_CNODE_NUM_INTEGER),
                                              newcnode_range(&A, JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX, 0.0, 100.0),
                                              NULL),
                                SJP_NONE)),

                            NULL),

          SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

void test_canonify_required(void)
{
  struct arena_info A = {0};

  // initial schema is not reduced (additional constraints are ANDed
  // together).  Reduction will occur on a later pass.
  const struct cnode_test tests[] = {
    {
      CANONIFY, NULL,

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_required(&A, stringset(&A, "foo", "bar", NULL)),
        SJP_NONE),

      // optimized
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                          newcnode_reqmask(&A, 2),
                          newcnode_mswitch(&A, 
                            // default case
                            newcnode_valid(),

                            newcnode_mcase(&A,
                              newmatchset(&A, RE_LITERAL, "bar", -1),
                              newcnode_reqbit(&A, 1)
                            ),

                            newcnode_mcase(&A,
                              newmatchset(&A, RE_LITERAL,  "foo", -1),
                              newcnode_reqbit(&A, 0)
                            ),

                          NULL),
                        NULL),
        SJP_NONE),
    },

    {
      CANONIFY, NULL,
      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_required(&A, stringset(&A, "foo", NULL)),
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
        SJP_NONE),

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_reqmask(&A, 1),
                          newcnode_mswitch(&A, 
                            // default case
                            newcnode_valid(),

                            newcnode_mcase(&A,
                              newmatchset(&A, RE_LITERAL, "bar", -1),
                              newcnode_switch(&A, 0, SJP_STRING, newcnode_valid(), SJP_NONE)
                            ),

                            newcnode_mcase(&A,
                              newmatchset(&A, RE_LITERAL, "foo", -1),
                              newcnode_bool(&A,JVST_CNODE_AND,
                                newcnode_switch(&A, 0, SJP_NUMBER, newcnode_valid(), SJP_NONE),
                                newcnode_reqbit(&A, 0),
                                NULL
                              )
                            ),

                            NULL
                          ),
                          NULL),
        SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_canonify_patterns(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      CANONIFY, NULL,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_strmatch(&A, RE_NATIVE, "a+b.d"),
        SJP_NONE),

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_mswitch(&A,
                      newcnode_invalid(),       // default case

                      newcnode_mcase(&A,
                        newmatchset(&A, RE_NATIVE, "a+b.d", -1),
                        newcnode_valid()
                      ),

                      NULL
                    ),
        SJP_NONE)
    },

    {
      CANONIFY,
      /*
      newschema_p(&A, 0,
          "pattern", "a+b.d",
          "minLength", 12,
          NULL),
      */
      NULL,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 12, 0, false),
                      newcnode_bool(&A, JVST_CNODE_AND,
                        newcnode_strmatch(&A, RE_NATIVE, "a+b.d"),
                        newcnode_valid(),
                        NULL),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_mswitch(&A,
                      // default case
                      newcnode_invalid(),

                      newcnode_mcase(&A,
                        newmatchset(&A, RE_NATIVE, "a+b.d", -1),
                        newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 12, 0, false)
                      ),

                      NULL
                    ),
        SJP_NONE),
    },

    {
      CANONIFY,
      /*
      newschema_p(&A, 0,
          "pattern", "a+b.d",
          "minLength", 12,
          NULL),
      */
      NULL,

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_bool(&A, JVST_CNODE_AND,
                      newcnode_counts(&A, JVST_CNODE_STR_LENGTH, 12, 0, false),
                      newcnode_valid(),
                      NULL),
        SJP_NONE),

      newcnode_switch(&A, 1,
        SJP_STRING, newcnode_mswitch(&A,
                      newcnode_mcase(&A,
                        NULL,
                        newcnode_counts(&A, JVST_CNODE_LENGTH_RANGE, 12, 0, false)
                      ),

                      NULL
                    ),
        SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}


static void test_xlate_minmax_items(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0, "minItems", 3, NULL),

      NULL,

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 3, 0, false),
                           newcnode_valid(),
                           NULL),
            SJP_NONE)
    },

    {
      TRANSLATE,
      newschema_p(&A, 0, "maxItems", 3, NULL),

      NULL,

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 0, 3, true),
                           newcnode_valid(),
                           NULL),
            SJP_NONE)
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "minItems", 1,
          "maxItems", 5,
          NULL),

      NULL,

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 1, 5, true),
                           newcnode_valid(),
                           NULL),
            SJP_NONE)
    },


    {
      TRANSLATE,
      newschema_p(&A, 0,
          "items_single", newschema_p(&A, JSON_VALUE_INTEGER, NULL),
          "maxItems", 5,
          NULL),

      NULL,

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
            SJP_NONE)
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "items", schema_set(&A, 
             newschema_p(&A, JSON_VALUE_STRING, NULL),
             newschema_p(&A, JSON_VALUE_STRING, NULL),
             NULL),
          "additionalItems", newschema_p(&A, JSON_VALUE_INTEGER, NULL),
          "minItems", 1,
          "maxItems", 7,
          NULL),

      NULL,

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_counts(&A, JVST_CNODE_ITEM_RANGE, 1, 7, true),
                           newcnode_bool(&A, JVST_CNODE_AND,
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
                             newcnode_valid(),
                             NULL),
                           NULL),
            SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_contains(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0,
          "contains", newschema_p(&A, 0, "minimum", 5.0, NULL),
          NULL),

      NULL,

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode_contains(&A,
                             newcnode_switch(&A, 1,
                               SJP_NUMBER, newcnode_bool(&A, JVST_CNODE_AND,
                                             newcnode_range(&A, JVST_CNODE_RANGE_MIN, 5.0, 0.0),
                                             newcnode_valid(),
                                             NULL),
                               SJP_NONE)
                           ),
                           newcnode_valid(),
                           NULL
                         ),
          SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_uniqueItems(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0, "uniqueItems", 1, NULL),

      NULL,

      newcnode_switch(&A, 1,
          SJP_ARRAY_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                           newcnode(&A, JVST_CNODE_ARR_UNIQUE),
                           newcnode_valid(),
                           NULL
                         ),
          SJP_NONE),
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_const(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0,
          "const", newjson_num(&A,5),
          NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_switch(&A, 0,
            SJP_NUMBER, newcnode_range(&A,
                          JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX,
                          5.0, 5.0),
            SJP_NONE),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL
      )
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "const", newjson_str(&A,"the quick brown fox jumps over the lazy dog"),
          NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_switch(&A, 0,
            SJP_STRING, newcnode_strmatch(&A, RE_LITERAL,
                          "the quick brown fox jumps over the lazy dog"),
            SJP_NONE),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL
      )
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "const", newjson_bool(&A, 1),
          NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_switch(&A, 0,
            SJP_TRUE, newcnode_valid(),
            SJP_NONE),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL
      )
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "const", newjson_bool(&A, 0),
          NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_switch(&A, 0,
            SJP_FALSE, newcnode_valid(),
            SJP_NONE),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL
      )
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "const", newjson_null(&A),
          NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_switch(&A, 0,
            SJP_NULL, newcnode_valid(),
            SJP_NONE),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL
      )
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "const", newjson_array(&A, 
                     newjson_num(&A,1),
                     newjson_num(&A,2),
                     newjson_str(&A,"foo"),
                     NULL
                   ),
          NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_switch(&A, 0,
            SJP_ARRAY_BEG, newcnode_items(&A,
                             newcnode_switch(&A, 0, SJP_NONE),
                             newcnode_switch(&A, 0,
                               SJP_NUMBER, newcnode_range(&A,
                                             JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX,
                                             1.0, 1.0),
                               SJP_NONE),
                             newcnode_switch(&A, 0,
                               SJP_NUMBER, newcnode_range(&A,
                                             JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX,
                                             2.0, 2.0),
                               SJP_NONE),
                             newcnode_switch(&A, 0,
                               SJP_STRING, newcnode_strmatch(&A, RE_LITERAL, "foo"),
                               SJP_NONE),
                             NULL),
            SJP_NONE),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL
      )
    },

    {
      TRANSLATE,
      newschema_p(&A, 0,
          "const", newjson_object(&A, 
                     "foo", newjson_str(&A, "bar"),
                     "baz", newjson_num(&A, 44.0),
                     NULL
                   ),
          NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_switch(&A, 0,
            SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                              newcnode_prop_default(&A, newcnode_invalid()),
                              newcnode_required(&A, stringset(&A, "foo", "baz", NULL)),
                              newcnode_propset(&A,
                                newcnode_prop_match(&A, RE_LITERAL, "foo",
                                  newcnode_switch(&A, 0,
                                    SJP_STRING, newcnode_strmatch(&A, RE_LITERAL, "bar"),
                                    SJP_NONE)),
                                newcnode_prop_match(&A, RE_LITERAL, "baz",
                                  newcnode_switch(&A, 0,
                                    SJP_NUMBER, newcnode_range(&A,
                                                  JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX,
                                                  44.0, 44.0),
                                    SJP_NONE)),
                                NULL),
                              NULL),
            SJP_NONE),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_enum(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0,
          "enum", newjson_num(&A,1),
          "enum", newjson_num(&A,2),
          "enum", newjson_str(&A,"zebra"),
          "enum", newjson_array(&A, newjson_str(&A,"first"), newjson_str(&A,"second"), NULL),
          "enum", newjson_object(&A, 
                     "this", newjson_str(&A, "the quick brown fox jumps over the lazy dog"),
                     "that", newjson_num(&A, 16.0),
                     NULL
                   ),
          NULL),

      NULL,

      newcnode_bool(&A, JVST_CNODE_AND,
        newcnode_bool(&A, JVST_CNODE_OR,
          newcnode_switch(&A, 0,
            SJP_NUMBER, newcnode_range(&A,
                          JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX,
                          1.0, 1.0),
            SJP_NONE),
          newcnode_switch(&A, 0,
            SJP_NUMBER, newcnode_range(&A,
                          JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX,
                          2.0, 2.0),
            SJP_NONE),
          newcnode_switch(&A, 0,
            SJP_STRING, newcnode_strmatch(&A, RE_LITERAL, "zebra"),
            SJP_NONE),
          newcnode_switch(&A, 0,
            SJP_ARRAY_BEG, newcnode_items(&A,
                             newcnode_switch(&A, 0, SJP_NONE),
                             newcnode_switch(&A, 0,
                               SJP_STRING, newcnode_strmatch(&A, RE_LITERAL, "first"),
                               SJP_NONE),
                             newcnode_switch(&A, 0,
                               SJP_STRING, newcnode_strmatch(&A, RE_LITERAL, "second"),
                               SJP_NONE),
                             NULL),
            SJP_NONE),
          newcnode_switch(&A, 0,
            SJP_OBJECT_BEG, newcnode_bool(&A, JVST_CNODE_AND,
                              newcnode_prop_default(&A, newcnode_invalid()),
                              newcnode_required(&A, stringset(&A, "this", "that", NULL)),
                              newcnode_propset(&A,
                                newcnode_prop_match(&A, RE_LITERAL, "this",
                                  newcnode_switch(&A, 0,
                                    SJP_STRING, newcnode_strmatch(&A, RE_LITERAL,
                                      "the quick brown fox jumps over the lazy dog"),
                                    SJP_NONE)),
                                newcnode_prop_match(&A, RE_LITERAL, "that",
                                  newcnode_switch(&A, 0,
                                    SJP_NUMBER, newcnode_range(&A,
                                                  JVST_CNODE_RANGE_MIN|JVST_CNODE_RANGE_MAX,
                                                  16.0, 16.0),
                                    SJP_NONE)),
                                NULL),
                              NULL),
            SJP_NONE),
          NULL
        ),
        newcnode_switch(&A, 1, SJP_NONE),
        NULL
      )
    },

    { STOP },
  };

  RUNTESTS(tests);
}

static void test_xlate_ref(void)
{
  struct arena_info A = {0};

  const struct cnode_test tests[] = {
    {
      TRANSLATE,
      newschema_p(&A, 0,
        "id", "#",
        "properties", newprops(&A, 
          "foo", newschema_p(&A, 0, "$ref", "#", NULL),
          NULL),
        "additionalProperties", false_schema(&A),
        NULL),

      NULL,

      newcnode_switch(&A, 1,
        SJP_OBJECT_BEG, newcnode_bool(&A,JVST_CNODE_AND,
                          newcnode_prop_default(&A, 
                            newcnode_switch(&A, 0, SJP_NONE)),
                          newcnode_bool(&A,JVST_CNODE_AND,
                            newcnode_propset(&A,
                              newcnode_prop_match(&A, RE_LITERAL, "foo",
                                newcnode_ref(&A, "#")),
                              NULL),
                            newcnode_valid(),
                            NULL),
                          NULL),
        SJP_NONE)
    },

    { STOP },
  };

  RUNTESTS(tests);
}

int main(void)
{
  test_xlate_empty_schema();
  test_xlate_true_false_schemas();

  test_xlate_type_number();
  test_xlate_type_object();
  test_xlate_type_several();
  test_xlate_type_integer();

  test_xlate_minimum();
  test_xlate_multiple_of();

  test_xlate_properties();
  test_xlate_propertynames();

  test_xlate_minproperties_1();
  test_xlate_minproperties_2();
  test_xlate_minproperties_3();
  test_xlate_maxproperties_1();
  test_xlate_maxproperties_2();
  test_xlate_minmaxproperties_1();

  test_xlate_required();
  test_xlate_dependencies();

  test_xlate_anyof_allof_oneof_1();
  test_xlate_anyof_2();
  test_xlate_allof_2();
  test_xlate_oneof_2();

  test_xlate_pattern_1();
  test_xlate_minmax_length_1();

  test_xlate_items_1();
  test_xlate_minmax_items();
  test_xlate_contains();

  test_xlate_uniqueItems();

  test_xlate_const();
  test_xlate_enum();

  test_xlate_ref();

  test_simplify_ands();
  test_simplify_ored_schema();
  test_simplify_propsets();
  test_simplify_propertynames();
  test_simplify_required();
  test_simplify_dependencies();
  test_simplify_patterns();
  test_simplify_allof_2();
  test_simplify_oneof_2();

  test_simplify_anded_counts();
  test_simplify_ored_counts();
  test_simplify_anded_ored_counts();
  test_simplify_not_counts();
  test_simplify_xored_counts();
  test_simplify_anded_items();
  test_simplify_anded_number_constraints();
  test_simplify_uniqueItems();

  test_canonify_ored_schema();
  test_canonify_propsets();
  test_canonify_propertynames();
  test_canonify_length_constraints();
  test_canonify_required();
  test_canonify_patterns();

  return report_tests();
}
