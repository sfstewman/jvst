/* Tests to bootstrap validation */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ast.h"
#include "validate.h"

int ntest;
int nfail;

struct validation_test {
  bool succeeds;
  const char *json;
  struct ast_schema *schema;
};

// arena allocators to make it easy to programmatically set up a schema
static struct ast_schema ar_schema[1024];
static struct ast_property_schema ar_props[1024];
static struct ast_string_set ar_stringsets[1024];

struct arena_info {
  size_t nschema;
  size_t nprop;
  size_t nstr;
};

// Returns a constant empty schema
static struct ast_schema *empty_schema(void)
{
  static struct ast_schema empty = { 0 };
  return &empty;
}

static struct json_string newstr(const char *s)
{
  struct json_string str = { .s = s, .len = strlen(s) };
  return str;
}

static struct ast_string_set *stringset(struct arena_info *A, ...)
{
  size_t max;
  struct ast_string_set *ss = NULL, **ssp = &ss;
  va_list args;

  max = sizeof ar_stringsets / sizeof ar_stringsets[0];
  va_start(args, A);
  for(;;) {
    struct ast_string_set *elt;
    struct json_string str;
    const char *s;
    size_t i;

    if (s = va_arg(args, const char *), s == NULL) {
      break;
    }

    i = A->nstr++;
    if (A->nstr >= max) {
      fprintf(stderr, "too many string sets: %zu max\n", max);
      abort();
    }

    elt = &ar_stringsets[i];
    elt->str = newstr(s);
    *ssp = elt;
    ssp = &elt->next;
  }
  va_end(args);

  return ss;
}

static struct ast_schema *newschema(struct arena_info *A, int types)
{
  size_t i,max;
  struct ast_schema *s;

  i = A->nschema++;
  max = sizeof ar_schema / sizeof ar_schema[0];
  if (A->nschema >= max) {
    fprintf(stderr, "too many schema: %zu max\n", max);
    abort();
  }

  s = &ar_schema[i];
  memset(s, 0, sizeof *s);
  s->types = types;
  return s;
}

static struct ast_schema *newschema_p(struct arena_info *A, int types, ...)
{
  size_t i,max;
  struct ast_schema *s;
  const char *pname;
  va_list args;

  i = A->nschema++;
  max = sizeof ar_schema / sizeof ar_schema[0];
  if (A->nschema >= max) {
    fprintf(stderr, "too many schema: %zu max\n", max);
    abort();
  }

  s = &ar_schema[i];
  memset(s, 0, sizeof *s);
  s->types = types;

  va_start(args, types);
  for(;;) {
    pname = va_arg(args, const char *);
    if (pname == NULL) {
      break;
    }

    // big dumb if-else chain gets the job done...
    if (strcmp(pname, "minProperties") == 0) {
      s->kws |= KWS_MINMAX_PROPERTIES;
      s->min_properties = va_arg(args, ast_count);
    } else if (strcmp(pname, "maxProperties") == 0) {
      s->kws |= KWS_MINMAX_PROPERTIES;
      s->max_properties = va_arg(args, ast_count);
    } else if (strcmp(pname, "properties") == 0) {
      s->properties.set = va_arg(args, struct ast_property_schema *);
    } else if (strcmp(pname, "required") == 0) {
      s->required.set = va_arg(args, struct ast_string_set *);
    } else {
      // okay to abort() a test if the test writer forgot to add a
      // property to the big dumb if-else chain
      fprintf(stderr, "unsupported schema properties '%s'\n", pname);
      abort();
    }
  }
  va_end(args);

  return s;
}

static struct ast_property_schema *newprops(struct arena_info *A, ...)
{
  struct ast_property_schema *props = NULL;
  struct ast_property_schema **pp = &props;
  size_t max = sizeof ar_props / sizeof ar_props[0];
  va_list args;

  va_start(args, A);

  for(;;) {
    const char *name;
    struct ast_schema *schema;
    struct ast_property_schema *p;
    size_t i;

    name = va_arg(args, const char *);
    if (name == NULL) {
      break;
    }

    i = A->nprop++;
    if (A->nprop >= max) {
      fprintf(stderr, "too many properties: %zu max\n", max);
      abort();
    }

    p = &ar_props[i];
    memset(p, 0, sizeof *p);

    p->pattern.str = newstr(name);
    p->schema = va_arg(args, struct ast_schema *);

    *pp = p;
    pp = &p->next;
  }

  va_end(args);

  return props;
}

static const char *jvst_ret2name(int ret)
{
  switch (ret) {
  case JVST_INVALID:
    return "INVALID";
  case JVST_VALID:
    return "VALID";
  case JVST_MORE:
    return "MORE";
  default:
    return "UNKNOWN";
  }
}

static int run_test(const struct validation_test *t)
{
  struct jvst_validator v;
  size_t n;
  int ret, failed;
  static char buf[4096];

  jvst_validate_init_defaults(&v, t->schema);
  n = strlen(t->json);
  if (n >= sizeof buf) {
    fprintf(stderr, "json exceeds buffer size (%s:%d)\n", __FILE__, __LINE__);
    abort();
  }

  // already checked buffer size
  strcpy(buf, t->json);
  ret = jvst_validate_more(&v, buf, n);
  /*
  fprintf(stderr, "jvst_validate_more(...\"%s\"...) : %s\n",
      buf, jvst_ret2name(ret));
      */
  failed = JVST_IS_INVALID(ret);

  ret = jvst_validate_close(&v);
  /*
  fprintf(stderr, "jvst_validate_close() : %s\n",
      jvst_ret2name(ret));
      */
  failed = failed || JVST_IS_INVALID(ret);

  return !failed;
}

#define RUNTESTS(testlist) runtests(__func__, (testlist))
static void runtests(const char *testname, const struct validation_test tests[])
{
  int i;

  for (i=0; tests[i].json != NULL; i++) {
    bool succ;
    ntest++;

    succ = !!run_test(&tests[i]);
    if (succ != tests[i].succeeds) {
      printf("%s_%d: failed (expected %s but found %s)\n",
          testname, i+1,
          tests[i].succeeds ? "success" : "failure",
          succ ? "success" : "failure");
      nfail++;
    }
  }
}

/* test to get us off the ground */
void test_empty_schema(void)
{
  const struct validation_test tests[] = {
    { true, "{}", empty_schema() },
    { true, "[]", empty_schema() },
    { true, "{ \"foo\" : \"bar\" }", empty_schema() },

    // one to make sure that we're checking for valid json
    { false, "{ 12 : \"bar\" }", empty_schema() },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

void test_type_number(void)
{
  struct ast_schema schema = {
    .types = JSON_VALUE_NUMBER,
  };

  const struct validation_test tests[] = {
    //  "an integer is a number",
    { true, "1", &schema, },

    //  "a float is a number",
    { true, "1.1", &schema, },

    //  "a string is not a number",
    { false, "\"foo\"", &schema, },

    //  "a string is still not a number, even if it looks like one",
    { false, "\"1\"", &schema, },

    //  "an object is not a number",
    { false, "{}", &schema, },

    //  "an array is not a number",
    { false, "[]", &schema, },

    //  "a boolean is not a number",
    { false, "true", &schema, },

    //  "null is not a number",
    { false, "null", &schema, },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

void test_type_object(void)
{
  struct ast_schema schema = {
    .types = JSON_VALUE_OBJECT,
  };

  const struct validation_test tests[] = {
    //  "an integer is not an object",
    { false, "1", &schema, },

    //  "a float is not an object",
    { false, "1.1", &schema, },

    //  "a string is not an object",
    { false, "\"foo\"", &schema, },

    //  "an object is an object",
    { true, "{}", &schema, },

    //  "an array is not an object",
    { false, "[]", &schema, },

    //  "a boolean is not an object",
    { false, "true", &schema, },

    //  "null is not an object",
    { false, "null", &schema, },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

void test_properties(void)
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

  const struct validation_test tests[] = {
    // "description": "both properties present and valid is valid",
    { true, "{\"foo\": 1, \"bar\": \"baz\"}", &schema },

    // "description": "one property invalid is invalid",
    { false, "{\"foo\": 1, \"bar\": {}}", &schema },

    // "description": "both properties invalid is invalid",
    { false, "{\"foo\": [], \"bar\": {}}", &schema },

    // "description": "doesn't invalidate other properties",
    { true, "{\"quux\": []}", &schema },

    // "description": "ignores non-objects",
    { true, "[]", &schema },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

void test_minproperties_1(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "minProperties", 1,
      NULL);

  const struct validation_test tests[] = {
    // "description": "longer is valid",
    { true, "{\"foo\": 1, \"bar\": 2}", schema },

    // "description": "exact length is valid",
    { true, "{\"foo\": 1}", schema },

    // "description": "too short is invalid",
    { false, "{}", schema },

    // "description": "ignores non-objects",
    { true, "\"\"", schema },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

void test_minproperties_2(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "minProperties", 1,
      "properties", newprops(&A,
        "foo", newschema(&A, JSON_VALUE_NUMBER), // XXX - JSON_VALUE_INTEGER
        "bar", newschema(&A, JSON_VALUE_STRING),
        NULL),
      NULL);

  const struct validation_test tests[] = {
    // "description": "longer is valid",
    { true, "{\"foo\": 1, \"bar\": \"baz\"}", schema },

    // "description": "satisfies minProperties but not properties"
    { false, "{\"foo\": 1, \"bar\": 2}", schema },

    // "description": "exact length is valid",
    { true, "{\"foo\": 1}", schema },

    // "description": "satisfies minProperties but not properties"
    { false, "{\"bar\": 1}", schema },

    // "description": "satisfies minProperties and properties"
    { true, "{\"bar\": \"baz\"}", schema },

    // "description": "too short is invalid",
    { false, "{}", schema },

    // "description": "ignores non-objects",
    { true, "\"\"", schema },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

void test_minproperties_3(void)
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

  const struct validation_test tests[] = {
    // "description": "longer is valid",
    { true, "{\"baz\": 1, \"bar\": \"baz\"}", schema },

    // "description": "root satisfies minProperties but foo is the wrong type"
    { false, "{\"foo\": 1, \"bar\": \"baz\"}", schema },

    // "description": "root satisfies minProperties but foo does not"
    { false, "{\"foo\": {}, \"bar\": \"baz\"}", schema },

    // "description": "root and foo have valid lengths",
    { true, "{\"foo\": {\"bar\": 3}}", schema },

    // "description": "satisfies minProperties but not properties"
    { false, "{\"bar\": 1}", schema },

    // "description": "satisfies minProperties and properties"
    { true, "{\"bar\": \"baz\"}", schema },

    // "description": "too short is invalid",
    { false, "{}", schema },

    // "description": "ignores non-objects",
    { true, "\"\"", schema },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

void test_maxproperties_1(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "maxProperties", 2,
      NULL);

  const struct validation_test tests[] = {
    // "description": "shorter is valid",
    { true, "{\"foo\": 1}", schema },

    // "description": "exact length is valid",
    { true, "{\"foo\": 1, \"bar\": 2}", schema },

    // "description": "too long is invalid",
    { false, "{\"foo\": 1, \"bar\": 2, \"baz\": 3}", schema },

    // "description": "ignores non-objects",
    { true, "\"foobar\"", schema },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

void test_maxproperties_2(void)
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

  const struct validation_test tests[] = {
    // "description": "shorter is valid",
    { true, "{}", schema },

    // "description": "root and foo satisfy maxProperties"
    { true, "{\"foo\": {} }", schema },

    // "description": "root exact length is valid"
    { true, "{\"bar\": \"baz\"}", schema },

    // "description": "root exact length is valid, but bar is wrong type"
    { false, "{\"bar\": 1}", schema },

    // "description": "root has too many properties",
    { false, "{\"baz\": 1, \"bar\": \"baz\"}", schema },

    // "description": "root exact length is valid, shorter foo is valid"
    { true, "{\"foo\": {}}", schema },

    // "description": "root exact length is valid, foo exact length is valid"
    { true, "{\"foo\": { \"bar\" :  1 }}", schema },

    // "description": "root exact length is valid, foo is too long"
    { false, "{\"foo\": { \"bar\" :  1, \"baz\" : \"quux\" }}", schema },

    // "description": "ignores non-objects",
    { true, "\"\"", schema },

    // "description": "ignores non-objects",
    { true, "[]", schema },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

void test_minmaxproperties_1(void)
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

  const struct validation_test tests[] = {
    // "description": "root and foo satisfy min/maxProperties"
    { true, "{\"foo\": { \"bar\" : 17 } }", schema },

    // "description": "root exact length is valid"
    { true, "{\"bar\": \"baz\"}", schema },

    // "description": "root has too few properties",
    { false, "{}", schema },

    // "description": "root exact length is valid, but bar is wrong type"
    { false, "{\"bar\": 1}", schema },

    // "description": "root has too many properties",
    { false, "{\"baz\": 1, \"bar\": \"baz\"}", schema },

    // "description": "root exact length is valid, shorter foo is invalid"
    { false, "{\"foo\": {}}", schema },

    // "description": "root exact length is valid, foo satisfies min/max length"
    { true, "{\"foo\": { \"bar\" :  1 }}", schema },

    // "description": "root exact length is valid, foo satisfies min/max length"
    { true, "{\"foo\": { \"bar\" :  1, \"baz\" : \"quux\" }}", schema },

    // "description": "root exact length is valid, foo is too long"
    { false, "{\"foo\": { \"bar\" :  1, \"baz\" : \"quux\", \"quux\" : 7 }}", schema },

    // "description": "ignores non-objects",
    { true, "\"\"", schema },

    // "description": "ignores non-objects",
    { true, "[]", schema },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

void test_required(void)
{
  struct arena_info A = {0};
  struct ast_schema *schema = newschema_p(&A, 0,
      "properties", newprops(&A,
        "foo", empty_schema(),
        "bar", empty_schema(),
        NULL),
      "required", stringset(&A, "foo", NULL),
      NULL);

  const struct validation_test tests[] = {
    // "description": "present required property is valid",
    { true, "{\"foo\": 1}", schema },

    // "description": "non-present required property is invalid",
    { false, "{\"bar\": 1}", schema },

    // "description": "ignores non-objects",
    { true, "12", schema },

    { false, NULL, NULL },
  };

  RUNTESTS(tests);
}

int main(void)
{
  test_empty_schema();

  test_type_number();
  test_type_object();

  test_properties();

  test_minproperties_1();
  test_minproperties_2();
  test_minproperties_3();
  test_maxproperties_1();
  test_maxproperties_2();
  test_minmaxproperties_1();

  test_required();

  printf("%d tests, %d failures\n", ntest, nfail);

  return (nfail == 0 && ntest > 0) ? 0 : 1;
}
