#ifndef SAS_COMPILER_H
#define SAS_COMPILER_H

#ifdef __GNUC__
#define cc_packed __attribute__((packed))
#else
#error "Unsupported compiler."
#endif

#endif
