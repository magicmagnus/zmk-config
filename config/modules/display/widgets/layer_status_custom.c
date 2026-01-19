#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/display/widgets/layer_status.h> // for layer APIs
#include <zmk/events/layer_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include <zmk/layers.h>
#include <string.h>
#include <stdio.h>
#include <lvgl.h>

#define OS_LAYER 5

static sys_slist_t widgets_custom = SYS_SLIST_STATIC_INIT(&widgets_custom);

struct layer_status_state_custom {
    zmk_keymap_layer_index_t index;
    const char *label;
};

static void set_layer_symbol_custom(lv_obj_t *label, struct layer_status_state_custom state) {
    uint32_t active_layers = zmk_keymap_active_layers();
    bool linux_mode = active_layers & BIT(OS_LAYER);
    const char *os_label = linux_mode ? "LNX" : "WIN";

    if (state.label == NULL || strlen(state.label) == 0) {
        char text[20] = {};
        snprintf(text, sizeof(text), LV_SYMBOL_KEYBOARD " %i %s", state.index, os_label);
        lv_label_set_text(label, text);
    } else {
        char text[24] = {};
        snprintf(text, sizeof(text), LV_SYMBOL_KEYBOARD " %s %s", state.label, os_label);
        lv_label_set_text(label, text);
    }
}

static void layer_status_update_cb_custom(struct layer_status_state_custom state) {
    struct zmk_widget_layer_status_custom *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets_custom, widget, node) { set_layer_symbol_custom(widget->obj, state); }
}

static struct layer_status_state_custom layer_status_get_state_custom(const zmk_event_t *eh) {
    uint32_t active_layers = zmk_keymap_active_layers();
    active_layers &= ~BIT(OS_LAYER);

    if (active_layers == 0) {
        active_layers = BIT(0);
    }

    zmk_keymap_layer_index_t index =
        zmk_keymap_highest_layer_from(active_layers);

    return (struct layer_status_state_custom){
        .index = index,
        .label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index))
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status_custom, struct layer_status_state_custom,
                            layer_status_update_cb_custom, layer_status_get_state_custom)

ZMK_SUBSCRIPTION(widget_layer_status_custom, zmk_layer_state_changed);

int zmk_widget_layer_status_custom_init(struct zmk_widget_layer_status_custom *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    sys_slist_append(&widgets_custom, &widget->node);
    widget_layer_status_custom_init();
    return 0;
}

lv_obj_t *zmk_widget_layer_status_custom_obj(struct zmk_widget_layer_status_custom *widget) {
    return widget->obj;
}
