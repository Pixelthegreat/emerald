#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/memory.h>
#include <emerald/log.h>
#include <emerald/refobj.h>

bool em_log_reflist = false; /* info log messages for reference lists */

/* initialize list */
EM_API em_result_t em_reflist_init(em_reflist_t *list) {

	if (!list) return EM_RESULT_FAILURE;
	else if (list->init) {

		em_log_fatal("List already initialized");
		return EM_RESULT_FAILURE;
	}

	list->nlock = 0;
	list->normal.first = NULL;
	list->normal.last = NULL;
	list->wait.first = NULL;
	list->wait.last = NULL;

	list->init = EM_TRUE;
	return EM_RESULT_SUCCESS;
}

/* add item to list */
EM_API void em_reflist_add(em_reflist_t *list, em_refobj_t *obj) {

	if (!list || !list->init || !obj) return;

	obj->prev = list->normal.last;
	if (!list->normal.first) list->normal.first = obj;
	if (list->normal.last) list->normal.last->next = obj;
	list->normal.last = obj;

	if (em_log_reflist) em_log_info("Added object %p", obj);
}

/* move item to destruction wait list */
EM_API void em_reflist_move(em_reflist_t *list, em_refobj_t *obj) {

	if (!list || !list->init || !obj) return;

	/* remove from normal list */
	if (list->normal.first == obj) list->normal.first = obj->next;
	if (list->normal.last == obj) list->normal.last = obj->prev;
	if (obj->prev) obj->prev->next = obj->next;
	if (obj->next) obj->next->prev = obj->prev;

	/* add to destruction wait list */
	obj->prev = list->wait.last;
	if (!list->wait.first) list->wait.first = obj;
	if (list->wait.last) list->wait.last->next = obj;
	list->wait.last = obj;
}

/* move item to normal list */
EM_API void em_reflist_move_back(em_reflist_t *list, em_refobj_t *obj) {

	if (!list || !list->init || !obj) return;

	/* remove from normal list */
	if (list->wait.first == obj) list->wait.first = obj->next;
	if (list->wait.last == obj) list->wait.last = obj->prev;
	if (obj->prev) obj->prev->next = obj->next;
	if (obj->next) obj->next->prev = obj->prev;

	/* add to destruction wait list */
	obj->prev = list->normal.last;
	if (!list->normal.first) list->normal.first = obj;
	if (list->normal.last) list->normal.last->next = obj;
	list->normal.last = obj;
}

/* remove item from list */
EM_API void em_reflist_remove(em_reflist_t *list, em_refobj_t *obj) {

	if (!list || !list->init || !obj) return;

	if (list->normal.first == obj) list->normal.first = obj->next;
	if (list->wait.first == obj) list->wait.first = obj->next;
	if (list->normal.last == obj) list->normal.last = obj->prev;
	if (list->wait.last == obj) list->wait.last = obj->prev;
	if (obj->prev) obj->prev->next = obj->next;
	if (obj->next) obj->next->prev = obj->prev;

	em_refobj_free(obj);

	if (em_log_reflist) em_log_info("Removed object %p", obj);
}

/* remove items from destruction wait list */
EM_API void em_reflist_cleanup(em_reflist_t *list) {

	if (!list || !list->init) return;

	em_refobj_t *obj = list->wait.first;
	while (obj) {

		em_refobj_t *next = obj->next;
		
		/* add back to normal list */
		if (obj->refcnt > 0)
			em_reflist_move_back(list, obj);
		else em_reflist_remove(list, obj);

		obj = next;
	}
}

/* lock list to prevent any immediate item destruction */
EM_API void em_reflist_lock(em_reflist_t *list) {

	if (!list || !list->init) return;

	list->nlock++;
}

/* unlock list */
EM_API void em_reflist_unlock(em_reflist_t *list) {

	if (!list || !list->init || list->nlock <= 0) return;

	if (!--list->nlock) em_reflist_cleanup(list);
}

/* destroy list */
EM_API void em_reflist_destroy(em_reflist_t *list) {

	if (!list || !list->init) return;
	list->nlock = -1;

	/* call free handlers */
	em_refobj_t *obj = list->normal.first;
	while (obj) {

		em_refobj_t *next = obj->next;
		
		if (obj->free) obj->free(obj);
		obj = next;
	}

	obj = list->wait.first;
	while (obj) {

		em_refobj_t *next = obj->next;

		if (obj->free) obj->free(obj);
		obj = next;
	}

	/* free objects */
	obj = list->normal.first;
	while (obj) {

		em_refobj_t *next = obj->next;
		
		em_refobj_free_bare(obj);
		obj = next;
	}

	obj = list->wait.first;
	while (obj) {

		em_refobj_t *next = obj->next;
		
		em_refobj_free_bare(obj);
		obj = next;
	}

	list->init = EM_FALSE;
}

/* create reference object */
EM_API em_refobj_t *em_refobj_new_full(em_reflist_t *list, size_t size, em_cleanup_mode_t mode, const char *file, em_ssize_t line) {

	if (!list || mode < 0 || mode >= EM_CLEANUP_MODE_COUNT) return NULL;

	em_refobj_t *obj = EM_REFOBJ(em_allocate(size, file, line));
	memset(obj, 0, size);

	obj->refcnt++;
	em_reflist_add(list, obj);

	obj->mode = mode;
	obj->list = list;

	return obj;
}

/* increase reference count */
EM_API em_refobj_t *em_refobj_incref(em_refobj_t *obj) {

	if (!obj) return NULL;

	obj->refcnt++;
	return obj;
}

/* decrease reference count */
EM_API void em_refobj_decref(em_refobj_t *obj) {

	if (!obj || !obj->refcnt) return;

	if (!--obj->refcnt) {

		if (obj->list->nlock || obj->mode == EM_CLEANUP_MODE_WAITLIST)
			em_reflist_move(obj->list, obj);
		else em_reflist_remove(obj->list, obj);
	}
}

/* free reference object */
EM_API void em_refobj_free(em_refobj_t *obj) {

	if (obj->free) obj->free(obj);
	em_refobj_free_bare(obj);
}

/* free without calling handler */
EM_API void em_refobj_free_bare(em_refobj_t *obj) {

	if (em_log_reflist) em_log_info("Freed object %p", obj);
	em_free(obj);
}
