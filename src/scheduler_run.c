#include "scheduler_run.h"

#include <lib/subghz/devices/devices.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/protocols/raw.h>
#include <applications/drivers/subghz/cc1101_ext/cc1101_ext_interconnect.h>

#include <devices/cc1101_int/cc1101_int_interconnect.h>
#include <devices/devices.h>

#include <subghz_protocol_registry.h>

#define TAG "SubGhzSchedulerRun"

struct ScheduleTxRun {
    Storage* storage;
    FlipperFormat* fff_head;
    FlipperFormat* fff_file;
    FlipperFormat* fff_data;
    SubGhzEnvironment* environment;
    FuriString* protocol;
    FuriString* preset;
    FuriString* data;
    bool filetype;
    uint32_t frequency;
};

static ScheduleTxRun* tx_run_alloc() {
    ScheduleTxRun* tx_run = malloc(sizeof(ScheduleTxRun));
    tx_run->storage = furi_record_open(RECORD_STORAGE);
    tx_run->fff_head = flipper_format_file_alloc(tx_run->storage);
    tx_run->fff_file = flipper_format_file_alloc(tx_run->storage);
    tx_run->fff_data = flipper_format_string_alloc();
    tx_run->environment = subghz_environment_alloc();
    tx_run->protocol = furi_string_alloc();
    tx_run->preset = furi_string_alloc();
    tx_run->data = furi_string_alloc();
    return tx_run;
}

static void tx_run_free(ScheduleTxRun* tx_run) {
    flipper_format_free(tx_run->fff_file);
    flipper_format_file_close(tx_run->fff_head);
    flipper_format_free(tx_run->fff_head);
    flipper_format_free(tx_run->fff_data);
    furi_string_free(tx_run->preset);
    furi_string_free(tx_run->protocol);
    furi_string_free(tx_run->data);
    subghz_environment_free(tx_run->environment);
    furi_record_close(RECORD_STORAGE);
    free(tx_run);
}

static FuriHalSubGhzPreset scheduler_get_subghz_preset_name(const char* preset_name) {
    FuriHalSubGhzPreset preset = FuriHalSubGhzPresetIDLE;
    if(!strcmp(preset_name, "FuriHalSubGhzPresetOok270Async")) {
        preset = FuriHalSubGhzPresetOok270Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPresetOok650Async")) {
        preset = FuriHalSubGhzPresetOok650Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPreset2FSKDev238Async")) {
        preset = FuriHalSubGhzPreset2FSKDev238Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPreset2FSKDev476Async")) {
        preset = FuriHalSubGhzPreset2FSKDev476Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPresetCustom")) {
        preset = FuriHalSubGhzPresetCustom;
    } else {
        FURI_LOG_E(TAG, "Unknown or incompatible preset");
    }
    return preset;
}

static void
    transmit(SchedulerApp* app, const SubGhzDevice* device, SubGhzTransmitter* transmitter) {
    notification_message(app->notifications, &sequence_blink_stop);
    notification_message(app->notifications, &sequence_blink_start_cyan);
    uint8_t repeats = scheduler_get_tx_repeats(app->scheduler);
    for(uint_fast8_t i = 0; i <= repeats; ++i) {
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "Scheduled Tx %d of %d", i + 1, repeats + 1);
#endif
        if(subghz_devices_start_async_tx(device, subghz_transmitter_yield, transmitter)) {
            while(!subghz_devices_is_async_complete_tx(device)) {
                furi_delay_ms(100);
            }
            subghz_devices_stop_async_tx(device);
        }
    }
    notification_message(app->notifications, &sequence_blink_stop);
}

static int32_t scheduler_tx(void* context) {
    furi_assert(context);
    SchedulerApp* app = context;
    ScheduleTxRun* tx_run = tx_run_alloc();

    const SubGhzDevice* device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);
    subghz_devices_begin(device);
    subghz_devices_reset(device);

    tx_run->filetype = scheduler_get_file_type(app->scheduler);
    if(tx_run->filetype == SchedulerFileTypePlaylist) {
        flipper_format_file_open_existing(tx_run->fff_head, furi_string_get_cstr(app->file_path));
        flipper_format_read_string(tx_run->fff_head, "sub", tx_run->data);
    } else {
        furi_string_set_str(tx_run->data, furi_string_get_cstr(app->file_path));
    }

    do {
        if(!flipper_format_file_open_existing(
               tx_run->fff_file, furi_string_get_cstr(tx_run->data))) {
            FURI_LOG_E(TAG, "Error loading file!");
            return -1;
        }

        flipper_format_read_uint32(tx_run->fff_file, "Frequency", &tx_run->frequency, 1);
        flipper_format_read_string(tx_run->fff_file, "Preset", tx_run->preset);
        flipper_format_read_string(tx_run->fff_file, "Protocol", tx_run->protocol);

        if(furi_string_equal(tx_run->protocol, "RAW")) {
            subghz_protocol_raw_gen_fff_data(
                tx_run->fff_data,
                furi_string_get_cstr(tx_run->data),
                subghz_devices_get_name(device));
        } else {
            stream_copy_full(
                flipper_format_get_raw_stream(tx_run->fff_file),
                flipper_format_get_raw_stream(tx_run->fff_data));
        }
        flipper_format_file_close(tx_run->fff_file);

        subghz_environment_set_protocol_registry(
            tx_run->environment, (void*)&subghz_protocol_registry);
        SubGhzTransmitter* transmitter = subghz_transmitter_alloc_init(
            tx_run->environment, furi_string_get_cstr(tx_run->protocol));

        subghz_transmitter_deserialize(transmitter, tx_run->fff_data);

        FuriHalSubGhzPreset preset_enum =
            scheduler_get_subghz_preset_name(furi_string_get_cstr(tx_run->preset));
        subghz_devices_load_preset(device, preset_enum, NULL);

        tx_run->frequency = subghz_devices_set_frequency(device, tx_run->frequency);

        transmit(app, device, transmitter);

        subghz_transmitter_free(transmitter);
        if(tx_run->filetype == SchedulerFileTypePlaylist) {
            furi_delay_ms(500);
        }
    } while(tx_run->filetype == SchedulerFileTypePlaylist &&
            flipper_format_read_string(tx_run->fff_head, "sub", tx_run->data));

    subghz_devices_sleep(device);
    subghz_devices_end(device);
    tx_run_free(tx_run);

    return 0;
}

static void
    scheduler_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    SchedulerApp* app = context;
    furi_assert(app->thread == thread);

    if(state == FuriThreadStateStopped) {
        FURI_LOG_I(TAG, "Thread stopped");
        furi_thread_free(thread);
        app->thread = NULL;
        app->is_transmitting = false;
        scheduler_reset_previous_time(app->scheduler);
    } else if(state == FuriThreadStateStopping) {
        //FURI_LOG_I(TAG, "Thread stopping");
    } else if(state == FuriThreadStateStarting) {
        FURI_LOG_I(TAG, "Thread starting");
        app->is_transmitting = true;
    } else if(state == FuriThreadStateRunning) {
        //FURI_LOG_I(TAG, "Thread running");
    }
}

void scheduler_start_tx(SchedulerApp* app) {
    app->thread = furi_thread_alloc_ex("SchedulerTxThread", 1024, scheduler_tx, app);
    furi_thread_set_state_callback(app->thread, scheduler_thread_state_callback);
    furi_thread_set_state_context(app->thread, app);
    furi_thread_start(app->thread);
}
