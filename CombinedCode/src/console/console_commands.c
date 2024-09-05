/**
 * @file console_commands.c
 * @author Ricardo Jorge Dias Sampaio
 * @brief Implementation of console commands for the Secure VLC Project.
 * @version 1.0
 * @date 2024-09-03
 *
 * \par License:
 *   \ref mit_license "MIT License".
 * @details This file contains the implementation of various console commands used for
 * configuring and controlling the Secure VLC Project, including encryption settings,
 * data transmission, and system information.
 */

#include "console_commands.h"


/** @brief Tag for logging messages related to console operations */
static const char *CONSOLE_TAG = "CONSOLE";

/** @brief Flag to indicate if TX encryption variables are set */
bool tx_encryption_set = false;

/** @brief Flag to indicate if RX encryption variables are set */
bool rx_encryption_set = false;

/** @brief Flag to indicate if a new line is needed in the custom vprintf function */
static bool is_new_line = true;

/** @brief Structure for setting encryption arguments */
static struct set_encryption_args_t {
    struct arg_lit *TX;
    struct arg_lit *RX;
    struct arg_str *map_type;
    struct arg_dbl *x1;
    struct arg_dbl *y1;
    struct arg_int *iterations1;
    struct arg_dbl *x2;
    struct arg_dbl *y2;
    struct arg_int *iterations2;
    struct arg_end *end;
} set_encryption_args;

/** @brief Structure for getting encryption arguments */
static struct get_encryption_args_t{
    struct arg_lit *TX;
    struct arg_lit *RX;
    struct arg_end *end;
} get_encryption_args;

/**
 * @brief Custom printf function for the console.
 *
 * This function formats and prints messages to the console, handling new lines and prompts.
 *
 * @param format Format string.
 * @param args Variable argument list.
 * @return int Number of characters printed.
 */
static int custom_vprintf(const char *format, va_list args) {
    char print_buf[MAX_CMDLINE_LENGTH];
    int ret = vsnprintf(print_buf, sizeof(print_buf), format, args);
    if (ret >= 0) {
        char *line = print_buf;
        char *next_line;
        while ((next_line = strchr(line, '\n')) != NULL) {
            *next_line = '\0';
            if (!is_new_line) {
                printf("\n");
            }
            printf("%s\n", line);
            line = next_line + 1;
            is_new_line = true;
        }
        if (*line != '\0') {
            if (is_new_line) {
                printf("%s", PROMPT_STR);
            }
            printf("%s", line);
            is_new_line = false;
        }
    }
    return ret;
}

/**
 * @brief Initializes the NVS (Non-Volatile Storage).
 */
static void initialize_nvs(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

/**
 * @brief Registers a console command.
 *
 * @param command The command string.
 * @param short_command The short command string.
 * @param help The help string.
 * @param hint The hint string.
 * @param func The function to execute the command.
 * @param argtable The argument table.
 */
static void register_command(const char *command, const char *short_command, const char *help, const char *hint, esp_console_cmd_func_t func, void *argtable) {
    const esp_console_cmd_t cmd = {
        .command = command,
        .help = help,
        .hint = hint,
        .func = func,
        .argtable = argtable,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    if (short_command != NULL) {
        const esp_console_cmd_t short_cmd = {
            .command = short_command,
            .help = NULL, // Hide short command from help
            .hint = hint,
            .func = func,
            .argtable = argtable,
        };
        ESP_ERROR_CHECK(esp_console_cmd_register(&short_cmd));
    }
}

/**
 * @brief Check if encryption settings are set.
 *
 * @param check_tx Check TX encryption.
 * @param check_rx Check RX encryption.
 * @return bool True if the required encryption settings are set, false otherwise.
 */
static bool check_encryption_settings(bool check_tx, bool check_rx) {
    if (tx_encryption_set && rx_encryption_set) {
        ESP_LOGI(CONSOLE_TAG, "Both TX and RX encryption values are set.");
        return true;
    }
    if (check_tx && check_rx) {
        if (!tx_encryption_set && !rx_encryption_set) {
            ESP_LOGW(CONSOLE_TAG, "Both TX and RX encryption values are not set.");
        } else if (!tx_encryption_set) {
            ESP_LOGW(CONSOLE_TAG, "TX encryption values are not set.");
        } else {
            ESP_LOGW(CONSOLE_TAG, "RX encryption values are not set.");
        }
        return false;
    }
    if (check_tx && !tx_encryption_set) {
        ESP_LOGW(CONSOLE_TAG, "TX encryption values are not set.");
        return false; // Return true if we're only checking TX
    }
    if (check_rx && !rx_encryption_set) {
        ESP_LOGW(CONSOLE_TAG, "RX encryption values are not set.");
        return false; // Return true if we're only checking RX
    }
    return true;
}

/**
 * @brief Checks if a double value is within the valid range for the given map type.
 *
 * @param value The double value to check.
 * @param name The name of the value (for logging).
 * @param map_type The type of map being used.
 * @return bool True if the value is within range, false otherwise.
 */
static bool check_double_range(double value, const char* name, map_type_t map_type) {
    double min_value, max_value;

    switch (map_type) {
        case MAP_DUFFING:
            min_value = -1.2;
            max_value = 1.2;
            break;
        case MAP_LOGISTIC:
            min_value = 0.0;
            max_value = 1.0;
            break;
        case MAP_2D_LOGISTIC:
            min_value = -1.0;
            max_value = 1.0;
            break;
        default:
            ESP_LOGE(CONSOLE_TAG, "Error: Unknown map type for range checking.");
            return false;
    }

    if (value < min_value || value > max_value) {
        ESP_LOGE(CONSOLE_TAG, "Error: %s value %.6f is out of range [%.6f, %.6f] for %s map. Please try again.", 
                name, value, min_value, max_value, 
                map_type == MAP_DUFFING ? "Duffing" : 
                map_type == MAP_LOGISTIC ? "Logistic" : "2D-MCCM");
        return false;
    }
    return true;
}

/**
 * @brief Checks if the number of iterations is within the valid range for the given map type.
 *
 * @param value The number of iterations to check.
 * @param name The name of the value (for logging).
 * @param map_type The type of map being used.
 * @return int The valid number of iterations.
 */
static int check_iterations(int value, const char* name, map_type_t map_type) {
    int min_iterations = 200;
    int max_iterations = 1000000;

    // You can add map-specific iteration limits here in the future
    switch (map_type) {
        case MAP_DUFFING:
        case MAP_LOGISTIC:
        case MAP_2D_LOGISTIC:
            // Currently using the same limits for all maps
            break;
        default:
            ESP_LOGE(CONSOLE_TAG, "Error: Unknown map type for iteration checking.");
            return min_iterations;
    }

    if (value < min_iterations) {
        ESP_LOGW(CONSOLE_TAG, "Warning: %s must be at least %d for %s map. Setting to %d.", 
                name, min_iterations, 
                map_type == MAP_DUFFING ? "Duffing" : 
                map_type == MAP_LOGISTIC ? "Logistic" : "2D-MCCM", 
                min_iterations);
        return min_iterations;
    }
    if (value > max_iterations) {
        ESP_LOGW(CONSOLE_TAG, "Warning: %s exceeds %d for %s map. Setting to %d.", 
                name, max_iterations, 
                map_type == MAP_DUFFING ? "Duffing" : 
                map_type == MAP_LOGISTIC ? "Logistic" : "2D-MCCM", 
                max_iterations);
        return max_iterations;
    }
    return value;
}

/**
 * @brief Command to set encryption variables.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int 0 on success, 1 on failure.
 */
static int cmd_set_encryption(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&set_encryption_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_encryption_args.end, argv[0]);
        return 1;
    }

    // Check for mutual exclusivity of TX and RX flags
    if (set_encryption_args.TX->count + set_encryption_args.RX->count != 1) {
        ESP_LOGE(CONSOLE_TAG, "Error: You must specify either -TX or -RX, but not both.");
        return 1;
    }

    // Check if map type is provided
    if (set_encryption_args.map_type->count == 0) {
        ESP_LOGE(CONSOLE_TAG, "Error: You must specify a map type (duffing, logistic, or mccm).");
        return 1;
    }

    map_type_t map_type;
    if ((strcmp(set_encryption_args.map_type->sval[0], "duffing") == 0) || (strcmp(set_encryption_args.map_type->sval[0], "d") == 0)) {
        map_type = MAP_DUFFING;
    } else if ((strcmp(set_encryption_args.map_type->sval[0], "logistic") == 0) || (strcmp(set_encryption_args.map_type->sval[0], "l") == 0)) {
        map_type = MAP_LOGISTIC;
    } else if ((strcmp(set_encryption_args.map_type->sval[0], "mccm") == 0) || (strcmp(set_encryption_args.map_type->sval[0], "m") == 0)) {
        map_type = MAP_2D_LOGISTIC;
    } else {
        ESP_LOGE(CONSOLE_TAG, "Error: Invalid map type. Must be duffing, logistic, or mccm.");
        return 1;
    }

    // Check all double values
    if (!check_double_range(set_encryption_args.x1->dval[0], "Map 1 x", map_type) ||
        !check_double_range(set_encryption_args.y1->dval[0], "Map 1 y", map_type) ||
        !check_double_range(set_encryption_args.x2->dval[0], "Map 2 x", map_type) ||
        !check_double_range(set_encryption_args.y2->dval[0], "Map 2 y", map_type)) {
        return 1; // Return if any double value is out of range
    }

    encryption_vars_t *vars_to_set;
    if (set_encryption_args.RX->count > 0) {
        ESP_LOGI(CONSOLE_TAG, "RX mode selected");
        vars_to_set = &RX_encryption_vars;
        rx_encryption_set = true;
    } else {
        ESP_LOGI(CONSOLE_TAG, "TX mode selected");
        vars_to_set = &TX_encryption_vars;
        tx_encryption_set = true;
    }

    // Log the input values
    ESP_LOGI(CONSOLE_TAG, "Input values:");
    ESP_LOGI(CONSOLE_TAG, "Map 1: x=%.6f, y=%.6f, iterations=%d", 
             set_encryption_args.x1->dval[0], set_encryption_args.y1->dval[0], set_encryption_args.iterations1->ival[0]);
    ESP_LOGI(CONSOLE_TAG, "Map 2: x=%.6f, y=%.6f, iterations=%d", 
             set_encryption_args.x2->dval[0], set_encryption_args.y2->dval[0], set_encryption_args.iterations2->ival[0]);

    // Set the encryption variables
    vars_to_set->type = map_type;
    vars_to_set->chaotic_map1.x = set_encryption_args.x1->dval[0];
    vars_to_set->chaotic_map1.y = set_encryption_args.y1->dval[0];
    vars_to_set->chaotic_map1.iterations = set_encryption_args.iterations1->ival[0];
    vars_to_set->chaotic_map2.x = set_encryption_args.x2->dval[0];
    vars_to_set->chaotic_map2.y = set_encryption_args.y2->dval[0];
    vars_to_set->chaotic_map2.iterations = set_encryption_args.iterations2->ival[0];

    // Now call check_iterations and check_double_range
    if (!check_double_range(vars_to_set->chaotic_map1.x, "Map 1 x", map_type) ||
        !check_double_range(vars_to_set->chaotic_map1.y, "Map 1 y", map_type) ||
        !check_double_range(vars_to_set->chaotic_map2.x, "Map 2 x", map_type) ||
        !check_double_range(vars_to_set->chaotic_map2.y, "Map 2 y", map_type)) {
        return 1; // Return if any double value is out of range
    }

    vars_to_set->chaotic_map1.iterations = check_iterations(vars_to_set->chaotic_map1.iterations, "Iterations Map 1", map_type);
    vars_to_set->chaotic_map2.iterations = check_iterations(vars_to_set->chaotic_map2.iterations, "Iterations Map 2", map_type);

    // Log the values after check_iterations and check_double_range
    ESP_LOGI(CONSOLE_TAG, "Values set after range checks:");
    ESP_LOGI(CONSOLE_TAG, "Map 1: x=%.6f, y=%.6f, iterations=%d", 
             vars_to_set->chaotic_map1.x, vars_to_set->chaotic_map1.y, vars_to_set->chaotic_map1.iterations);
    ESP_LOGI(CONSOLE_TAG, "Map 2: x=%.6f, y=%.6f, iterations=%d", 
             vars_to_set->chaotic_map2.x, vars_to_set->chaotic_map2.y, vars_to_set->chaotic_map2.iterations);

    // Allocate and initialize msws32_variables
    if (vars_to_set->msws32_variables != NULL) {
        free(vars_to_set->msws32_variables);
    }
    vars_to_set->msws32_variables = malloc(sizeof(msws32_var_t));
    if (vars_to_set->msws32_variables == NULL) {
        ESP_LOGE(CONSOLE_TAG, "Failed to allocate memory for MSWS32 variables");
        return 1;
    }
    memset(vars_to_set->msws32_variables, 0, sizeof(msws32_var_t));

    key_generator_setup(vars_to_set);

    return 0;
}
/**
 * @brief Command to get encryption variables.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int 0 on success, 1 on failure.
 */
static int cmd_get_encryption(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&get_encryption_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, get_encryption_args.end, argv[0]);
        return 1;
    }

    // Check for mutual exclusivity of TX and RX flags
    if (get_encryption_args.TX->count + get_encryption_args.RX->count != 1) {
        ESP_LOGE(CONSOLE_TAG, "Error: You must specify either -TX or -RX, but not both.");
        return 1;
    }

    bool is_tx = (get_encryption_args.TX->count > 0);
    if (!check_encryption_settings(is_tx, !is_tx)) {
        return 1; // Exit if requested encryption settings are not set
    }

    encryption_vars_t *vars;
    const char *mode;
    if (is_tx) {
        vars = &TX_encryption_vars;
        mode = "TX";
    } else {
        vars = &RX_encryption_vars;
        mode = "RX";
    }

    const char *map_name;
    switch (vars->type) {
        case MAP_DUFFING:
            map_name = "Duffing";
            break;
        case MAP_LOGISTIC:
            map_name = "Logistic";
            break;
        case MAP_2D_LOGISTIC:
            map_name = "2D-LOGISTIC";
            break;
        default:
            map_name = "Unknown";
    }

    ESP_LOGI(CONSOLE_TAG, "Current %s encryption variables:", mode);
    ESP_LOGI(CONSOLE_TAG, "Current Map: %s", map_name);
    ESP_LOGI(CONSOLE_TAG, "Map 1: x=%.6f, y=%.6f, iterations=%d", 
            vars->chaotic_map1.x, vars->chaotic_map1.y, vars->chaotic_map1.iterations);
    ESP_LOGI(CONSOLE_TAG, "Map 2: x=%.6f, y=%.6f, iterations=%d", 
            vars->chaotic_map2.x, vars->chaotic_map2.y, vars->chaotic_map2.iterations);
    ESP_LOGI(CONSOLE_TAG, "MSWS32: x=%llu, w=%llu, s=%llu", 
            vars->msws32_variables->x, vars->msws32_variables->w, vars->msws32_variables->s);
    return 0;
}


/**
 * @brief Registers the set encryption command.
 */
static void register_set_encryption_command(void) {
    set_encryption_args.TX = arg_litn("T", "TX", 0 , 1, "TX mode");
    set_encryption_args.RX = arg_litn("R", "RX", 0 , 1, "RX mode");
    set_encryption_args.map_type = arg_str1(NULL, NULL, "<map_type>", "Map type (duffing, logistic, or mccm)");
    set_encryption_args.x1 = arg_dbl1(NULL, NULL, "<x1>", "Map 1 x value");
    set_encryption_args.y1 = arg_dbl1(NULL, NULL, "<y1>", "Map 1 y value");
    set_encryption_args.iterations1 = arg_int1(NULL, NULL, "<iterations1>", "Map 1 number of iterations");
    set_encryption_args.x2 = arg_dbl1(NULL, NULL, "<x2>", "Map 2 x value");
    set_encryption_args.y2 = arg_dbl1(NULL, NULL, "<y2>", "Map 2 y value");
    set_encryption_args.iterations2 = arg_int1(NULL, NULL, "<iterations2>", "Map 2 number of iterations");
    set_encryption_args.end = arg_end(10);
    
    const char *syntax = "[-TX | -RX] <map_type> <x1> <y1> <iterations1> <x2> <y2> <iterations2>";
    const char *description = "Set encryption variables for specified map type in TX or RX mode";
    register_command("set_encryption", "se", description, syntax, &cmd_set_encryption, &set_encryption_args);
}

/**
 * @brief Registers the get encryption command.
 */
static void register_get_encryption_command(void) {
    get_encryption_args.TX = arg_litn("T", "TX", 0 , 1, "Get TX encryption variables");
    get_encryption_args.RX = arg_litn("R", "RX", 0 , 1, "Get RX encryption variables");
    get_encryption_args.end = arg_end(3);
    register_command("get_encryption", "ge", "Get current encryption variables for TX or RX", "[-TX | -RX]", &cmd_get_encryption, &get_encryption_args);
}

/**
 * @brief Processes data to be transmitted.
 *
 * @param data The data to be processed.
 */
static void process_data_to_transmit(const char *data) {
    ESP_LOGI(CONSOLE_TAG, "Processing data: %s", data);
    add_str_to_buffer(data);
}

/**
 * @brief Command to transmit data.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int 0 on success, 1 on failure.
 */
static int cmd_transmit(int argc, char **argv) {
    if (argc < 2) {
        ESP_LOGE(CONSOLE_TAG, "No data provided");
        return 1;
    }

    // Check if both TX and RX encryption are set
    if (!check_encryption_settings(true, true)) {
        ESP_LOGE(CONSOLE_TAG, "Cannot transmit: Both TX and RX encryption values must be set.");
        return 1;
    }

    // Combine all arguments into a single string
    char buffer[MAX_DATA_LENGTH] = {0};
    int buffer_index = 0;
    for (int i = 1; i < argc; i++) {
        int len = strlen(argv[i]);
        if (buffer_index + len < MAX_DATA_LENGTH - 1) {
            strncpy(buffer + buffer_index, argv[i], len);
            buffer_index += len;
            // Add space between arguments, except for the last one
            if (i < argc - 1 && buffer_index < MAX_DATA_LENGTH - 1) {
                buffer[buffer_index++] = ' ';
            }
        } else {
            ESP_LOGW(CONSOLE_TAG, "Buffer full, truncating data");
            break;
        }
    }
    buffer[buffer_index] = '\0';
    process_data_to_transmit(buffer);
    return 0;
}

/**
 * @brief Registers the transmit command.
 */
static void register_transmit_command(void) {
    register_command("transmit", "t", "Send data for transmission", " ", &cmd_transmit, NULL);
}

/**
 * @brief Command to clear the console.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int 0 on success.
 */
static int cmd_clear(int argc, char **argv) {
    // ANSI escape sequence to clear the screen
    printf("\033[H\033[J");
    return 0;
}

/**
 * @brief Registers the clear command.
 */
static void register_clear_command(void) {
    register_command("clear", "c", "Clear the console output", NULL, &cmd_clear, NULL);
}

/**
 * @brief Command to print the frequency of communication.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int 0 on success.
 */
static int cmd_frequency(int argc, char **argv) {
    double frequency = (double)TIMER_RESOLUTION_HZ / TX_PERIOD_MICROS;
    ESP_LOGI(CONSOLE_TAG, "Frequency of communication: %.2f Hz", frequency);
    return 0;
}

/**
 * @brief Registers the frequency command.
 */
static void register_frequency_command(void) {
    register_command("freq", "f", "Print the frequency of communication", NULL, &cmd_frequency, NULL);
}

/**
 * @brief Initializes the console for the Secure VLC Project.
 *
 * This function sets up the console, configures the REPL (Read-Eval-Print Loop),
 * and registers all available commands. It performs the following tasks:
 * 1. Creates and configures a new REPL instance.
 * 2. Sets up the UART for console communication (if configured).
 * 3. Registers all available commands, including:
 *    - Help command
 *    - Set encryption command
 *    - Get encryption command
 *    - Transmit command
 *    - Clear console command
 *    - Frequency command
 *
 * @return esp_console_repl_t* Pointer to the initialized REPL instance.
 *         Returns NULL if initialization fails.
 */
static esp_console_repl_t* initialize_console(void) {
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = PROMPT_STR;
    repl_config.max_cmdline_length = MAX_CMDLINE_LENGTH;

    #if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));
    #endif

    /* Register commands */
    esp_console_register_help_command();
    register_set_encryption_command();
    register_get_encryption_command();
    register_transmit_command();
    register_clear_command();
    register_frequency_command();

    return repl;
}

/**
 * @brief Task for initializing and managing the REPL console and logging.
 *
 * This task initializes NVS, sets up custom logging, initializes the console,
 * and starts the REPL.
 *
 * @param pvParameters Pointer to task parameters (not used in this case)
 */
void console_and_logging_task(void *pvParameters) {
    // Initialize NVS
    initialize_nvs();
    
    // Set custom logging function
    esp_log_set_vprintf(custom_vprintf);

    // Initialize and start REPL console
    esp_console_repl_t *repl = initialize_console();
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
    ESP_LOGI(CONSOLE_TAG, "Console initialized");
    
    // This task should not return, so add an infinite loop
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay to prevent tight loop
    }
}