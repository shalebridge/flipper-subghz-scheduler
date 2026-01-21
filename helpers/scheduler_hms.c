#include "scheduler_hms.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>

//static bool parse_u32(const char* s, size_t len, uint32_t* out) {
//    if(!s || !out || len == 0) return false;
//    uint32_t v = 0;
//    for(size_t i = 0; i < len; i++) {
//        const char c = s[i];
//        if(!isdigit((unsigned char)c)) return false;
//        const uint32_t d = (uint32_t)(c - '0');
//        if(v > (UINT32_MAX - d) / 10u) return false;
//        v = v * 10u + d;
//    }
//    *out = v;
//    return true;
//}

void scheduler_seconds_to_hms_string(uint32_t seconds, char* out, size_t out_size) {
    uint32_t h = (seconds / 3600) % 100;
    uint32_t m = (seconds / 60) % 60;
    uint32_t s = seconds % 60;

    // "HH:MM:SS" -> 8 chars + NUL
    // Example: "02:05:09"
    snprintf(out, out_size, "%02lu:%02lu:%02lu", h, m, s);
}

void scheduler_seconds_to_hms(uint32_t seconds, SchedulerHMS* time) {
    time->h = (seconds / 3600) % 100;
    time->m = (seconds / 60) % 60;
    time->s = seconds % 60;
}

uint32_t scheduler_hms_to_seconds(SchedulerHMS* time) {
    return ((uint32_t)time->h * 3600) + ((uint32_t)time->m * 60) + (uint32_t)time->s;
}
/*
bool scheduler_hms_to_seconds(const char* hms, uint32_t* out_seconds) {
    if(!hms || !out_seconds) return false;

    // Find first colon
    const char* c1 = NULL;
    const char* c2 = NULL;
    for(const char* p = hms; *p; p++) {
        if(*p == ':' && !c1)
            c1 = p;
        else if(*p == ':' && c1 && !c2) {
            c2 = p;
            break;
        }
    }
    if(!c1 || !c2) return false;

    const size_t len_h = (size_t)(c1 - hms);
    const size_t len_m = (size_t)(c2 - (c1 + 1));
    const char* s_ptr = c2 + 1;
    size_t len_s = 0;
    while(s_ptr[len_s] != '\0')
        len_s++;

    // Require at least 1 digit hours, and exactly 2 digits for mm/ss (Clock-like)
    if(len_h < 1) return false;
    if(len_m != 2) return false;
    if(len_s != 2) return false;

    uint32_t hh = 0, mm = 0, ss = 0;
    if(!parse_u32(hms, len_h, &hh)) return false;
    if(!parse_u32(c1 + 1, len_m, &mm)) return false;
    if(!parse_u32(s_ptr, len_s, &ss)) return false;

    if(mm > 59u || ss > 59u) return false;

    // total = hh*3600 + mm*60 + ss with overflow checks
    uint32_t total = 0;

    if(hh > UINT32_MAX / 3600u) return false;
    total = hh * 3600u;

    if(mm > (UINT32_MAX - total) / 60u) return false;
    total += mm * 60u;

    if(ss > (UINT32_MAX - total)) return false;
    total += ss;

    *out_seconds = total;
    return true;
}
*/
