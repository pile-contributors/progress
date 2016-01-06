/**
 * @file progress.cc
 * @brief Definitions for Progress class.
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#include "progress.h"
#include "progress-private.h"
#include <limits.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>


#if DEBUG_OFF
#   define PRGR_DEBUG DBG_PMESSAGE
#else
#   define PRGR_DEBUG black_hole
#endif

#if DEBUG_OFF
#   define V_PRGR_DEBUG DBG_PMESSAGE
#else
#   define V_PRGR_DEBUG black_hole
#endif

#if DEBUG_OFF
#   define PRGR_TRACE_ENTRY DBG_TRACE_ENTRY
#else
#   define PRGR_TRACE_ENTRY
#endif

#if DEBUG_OFF
#   define PRGR_TRACE_EXIT DBG_TRACE_EXIT
#else
#   define PRGR_TRACE_EXIT
#endif

#if DEBUG_OFF
#   define PORTION_DUMP(__t__, __p__) DBG_PMESSAGE (\
    __t__ ": (portion)\n" \
    "    - offset_in_parent_ = %"PRIi64"\t" \
    "    - size_in_parent_ = %"PRIi64"\t" \
    "    - tot_size_ = %"PRIi64"\n" \
    "    - progress_ = %"PRIi64"\t" \
    "    - user_data_ = %p\t" \
    "    - current_status_ = <%s>\n",  \
    __p__.offset_in_parent_, \
    __p__.size_in_parent_, \
    __p__.tot_size_, \
    __p__.progress_, \
    (void*)__p__.user_data_, \
    TMP_A(__p__.current_status_));
#else
#   define PORTION_DUMP(__t__, __p__)
#endif

#if DEBUG_OFF
#   define PRGR_DUMP(__t__, __p__) DBG_PMESSAGE (\
    __t__ ": (progress)\n" \
    "    - stack size: %d\t" \
    "    - cutoff_level_: %"PRIi64"\t" \
    "    - granularity_: %"PRIi64"\t" \
    "    - prev_prog_: %"PRIi64"\n" \
    "    - b_should_stop_: %s\t" \
    "    - current_status_: <%s>\t" \
    "    - user_data_: %p\n" \
    "    - kb_simple_signal_: %p\t" \
    "    - kb_full_signal_: %p\n", \
    __p__.stack_.size (), \
    __p__.cutoff_level_, \
    __p__.granularity_, \
    __p__.prev_prog_, \
    __p__.b_should_stop_ ? "true" : "false", \
    TMP_A(__p__.current_status_), \
    (void*)__p__.user_data_, \
    (void*)__p__.kb_simple_signal_, \
    (void*)__p__.kb_full_signal_);
#else
#   define PRGR_DUMP(__t__, __p__)
#endif


/**
 * @class Progress
 *
 * The class helps in keeping track of an asynchronous operation.
 * It divides the task (total_progress_) into portions and has
 * a current portion defined by an offset and a size. The progress that
 * is reported is scaled inside the portion and the offset is applied
 * for the final report.
 *
 * The mechanism allows the caller to initialize the structure,
 * set current portion then allow the sub-task to report the progress
 * without knowing anything about its internals or size. The parent task
 * only decides how much of its progress the sub-task is going to take.
 *
 * The class does not store a list for each level and stepping to a new portion
 * removes previous portion at the same level.
 *
 * The label for current operation is given by first non-empty label that
 * was provided, starting from the last portion. Thus sub-tasks have the
 * option to leave the label empty, thus inheriting the text from the
 * parent. In practice this value is cached in current_status_ to avoid
 * recursive search each time.
 *
 * To use the class for a simple task that only has a single level
 * simply call init () at the beginning, step() in the loop
 * and finish () at the end.
 *
 * The user has the ability to control the number of signals the
 * class emits by choosing the maximum depth of the stack for which the
 * signals are emitted (0 disables the signals) and by choosing
 * the granularity (the *total progress* must
 * advance by at least that much to trigger a signal).
 * By default all levels emit signals and the granularity is 1.
 *
 */
/*  DEFINITIONS    ========================================================= */
//
//
//
//
/*  DATA    ---------------------------------------------------------------- */

/*  DATA    ================================================================ */
//
//
//
//
/*  FUNCTIONS    ----------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
Progress::Progress () :
    stack_(),
    cutoff_level_(INT_MAX),
    granularity_(1),
    prev_prog_ (0),
    b_should_stop_(false),
    current_status_(),
    user_data_(NULL),
    kb_simple_signal_(NULL),
    kb_full_signal_(NULL)
{
    PRGR_TRACE_ENTRY;

    PRGR_DUMP("  default values", (*this));

    PRGR_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
Progress::~Progress ()
{
    PRGR_TRACE_ENTRY;
    end ();
    PRGR_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * The function creates a first entry in the stack and initializes with
 * offset 0 and provided size.
 *
 * The caller does not need to call end () as the functions starts by doing that.
 *
 * @param title The name of this job; will be shown when the subtasks
 *              do not provide their own title.
 * @param total_size Total size of this task; the progress will be
 *                   in same units as this value.
 * @return true if everything went fine
 */
bool Progress::init (const QString & title, int64_t total_size)
{
    PRGR_TRACE_ENTRY;
    bool b_ret = false;
    for (;;) {
        end ();

        if (total_size <= 0) {
            PRGR_DEBUG (
                        "  total size (%" PRIi64 ") must be a "
                        "positive integer\n", total_size);
            break;
        }

        Portion p;
        p.offset_in_parent_ = 0;
        p.size_in_parent_ = total_size;
        p.progress_ = 0;
        p.tot_size_ = total_size;
        p.user_data_ = NULL;
        p.current_status_ = title;
        stack_.push_front (p);

        current_status_ = title;
        prev_prog_ = 0;

        b_should_stop_ = false;

        PRGR_DUMP("  initialized", (*this));
        PORTION_DUMP("  base portion", p);

        b_ret = true;
        break;
    }

    PRGR_TRACE_EXIT;
    return b_ret;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void Progress::end ()
{
    PRGR_TRACE_ENTRY;
    PRGR_DUMP("  before end()", (*this));
    stack_.clear ();
    b_should_stop_ = true;
    current_status_.clear ();
    PRGR_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * If the instance was not initialized the method will do that
 * before proceeding.
 *
 * @param parent_size
 * @param total_size
 * @param parent_offset
 * @param label
 * @param portion_data
 */
void Progress::enter (
        int64_t parent_size, const QString &label, int64_t total_size,
        int64_t parent_offset, void * portion_data)
{
    PRGR_TRACE_ENTRY;
    bool b_ret = false;
    for (;;) {

        // special case when initialization is done via enter ()
        if (!isInitialized ()) {
            if (!init (label, total_size)) {
                break;
            }
            Portion & f = stack_.first ();
            f.offset_in_parent_ = parent_offset;
            f.user_data_ = portion_data;

            PORTION_DUMP("  init via enter(); altered first", f);

            b_ret = true;
            break;
        }

        // current first (parent of this one)
        Portion & f = stack_.first ();

        // create a new structure for it
        Portion p;
        if (parent_offset < 0) {
            p.offset_in_parent_ = f.progress_;
        } else {
            p.offset_in_parent_ = parent_offset;
        }
        p.size_in_parent_ = parent_size;
        p.progress_ = 0;
        p.tot_size_ = total_size;
        p.user_data_ = portion_data;
        p.current_status_ = label;
        stack_.push_front (p);

        // update the top label
        if (!label.isEmpty ()) {
            current_status_ = label;
        }

        PORTION_DUMP("  new in enter()", p);

        signalChange ();

        b_ret = true;
        break;
    }
    Q_UNUSED(b_ret);
    PRGR_DUMP("  after enter()", (*this));
    PRGR_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * @brief Progress::finish
 * @param update_parent
 */
void Progress::finish (bool update_parent)
{
    PRGR_TRACE_ENTRY;
    for (;;) {
        if (stack_.isEmpty ()) break;

        // the portion to be dropped
        Portion & f = stack_.first ();
        PORTION_DUMP("  to be dropped", f);

        bool b_update_title = !f.current_status_.isEmpty ();
        int64_t offset_in_parent = f.offset_in_parent_;
        int64_t size_in_parent = f.size_in_parent_;

        // remove it from the list
        // Portion & f no longer valid
        stack_.pop_front ();

        if (b_update_title) {
            current_status_ = searchCurrentLabel ();
        }

        if (stack_.isEmpty ()) {
            end ();
        } else {
            if (update_parent) {
                Portion & newf = stack_.first ();
                newf.progress_ = offset_in_parent + size_in_parent;
            }
        }

        PRGR_DUMP("  after finish()", (*this));

        signalChange ();
        break;
    }
    PRGR_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * Advances the progress for current portion if offset is negative
 * by given chunk size or sets the progress for current portion
 * by summing the offset and the size of the chunk.
 *
 * The user can use this method in two ways. The progress can be
 * updated by only providing the size of the chunk:
 *
 * @code
 * Progress p;
 * p.init ("example task), 500;
 * for (int i == 0; i < 500; ++i {
 *     p.step (1);
 * }
 * p.finish ();
 * @endcode
 *
 * The progress can alternatively be set directly:
 *
 * @code
 * Progress p;
 * p.init ("example task), 500;
 * for (int i == 0; i < 500; ++i {
 *     p.step (0, i);
 * }
 * p.finish ();
 * @endcode
 *
 * @param chunk_size Size of current portion not including the previous work.
 * @param offset Previous work (use cached value by default).
 * @return true if the process should continue, false to stop
 */
bool Progress::step (int64_t chunk_size, int64_t offset)
{
    PRGR_TRACE_ENTRY;
    bool b_ret = false;
    for (;;) {
        if (stack_.isEmpty ()) {
            PRGR_DEBUG (
                        "  please call init() before stepping\n");
            break;
        }

        if (chunk_size < 0) {
            PRGR_DEBUG (
                        "  chunk size (%" PRIi64 ") must be a "
                        "positive integer\n", chunk_size);
            break;
        }

        Portion & p = stack_.first ();

        if (offset < 0) {
            p.progress_ += chunk_size;
        } else {
            p.progress_ = offset + chunk_size;
        }
        PORTION_DUMP("  after step()", p);

        // signal a change
        signalChange ();

        b_ret = true;
        break;
    }

    PRGR_TRACE_EXIT;
    return b_ret && !b_should_stop_;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * @brief Progress::emitSigal
 * @return true if the process should continue, false to stop
 */
bool Progress::emitSigal()
{
    PRGR_TRACE_ENTRY;

    if (!isInitialized ()) {
        PRGR_DEBUG (" can't emit signal before initialization\n");
        return false;
    }

    signalChange (true);

    PRGR_TRACE_EXIT;
    return !b_should_stop_;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/**
 * This method is useful when the parent passes to the child
 * a Progress instance that has already been prepared by calling
 * enter(). The child process then may need to adjust the values
 * for the total span and (less likely) the position.
 *
 * @param total_size
 * @param progress
 */
void Progress::setLevelCharact (int64_t total_size, int64_t progress)
{
    PRGR_TRACE_ENTRY;
    if (!isInitialized ()) {
        PRGR_DEBUG (" can't set level characteristics before initialization\n");
        return;
    }

    Portion & f = stack_.first ();

    f.tot_size_ = total_size;
    f.progress_ = progress;

    PORTION_DUMP("  after setLevelCharact()", f);
    PRGR_TRACE_EXIT;
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
QString Progress::searchCurrentLabel()
{
    foreach (const Portion & p, stack_) {
        if (!p.current_status_.isEmpty ()) return p.current_status_;
    }
    return QString ();
}
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
void Progress::signalChange (bool b_bypass_checks)
{
    PRGR_TRACE_ENTRY;
    for (;;) {
        if (!b_bypass_checks && (stack_.size () > cutoff_level_)) {
            V_PRGR_DEBUG (" drop signal (size %" PRIi64 " > cutoff %" PRIi64 ")",
                              stack_.size (), cutoff_level_);
            break;
        }

        if (stack_.isEmpty ()) break;

        const Portion & f = stack_.first ();
        int64_t in_parent = f.progress_;
        int64_t total_progress = 0;
        int i_level = 0;

        foreach (const Portion & p, stack_) {
            int64_t updated_value;
            total_progress = p.tot_size_;
            updated_value =
                    p.offset_in_parent_ +
                    (in_parent * p.size_in_parent_) /
                    total_progress;

            V_PRGR_DEBUG ("    at level %d progress is %" PRIi64 " out of %" PRIi64 "\n",
                              i_level, in_parent, total_progress);

            ++i_level;
            in_parent = updated_value;
        }

        V_PRGR_DEBUG ("  new progress is %" PRIi64 ", old one is %" PRIi64 "\n",
                          in_parent, prev_prog_);

        // compute the difference and see if is above the threshold
        if (!b_bypass_checks) {
            int64_t difference = in_parent - prev_prog_;
            if (difference < granularity_) {
                V_PRGR_DEBUG ("  dropping update because of granularity rule\n");
                break;
            }
        }
        prev_prog_ = in_parent;

        if (kb_simple_signal_ != NULL) {
            b_should_stop_ = b_should_stop_ &
                    kb_simple_signal_ (total_progress, in_parent);
        }

        if (kb_full_signal_ != NULL) {
            b_should_stop_ = b_should_stop_ &
                    kb_full_signal_ (
                        total_progress,
                        in_parent,
                        current_status_,
                        f.user_data_,
                        user_data_);
        }
        V_PRGR_DEBUG ("  b_should_stop_ = %s\n", b_should_stop_ ? "true" : "false");

        break;
    }

    PRGR_TRACE_EXIT;
}
/* ========================================================================= */

void Progress::anchorVtable () const {}
