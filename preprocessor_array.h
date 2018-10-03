#define CONCAT(X,Y)         X ## Y
#define EXPAND_CONCAT(X,Y)  CONCAT(X, Y)

#define ARGN(N, LIST)       EXPAND_CONCAT(ARG_, N) LIST
#define ARG_0(C0, ...)      C0
#define ARG_1(C0, C1, ...)  C1
#define ARG_2(C0, C1, C2, ...)      C2
#define ARG_3(C0, C1, C2, C3, ...)  C3
#define ARG_4(C0, C1, C2, C3, C4, ...)      C4
#define ARG_5(C0, C1, C2, C3, C4, C5, ...)  C5
#define ARG_6(C0, C1, C2, C3, C4, C5, C6, ...)      C6
#define ARG_7(C0, C1, C2, C3, C4, C5, C6, C7, ...)  C7
#define ARG_8(C0, C1, C2, C3, C4, C5, C6, C7, C8, ...)      C8
#define ARG_9(C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, ...)  C9
#define ARG_10(C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, ...)    C10