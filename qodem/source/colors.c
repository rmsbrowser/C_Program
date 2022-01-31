/*
 * colors.c
 *
 * qodem - Qodem Terminal Emulator
 *
 * Written 2003-2017 by Kevin Lamonte
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to the
 * public domain worldwide. This software is distributed without any
 * warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include "qcurses.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "qodem.h"
#include "colors.h"

#ifdef __clang__
/*
 * Disable the UTF-8 encoding check for this file.  We use TheDraw to make
 * the info screen, which saves in raw 8-bit bytes.
 */
#pragma clang diagnostic ignored "-Winvalid-source-encoding"
#endif

/**
 * The color pair number that is white foreground black background.
 *
 * On the raw Linux console, this will be 0.  But most X11-based terminal
 * emulators support lots of colors, so I can define my own white-on-black
 * color pair number; this is handy to always really get white-on-black for
 * terminals that have a different default color.
 */
short q_white_color_pair_num = 0;

/**
 * The offset between normal and bolded colors.  screen.c needs to peek at
 * this, so it's not static.  By default set it such that A_BOLD will not be
 * sent directly to the screen, using instead colors that match the DOS CGA
 * colors (e.g. bold brown = yellow).
 */
short q_color_bold_offset = 64;

/*
 * convert_thedraw_screen() defines its colors in terms of the CGA bitmask.
 * This maps those bits to a curses color number.
 */
static short pc_to_curses_map[] = {
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,

    /*
     * This is really brown
     */
    COLOR_YELLOW,

    /*
     * Really light gray
     */
    COLOR_WHITE

    /*
     * The bold colors are:
     *
     * dark gray
     * light blue
     * light green
     * light cyan
     * light red
     * light magenta
     * yellow
     * white
     */
};

/**
 * Global colormap table.
 */
struct q_text_color_struct q_text_colors[Q_COLOR_MAX];

#define COLOR_CONFIG_FILENAME "colors.cfg"

/**
 * Get the full path to the colors.cfg file.
 *
 * @return the full path to colors.cfg (usually ~/qodem/colors.cfg or My
 * Documents\\qodem\\prefs\\colors.cfg).
 */
char * get_colors_filename() {
    return get_datadir_filename(COLOR_CONFIG_FILENAME);
}

/**
 * Convert a color string to a curses color number.
 *
 * @param original the color to return if new_color does not specify a valid
 * color string
 * @param new_color the color string.  The valid color names are "blue",
 * "red", etc., with "yellow" and "brown" being synonyms.
 * @return the color specified by new_color, or original if this was an
 * unknown string
 */
static short color_from_string(short original, char * new_color) {
    if (strncasecmp(new_color, "blue", strlen("blue")) == 0) {
        return COLOR_BLUE;
    }
    if (strncasecmp(new_color, "white", strlen("white")) == 0) {
        return COLOR_WHITE;
    }
    if (strncasecmp(new_color, "black", strlen("black")) == 0) {
        return COLOR_BLACK;
    }
    if (strncasecmp(new_color, "red", strlen("red")) == 0) {
        return COLOR_RED;
    }
    if (strncasecmp(new_color, "green", strlen("green")) == 0) {
        return COLOR_GREEN;
    }
    if (strncasecmp(new_color, "yellow", strlen("yellow")) == 0) {
        return COLOR_YELLOW;
    }
    if (strncasecmp(new_color, "cyan", strlen("cyan")) == 0) {
        return COLOR_CYAN;
    }
    if (strncasecmp(new_color, "magenta", strlen("magenta")) == 0) {
        return COLOR_MAGENTA;
    }

    /*
     * Synonyms
     */
    if (strncasecmp(new_color, "brown", strlen("brown")) == 0) {
        return COLOR_YELLOW;
    }
    if (strncasecmp(new_color, "grey", strlen("grey")) == 0) {
        return COLOR_WHITE;
    }
    if (strncasecmp(new_color, "gray", strlen("gray")) == 0) {
        return COLOR_WHITE;
    }

    return original;
}

/**
 * Create a color configuration file with reasonable defaults.
 */
static void create_color_config() {
    FILE * file;
    char * full_filename;

    if (q_status.read_only == Q_TRUE) {
        return;
    }

    file = open_datadir_file(COLOR_CONFIG_FILENAME, &full_filename, "w");
    if (file == NULL) {
        /*
         * Can't create the colors file
         */
        Xfree(full_filename, __FILE__, __LINE__);
        return;
    }

    fprintf(file, "# Qodem colors config file.\n");
    fprintf(file, "#\n");
    fprintf(file,
        "# Each line takes one of the forms below (second line indicates bold\n");
    fprintf(file, "# foreground):\n");
    fprintf(file, "#\n");
    fprintf(file, "#   <label> = <foreground>, <background>\n");
    fprintf(file, "#   <label> = <foreground>, <background>, bold\n");
    fprintf(file, "#\n");
    fprintf(file,
        "# Below are several sections that closely match the color themes supplied\n");
    fprintf(file, "# with Qmodem 4.6/5.0.\n");
    fprintf(file, "#\n");
    fprintf(file,
        "# Qodem remaps the curses colors to match DOS / VGA colors,\n");
    fprintf(file,
        "# i.e. \"dark yellow\" is actually brown, \"bright black\"\n");
    fprintf(file,
        "# is actually dark grey, etc.  For both the X11 build and most\n");
    fprintf(file,
        "# X terminals, this works smoothly with no user intervention\n");
    fprintf(file,
        "# required.  If for some reason XTerm does not display them\n");
    fprintf(file,
        "# the same, the following lines can be added to the ~/.Xresources\n");
    fprintf(file, "# file (remove the leading \"# \" first from each line):\n");
    fprintf(file, "#\n");
    fprintf(file, "# ! XTerm configuration to reproduce DOS colors.\n");
    fprintf(file,
            "# ! Credits to Emil Mikulic at http://dmr.ath.cx/notes/xterm.html\n");
    fprintf(file, "# xterm*foreground: rgb:a8/a8/a8\n");
    fprintf(file, "# xterm*background: rgb:00/00/00\n");
    fprintf(file, "# xterm*color0: rgb:00/00/00\n");
    fprintf(file, "# xterm*color1: rgb:a8/00/00\n");
    fprintf(file, "# xterm*color2: rgb:00/a8/00\n");
    fprintf(file, "# xterm*color3: rgb:a8/54/00\n");
    fprintf(file, "# xterm*color4: rgb:00/00/a8\n");
    fprintf(file, "# xterm*color5: rgb:a8/00/a8\n");
    fprintf(file, "# xterm*color6: rgb:00/a8/a8\n");
    fprintf(file, "# xterm*color7: rgb:a8/a8/a8\n");
    fprintf(file, "# xterm*color8: rgb:54/54/54\n");
    fprintf(file, "# xterm*color9: rgb:fc/54/54\n");
    fprintf(file, "# xterm*color10: rgb:54/fc/54\n");
    fprintf(file, "# xterm*color11: rgb:fc/fc/54\n");
    fprintf(file, "# xterm*color12: rgb:54/54/fc\n");
    fprintf(file, "# xterm*color13: rgb:fc/54/fc\n");
    fprintf(file, "# xterm*color14: rgb:54/fc/fc\n");
    fprintf(file, "# xterm*color15: rgb:fc/fc/fc\n");
    fprintf(file, "# xterm*boldMode: false\n");
    fprintf(file, "# xterm*colorBDMode: true\n");
    fprintf(file, "# xterm*colorBD: rgb:fc/fc/fc\n");
    fprintf(file, "#\n");
    fprintf(file, "#\n");
    fprintf(file,
        "# These colors match the Qmodem 5.0 \"Blue Shades\" theme, and are also the\n");
    fprintf(file,
        "# default qodem theme if the colors.cfg file does not exist.\n");
    fprintf(file, "\n");
    fprintf(file, "status_line.item_enabled  = blue, white\n");
    fprintf(file, "status_line.item_disabled = black, white\n");
    fprintf(file, "terminal.header_lines     = cyan, black, bold\n");
    fprintf(file, "terminal.normal           = white, black\n");
    fprintf(file, "terminal.background       = white, black\n");
    fprintf(file, "terminal.debug_sent_color = green, black, bold\n");
    fprintf(file, "window.border             = blue, black, bold\n");
    fprintf(file, "window.normal             = black, blue\n");
    fprintf(file, "menu_item.command         = yellow, blue, bold\n");
    fprintf(file, "menu_item.command_unavailable = black, blue, bold\n");
    fprintf(file, "menu_item.text            = white, blue\n");
    fprintf(file, "field_label               = white, blue\n");
    fprintf(file, "field_text                = yellow, black, bold\n");
    fprintf(file, "phonebook.entry           = white, blue\n");
    fprintf(file, "phonebook.entry_tagged    = yellow, blue, bold\n");
    fprintf(file, "phonebook.selected        = white, black\n");
    fprintf(file, "phonebook.selected_tagged = yellow, black, bold\n");
    fprintf(file, "phonebook.field_text      = black, white\n");
    fprintf(file, "script.label_running      = green, blue, bold\n");
    fprintf(file, "script.label_finished     = red, blue, bold\n");
    fprintf(file, "script.label_finished_ok  = yellow, blue, bold\n");
    fprintf(file, "help.border               = white, black, bold\n");
    fprintf(file, "help.background           = white, blue\n");
    fprintf(file, "help.bold                 = yellow, blue, bold\n");
    fprintf(file, "help.link                 = black, cyan\n");
    fprintf(file, "help.link_selected        = white, cyan, bold\n");
    fprintf(file, "\n");
    fprintf(file, "\n");
    fprintf(file,
        "# These colors match the Qmodem 5.0 \"Custom Shades\" theme from a fresh install\n");
    fprintf(file, "\n");
    fprintf(file, "# status_line.item_enabled  = blue, white\n");
    fprintf(file, "# status_line.item_disabled = black, white\n");
    fprintf(file, "# terminal.header_lines     = yellow, blue, bold\n");
    fprintf(file, "# terminal.normal           = white, blue\n");
    fprintf(file, "# terminal.background       = white, blue\n");
    fprintf(file, "# terminal.debug_sent_color = green, blue, bold\n");
    fprintf(file, "# window.border             = black, cyan, bold\n");
    fprintf(file, "# window.normal             = blue, cyan\n");
    fprintf(file, "# menu_item.command         = yellow, cyan, bold\n");
    fprintf(file, "# menu_item.command_unavailable = black, cyan, bold\n");
    fprintf(file, "# menu_item.text            = blue, cyan\n");
    fprintf(file, "# field_label               = blue, cyan\n");
    fprintf(file, "# field_text                = white, black, bold\n");
    fprintf(file, "# phonebook.entry           = blue, cyan\n");
    fprintf(file, "# phonebook.entry_tagged    = yellow, cyan, bold\n");
    fprintf(file, "# phonebook.selected        = white, black, bold\n");
    fprintf(file, "# phonebook.selected_tagged = white, black, bold\n");
    fprintf(file, "# phonebook.field_text      = black, white\n");
    fprintf(file, "# script.label_running      = green, cyan, bold\n");
    fprintf(file, "# script.label_finished     = red, cyan, bold\n");
    fprintf(file, "# script.label_finished_ok  = yellow, cyan, bold\n");
    fprintf(file, "# help.border               = white, black, bold\n");
    fprintf(file, "# help.background           = white, blue\n");
    fprintf(file, "# help.bold                 = yellow, blue, bold\n");
    fprintf(file, "# help.link                 = black, magenta\n");
    fprintf(file, "# help.link_selected        = white, magenta, bold\n");
    fprintf(file, "\n");
    fprintf(file, "\n");
    fprintf(file,
        "# These colors match the Qmodem 5.0 \"Purple Shades\" theme\n");
    fprintf(file, "\n");
    fprintf(file, "# status_line.item_enabled  = magenta, white, bold\n");
    fprintf(file, "# status_line.item_disabled = black, white, bold\n");
    fprintf(file, "# terminal.header_lines     = magenta, black, bold\n");
    fprintf(file, "# terminal.normal           = cyan, black\n");
    fprintf(file, "# terminal.background       = cyan, black\n");
    fprintf(file, "# terminal.debug_sent_color = green, black, bold\n");
    fprintf(file, "# window.border             = magenta, black, bold\n");
    fprintf(file, "# window.normal             = white, magenta\n");
    fprintf(file, "# menu_item.command         = yellow, magenta, bold\n");
    fprintf(file, "# menu_item.command_unavailable = black, magenta, bold\n");
    fprintf(file, "# menu_item.text            = white, magenta\n");
    fprintf(file, "# field_label               = white, magenta\n");
    fprintf(file, "# field_text                = magenta, black, bold\n");
    fprintf(file, "# phonebook.entry           = white, magenta\n");
    fprintf(file, "# phonebook.entry_tagged    = yellow, magenta, bold\n");
    fprintf(file, "# phonebook.selected        = magenta, black, bold\n");
    fprintf(file, "# phonebook.selected_tagged = magenta, black, bold\n");
    fprintf(file, "# phonebook.field_text      = black, white\n");
    fprintf(file, "# script.label_running      = green, magenta, bold\n");
    fprintf(file, "# script.label_finished     = red, black\n");
    fprintf(file, "# script.label_finished_ok  = yellow, magenta, bold\n");
    fprintf(file, "# help.border               = white, cyan, bold\n");
    fprintf(file, "# help.background           = white, cyan\n");
    fprintf(file, "# help.bold                 = yellow, magenta, bold\n");
    fprintf(file, "# help.link                 = black, white\n");
    fprintf(file, "# help.link_selected        = white, white, bold\n");
    fprintf(file, "\n");
    fprintf(file, "\n");
    fprintf(file, "# These colors match the Qmodem 5.0 \"Red Shades\" theme\n");
    fprintf(file, "\n");
    fprintf(file, "# status_line.item_enabled  = white, red\n");
    fprintf(file, "# status_line.item_disabled = black, red, bold\n");
    fprintf(file, "# terminal.header_lines     = yellow, black, bold\n");
    fprintf(file, "# terminal.normal           = brown, black\n");
    fprintf(file, "# terminal.background       = brown, black\n");
    fprintf(file, "# terminal.debug_sent_color = green, black, bold\n");
    fprintf(file, "# window.border             = red, black, bold\n");
    fprintf(file, "# window.normal             = white, red\n");
    fprintf(file, "# menu_item.command         = yellow, red, bold\n");
    fprintf(file, "# menu_item.command_unavailable = black, red, bold\n");
    fprintf(file, "# menu_item.text            = white, red\n");
    fprintf(file, "# field_label               = white, red\n");
    fprintf(file, "# field_text                = yellow, black, bold\n");
    fprintf(file, "# phonebook.entry           = white, red\n");
    fprintf(file, "# phonebook.entry_tagged    = yellow, red, bold\n");
    fprintf(file, "# phonebook.selected        = yellow, black\n");
    fprintf(file, "# phonebook.selected_tagged = yellow, black, bold\n");
    fprintf(file, "# phonebook.field_text      = black, white\n");
    fprintf(file, "# script.label_running      = green, red, bold\n");
    fprintf(file, "# script.label_finished     = red, black\n");
    fprintf(file, "# script.label_finished_ok  = yellow, red, bold\n");
    fprintf(file, "# help.border               = white, red, bold\n");
    fprintf(file, "# help.background           = white, red\n");
    fprintf(file, "# help.bold                 = yellow, red, bold\n");
    fprintf(file, "# help.link                 = black, white\n");
    fprintf(file, "# help.link_selected        = white, white, bold\n");
    fprintf(file, "\n");
    fprintf(file, "\n");
    fprintf(file,
        "# These colors match the Qmodem 5.0 \"Monochrome Shades\" theme\n");
    fprintf(file, "\n");
    fprintf(file, "# status_line.item_enabled  = black, white\n");
    fprintf(file, "# status_line.item_disabled = black, white, bold\n");
    fprintf(file, "# terminal.header_lines     = white, black, bold\n");
    fprintf(file, "# terminal.normal           = white, black\n");
    fprintf(file, "# terminal.background       = white, black\n");
    fprintf(file, "# terminal.debug_sent_color = black, white, bold\n");
    fprintf(file, "# window.border             = white, black\n");
    fprintf(file, "# window.normal             = white, black\n");
    fprintf(file, "# menu_item.command         = white, black, bold\n");
    fprintf(file, "# menu_item.command_unavailable = black, black, bold\n");
    fprintf(file, "# menu_item.text            = white, black\n");
    fprintf(file, "# field_label               = white, black\n");
    fprintf(file, "# field_text                = black, white\n");
    fprintf(file, "# phonebook.entry           = white, black\n");
    fprintf(file, "# phonebook.entry_tagged    = white, black, bold\n");
    fprintf(file, "# phonebook.selected        = black, white\n");
    fprintf(file, "# phonebook.selected_tagged = black, white\n");
    fprintf(file, "# phonebook.field_text      = black, white\n");
    fprintf(file, "# script.label_running      = white, black, bold\n");
    fprintf(file, "# script.label_finished     = white, black\n");
    fprintf(file, "# script.label_finished_ok  = white, black, bold\n");
    fprintf(file, "# help.border               = white, black\n");
    fprintf(file, "# help.background           = white, black\n");
    fprintf(file, "# help.bold                 = white, black, bold\n");
    fprintf(file, "# help.link                 = black, white\n");
    fprintf(file, "# help.link_selected        = white, white, bold\n");
    fprintf(file, "\n");
    fprintf(file, "\n");
    fprintf(file,
        "# These colors match the Qmodem 5.0 \"LCD Laptop Shades\" theme\n");
    fprintf(file, "\n");
    fprintf(file, "# status_line.item_enabled  = white, white, bold\n");
    fprintf(file, "# status_line.item_disabled = black, white\n");
    fprintf(file, "# terminal.header_lines     = white, black\n");
    fprintf(file, "# terminal.normal           = white, black\n");
    fprintf(file, "# terminal.background       = white, black\n");
    fprintf(file, "# terminal.debug_sent_color = black, white, bold\n");
    fprintf(file, "# window.border             = white, black\n");
    fprintf(file, "# window.normal             = white, black\n");
    fprintf(file, "# menu_item.command         = white, black\n");
    fprintf(file, "# menu_item.command_unavailable = black, black, bold\n");
    fprintf(file, "# menu_item.text            = white, black\n");
    fprintf(file, "# field_label               = white, black\n");
    fprintf(file, "# field_text                = black, white\n");
    fprintf(file, "# phonebook.entry           = white, black\n");
    fprintf(file, "# phonebook.entry_tagged    = white, black\n");
    fprintf(file, "# phonebook.selected        = black, white\n");
    fprintf(file, "# phonebook.selected_tagged = black, white\n");
    fprintf(file, "# phonebook.field_text      = black, white\n");
    fprintf(file, "# script.label_running      = white, black\n");
    fprintf(file, "# script.label_finished     = white, black\n");
    fprintf(file, "# script.label_finished_ok  = white, black\n");
    fprintf(file, "# help.border               = white, black\n");
    fprintf(file, "# help.background           = white, black\n");
    fprintf(file, "# help.bold                 = white, black, bold\n");
    fprintf(file, "# help.link                 = black, white\n");
    fprintf(file, "# help.link_selected        = white, white, bold\n");

    /*
     * All done.
     */
    Xfree(full_filename, __FILE__, __LINE__);
    fclose(file);
}

/**
 * Set a text color from a line in the configuration file.  Each line has the
 * form <foreground>, <background> [, bold ]
 *
 * @param color the UI element color
 * @param line the line from the config file
 */
static void set_q_color(struct q_text_color_struct * color, const char * line) {
    char * begin;
    char * end;

    begin = (char *) line;
    end = strchr(begin, ',');
    if (end == NULL) {
        return;
    }

    /*
     * Default no bold
     */
    color->bold = Q_FALSE;

    *end = 0;
    color->fg = color_from_string(color->fg, begin);
    begin = end + 1;
    while ((*begin != 0) && (q_isspace(*begin))) {
        begin++;
    }
    if (*begin == 0) {
        return;
    }

    end = strchr(begin, ',');
    if (end == NULL) {
        color->bg = color_from_string(color->bg, begin);
        return;
    }
    *end = 0;
    color->bg = color_from_string(color->bg, begin);
    begin = end + 1;
    while ((*begin != 0) && (q_isspace(*begin))) {
        begin++;
    }
    if (*begin == 0) {
        return;
    }
    if (strncasecmp(begin, "bold", strlen("bold")) == 0) {
        color->bold = Q_TRUE;
    }
}

/**
 * Load (or reload) colors from the colors.cfg file.
 */
void load_colors() {
    FILE * file;
    char * full_filename;
    char line[OPTIONS_LINE_SIZE];
    char * key;
    char * value;
    char * color_key;

    file = open_datadir_file(COLOR_CONFIG_FILENAME, &full_filename, "r");
    if (file == NULL) {
        /*
         * Create the colors file
         */
        create_color_config();

        /*
         * Quietly exit.
         */
        Xfree(full_filename, __FILE__, __LINE__);
        return;
    }

    memset(line, 0, sizeof(line));
    while (!feof(file)) {

        if (fgets(line, sizeof(line), file) == NULL) {
            /*
             * This will cause the outer while's feof() check to fail and
             * smoothly exit the while loop.
             */
            continue;
        }
        line[sizeof(line) - 1] = 0;

        if ((strlen(line) == 0) || (line[0] == '#')) {
            /*
             * Empty or comment line
             */
            continue;
        }

        /*
         * Nix trailing whitespace
         */
        while ((strlen(line) > 0) && (q_isspace(line[strlen(line) - 1]))) {
            line[strlen(line) - 1] = 0;
        }
        key = line;
        while ((strlen(key) > 0) && (q_isspace(*key))) {
            key++;
        }

        value = strchr(key, '=');
        if (value == NULL) {
            /*
             * Invalid line
             */
            continue;
        }

        *value = 0;
        value++;
        while ((strlen(value) > 0) && (q_isspace(*value))) {
            value++;
        }
        if (*value == 0) {
            /*
             * No data
             */
            continue;
        }

        color_key = "status_line.item_enabled";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_STATUS], value);
            continue;
        }
        color_key = "status_line.item_disabled";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_STATUS_DISABLED], value);
            continue;
        }
        color_key = "terminal.header_lines";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_CONSOLE], value);
            continue;
        }
        color_key = "terminal.normal";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_CONSOLE_TEXT], value);
            continue;
        }
        color_key = "terminal.background";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_CONSOLE_BACKGROUND], value);
            continue;
        }
        color_key = "terminal.debug_sent_color";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_DEBUG_ECHO], value);
            continue;
        }
        color_key = "window.border";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_WINDOW_BORDER], value);
            continue;
        }
        color_key = "window.normal";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_WINDOW], value);
            continue;
        }
        color_key = "menu_item.command_unavailable";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_MENU_COMMAND_UNAVAILABLE],
                        value);
            continue;
        }
        color_key = "menu_item.command";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_MENU_COMMAND], value);
            continue;
        }
        color_key = "menu_item.text";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_MENU_TEXT], value);
            continue;
        }
        color_key = "field_label";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_WINDOW_FIELD_HIGHLIGHTED],
                        value);
            continue;
        }
        color_key = "field_text";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_WINDOW_FIELD_TEXT_HIGHLIGHTED],
                        value);
            continue;
        }
        color_key = "phonebook.entry_tagged";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_PHONEBOOK_TAGGED], value);
            continue;
        }
        color_key = "phonebook.entry";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_PHONEBOOK_ENTRY], value);
            continue;
        }
        color_key = "phonebook.selected_tagged";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_PHONEBOOK_SELECTED_TAGGED],
                        value);
            continue;
        }
        color_key = "phonebook.selected";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_PHONEBOOK_SELECTED], value);
            continue;
        }
        color_key = "phonebook.field_text";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_PHONEBOOK_FIELD_TEXT], value);
            continue;
        }
        color_key = "script.label_running";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_SCRIPT_RUNNING], value);
            continue;
        }
        color_key = "script.label_finished_ok";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_SCRIPT_FINISHED_OK], value);
            continue;
        }
        color_key = "script.label_finished";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_SCRIPT_FINISHED], value);
            continue;
        }

        color_key = "help.border";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_HELP_BORDER], value);
            continue;
        }
        color_key = "help.background";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_HELP_BACKGROUND], value);
            continue;
        }
        color_key = "help.bold";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_HELP_BOLD], value);
            continue;
        }
        color_key = "help.link_selected";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_HELP_LINK_SELECTED], value);
            continue;
        }
        color_key = "help.link";
        if (strncmp(key, color_key, strlen(color_key)) == 0) {
            set_q_color(&q_text_colors[Q_COLOR_HELP_LINK], value);
            continue;
        }
    }

    /*
     * All done.
     */
    Xfree(full_filename, __FILE__, __LINE__);
    fclose(file);
}

/**
 * This must be called to initialize the colors from the config file.
 */
void q_setup_colors() {
    short i;
    const short COLOR_DEFAULT = COLOR_BLACK;

    q_text_colors[Q_COLOR_STATUS].bold = Q_FALSE;
    q_text_colors[Q_COLOR_STATUS].fg = COLOR_BLUE;
    q_text_colors[Q_COLOR_STATUS].bg = COLOR_WHITE;

    q_text_colors[Q_COLOR_STATUS_DISABLED].bold = Q_TRUE;
    q_text_colors[Q_COLOR_STATUS_DISABLED].fg = COLOR_BLACK;
    q_text_colors[Q_COLOR_STATUS_DISABLED].bg = COLOR_WHITE;

    q_text_colors[Q_COLOR_CONSOLE].bold = Q_TRUE;
    q_text_colors[Q_COLOR_CONSOLE].fg = COLOR_CYAN;
    q_text_colors[Q_COLOR_CONSOLE].bg = COLOR_DEFAULT;

    q_text_colors[Q_COLOR_CONSOLE_TEXT].bold = Q_FALSE;
    q_text_colors[Q_COLOR_CONSOLE_TEXT].fg = COLOR_WHITE;
    q_text_colors[Q_COLOR_CONSOLE_TEXT].bg = COLOR_DEFAULT;

    q_text_colors[Q_COLOR_CONSOLE_BACKGROUND].bold = Q_FALSE;
    q_text_colors[Q_COLOR_CONSOLE_BACKGROUND].fg = COLOR_WHITE;
    q_text_colors[Q_COLOR_CONSOLE_BACKGROUND].bg = COLOR_BLACK;

    q_text_colors[Q_COLOR_DEBUG_ECHO].bold = Q_TRUE;
    q_text_colors[Q_COLOR_DEBUG_ECHO].fg = COLOR_GREEN;
    q_text_colors[Q_COLOR_DEBUG_ECHO].bg = COLOR_DEFAULT;

    q_text_colors[Q_COLOR_WINDOW_BORDER].bold = Q_TRUE;
    q_text_colors[Q_COLOR_WINDOW_BORDER].fg = COLOR_BLUE;
    q_text_colors[Q_COLOR_WINDOW_BORDER].bg = COLOR_BLACK;

    q_text_colors[Q_COLOR_WINDOW].bold = Q_FALSE;
    q_text_colors[Q_COLOR_WINDOW].fg = COLOR_DEFAULT;
    q_text_colors[Q_COLOR_WINDOW].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_MENU_COMMAND].bold = Q_TRUE;
    q_text_colors[Q_COLOR_MENU_COMMAND].fg = COLOR_YELLOW;
    q_text_colors[Q_COLOR_MENU_COMMAND].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_MENU_COMMAND_UNAVAILABLE].bold = Q_TRUE;
    q_text_colors[Q_COLOR_MENU_COMMAND_UNAVAILABLE].fg = COLOR_BLACK;
    q_text_colors[Q_COLOR_MENU_COMMAND_UNAVAILABLE].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_MENU_TEXT].bold = Q_FALSE;
    q_text_colors[Q_COLOR_MENU_TEXT].fg = COLOR_WHITE;
    q_text_colors[Q_COLOR_MENU_TEXT].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_WINDOW_FIELD_HIGHLIGHTED].bold = Q_FALSE;
    q_text_colors[Q_COLOR_WINDOW_FIELD_HIGHLIGHTED].fg = COLOR_WHITE;
    q_text_colors[Q_COLOR_WINDOW_FIELD_HIGHLIGHTED].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_WINDOW_FIELD_TEXT_HIGHLIGHTED].bold = Q_TRUE;
    q_text_colors[Q_COLOR_WINDOW_FIELD_TEXT_HIGHLIGHTED].fg = COLOR_YELLOW;
    q_text_colors[Q_COLOR_WINDOW_FIELD_TEXT_HIGHLIGHTED].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_PHONEBOOK_ENTRY].bold = Q_FALSE;
    q_text_colors[Q_COLOR_PHONEBOOK_ENTRY].fg = COLOR_WHITE;
    q_text_colors[Q_COLOR_PHONEBOOK_ENTRY].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_PHONEBOOK_SELECTED].bold = Q_FALSE;
    q_text_colors[Q_COLOR_PHONEBOOK_SELECTED].fg = COLOR_WHITE;
    q_text_colors[Q_COLOR_PHONEBOOK_SELECTED].bg = COLOR_BLACK;

    q_text_colors[Q_COLOR_PHONEBOOK_SELECTED_TAGGED].bold = Q_TRUE;
    q_text_colors[Q_COLOR_PHONEBOOK_SELECTED_TAGGED].fg = COLOR_YELLOW;
    q_text_colors[Q_COLOR_PHONEBOOK_SELECTED_TAGGED].bg = COLOR_BLACK;

    q_text_colors[Q_COLOR_PHONEBOOK_TAGGED].bold = Q_TRUE;
    q_text_colors[Q_COLOR_PHONEBOOK_TAGGED].fg = COLOR_YELLOW;
    q_text_colors[Q_COLOR_PHONEBOOK_TAGGED].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_PHONEBOOK_FIELD_TEXT].bold = Q_FALSE;
    q_text_colors[Q_COLOR_PHONEBOOK_FIELD_TEXT].fg = COLOR_BLACK;
    q_text_colors[Q_COLOR_PHONEBOOK_FIELD_TEXT].bg = COLOR_WHITE;

    q_text_colors[Q_COLOR_SCRIPT_RUNNING].bold = Q_TRUE;
    q_text_colors[Q_COLOR_SCRIPT_RUNNING].fg = COLOR_GREEN;
    q_text_colors[Q_COLOR_SCRIPT_RUNNING].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_SCRIPT_FINISHED].bold = Q_TRUE;
    q_text_colors[Q_COLOR_SCRIPT_FINISHED].fg = COLOR_RED;
    q_text_colors[Q_COLOR_SCRIPT_FINISHED].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_SCRIPT_FINISHED_OK].bold = Q_TRUE;
    q_text_colors[Q_COLOR_SCRIPT_FINISHED_OK].fg = COLOR_YELLOW;
    q_text_colors[Q_COLOR_SCRIPT_FINISHED_OK].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_HELP_BORDER].bold = Q_TRUE;
    q_text_colors[Q_COLOR_HELP_BORDER].fg = COLOR_WHITE;
    q_text_colors[Q_COLOR_HELP_BORDER].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_HELP_BACKGROUND].bold = Q_FALSE;
    q_text_colors[Q_COLOR_HELP_BACKGROUND].fg = COLOR_WHITE;
    q_text_colors[Q_COLOR_HELP_BACKGROUND].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_HELP_BOLD].bold = Q_TRUE;
    q_text_colors[Q_COLOR_HELP_BOLD].fg = COLOR_YELLOW;
    q_text_colors[Q_COLOR_HELP_BOLD].bg = COLOR_BLUE;

    q_text_colors[Q_COLOR_HELP_LINK].bold = Q_FALSE;
    q_text_colors[Q_COLOR_HELP_LINK].fg = COLOR_BLACK;
    q_text_colors[Q_COLOR_HELP_LINK].bg = COLOR_CYAN;

    q_text_colors[Q_COLOR_HELP_LINK_SELECTED].bold = Q_TRUE;
    q_text_colors[Q_COLOR_HELP_LINK_SELECTED].fg = COLOR_WHITE;
    q_text_colors[Q_COLOR_HELP_LINK_SELECTED].bg = COLOR_CYAN;

    /*
     * Override with whatever may be in colors.cfg.
     */
    load_colors();

    /*
     * Initialize the 64 curses colors.
     */
    if ((COLORS >= 16) && (COLOR_PAIRS >= 2 * q_color_bold_offset) &&
        (can_change_color() == TRUE)
    ) {
        /*
         * Complete re-map both the colors and color pairs.  Note that the
         * max color value is 1000.  These values are gamma-corrected.  Some
         * other potential values:
         *
         *  gamma  |  333  |  666  | 999
         * ------------------------------
         *    1.0  |  333  |  666  | 999
         *    1.2  |  267  |  614  | 999
         *    1.4  |  215  |  568  | 999
         *    1.6  |  172  |  522  | 999
         *    1.8  |  138  |  481  | 999
         *    2.0  |  111  |  444  | 999
         *    2.2  |   89  |  409  | 999
         *    2.4  |   71  |  377  | 999
         */
#define GAMMA_000 000
#define GAMMA_333 267
#define GAMMA_666 614
#define GAMMA_999 999

        /*
         * Normal intensity colors
         */
        init_color(COLOR_BLACK,   GAMMA_000, GAMMA_000, GAMMA_000);
        init_color(COLOR_RED,     GAMMA_666, GAMMA_000, GAMMA_000);
        init_color(COLOR_GREEN,   GAMMA_000, GAMMA_666, GAMMA_000);
        init_color(COLOR_YELLOW,  GAMMA_666, GAMMA_333, GAMMA_000);
        init_color(COLOR_BLUE,    GAMMA_000, GAMMA_000, GAMMA_666);
        init_color(COLOR_MAGENTA, GAMMA_666, GAMMA_000, GAMMA_666);
        init_color(COLOR_CYAN,    GAMMA_000, GAMMA_666, GAMMA_666);
        init_color(COLOR_WHITE,   GAMMA_666, GAMMA_666, GAMMA_666);

        /*
         * Bright intensity colors
         */
        init_color(8 + COLOR_BLACK,   GAMMA_333, GAMMA_333, GAMMA_333);
        init_color(8 + COLOR_RED,     GAMMA_999, GAMMA_333, GAMMA_333);
        init_color(8 + COLOR_GREEN,   GAMMA_333, GAMMA_999, GAMMA_333);
        init_color(8 + COLOR_YELLOW,  GAMMA_999, GAMMA_999, GAMMA_333);
        init_color(8 + COLOR_BLUE,    GAMMA_333, GAMMA_333, GAMMA_999);
        init_color(8 + COLOR_MAGENTA, GAMMA_999, GAMMA_333, GAMMA_999);
        init_color(8 + COLOR_CYAN,    GAMMA_333, GAMMA_999, GAMMA_999);
        init_color(8 + COLOR_WHITE,   GAMMA_999, GAMMA_999, GAMMA_999);

        /*
         * Now init the pairs
         */

        /*
         * Normal intensity
         */
        for (i = 1; i < q_color_bold_offset; i++) {
            init_pair(i, (short) ((i & 0x38) >> 3), (short) (i & 0x07));
        }

        /*
         * Bright intensity
         */
        for (i = 1 + q_color_bold_offset; i < 2 * q_color_bold_offset; i++) {
            init_pair(i,
                      (short) ((((i - q_color_bold_offset) & 0x38) >> 3) + 8),
                      (short) ((i - q_color_bold_offset) & 0x07));
        }

        /*
         * Special case: put black-on-black at 0x38 and 64 + 0x38
         */
        init_pair(0x38                      , COLOR_BLACK    , COLOR_BLACK);
        init_pair(0x38 + q_color_bold_offset, COLOR_BLACK + 8, COLOR_BLACK);

        /*
         * Special case: put white-on-black at 64 and 64 + 64
         */
        init_pair(64                      , COLOR_WHITE    , COLOR_BLACK);
        init_pair(64 + q_color_bold_offset, COLOR_WHITE + 8, COLOR_BLACK);

        q_white_color_pair_num = 64;

        /*
         * Bail out here.
         */
        return;
    }

    for (i = 1; (i < 64) && (i < COLOR_PAIRS); i++) {
        init_pair(i, (short) ((i & 0x38) >> 3), (short) (i & 0x07));
    }

    /*
     * Special case: put black-on-black at 0x38
     */
    init_pair(0x38, 0x00, 0x00);

    /*
     * We will send A_BOLD directly to the screen.
     */
    q_color_bold_offset = 0;

    if (COLOR_PAIRS > 64) {
        /*
         * Make my own white-on-black color
         */
        init_pair(64, COLOR_WHITE, COLOR_BLACK);
        q_white_color_pair_num = 64;
    } else {
        /*
         * Assume color pair 0 is white on black
         */
        assume_default_colors(COLOR_WHITE, COLOR_BLACK);
        q_white_color_pair_num = 0;
    }
}

/**
 * Use up to 256 characters for the HTML/CSS color tag.
 */
static char font_color[256];

/**
 * Convert a curses attr_t into an HTML &lt;font color&gt; tag.  Note that
 * the string returned is a single static buffer, i.e. this is NOT
 * thread-safe.
 *
 * @param attr the curses attribute
 * @return the HTML string
 */
char * color_to_html(const attr_t attr) {
    char * font_weight = "normal";
    char * text_decoration = "none";
    char * fg_text;
    char * bg_text;

    char * bitmask_to_html_map[] = {
        "#000000",              /* COLOR_BLACK   */
        "#AB0000",              /* COLOR_RED     */
        "#00AB00",              /* COLOR_GREEN   */
        "#996600",              /* COLOR_YELLOW  */
        "#0000AB",              /* COLOR_BLUE    */
        "#990099",              /* COLOR_MAGENTA */
        "#009999",              /* COLOR_CYAN    */
        "#ABABAB",              /* COLOR_WHITE   */
    };

    char * bitmask_to_html_map_bright[] = {
        "#545454",              /* COLOR_BLACK   */
        "#FF6666",              /* COLOR_RED     */
        "#66FF66",              /* COLOR_GREEN   */
        "#FFFF66",              /* COLOR_YELLOW  */
        "#6666FF",              /* COLOR_BLUE    */
        "#FF66FF",              /* COLOR_MAGENTA */
        "#66FFFF",              /* COLOR_CYAN    */
        "#FFFFFF",              /* COLOR_WHITE   */
    };

    short fg;
    short bg;

    fg = (short) (PAIR_NUMBER(attr) >> 3);
    bg = (short) (PAIR_NUMBER(attr) & 0x07);

    memset(font_color, 0, sizeof(font_color));

    if ((attr & A_BLINK) && (attr & A_UNDERLINE)) {
        text_decoration = "blink, underline";
    } else if (attr & A_UNDERLINE) {
        text_decoration = "underline";
    } else if (attr & A_BLINK) {
        text_decoration = "blink";
    }
    if (attr & A_REVERSE) {
        fg_text = bitmask_to_html_map[bg];
        if (attr & A_BOLD) {
            bg_text = bitmask_to_html_map_bright[fg];
        } else {
            bg_text = bitmask_to_html_map[fg];
        }
    } else {
        bg_text = bitmask_to_html_map[bg];
        if (attr & A_BOLD) {
            fg_text = bitmask_to_html_map_bright[fg];
        } else {
            fg_text = bitmask_to_html_map[fg];
        }
    }

    sprintf(font_color, "style=\"color: %s; background-color: %s; " \
        "text-decoration: %s; font-weight: %s\"",
        fg_text, bg_text, text_decoration, font_weight);

    return font_color;
}

/* Thank you to TheDraw 4.63! */

/**
 * Convert an array of 8-bit attribute/character VGA cells into scrollback
 * lines that can be displayed through curses.
 *
 * @param screen the data array
 * @param length the number of bytes in screen
 * @param output_line a previously-allocated scrollback line to contain the
 * first row of screen data.  Additional lines are allocated and added to the
 * list as needed.
 */
void convert_thedraw_screen(const unsigned char * screen, const int length,
                            struct q_scrolline_struct * output_line) {
    int i;
    attr_t bold;
    short fg;
    short bg;
    short curses_color;

    output_line->length = 0;

    for (i = 0; i < length; i += 2) {
        /*
         * First byte: character.  Note this is CP437 because that is what
         * TheDraw saved it as.
         */
        output_line->chars[output_line->length] = cp437_chars[screen[i]];

        /*
         * Second byte: PC color
         */
        bold = A_NORMAL;
        fg = screen[i + 1] & 0x07;
        if ((screen[i + 1] & 0x08) != 0) {
            bold = A_BOLD;
        }
        bg = (screen[i + 1] >> 4) & 0x07;
        fg = pc_to_curses_map[fg];
        bg = pc_to_curses_map[bg];

        curses_color = (fg << 3) | bg;

        output_line->colors[output_line->length] =
            (bold | COLOR_PAIR(curses_color));
        output_line->length++;

        /*
         * Wrap if necessary
         */
        if (output_line->length == 80) {
            struct q_scrolline_struct *new_line =
                (struct q_scrolline_struct *)
                Xmalloc(sizeof(struct q_scrolline_struct), __FILE__, __LINE__);
            memset(new_line, 0, sizeof(struct q_scrolline_struct));
            new_line->prev = output_line;
            output_line->next = new_line;
            output_line = output_line->next;
        }
    }
}

#ifndef QMODEM_INFO_SCREEN

/*
 * This is the new splash screen.
 */
unsigned char q_info_screen[] = {
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x1B, '�', 0x1B,
    '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B,
    '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x1B, ' ', 0x1B, ' ', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B,
    '�', 0x1B, '�', 0x1B, '�', 0x1B, ' ', 0x1B, ' ', 0x1B, '�', 0x1B,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x1B, ' ', 0x1B, ' ', 0x1B, '�', 0x1B,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x1B, ' ', 0x1B,
    ' ', 0x1B, '�', 0x1B, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x1B, ' ', 0x1B,
    ' ', 0x1B, '�', 0x1B, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x1B, ' ', 0x1B, ' ', 0x1B, '�', 0x1B, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x1B, ' ', 0x1B, ' ', 0x1B, '�', 0x1B, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x1B, ' ', 0x1B, ' ', 0x1B, '�', 0x1B,
    '�', 0x09, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x0A, '�', 0x0A, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x09, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A,
    '�', 0x09, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x1B, ' ', 0x1B, ' ', 0x1B, '�', 0x1B,
    '�', 0x09, '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B, ' ', 0x1B,
    ' ', 0x1B, '�', 0x1B, '�', 0x09, '�', 0x0A, ' ', 0x0A, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, ' ', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x0A,
    ' ', 0x0A, '�', 0x0A, '�', 0x0A, ' ', 0x0A, '�', 0x0A, '�', 0x0A,
    '�', 0x09, '�', 0x0A, ' ', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x0A, ' ', 0x0A, '�', 0x0A,
    ' ', 0x0A, '�', 0x0A, ' ', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x08,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x1B, ' ', 0x1B,
    ' ', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B, ' ', 0x1B, '�', 0x1B,
    '�', 0x1B, ' ', 0x1B, ' ', 0x1B, '�', 0x1B, '�', 0x09, '�', 0x0A,
    ' ', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, ' ', 0x0A, '�', 0x0A,
    '�', 0x09, '�', 0x0A, ' ', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A,
    ' ', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x0A, ' ', 0x0A, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x0A,
    ' ', 0x0A, '�', 0x0A, ' ', 0x0A, '�', 0x0A, ' ', 0x0A, '�', 0x0A,
    '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B,
    '�', 0x1B, ' ', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x1B,
    '�', 0x09, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x0A, '�', 0x0A, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A,
    '�', 0x09, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A, '�', 0x0A,
    '�', 0x0A, '�', 0x0A, '�', 0x09, '�', 0x08, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x1B, '�', 0x1B, '�', 0x1B, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09,
    '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x09, '�', 0x08,
    '�', 0x09, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08
};

#else

/*
 * The original splash screen, a scrape (THEGRAB.EXE) from Qmodem(tm) 5.01.
 *
 * I liked that one, but we're pretty far away from Qmodem(tm) now, and I
 * don't want my users to see this homage to Qmodem(tm) in a negative way.
 */

unsigned char q_info_screen[] = {
    '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x7F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, '�', 0x78,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x7F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x1F,
    'Q', 0x1F, 'o', 0x1F, 'd', 0x1F, 'e', 0x1F, 'm', 0x1F, ' ', 0x1F,

/*
 *   /--- VERSION is HERE
 *   |
 *   V
 */

    '1', 0x1F, '.', 0x1F, '0', 0x1F, '.' , 0x1F, '0', 0x1F, ' ', 0x1F,

/*   ^
 *   |
 *   \---- VERSION is here
 */

    'P', 0x1F, 'u', 0x1F, 'b', 0x1F, 'l', 0x1F, 'i', 0x1F, 'c', 0x1F,
    ' ', 0x1F, 'D', 0x1F, 'o', 0x1F, 'm', 0x1F, 'a', 0x1F, 'i', 0x1F,
    'n', 0x1F, ' ', 0x1F, 'T', 0x1F, 'e', 0x1F, 'r', 0x1F, 'm', 0x1F,
    'i', 0x1F, 'n', 0x1F, 'a', 0x1F, 'l', 0x1F, ' ', 0x1F, ' ', 0x0F,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x1F, 'h', 0x1F, 't', 0x1F, 't', 0x1F, 'p', 0x1F,
    ':', 0x1F, '/', 0x1F, '/', 0x1F, 'q', 0x1F, 'o', 0x1F, 'd', 0x1F,
    'e', 0x1F, 'm', 0x1F, '.', 0x1F, 's', 0x1F, 'o', 0x1F, 'u', 0x1F,
    'r', 0x1F, 'c', 0x1F, 'e', 0x1F, 'f', 0x1F, 'o', 0x1F, 'r', 0x1F,
    'g', 0x1F, 'e', 0x1F, '.', 0x1F, 'n', 0x1F, 'e', 0x1F, 't', 0x1F,
    ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F,
    ' ', 0x1F, ' ', 0x0F, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F,
    ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F,
    ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F,
    ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F,
    ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F,
    ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x1F,
    ' ', 0x1F, ' ', 0x1F, ' ', 0x1F, ' ', 0x0F, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x7F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x08, '�', 0x08,
    '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, '�', 0x78, '�', 0x78,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x7F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    '�', 0x78, '�', 0x78, ' ', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, '�', 0x78, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, '�', 0x78, '�', 0x78, ' ', 0x78, '�', 0x78, '�', 0x78,
    ' ', 0x78, '�', 0x78, '�', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, '�', 0x78, '�', 0x78, ' ', 0x78,
    '�', 0x78, ' ', 0x78, '�', 0x78, '�', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, '�', 0x78, '�', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, '�', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, ' ', 0x78, ' ', 0x78,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, '�', 0x7F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, '�', 0x78, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    ' ', 0x78, ' ', 0x78, ' ', 0x78, ' ', 0x78, '�', 0x7F, '�', 0x3F,
    '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x03, '�', 0x03, '�', 0x03, ' ', 0x33, '�', 0x3F, '�', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, '�', 0x78, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, ' ', 0x78, ' ', 0x78, '�', 0x7F, '�', 0x3F,
    '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x3F, '�', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, '�', 0x7F, '�', 0x3F, '�', 0x03, '�', 0x03,
    '�', 0x03, '�', 0x03, '�', 0x3F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, '�', 0x78, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, ' ', 0x78, '�', 0x3F,
    '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x3F, '�', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    '�', 0x7F, '�', 0x3F, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x03,
    '�', 0x0F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, '�', 0x78, '�', 0x78, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x70,
    ' ', 0x70, '�', 0x3F, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x3F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, '�', 0x3F, ' ', 0x3F,
    '�', 0x03, '�', 0x03, '�', 0x0F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, '�', 0x48, '�', 0x78, '�', 0x04, '�', 0x04,
    '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x08, '�', 0x04,
    '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x48, '�', 0x48,
    '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04,
    '�', 0x08, '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x48, '�', 0x04,
    '�', 0x04, '�', 0x04, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x78, '�', 0x3F, '�', 0x03, '�', 0x03,
    '�', 0x03, '�', 0x3F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    '�', 0x3F, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x0F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, '�', 0x48, '�', 0x48, '�', 0x78,
    '�', 0x04, '�', 0x04, ' ', 0x04, '�', 0x08, '�', 0x04, '�', 0x04,
    ' ', 0x04, '�', 0x04, '�', 0x04, ' ', 0x04, '�', 0x08, '�', 0x04,
    '�', 0x04, ' ', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04,
    '�', 0x04, '�', 0x04, '�', 0x08, '�', 0x04, '�', 0x04, '�', 0x04,
    '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, ' ', 0x04, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x78, '�', 0x3F,
    '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x3F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, '�', 0x3F, '�', 0x03, '�', 0x03, '�', 0x03,
    '�', 0x0F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, '�', 0x48, '�', 0x48,
    '�', 0x48, '�', 0x78, '�', 0x04, '�', 0x04, ' ', 0x04, '�', 0x08,
    '�', 0x04, '�', 0x04, ' ', 0x04, '�', 0x04, '�', 0x04, ' ', 0x04,
    '�', 0x08, '�', 0x04, '�', 0x04, ' ', 0x04, '�', 0x04, '�', 0x04,
    '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, ' ', 0x04, '�', 0x04,
    '�', 0x04, ' ', 0x04, '�', 0x04, ' ', 0x04, '�', 0x04, '�', 0x04,
    ' ', 0x04, '�', 0x08, '�', 0x08, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F,
    ' ', 0x78, '�', 0x3F, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x3F,
    ' ', 0x7F, '�', 0x7F, '�', 0x3F, '�', 0x3F, '�', 0x03, '�', 0x03,
    '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x03,
    '�', 0x3F, '�', 0x3F, '�', 0x7F, ' ', 0x7F, '�', 0x3F, '�', 0x03,
    '�', 0x03, '�', 0x03, '�', 0x0F, ' ', 0x7F, ' ', 0x7F, '�', 0x48,
    '�', 0x48, '�', 0x48, '�', 0x48, '�', 0x78, '�', 0x04, '�', 0x04,
    '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, ' ', 0x04, '�', 0x04,
    '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, ' ', 0x04,
    '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04, '�', 0x04,
    '�', 0x08, '�', 0x04, '�', 0x04, ' ', 0x04, '�', 0x08, '�', 0x08,
    '�', 0x04, '�', 0x04, ' ', 0x04, '�', 0x08, '�', 0x08, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, ' ', 0x0F, ' ', 0x0F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x78, '�', 0x3F, '�', 0x03, '�', 0x03,
    '�', 0x03, '�', 0x03, '�', 0x3F, '�', 0x3F, '�', 0x03, '�', 0x3F,
    '�', 0x3F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x7F,
    '�', 0x7F, '�', 0x3F, '�', 0x3F, '�', 0x03, '�', 0x03, '�', 0x3F,
    '�', 0x3F, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x3F, ' ', 0x7F,
    '�', 0x48, '�', 0x48, '�', 0x48, '�', 0x48, '�', 0x48, '�', 0x78,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78,
    '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, '�', 0x78, ' ', 0x0F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x78, ' ', 0x78,
    '�', 0x7F, '�', 0x3F, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x03,
    '�', 0x3F, '�', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, '�', 0x7F, '�', 0x3F,
    '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x48, '�', 0x0F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x0F, ' ', 0x0F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, '�', 0x7F, '�', 0x3F,
    ' ', 0x3F, ' ', 0x3F, '�', 0x03, '�', 0x03, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x03, '�', 0x03, '�', 0x3F, '�', 0x3F, '�', 0x3F, ' ', 0x3F,
    ' ', 0x3F, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x03, '�', 0x03,
    '�', 0x03, '�', 0x3F, '�', 0x48, '�', 0x0F, ' ', 0x3F, ' ', 0x3F,
    ' ', 0x3F, '�', 0x3A, ' ', 0x3A, ' ', 0x3A, ' ', 0x3A, ' ', 0x3A,
    '�', 0x00, ' ', 0x30, ' ', 0x30, ' ', 0x30, ' ', 0x30, '�', 0x3A,
    ' ', 0x3A, ' ', 0x3A, ' ', 0x3A, ' ', 0x3A, '�', 0x3A, ' ', 0x3A,
    ' ', 0x3A, ' ', 0x3A, ' ', 0x3A, '�', 0x00, ' ', 0x30, ' ', 0x30,
    ' ', 0x30, ' ', 0x30, '�', 0x3A, ' ', 0x3A, ' ', 0x3A, ' ', 0x3A,
    ' ', 0x3A, '�', 0x00, ' ', 0x30, ' ', 0x30, ' ', 0x30, '�', 0x3E,
    '�', 0x7E, '�', 0x7E, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x7E, ' ', 0x7E, ' ', 0x7E, ' ', 0x7E,
    ' ', 0x7E, ' ', 0x7E, '�', 0x7F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x7F,
    ' ', 0x7F, '�', 0x7F, '�', 0x7F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x48, '�', 0x0F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F, '�', 0x3F,
    '�', 0x3F, '�', 0x3F, '�', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F,
    ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x0F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F,
    ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, ' ', 0x7F, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x70, '�', 0x70, '�', 0x70, '�', 0x70, '�', 0x70, '�', 0x70,
    ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70,
    ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70,
    ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70,
    ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70, ' ', 0x70, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08, '�', 0x08,
    '�', 0x08, '�', 0x70, '�', 0x70, '�', 0x70, '�', 0x70, '�', 0x70,
    '�', 0x70, '�', 0x70
};

#endif
