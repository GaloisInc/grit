#ifndef FROMAGER_H
#define FROMAGER_H

#ifdef __cplusplus
extern "C" {
#endif

// Indicate that the current trace is invalid.
void __cc_flag_invalid(void);
// Indicate that the current trace has exhibited a bug.
void __cc_flag_bug(void);

// Print a message during evaluation in the MicroRAM interpreter.
void __cc_trace(const char* msg);

// Assert that the trace is valid only if `cond` is non-zero.  If the trace is
// invalid, `msg` will be printed when running in the MicroRAM interpreter.
inline void __cc_valid_if(int cond, const char* msg) {
    if (!cond) {
        __cc_flag_invalid();
#ifdef FROMAGER_DEBUG
        __cc_trace(msg);
#endif
    }
}

// Indicate that the program has exhibited a bug if `cond` is non-zero.  If the
// bug is present, `msg` will be printed when running in the MicroRAM
// interpreter.
inline void __cc_bug_if(int cond, const char* msg) {
    if (cond) {
        __cc_flag_bug();
#ifdef FROMAGER_DEBUG
        __cc_trace(msg);
#endif
    }
}

#ifdef __cplusplus
}
#endif

#endif // FROMAGER_H
