#pragma once

#include "scenes/scheduler_scene.h"
#include "scenes/scheduler_scene_settings.h"
#include "scheduler_app.h"
#include "scheduler_custom_event.h"
#include "subghz_scheduler.h"
#include "views/scheduler_run_view.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/modules/variable_item_list.h>
#include <expansion/expansion.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>

#define SCHEDULER_APP_NAME    "Sub-GHz Scheduler"
#define SCHEDULER_APP_VERSION "2.3"
#define SCHEDULER_APP_AUTHOR  "Patrick Edwards"

#define GUI_DISPLAY_HEIGHT 64
#define GUI_DISPLAY_WIDTH  128
#define GUI_MARGIN         5
#define GUI_TEXT_GAP       10

#define GUI_TEXTBOX_HEIGHT 12
#define GUI_TABLE_ROW_A    13
#define GUI_TABLE_ROW_B    (GUI_TABLE_ROW_A + GUI_TEXTBOX_HEIGHT) - 1

struct SchedulerApp {
    Expansion* expansion;
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    DialogsApp* dialogs;
    VariableItemList* var_item_list;
    Submenu* menu;
    Widget* about_widget;
    FuriThread* thread;
    Scheduler* scheduler;
    FuriString* file_path;

    SchedulerRunView* run_view;

    volatile bool is_transmitting;
    bool ext_radio_present;
};

typedef enum {
    SchedulerAppViewMenu,
    SchedulerAppViewVarItemList,
    SchedulerAppViewRunSchedule,
    SchedulerAppViewAbout,
    SchedulerAppViewExitConfirm,
} SchedulerAppView;
