#ifdef _WIN32
#ifdef WIMP_EXPORTS

#define WIMP_API __declspec(dllexport)

#else

#define WIMP_API __declspec(dllimport)
#endif

#else

#define WIMP_API __attribute__((visibility("default")))

#endif