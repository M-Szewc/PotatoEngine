#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "game_types.h"

// Externally-defined function to create a game.
extern b8 create_game(game* out_game);


/**
 * The main entry point of the application.
 */
int main(void){
    // Request the game instance from the application
    game game_inst;
    if (!create_game(&game_inst)) {
        PE_FATAL("Could not create game!");
        return -1;
    }

    // Ensure the function pointers exist
    if(!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.on_resize) {
        PE_FATAL("The game's function pointers must be assigned!");
        return -2;
    }


    // Initialization
    if(!application_create(&game_inst)) {
        PE_INFO("Application failed to create!");
        return 1;
    }

    // Begin the game loop
    if(!application_run()) {
        PE_INFO("Application did not shutdown gracefully.");
        return 2;
    }

    return 0;
}
