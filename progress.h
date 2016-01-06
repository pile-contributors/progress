/**
 * @file progress.h
 * @brief Declarations for Progress class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_PROGRESS_H_INCLUDE
#define GUARD_PROGRESS_H_INCLUDE

#include <progress/progress-config.h>
#include <QList>
#include <QString>
#include <stdint.h>

//! Report progress.
class PROGRESS_EXPORT Progress {
    //
    //
    //
    //
    /*  DEFINITIONS    ----------------------------------------------------- */

    //! Represents a level in our list of levels.
    struct Portion {
        // cppcheck-suppress unusedStructMember
        int64_t offset_in_parent_;
        // cppcheck-suppress unusedStructMember
        int64_t size_in_parent_;

        // cppcheck-suppress unusedStructMember
        int64_t tot_size_;
        // cppcheck-suppress unusedStructMember
        int64_t progress_;

        // cppcheck-suppress unusedStructMember
        void * user_data_;

        QString current_status_;
    };

    //! Callback used for signaling progress.
    typedef bool (*KbSignal) (
            int64_t total_size,
            int64_t progress,
            const QString & status,
            void * level_data,
            void * global_data);

    //! Simple callback used for signaling progress.
    typedef bool (*KbSignalSimple) (
            int64_t total_size,
            int64_t progress);

public:

    /*  DEFINITIONS    ===================================================== */
    //
    //
    //
    //
    /*  DATA    ------------------------------------------------------------ */

private:

    QList<Portion> stack_; /**< the list of nested portions */

    int cutoff_level_; /**< only emit signals if the size of the
                       stack is smaller than this value */
    int64_t granularity_; /**< advance by at least this much to generate signals */

    int64_t prev_prog_; /**< value last computed by signalChange () */

    bool b_should_stop_;
    QString current_status_;
    void * user_data_;

    KbSignalSimple kb_simple_signal_;
    KbSignal kb_full_signal_;

    /*  DATA    ============================================================ */
    //
    //
    //
    //
    /*  FUNCTIONS    ------------------------------------------------------- */


public:

    //! Constructor; creates an empty progress object.
    explicit Progress ();

    //! Destructor; releases all resources.
    virtual
    ~Progress ();

    //! assignment operator
    Progress& operator=( const Progress& other) {
        stack_ = other.stack_;
        cutoff_level_ = other.cutoff_level_;
        granularity_ = other.granularity_;
        prev_prog_ = other.prev_prog_;
        b_should_stop_ = other.b_should_stop_;
        current_status_ = other.current_status_;
        user_data_ = other.user_data_;
        kb_simple_signal_ = other.kb_simple_signal_;
        kb_full_signal_ = other.kb_full_signal_;
        return *this;
    }


    //! Prepares the progress for a run.
    bool
    init (
            const QString & title = QString (),
            int64_t total_size = 100);

    //! Terminate a run (clear internal states).
    void
    end ();

    //! Tell if the instance was initialized (init() was called).
    inline bool
    isInitialized () const {
        return !stack_.isEmpty ();
    }


    //! Enters a new portion
    void
    enter (
            int64_t parent_size,
            const QString &label = QString (),
            int64_t total_size = 100,
            int64_t parent_offset = -1,
            void * portion_data = NULL);


    //! Ends current portion; calls end() if this is the last one.
    void
    finish (
            bool update_parent = true);


    //! Tell the label for current operation.
    inline const QString &
    currentStatus () const {
        return current_status_;
    }


    //! Size of the active portion of the stack.
    inline int
    cutoffLevel () const {
        return cutoff_level_;
    }

    //! Size of the active portion of the stack.
    inline void
    setCutoffLevel (
            int value) {
        cutoff_level_ = value;
    }


    //! Emit signals when progress advances by at least this much.
    inline int64_t
    granularity () const {
        return granularity_;
    }

    //! Emit signals when progress advances by at least this much.
    inline void
    setGranularity (int64_t value) {
        granularity_ = value;
    }


    //! User data associated with the instance.
    inline void *
    userData () const {
        return user_data_;
    }

    //! User data associated with the instance.
    inline void
    setUserData (void * value) {
        user_data_ = value;
    }


    //! Simple callback to be used when progress changes.
    inline KbSignalSimple
    simpleCallback () const {
        return kb_simple_signal_;
    }

    //! Simple callback to be used when progress changes.
    inline void
    setSimpleCallback (KbSignalSimple value) {
        kb_simple_signal_ = value;
    }


    //! Callback to be used when progress changes.
    inline KbSignal
    callback () const {
        return kb_full_signal_;
    }

    //! Callback to be used when progress changes.
    inline void
    setCallback (KbSignal value) {
        kb_full_signal_ = value;
    }


    //! Perform a step in the context of top portion.
    bool
    step (
            int64_t chunk_size = 1,
            int64_t offset = -1);

    //! Sets the internal state to signal the process should terminate.
    inline void
    setStop () {
        b_should_stop_ = true;
    }

    //! Resets the internal state to signal the process should terminate.
    inline void
    resetStop () {
        b_should_stop_ = false;
    }

    //! Tell if the process / operation should stop.
    inline bool
    shouldStop () const {
        return b_should_stop_;
    }

    //! Force emmit a signal bypassing all checks (granularity, stack).
    inline bool
    emitSigal ();

    //! Set characteristics for current level.
    inline void
    setLevelCharact (
            int64_t total_size,
            int64_t progress = 0);


private:

    //! Starts from the bottom of the list and searches for a label.
    QString
    searchCurrentLabel ();

    //! Signals a change in the progress.
    void
    signalChange (
            bool b_bypass_checks = false);


public: virtual void anchorVtable() const;
}; // class Progress


#endif // GUARD_PROGRESS_H_INCLUDE
