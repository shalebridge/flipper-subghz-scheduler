#pragma once

typedef enum {
    SchedulerStartEventSetInterval,
    SchedulerStartEventSetTiming,
    SchedulerStartEventSetRepeats,
    SchedulerStartEventSetImmediate,
    SchedulerStartEventSetTxDelay,
    SchedulerStartEventSetRadio,
    SchedulerStartEventSelectFile,
    SchedulerStartEventSaveSchedule,
    SchedulerStartRunEvent,
    SchedulerCustomEventErrorBack,
} SchedulerCustomEvent;
