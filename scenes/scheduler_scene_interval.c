#include "subghz_scheduler_icons.h"
#include "src/scheduler_app_i.h"
#include "src/subghz_scheduler.h"
#include "views/scheduler_interval_view.h"

#include <furi.h>
#include <gui/elements.h>
#include <stdio.h>

void scheduler_scene_interval_on_enter(void* context) {
    SchedulerApp* app = context;

    scheduler_interval_view_set_seconds(
        app->interval_view, scheduler_get_interval_seconds(app->scheduler));

    view_dispatcher_switch_to_view(app->view_dispatcher, SchedulerAppViewInterval);
}

bool scheduler_scene_interval_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void scheduler_scene_interval_on_exit(void* context) {
    SchedulerApp* app = context;

    uint32_t sec = scheduler_interval_view_get_seconds(app->interval_view);
    scheduler_set_interval_seconds(app->scheduler, sec);
}