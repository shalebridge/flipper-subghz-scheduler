#include "src/scheduler_app_i.h"

#include <gui/icon_i.h>
#include <gui/modules/widget.h>
#include <subghz_scheduler_icons.h>

#define TAG "SubGHzSchedulerAbout"

typedef enum {
    AboutPageMain = 0,
    AboutPageInfo = 1,
    AboutPageArt = 2,
    AboutPageCount
} AboutPage;

typedef enum {
    AboutEvtNext = 0xF100,
    AboutEvtPrev = 0xF101,
} AboutEvt;

#define TEXT_ROW (7 + GUI_TEXTBOX_HEIGHT)

static void about_button_cb(GuiButtonType btn, InputType type, void* context) {
    if(type != InputTypeShort) return;

    SchedulerApp* app = context;
    if(btn == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, AboutEvtNext);
    } else if(btn == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, AboutEvtPrev);
    }
}

static void about_build_page(SchedulerApp* app, AboutPage page) {
    widget_reset(app->about_widget);

    if(page == AboutPageMain) {
        widget_add_icon_element(app->about_widget, GUI_DISPLAY_WIDTH - 18, 2, &I_sub1_10px);

        widget_add_string_element(
            app->about_widget, 4, 3, AlignLeft, AlignTop, FontPrimary, SCHEDULER_APP_NAME);

        widget_add_string_element(
            app->about_widget, GUI_MARGIN, TEXT_ROW, AlignLeft, AlignTop, FontSecondary, "Author:");
        widget_add_string_element(
            app->about_widget,
            40,
            TEXT_ROW,
            AlignLeft,
            AlignTop,
            FontSecondary,
            SCHEDULER_APP_AUTHOR);
        widget_add_string_element(
            app->about_widget,
            40,
            TEXT_ROW + GUI_TEXTBOX_HEIGHT,
            AlignLeft,
            AlignTop,
            FontSecondary,
            "@shalebridge");

        widget_add_string_element(
            app->about_widget,
            GUI_MARGIN,
            TEXT_ROW + (GUI_TEXTBOX_HEIGHT * 2),
            AlignLeft,
            AlignTop,
            FontSecondary,
            "Version:");
        widget_add_string_element(
            app->about_widget,
            40,
            TEXT_ROW + (GUI_TEXTBOX_HEIGHT * 2),
            AlignLeft,
            AlignTop,
            FontSecondary,
            SCHEDULER_APP_VERSION);

        //widget_add_button_element(
        //    app->about_widget, GuiButtonTypeRight, "Next", about_button_cb, app);
    } else if(page == AboutPageInfo) {
        widget_add_string_element(
            app->about_widget,
            GUI_MARGIN,
            GUI_TEXTBOX_HEIGHT,
            AlignLeft,
            AlignTop,
            FontSecondary,
            "Originally designed for my cat");
        widget_add_string_element(
            app->about_widget,
            GUI_MARGIN,
            GUI_TEXTBOX_HEIGHT * 2,
            AlignLeft,
            AlignTop,
            FontSecondary,
            "Nutmeg, to periodically reset");
        widget_add_string_element(
            app->about_widget,
            GUI_MARGIN,
            GUI_TEXTBOX_HEIGHT * 3,
            AlignLeft,
            AlignTop,
            FontSecondary,
            "his heating pad in the winter.");

        //widget_add_button_element(
        //    app->about_widget, GuiButtonTypeLeft, "Prev", about_button_cb, app);
        //widget_add_button_element(
        //    app->about_widget, GuiButtonTypeRight, "Next", about_button_cb, app);
    } else {
        widget_add_string_element(
            app->about_widget, 12, GUI_TEXTBOX_HEIGHT, AlignLeft, AlignTop, FontPrimary, "Nutmeg");
        widget_add_string_element(
            app->about_widget,
            GUI_MARGIN,
            GUI_TEXTBOX_HEIGHT * 3,
            AlignLeft,
            AlignTop,
            FontSecondary,
            "2009-");
        widget_add_icon_element(app->about_widget, GUI_DISPLAY_WIDTH - 64, 4, &I_cat_64px);
        //widget_add_button_element(
        //    app->about_widget, GuiButtonTypeLeft, "Prev", about_button_cb, app);
    }
}

void scheduler_scene_about_on_enter(void* context) {
    furi_assert(context);
    SchedulerApp* app = context;

    uint32_t page = scene_manager_get_scene_state(app->scene_manager, SchedulerSceneAbout);
    if(page >= AboutPageCount) page = AboutPageMain;

    about_build_page(app, (AboutPage)page);
    view_dispatcher_switch_to_view(app->view_dispatcher, SchedulerAppViewAbout);
}

bool scheduler_scene_about_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    SchedulerApp* app = context;

    if(event.type == SceneManagerEventTypeCustom) {
        uint32_t page = scene_manager_get_scene_state(app->scene_manager, SchedulerSceneAbout);
        if(page >= AboutPageCount) page = AboutPageMain;

        if(event.event == AboutEvtNext) {
            if(page + 1 < AboutPageCount) page++;
        } else if(event.event == AboutEvtPrev) {
            if(page > 0) page--;
        } else {
            return false;
        }

        scene_manager_set_scene_state(app->scene_manager, SchedulerSceneAbout, page);
        about_build_page(app, (AboutPage)page);
        return true;
    }

    return false;
}

void scheduler_scene_about_on_exit(void* context) {
    furi_assert(context);
    SchedulerApp* app = context;

    widget_reset(app->about_widget);

    // Always return to page 1 on next entry
    scene_manager_set_scene_state(app->scene_manager, SchedulerSceneAbout, AboutPageMain);
}
