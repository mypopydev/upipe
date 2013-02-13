/*
 * Copyright (C) 2013 OpenHeadend S.A.R.L.
 *
 * Authors: Benjamin Cohen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/** @file
 * @short Upipe null module - free incoming urefs
 */

#include <upipe/ubase.h>
#include <upipe/urefcount.h>
#include <upipe/uprobe.h>
#include <upipe/uref.h>
#include <upipe/upipe.h>
#include <upipe/udict.h>
#include <upipe/udict_dump.h>
#include <upipe/upipe_helper_upipe.h>
#include <upipe-modules/upipe_null.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <assert.h>


/** upipe_null structure */
struct upipe_null {
    /** dump dict */
    bool dump;
    /** refcount management structure */
    urefcount refcount;
    /** public upipe structure */
    struct upipe upipe;
};
UPIPE_HELPER_UPIPE(upipe_null, upipe);

/** @internal @This allocates a null pipe.
 *
 * @param mgr common management structure
 * @param uprobe structure used to raise events
 * @return pointer to upipe or NULL in case of allocation error
 */
static struct upipe *upipe_null_alloc(struct upipe_mgr *mgr,
                                       struct uprobe *uprobe)
{
    struct upipe_null *upipe_null = malloc(sizeof(struct upipe_null));
    if (unlikely(!upipe_null)) return NULL;
    upipe_init(&upipe_null->upipe, mgr, uprobe);
    urefcount_init(&upipe_null->refcount);
    upipe_throw_ready(&upipe_null->upipe);
    return &upipe_null->upipe;
}

/** @internal @This sends data to devnull.
 *
 * @param upipe description structure of the pipe
 * @param uref uref structure
 * @param upump pump that generated the buffer
 */
static void upipe_null_input(struct upipe *upipe, struct uref *uref, struct upump *upump)
{
    struct upipe_null *upipe_null = upipe_null_from_upipe(upipe);
    upipe_dbg(upipe, "sending uref to devnull");
    if (upipe_null->dump) {
        udict_dump(uref->udict, upipe->uprobe);
    }
    uref_free(uref);
}

/** @internal @This processes control commands.
 *
 * @param upipe description structure of the pipe
 * @param command type of command to process
 * @param args arguments of the command
 * @return false in case of error
 */
static bool upipe_null_control(struct upipe *upipe, enum upipe_command command,
                               va_list args)
{
    struct upipe_null *upipe_null = upipe_null_from_upipe(upipe);
    switch(command) {
        case UPIPE_NULL_DUMP_DICT: {
            int signature = va_arg(args, int);
            assert (signature == UPIPE_NULL_SIGNATURE);
            upipe_null->dump = (va_arg(args, int) == 1 ? true : false);
            return true;
        }
        default:
            return false;
    }
}

/** @This increments the reference count of a upipe.
 *
 * @param upipe description structure of the pipe
 */
static void upipe_null_use(struct upipe *upipe)
{
    struct upipe_null *upipe_null = upipe_null_from_upipe(upipe);
    urefcount_use(&upipe_null->refcount);
}

/** @internal @This frees all resources allocated.
 *
 * @param upipe description structure of the pipe
 */

static void upipe_null_release(struct upipe *upipe)
{
    struct upipe_null *upipe_null = upipe_null_from_upipe(upipe);
    if (unlikely(urefcount_release(&upipe_null->refcount))) {
        upipe_throw_dead(upipe);
        upipe_clean(upipe);
        urefcount_clean(&upipe_null->refcount);
        free(upipe_null);
    }
}

/** upipe_null (/dev/null) */
static struct upipe_mgr upipe_null_mgr = {
    .upipe_alloc = upipe_null_alloc,
    .upipe_input = upipe_null_input,
    .upipe_control = upipe_null_control,
    .upipe_release = upipe_null_release,
    .upipe_use = upipe_null_use,
    .upipe_mgr_release = NULL
};

/** @This returns the management structure for null pipes
 *
 * @return pointer to manager
 */
struct upipe_mgr *upipe_null_mgr_alloc(void)
{
    return &upipe_null_mgr;
}
