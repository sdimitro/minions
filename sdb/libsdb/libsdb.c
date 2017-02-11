#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libsdb.h"

/*
 * Unix-style error
 */
static void
unix_error(char *msg)
{
	(void) fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(EXIT_FAILURE);
}

static void
lsdb_obj_clear_err(lsdb_obj_t *obj)
{
	obj->err = 0;
	obj->err_msg = NULL;
}

/*
 * Used for allocating lsdb_objects. Note that it
 * kills the sdb right away when it prints the unix
 * error.
 */
lsdb_obj_t *
lsdb_obj_alloc(void)
{
	lsdb_obj_t *obj = malloc(sizeof(lsdb_obj_t));
	if (obj == NULL)
		unix_error("libsdb error: malloc failed");
	obj->pph = NULL;
	lsdb_obj_clear_err(obj);
	return (obj);
}

void
lsdb_obj_free(lsdb_obj_t *obj)
{
	free(obj);
}

int
lsdb_lwp_grab(lsdb_obj_t *lobj, lwpid_t lwpid)
{
	int err = 0;

	assert(lobj != NULL);
	assert(lobj->pph != NULL);

        lobj->plh = Lgrab(lobj->pph, lwpid, &err);
        if (lobj->plh == NULL) {
		lobj->err = err;
		lobj->err_msg = Lgrab_error(err);
		return (-1);
        }
	return (0);
}

void
lsdb_lwp_free(lsdb_obj_t *lobj)
{
	assert(lobj != NULL);
	assert(lobj->pph != NULL);
	assert(lobj->plh != NULL);
	/* TODO: investigate what happens if we call it twice! */
	/*
	 * Since we are releasing a core file we don't need to
	 * pass any specific flag.
	 */
	Lfree(lobj->plh);
}

int
lsdb_attach2pid(lsdb_obj_t *lobj, long pid, int flags)
{
	int err = 0;

	assert(lobj != NULL);

        lobj->pph = Pgrab(pid, flags, &err);
        if (lobj->pph == NULL) {
		lobj->err = err;
		lobj->err_msg = Pgrab_error(err);
		return (-1);
        }
	return (0);
}

/*
 * Returns 0 for success and -1 for failure. To check
 * the reason for failure the caller will have to check
 * the error code and/or the error message stored in
 * the lsdb object.
 */
int
lsdb_grab_core(lsdb_obj_t *lobj, const char *core, const char *aout, int flags)
{
	int err = 0;

	assert(lobj != NULL);

        lobj->pph = Pgrab_core(core, aout, flags, &err);
        if (lobj->pph == NULL) {
		lobj->err = err;
		lobj->err_msg = Pgrab_error(err);
		return (-1);
        }
	return (0);
}

/*
 * Returns 0 for succers and -1 for failure. To check
 * the reason for failure the caller will have to check
 * the error code and/or the error message stored in
 * the lsdb object.
 */
int
lsdb_grab_file(lsdb_obj_t *lobj, const char *fname)
{
	int err = 0;

	assert(lobj != NULL);

        lobj->pph = Pgrab_file(fname, &err);
        if (lobj->pph == NULL) {
		lobj->err = err;
		lobj->err_msg = Pgrab_error(err);
		return (-1);
        }
	return (0);
}

void
lsdb_release(lsdb_obj_t *lobj, int flags)
{
	assert(lobj != NULL);
	/* TODO: investigate what happens if we call it twice! */
	/*
	 * Since we are releasing a core file we don't need to
	 * pass any specific flag.
	 */
	Prelease(lobj->pph, flags);
}

int
lsdb_lwp_getfregs(lsdb_obj_t *lobj, lwpid_t lwpid, prfpregset_t *fpregs)
{
	int ret;

	assert(lobj != NULL);
	assert(lobj->pph != NULL);

	ret = Plwp_getfpregs(lobj->pph, lwpid, fpregs);
	if (ret == -1) {
		lobj->err = errno;
		lobj->err_msg = strerror(lobj->err);
	}
	return (ret);
}

int
lsdb_lwp_getregs(lsdb_obj_t *lobj, lwpid_t lwpid, prgregset_t gregs)
{
	int ret;

	assert(lobj != NULL);
	assert(lobj->pph != NULL);

	ret = Plwp_getregs(lobj->pph, lwpid, gregs);
	if (ret == -1) {
		lobj->err = errno;
		lobj->err_msg = strerror(lobj->err);
	}
	return (ret);
}

#if 0
int
lsdb_lwp_getxregs(lsdb_obj_t *lobj, lwpid_t lwpid, prxregset_t xregs)
{
	int ret;

	assert(lobj != NULL);
	assert(lobj->pph != NULL);

	ret = Plwp_getxregs(lobj->pph, lwpid, xregs);
	if (ret == -1) {
		lobj->err = errno;
		lobj->err_msg = strerror(lobj->err);
	}
	return (ret);
}
#endif

int
lsdb_pstate(lsdb_obj_t *lobj)
{
	assert(lobj != NULL);
	assert(lobj->pph != NULL);
	return (Pstate(lobj->pph));
}

const psinfo_t *
lsdb_ppsinfo(lsdb_obj_t *lobj)
{
	const psinfo_t *pst;

	assert(lobj != NULL);
	assert(lobj->pph != NULL);

	lsdb_obj_clear_err(lobj);

	pst = Ppsinfo(lobj->pph);
	if (pst == NULL) {
		lobj->err = -1;
		lobj->err_msg = "ps information not found for this process.";
	}
	return (pst);
}

int
lsdb_getauxval(lsdb_obj_t *lobj, int type)
{
	int ret;

	assert(lobj != NULL);
	assert(lobj->pph != NULL);

	lsdb_obj_clear_err(lobj);

	ret = Pgetauxval(lobj->pph, type);
	if (ret == -1) {
		lobj->err = -1;
		lobj->err_msg = "auxiliary vector or type entry not found.";
	}
	return (ret);
}

int
lsdb_getenv(lsdb_obj_t *lobj, const char *name, char *buf, size_t buflen)
{
	char *ret;

	assert(lobj != NULL);
	assert(lobj->pph != NULL);

	lsdb_obj_clear_err(lobj);

	ret = Pgetenv(lobj->pph, name, buf, buflen);
        if (ret == NULL) {
		lobj->err = -1;
		lobj->err_msg = "environment variable not found.";
		return (-1);
        }
	return (0);
}


/*
 * Returns 0 if we are able to find the full path to
 * the process executable else we return 1. Failing to
 * find the executable should not signify error, since
 * it can be a fairly frequent case, plus we can go on
 * with our debugging. Thus in this case we return 1
 * and not -1.
 */
int
lsdb_execname(lsdb_obj_t *lobj, char *path, size_t buflen)
{
	char *ret;

	assert(lobj != NULL);
	assert(path != NULL);

	lsdb_obj_clear_err(lobj);

	ret = Pexecname(lobj->pph, path, buflen);
        if (ret == NULL) {
		lobj->err = 1;
		lobj->err_msg = "Could not find executable.";
		return (1);
        }
	return (0);
}

core_content_t
lsdb_getcontents(lsdb_obj_t *lobj)
{
	core_content_t cct;

	assert(lobj != NULL);
	assert(lobj->pph != NULL);

	cct = Pcontent(lobj->pph);
	if (cct == CC_CONTENT_INVALID) {
		lobj->err = -1;
		lobj->err_msg = "Invalid content.";
		return (CC_CONTENT_INVALID);
        }

	return cct;
}

/*
 * Since we literally return the register value,
 * callers shoud check the error code and message
 * of the lsdb object to ensure that the register
 * value is valid.
 *
 * XXX:
 * This is an architecture dependent code (eax/rax,
 * longlong/long/int, etc..). Leave it like this for
 * now.
 */
long
lsdb_getareg(lsdb_obj_t *lobj, int regno)
{
	prgreg_t peax;

	lsdb_obj_clear_err(lobj);

	int err = Pgetareg(lobj->pph, regno, &peax);
        if (err == -1) {
		lobj->err = errno;
		lobj->err_msg = strerror(lobj->err);
		return (-1);
        }
	return ((long) peax);
}

int
lsdb_getplatform(lsdb_obj_t *lobj, char *buf, size_t bufsize)
{
	assert(lobj != NULL);
	assert(lobj->pph != NULL);

	if (!Pplatform(lobj->pph, buf, bufsize)) {
		lobj->err = errno;
		lobj->err_msg = strerror(lobj->err);
		return (-1);
	}
	return (0);
}
