#include "core/input.h"
#include "core/event.h"
#include "core/pe_memory.h"
#include "core/logger.h"

typedef struct keyboard_state {
    b8 keys[KEYS_MAX_KEYS];
} keyboard_state;

typedef struct mouse_state {
    i16 x;
    i16 y;
    b8 buttons[BUTTON_MAX_BUTTONS];
} mouse_state;

typedef struct input_state {
    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;
    mouse_state mouse_current;
    mouse_state mouse_previous;
} input_state;

// Internal input state
static input_state* state_ptr;

b8 input_system_initialize(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(input_state);
    if (state == 0) {
        return false;
    }
    pe_zero_memory(state, sizeof(input_state));
    state_ptr = state;

    PE_INFO("Input subsystem initialized.");
    return true;
}

void input_system_shutdown(void* state) {
    // TODO: Add shutdown routines when needed.
    state_ptr = 0;
}

void input_update(f64 delta_time) {
    if (!state_ptr) {
        return;
    }

    // Copy current states to previous states
    pe_copy_memory(&state_ptr->keyboard_previous, &state_ptr->keyboard_current, sizeof(keyboard_state));
    pe_copy_memory(&state_ptr->mouse_previous, &state_ptr->mouse_current, sizeof(mouse_state));
}

void input_process_key(keys key, b8 pressed) {
    // Only handle if the state actually changed
    if (state_ptr && state_ptr->keyboard_current.keys[key] != pressed) {
        // Update internal state
        state_ptr->keyboard_current.keys[key] = pressed;

        if (key == KEY_LALT) {
            PE_INFO("Left alt pressed.");
        } else if (key == KEY_RALT) {
            PE_INFO("Right alt pressed.");
        }
        
        if (key == KEY_LCONTROL) {
            PE_INFO("Left control pressed.");
        } else if (key == KEY_RCONTROL) {
            PE_INFO("Right control pressed.");
        }
        
        if (key == KEY_LSHIFT) {
            PE_INFO("Left shift pressed.");
        } else if (key == KEY_RSHIFT) {
            PE_INFO("Right shift pressed.");
        }

        // Fire off an event for immediate processing.
        event_context context;
        context.data.u16[0] = key;
        event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void input_process_button(buttons button, b8 pressed) {
    // If the state changed, fire an event
    if (state_ptr->mouse_current.buttons[button] != pressed) {
        state_ptr->mouse_current.buttons[button] = pressed;

        // Fire the event
        event_context context;
        context.data.u16[0] = button;
        event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void input_process_mouse_move(i16 x, i16 y) {
    // Only process if actually different
    if (state_ptr->mouse_current.x != x || state_ptr->mouse_current.y != y) {

        // Update internal state
        state_ptr->mouse_current.x = x;
        state_ptr->mouse_current.y = y;

        // Fire the event
        event_context context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        event_fire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void input_process_mouse_wheel(i8 z_delta) {
    // Note: no internal state to update

    // Fire the event
    event_context context;
    context.data.u16[0] = z_delta;
    event_fire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 input_is_key_down(keys key) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->keyboard_current.keys[key] == true;
}

b8 input_is_key_up(keys key) {
    if (!state_ptr) {
        return true;
    }
    return state_ptr->keyboard_current.keys[key] == false;
}

b8 input_was_key_down(keys key) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->keyboard_previous.keys[key] == true;
}

b8 input_was_key_up(keys key) {
    if (!state_ptr) {
        return true;
    }
    return state_ptr->keyboard_previous.keys[key] == false;
}

// Mouse input
b8 input_is_button_down(buttons button) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->mouse_current.buttons[button] == true;
}

b8 input_is_button_up(buttons button) {
    if (!state_ptr) {
        return true;
    }
    return state_ptr->mouse_current.buttons[button] == false;
}

b8 input_was_button_down(buttons button) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->mouse_previous.buttons[button] == true;
}

b8 input_was_button_up(buttons button) {
    if (!state_ptr) {
        return true;
    }
    return state_ptr->mouse_previous.buttons[button] == false;
}

void input_get_mouse_position(i32* x, i32* y) {
    if (!state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state_ptr->mouse_current.x;
    *y = state_ptr->mouse_current.y;
}

void input_get_previous_mouse_position(i32* x, i32* y) {
    if (!state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state_ptr->mouse_previous.x;
    *y = state_ptr->mouse_previous.y;
}