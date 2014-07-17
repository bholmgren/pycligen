#ifndef __PY_CLIGEN_H__
#define __PY_CLIGEN_H__

struct _CLIgen;

typedef struct {
    int                      ch_magic;    /* magic (HDR)*/
    struct _CLIgen	    *ch_self;
    cligen_handle            ch_cligen;   /* cligen handle */
} *CLIgen_handle;


typedef struct _CLIgen {
    PyObject_HEAD
    CLIgen_handle handle;
    PyObject *globals;
    parse_tree pt;
} CLIgen;

cg_fnstype_t *CLIgen_str2fn(char *name, void *arg, char **error);

#endif /* __PY_CLIGEN_H__ */
