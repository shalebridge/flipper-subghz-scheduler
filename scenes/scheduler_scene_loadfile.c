#include "src/scheduler_app_i.h"
#include "scheduler_scene_loadfile.h"

#include <furi_hal.h>
#include <types.h>
#include <dialogs/dialogs.h>
#include <subghz_scheduler_icons.h>
#include <lib/toolbox/stream/stream.h>
//#include <lib/subghz/protocols/raw.h>
#include <flipper_format/flipper_format_i.h>

#define TAG "SubGHzSchedulerLoadFile"

static int count_playlist_items(Storage* storage, const char* file_path) {
    FlipperFormat* format = flipper_format_file_alloc(storage);
    FuriString* data = furi_string_alloc();
    int count = 0;

    if(!flipper_format_file_open_existing(format, file_path)) {
        return FuriStatusError;
    }
    while(flipper_format_read_string(format, "sub", data)) {
        ++count;
    }

    flipper_format_file_close(format);
    flipper_format_free(format);
    furi_string_free(data);
    return count;
}

bool check_file_extension(const char* filename) {
    const char* extension = strrchr(filename, '.');
    if(extension == NULL) {
        return false;
    } else {
        return strcmp(extension, ".txt") == 0 || strcmp(extension, ".sub") == 0;
    }
}

static bool load_protocol_from_file(SchedulerApp* app) {
    furi_assert(app);
    FuriString* file_path = furi_string_alloc();
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, NULL, &I_sub1_10px);
    browser_options.base_path = SUBGHZ_APP_FOLDER;

    // Input events and views are managed by file_select
    bool res =
        dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options);

    const char* filestr = furi_string_get_cstr(app->file_path);
    if(res) {
        if(check_file_extension(filestr)) {
            Storage* storage = furi_record_open(RECORD_STORAGE);
            int8_t count = count_playlist_items(storage, filestr);

            scheduler_set_file_name(app->scheduler, filestr);
            if(count == 0) {
                scheduler_set_file_type(app->scheduler, SchedulerFileTypeSingle);
            } else {
                scheduler_set_file_type(app->scheduler, SchedulerFileTypePlaylist);
            }
        }
        furi_record_close(RECORD_STORAGE);
    }

    furi_string_free(file_path);
    return res;
}

void scheduler_scene_load_on_enter(void* context) {
    furi_assert(context);
    SchedulerApp* app = context;
    if(load_protocol_from_file(app)) {
        // Need checking
    }
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, SchedulerSceneStart);
}

void scheduler_scene_load_on_exit(void* context) {
    furi_assert(context);
    SchedulerApp* app = context;
    scene_manager_set_scene_state(app->scene_manager, SchedulerSceneStart, 0);
}

bool scheduler_scene_load_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    SchedulerApp* app = context;
    UNUSED(app);
    UNUSED(event);
    return false;
}
