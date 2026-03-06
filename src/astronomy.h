#ifndef KEEPERFX_ASTRONOMY_H
#define KEEPERFX_ASTRONOMY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct astro_time_t {
    double ut;
    double tt;
} astro_time_t;

typedef struct astro_angle_result_t {
    int status;
    double angle;
} astro_angle_result_t;

#define ASTRO_SUCCESS (0)

astro_time_t Astronomy_CurrentTime(void);
astro_angle_result_t Astronomy_MoonPhase(astro_time_t time);

#ifdef __cplusplus
}
#endif

#endif
