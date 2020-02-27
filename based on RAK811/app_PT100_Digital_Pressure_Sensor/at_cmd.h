#ifndef _AT_CMD_H_
#define _AT_CMD_H_

#include "board.h"
#include "app.h"
#include "LoRaMac.h"

#include "rw_lora.h"

#define MAX_ARGV        20

/** Structure for registering CLI commands */
struct cli_cmd {
    /** The name of the CLI command */
    const char *name;
    /** The help text associated with the command */
    //const char *help;
    /** The function that should be invoked for this command. */
    void (*function) (int argc, char *argv[]);
};

static void lora_version(int argc, char *argv[]);
static void lora_join (int argc, char *argv[]);
static void lora_read_config(int argc, char *argv[]);
static void lora_write_config(int argc, char *argv[]);
static void lora_send(int argc, char *argv[]);
static void lora_region(int argc, char *argv[]);
static void atcmd_help(int argc, char *argv[]);


#endif