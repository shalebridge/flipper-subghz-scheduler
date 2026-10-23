#include "src/scheduler_app_i.h"

#include <types.h>
#include <string.h>

typedef enum {
    PlaylistActionAdd = 100,
    PlaylistActionWait,
    PlaylistActionRemoveLast,
    PlaylistActionClear,
    PlaylistActionSave,
    PlaylistActionManage,
} PlaylistAction;

static void playlist_menu_callback(void* context, uint32_t index) {
    SchedulerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void playlist_rebuild(SchedulerApp* app) {
    submenu_reset(app->menu);
    submenu_set_header(app->menu, "Build Playlist");

    char label[80];
    for(uint8_t i = 0; i < app->playlist_count; i++) {
        const char* path = furi_string_get_cstr(app->playlist_items[i]);
        if(!strncmp(path, "time:", 5)) {
            snprintf(label, sizeof(label), "%u. Wait %ss", (unsigned)(i + 1), path + 6);
        } else {
            const char* name = strrchr(path, '/');
            snprintf(label, sizeof(label), "%u. %s", (unsigned)(i + 1), name ? name + 1 : path);
        }
        submenu_add_item(app->menu, label, i, playlist_menu_callback, app);
    }

    submenu_add_item(app->menu, "Add signal...", PlaylistActionAdd, playlist_menu_callback, app);
    submenu_add_item(app->menu, "Add wait...", PlaylistActionWait, playlist_menu_callback, app);
    if(app->playlist_count) {
        submenu_add_item(
            app->menu, "Remove last", PlaylistActionRemoveLast, playlist_menu_callback, app);
        submenu_add_item(
            app->menu, "Clear list", PlaylistActionClear, playlist_menu_callback, app);
        submenu_add_item(
            app->menu, "Save playlist...", PlaylistActionSave, playlist_menu_callback, app);
    }
    submenu_add_item(
        app->menu, "Manage playlists...", PlaylistActionManage, playlist_menu_callback, app);
    submenu_set_selected_item(app->menu, PlaylistActionAdd);
    view_dispatcher_switch_to_view(app->view_dispatcher, SchedulerAppViewMenu);
}

void scheduler_scene_playlist_on_enter(void* context) {
    SchedulerApp* app = context;
    playlist_rebuild(app);
}

bool scheduler_scene_playlist_on_event(void* context, SceneManagerEvent event) {
    SchedulerApp* app = context;
    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case PlaylistActionAdd: {
        if(app->playlist_count == SCHEDULER_PLAYLIST_MAX_ITEMS) {
            dialog_message_show_storage_error(app->dialogs, "Playlist is full");
            return true;
        }

        DialogsFileBrowserOptions options;
        dialog_file_browser_set_basic_options(&options, ".sub", &I_sub1_10px);
        options.skip_assets = true;
        options.hide_ext = false;
        options.base_path = SUBGHZ_APP_FOLDER;

        FuriString* path = furi_string_alloc_set_str(SUBGHZ_APP_FOLDER);
        if(dialog_file_browser_show(app->dialogs, path, path, &options)) {
            app->playlist_items[app->playlist_count++] = path;
        } else {
            furi_string_free(path);
        }
        playlist_rebuild(app);
        return true;
    }
    case PlaylistActionRemoveLast:
        if(app->playlist_count) {
            furi_string_free(app->playlist_items[--app->playlist_count]);
            app->playlist_items[app->playlist_count] = NULL;
        }
        playlist_rebuild(app);
        return true;
    case PlaylistActionWait:
        if(app->playlist_count == SCHEDULER_PLAYLIST_MAX_ITEMS) {
            dialog_message_show_storage_error(app->dialogs, "Playlist is full");
        } else {
            scene_manager_next_scene(app->scene_manager, SchedulerScenePlaylistWait);
        }
        return true;
    case PlaylistActionClear:
        while(app->playlist_count) {
            furi_string_free(app->playlist_items[--app->playlist_count]);
            app->playlist_items[app->playlist_count] = NULL;
        }
        playlist_rebuild(app);
        return true;
    case PlaylistActionSave:
        if(app->playlist_count) {
            const char* last = furi_string_get_cstr(app->playlist_items[app->playlist_count - 1]);
            if(!strncmp(last, "time:", 5)) {
                dialog_message_show_storage_error(app->dialogs, "Add a signal after wait");
                return true;
            }
            bool has_signal = false;
            for(uint8_t i = 0; i < app->playlist_count; i++)
                has_signal |= strncmp(furi_string_get_cstr(app->playlist_items[i]), "time:", 5) !=
                              0;
            if(!has_signal) {
                dialog_message_show_storage_error(app->dialogs, "Add a signal first");
                return true;
            }
            scene_manager_next_scene(app->scene_manager, SchedulerScenePlaylistName);
        }
        return true;
    case PlaylistActionManage:
        scene_manager_next_scene(app->scene_manager, SchedulerScenePlaylistPick);
        return true;
    default:
        return event.event < SCHEDULER_PLAYLIST_MAX_ITEMS;
    }
}

void scheduler_scene_playlist_on_exit(void* context) {
    SchedulerApp* app = context;
    submenu_reset(app->menu);
}
