#include "helpers/scheduler_hms.h"
#include "helpers/scheduler_settings_io.h"
#include "scheduler_scene_loadfile.h"
#include "src/scheduler_app_i.h"

#define TAG "Sub-GHzSchedulerSceneStart"

typedef enum MenuIndices {
    MenuIndexInterval,
    MenuIndexTiming,
    MenuIndexTxCount,
    MenuIndexTxMode,
    MenuIndexTxDelay,
    MenuIndexTxRadio,
    MenuIndexSelectFile,
    MenuIndexSaveSchedule,
    MenuIndexStartSchedule
};

static uint8_t get_timing_idx(const Scheduler* s) {
    return scheduler_get_timing_mode((Scheduler*)s);
}
static void set_timing_idx(Scheduler* s, uint8_t idx) {
    scheduler_set_timing_mode(s, idx);
}

static uint8_t get_tx_count_idx(const Scheduler* s) {
    return scheduler_get_tx_count((Scheduler*)s);
}
static void set_tx_count_idx(Scheduler* s, uint8_t idx) {
    scheduler_set_tx_count(s, idx);
}

static uint8_t get_tx_mode_idx(const Scheduler* s) {
    return (uint8_t)scheduler_get_tx_mode((Scheduler*)s);
}
static void set_tx_mode_idx(Scheduler* s, uint8_t idx) {
    scheduler_set_tx_mode(s, (SchedulerTxMode)idx);
}

static uint8_t get_radio_idx(const Scheduler* s) {
    return scheduler_get_radio((Scheduler*)s);
}
static void set_radio_idx(Scheduler* s, uint8_t idx) {
    scheduler_set_radio(s, idx);
}

typedef uint8_t (*SchedulerGetIdxFn)(const Scheduler* scheduler);
typedef void (*SchedulerSetIdxFn)(Scheduler* scheduler, uint8_t idx);
static VariableItem* add_scheduler_option_item(
    VariableItemList* list,
    SchedulerApp* app,
    const char* label,
    uint8_t count,
    VariableItemChangeCallback on_change,
    SchedulerGetIdxFn get_idx,
    SchedulerSetIdxFn set_idx,
    const char* const* text_table) {
    furi_assert(app);

    VariableItem* item = variable_item_list_add(list, label, count, on_change, app);

    uint8_t idx = get_idx(app->scheduler);
    if(idx >= count) {
        idx = 0;
    }

    variable_item_set_current_value_index(item, idx);
    variable_item_set_current_value_text(item, text_table[idx]);

    set_idx(app->scheduler, idx);

    return item;
}

static void scheduler_scene_start_var_list_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    SchedulerApp* app = context;

    if(index == MenuIndexInterval) {
        scene_manager_next_scene(app->scene_manager, SchedulerSceneInterval);
    } else if(index == MenuIndexStartSchedule) {
        view_dispatcher_send_custom_event(app->view_dispatcher, SchedulerStartRunEvent);
    } else if(index == MenuIndexSelectFile) {
        view_dispatcher_send_custom_event(app->view_dispatcher, SchedulerStartEventSelectFile);
    } else if(index == MenuIndexSaveSchedule) {
        view_dispatcher_send_custom_event(app->view_dispatcher, SchedulerStartEventSaveSchedule);
    }
}

static void scheduler_scene_start_set_timing(VariableItem* item) {
    SchedulerApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, timing_mode_text[index]);
    scheduler_set_timing_mode(app->scheduler, index);
}

static void scheduler_scene_start_set_tx_count(VariableItem* item) {
    SchedulerApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, tx_count_text[index]);
    scheduler_set_tx_count(app->scheduler, index);
}

static void scheduler_scene_start_set_mode(VariableItem* item) {
    SchedulerApp* app = variable_item_get_context(item);
    SchedulerTxMode index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, tx_mode_text[index]);
    scheduler_set_tx_mode(app->scheduler, index);
}

static void scheduler_scene_start_set_tx_delay(VariableItem* item) {
    SchedulerApp* app = variable_item_get_context(item);
    uint8_t idx = variable_item_get_current_value_index(item);

    uint16_t ms = (uint16_t)idx * TX_DELAY_STEP_MS;

    char buf[12];
    snprintf(buf, sizeof(buf), "%ums", (unsigned)ms);
    variable_item_set_current_value_text(item, buf);

    scheduler_set_tx_delay_ms(app->scheduler, ms);
}

static void scheduler_scene_start_set_radio(VariableItem* item) {
    SchedulerApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, radio_device_text[index]);
    scheduler_set_radio(app->scheduler, index);
}

void scheduler_scene_start_on_enter(void* context) {
    SchedulerApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;
    char buffer[20];

    scheduler_time_reset(app->scheduler);
    if(app->should_reset) {
        scheduler_full_reset(app->scheduler);
        furi_string_reset(app->tx_file_path);
        app->should_reset = false;
    }

    variable_item_list_set_enter_callback(
        var_item_list, scheduler_scene_start_var_list_enter_callback, app);

    VariableItem* interval_item = variable_item_list_add(var_item_list, "Interval:", 0, NULL, app);
    const uint32_t sec = scheduler_get_interval_seconds(app->scheduler);
    scheduler_seconds_to_hms_string(sec, buffer, sizeof(buffer));
    variable_item_set_current_value_text(interval_item, buffer);

    add_scheduler_option_item(
        var_item_list,
        app,
        "Timing:",
        TIMING_MODE_COUNT,
        scheduler_scene_start_set_timing,
        get_timing_idx,
        set_timing_idx,
        timing_mode_text);

    add_scheduler_option_item(
        var_item_list,
        app,
        "TX Count:",
        TX_COUNT,
        scheduler_scene_start_set_tx_count,
        get_tx_count_idx,
        set_tx_count_idx,
        tx_count_text);

    add_scheduler_option_item(
        var_item_list,
        app,
        "TX Mode:",
        SchedulerTxModeSettingsNum,
        scheduler_scene_start_set_mode,
        get_tx_mode_idx,
        set_tx_mode_idx,
        tx_mode_text);

    VariableItem* txd = variable_item_list_add(
        var_item_list, "TX Delay:", TX_DELAY_COUNT, scheduler_scene_start_set_tx_delay, app);
    const uint8_t idx = scheduler_get_tx_delay_index(app->scheduler);
    variable_item_set_current_value_index(txd, idx);
    snprintf(buffer, sizeof(buffer), "%ums", (unsigned)((uint16_t)idx * TX_DELAY_STEP_MS));
    variable_item_set_current_value_text(txd, buffer);

    const uint8_t radio_count = app->ext_radio_present ? RADIO_DEVICE_COUNT : 1;
    add_scheduler_option_item(
        var_item_list,
        app,
        "Radio:",
        radio_count,
        scheduler_scene_start_set_radio,
        get_radio_idx,
        set_radio_idx,
        radio_device_text);

    VariableItem* item = variable_item_list_add(var_item_list, "Select File", 0, NULL, app);
    if(!furi_string_empty(app->tx_file_path)) {
        if(check_file_extension(furi_string_get_cstr(app->tx_file_path))) {
            scene_manager_set_scene_state(
                app->scene_manager, SchedulerSceneStart, SchedulerStartRunEvent);
            if(scheduler_get_file_type(app->scheduler) == SchedulerFileTypeSingle) {
                variable_item_set_current_value_text(item, "[Single]");
            } else if(scheduler_get_file_type(app->scheduler) == SchedulerFileTypePlaylist) {
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "[Playlist of %d]",
                    scheduler_get_list_count(app->scheduler));
                variable_item_set_current_value_text(item, buffer);
            }
        }

        variable_item_list_add(var_item_list, "Save Schedule...", 0, NULL, app);

        variable_item_list_add(var_item_list, "Start", 0, NULL, app);
    }

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, SchedulerSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, SchedulerAppViewVarItemList);
}

bool scheduler_scene_start_on_event(void* context, SceneManagerEvent event) {
    SchedulerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SchedulerStartRunEvent) {
            if(furi_string_empty(app->tx_file_path)) {
                dialog_message_show_storage_error(
                    app->dialogs, "Please select\nplaylist (*.txt) or\n *.sub file!");
            } else {
                scene_manager_next_scene(app->scene_manager, SchedulerSceneRunSchedule);
            }
        } else if(event.event == SchedulerStartEventSelectFile) {
            scene_manager_next_scene(app->scene_manager, SchedulerSceneLoadFile);
        } else if(event.event == SchedulerStartEventSaveSchedule) {
            scene_manager_next_scene(app->scene_manager, SchedulerSceneSaveName);
        }
        consumed = true;
    }
    return consumed;
}

void scheduler_scene_start_on_exit(void* context) {
    SchedulerApp* app = context;
    variable_item_list_reset(app->var_item_list);
}
