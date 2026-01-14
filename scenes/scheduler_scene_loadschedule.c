#include "src/scheduler_app_i.h"

#include <dialogs/dialogs.h>

#define TAG "SubGHzSchedulerLoadSchedule"

void scheduler_scene_loadschedule_on_enter(void* context) {
    furi_assert(context);
    SchedulerApp* app = context;

    DialogMessage* msg = dialog_message_alloc();
    dialog_message_set_header(msg, "Load Schedule", 64, 8, AlignCenter, AlignTop);
    dialog_message_set_text(
        msg,
        "Not implemented yet.\nWill load options from a .txt file.",
        64,
        28,
        AlignCenter,
        AlignTop);
    dialog_message_set_buttons(msg, NULL, "OK", NULL);

    dialog_message_show(app->dialogs, msg);
    dialog_message_free(msg);

    scene_manager_previous_scene(app->scene_manager);
}

bool scheduler_scene_loadschedule_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void scheduler_scene_loadschedule_on_exit(void* context) {
    UNUSED(context);
}
