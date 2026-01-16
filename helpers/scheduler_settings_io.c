#include "scheduler_settings_io.h"

#include "src/scheduler_app_i.h"
#include "scheduler_custom_file_types.h"

#include <furi.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>

#define TAG "SchedulerSettingsIO"

// Settings file keys
#define KEY_FILETYPE      "FileType"
#define KEY_VERSION       "FileVersion"
#define KEY_INTERVAL_IDX  "IntervalIdx"
#define KEY_TIMING_MODE   "TimingModeIdx"
#define KEY_REPEATS_IDX   "RepeatsIdx"
#define KEY_MODE_IDX      "TxModeIdx"
#define KEY_TX_DELAY_IDX  "TxDelayIdx"
#define KEY_RADIO_IDX     "RadioIdx"
#define KEY_SCHEDULE_FILE "TxFile"
#define KEY_SCHEDULE_TYPE "TxFileType"
#define KEY_LIST_COUNT    "ListCount"

bool scheduler_settings_save_to_path(SchedulerApp* app, const char* full_path) {
    furi_assert(app);

    bool ok = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    do {
        // true if created or already exists
        if(!storage_common_mkdir(storage, SCHEDULER_APP_FOLDER)) {
            FURI_LOG_E(TAG, "Failed to create/open dir: %s", SCHEDULER_APP_FOLDER);
            dialog_message_show_storage_error(app->dialogs, "Failed to create/open directory");
            break;
        }

        if(!flipper_format_file_open_always(ff, full_path)) {
            FuriString* message =
                furi_string_alloc_set_str("Failed to open settings file for write: ");
            furi_string_cat_str(message, full_path);
            dialog_message_show_storage_error(app->dialogs, furi_string_get_cstr(message));
            FURI_LOG_E(TAG, furi_string_get_cstr(message));
            break;
        }

        // Header
        flipper_format_write_string_cstr(ff, KEY_FILETYPE, SCHEDULER_APP_SETTINGS_FILE_TYPE);
        uint32_t ver = SCHEDULER_APP_SETTINGS_FILE_VERSION;
        flipper_format_write_uint32(ff, KEY_VERSION, &ver, 1);

        // Options
        uint32_t v = 0;
        v = scheduler_get_interval(app->scheduler);
        flipper_format_write_uint32(ff, KEY_INTERVAL_IDX, &v, 1);

        v = scheduler_get_timing_mode(app->scheduler);
        flipper_format_write_uint32(ff, KEY_TIMING_MODE, &v, 1);

        v = scheduler_get_tx_repeats(app->scheduler);
        flipper_format_write_uint32(ff, KEY_REPEATS_IDX, &v, 1);

        v = scheduler_get_mode(app->scheduler);
        flipper_format_write_uint32(ff, KEY_MODE_IDX, &v, 1);

        v = scheduler_get_tx_delay_index(app->scheduler);
        flipper_format_write_uint32(ff, KEY_TX_DELAY_IDX, &v, 1);

        v = scheduler_get_radio(app->scheduler);
        flipper_format_write_uint32(ff, KEY_RADIO_IDX, &v, 1);

        flipper_format_write_string(ff, KEY_SCHEDULE_FILE, app->tx_file_path);

        v = scheduler_get_file_type(app->scheduler);
        flipper_format_write_uint32(ff, KEY_SCHEDULE_TYPE, &v, 1);

        v = scheduler_get_list_count(app->scheduler);
        flipper_format_write_uint32(ff, KEY_LIST_COUNT, &v, 1);

        ok = true;
    } while(false);

    flipper_format_file_close(ff);
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
    return ok;
}

bool scheduler_settings_load_from_path(SchedulerApp* app, const char* full_path) {
    furi_assert(app);
    furi_assert(full_path);

    bool ok = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    FuriString* filetype = furi_string_alloc();
    FuriString* sched_file = furi_string_alloc();

    do {
        if(!flipper_format_file_open_existing(ff, full_path)) {
            FURI_LOG_E(TAG, "Open failed: %s", full_path);
            break;
        }

        if(!flipper_format_read_string(ff, KEY_FILETYPE, filetype)) {
            FURI_LOG_E(TAG, "Missing Filetype");
            break;
        }
        if(strcmp(furi_string_get_cstr(filetype), SCHEDULER_APP_SETTINGS_FILE_TYPE) != 0) {
            FURI_LOG_E(TAG, "Wrong Filetype: %s", furi_string_get_cstr(filetype));
            break;
        }

        uint32_t ver;
        flipper_format_read_uint32(ff, KEY_VERSION, &ver, 1);
        if(ver != SCHEDULER_APP_SETTINGS_FILE_VERSION) {
            FURI_LOG_E(TAG, "Unsupported Version: %lu", (unsigned long)ver);
            break;
        }

        uint8_t interval_idx;
        uint8_t timing_mode;
        uint8_t repeats_idx;
        uint8_t mode_idx;
        uint8_t tx_delay_idx;
        uint8_t radio_idx;

        flipper_format_read_uint32(ff, KEY_INTERVAL_IDX, (uint32_t*)&interval_idx, 1);
        flipper_format_read_uint32(ff, KEY_TIMING_MODE, (uint32_t*)&timing_mode, 1);
        flipper_format_read_uint32(ff, KEY_REPEATS_IDX, (uint32_t*)&repeats_idx, 1);
        flipper_format_read_uint32(ff, KEY_MODE_IDX, (uint32_t*)&mode_idx, 1);
        flipper_format_read_uint32(ff, KEY_TX_DELAY_IDX, (uint32_t*)&tx_delay_idx, 1);
        flipper_format_read_uint32(ff, KEY_RADIO_IDX, (uint32_t*)&radio_idx, 1);

        scheduler_set_interval(app->scheduler, interval_idx);
        scheduler_set_timing_mode(app->scheduler, timing_mode);
        scheduler_set_tx_repeats(app->scheduler, repeats_idx);
        scheduler_set_mode(app->scheduler, (SchedulerTxMode)mode_idx);
        scheduler_set_tx_delay(app->scheduler, tx_delay_idx);
        scheduler_set_radio(app->scheduler, radio_idx);

        if(flipper_format_read_string(ff, KEY_SCHEDULE_FILE, sched_file)) {
            // Update app file_path WITHOUT losing the scheduler settings file path
            furi_string_set(app->tx_file_path, furi_string_get_cstr(sched_file));

            uint32_t file_type;
            flipper_format_read_uint32(ff, KEY_SCHEDULE_TYPE, &file_type, 1);
            uint32_t list_count;
            flipper_format_read_uint32(ff, KEY_LIST_COUNT, &list_count, 1);

            if((FileTxType)file_type == SchedulerFileTypeSingle) {
                scheduler_set_file(app->scheduler, furi_string_get_cstr(sched_file), 0);
            } else {
                // If list_count missing, fall back to "playlist" with count 1
                if(list_count == 0) list_count = 1;
                scheduler_set_file(
                    app->scheduler, furi_string_get_cstr(sched_file), (int8_t)list_count);
            }
        }

        ok = true;
    } while(false);

    flipper_format_file_close(ff);
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    furi_string_free(filetype);
    furi_string_free(sched_file);

    return ok;
}
