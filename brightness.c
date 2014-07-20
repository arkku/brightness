/*
 * brightness.c: A command-line utility to set the display brightness
 * programmatically on Mac OS X. There are other similar utilities, but
 * this does not use the deprecated `CGDisplayIOServicePort()` function
 * and allows specifying displays by their human-readable names.
 *
 * Compilation:
 *      clang -std=c99 -DMACOSX_DEPLOYMENT_TARGET=10.5 \
 *            -o brightness brightness.c \
 *            -framework IOKit -framework ApplicationServices
 *
 * Copyright (c) 2014 Kimmo Kulovesi, arkku.com
 * Use and distribute freely, mark modified copies as such.
 * Absolutely no warranty, USE AT YOUR OWN RISK ONLY!
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <ApplicationServices/ApplicationServices.h>

static const CFStringRef kDisplayBrightness = CFSTR(kIODisplayBrightnessKey);
static const int NAME_WIDTH = 20;
#define BR_FMT "%.3f"


static void
usage (int status) {
    (void) fprintf(stderr,
       "Usage: brightness [-d name_of_display] [-s] [brightness_to_set]\n\n"
       "- brightness range is from 0.0 (min) to 1.0 (max) on all\n"
       "  display's I've tested, but other values are supported\n"
       "- all applicable displays are set unless a single one is\n"
       "  named with the -d option\n"
       "- names of displays and their current brightnesses\n"
       "  are printed if no arguments are given\n"
       "- the option -s silences non-error output\n\n"
       "Copyright (c) 2014 Kimmo Kulovesi, arkku.com\n"
       );
    exit(status);
}

static char *
c_string_from_cfstring (CFStringRef cf) {
    CFIndex len = CFStringGetLength(cf);
    CFIndex sz = CFStringGetMaximumSizeForEncoding(len, kCFStringEncodingUTF8);
    char *str = malloc(sz + 1);
    if (str) {
        CFStringGetCString(cf, str, sz, kCFStringEncodingUTF8);
    } else {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return str;
}

static char *
name_of_display (io_object_t display) {
    char *name = NULL;
    CFDictionaryRef info;
    info = IODisplayCreateInfoDictionary(display, kIODisplayOnlyPreferredName);
    if (info) {
        CFDictionaryRef names;
        names = CFDictionaryGetValue(info, CFSTR(kDisplayProductName));
        if (names) {
            CFIndex count = CFDictionaryGetCount(names);
            if (count >= 1 && count <= 2) {
                CFTypeRef values[2];
                CFDictionaryGetKeysAndValues(names, NULL, values);
                name = c_string_from_cfstring(values[0]);
            }
        }
        CFRelease(info);
    }
    return name;
}

int
main (int argc, char *argv[]) {
    FILE *outfile = stdout;
    const char *only_set_one_display = NULL;
    float brightness = NAN;
    io_iterator_t iter;
    io_object_t display;
    IOReturn err;

    while (--argc) {
        char *arg = *(++argv);
        if (arg[0] == '-' && arg[1] && arg[2] == '\0') {
            switch (arg[1]) {
            case 's':
            case 'q':
                outfile = NULL;
                break;
            case 'h':
            case '?':
                usage(EXIT_SUCCESS);
                break;
            case 'd':
            case 'm':
                if (--argc) {
                    only_set_one_display = *(++argv);
                    break;
                }
            default:
                (void) fprintf(stderr, "Invalid option: %s\n", arg);
                usage(EXIT_FAILURE);
            }
            continue;
        } else if (isnan(brightness)) {
            char *eptr = arg;
            errno = 0;
            brightness = strtof(arg, &eptr);
            if (eptr != arg && errno != ERANGE) {
                continue;
            }
        }
        (void) fprintf(stderr, "Invalid argument: %s\n", arg);
        usage(EXIT_FAILURE);
    }

#define number_of_displays_set argc
    assert(number_of_displays_set == 0);

    // Iterate over all displays
    if ((err = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                     IOServiceMatching("IODisplayConnect"),
                                     &iter)) != kIOReturnSuccess) {
        (void) fprintf(stderr, "Error: Could not get displays (code %lu)\n",
                       (unsigned long)err);
        return EXIT_FAILURE;
    }
    while (IOIteratorIsValid(iter) && (display = IOIteratorNext(iter))) {
        char *name = name_of_display(display);

        if (only_set_one_display &&
            (!name || strcmp(only_set_one_display, name) != 0)) {
                goto next_display;
        }
        if (outfile) {
            int len = (int) strlen(name);
            len = (len < NAME_WIDTH) ? (NAME_WIDTH - len) : 0;
            (void) fprintf(outfile, "'%s'%*s ", name, len, "");
            (void) fflush(outfile);
        }

        {
            // Get the current brightness
            float b;
            err = IODisplayGetFloatParameter(display, kNilOptions,
                                             kDisplayBrightness, &b);
            if (err == kIOReturnSuccess && outfile) {
                (void) fprintf(outfile, BR_FMT, b);
            }
        }

        if (err == kIOReturnSuccess && !isnan(brightness)) {
            // Set the brightness (unless just listing)
            err = IODisplaySetFloatParameter(display, kNilOptions,
                                             kDisplayBrightness, brightness);
            if (err == kIOReturnSuccess) {
                ++number_of_displays_set;
                if (outfile) {
                    (void) fprintf(outfile, " -> " BR_FMT, brightness);
                }
            } else {
                (void) fprintf(stderr,
                               "\nError: Could not set brightness (%lu).\n",
                               (unsigned long)err);
            }
        }

        if (outfile) {
            (void) fprintf(outfile, "\n");
        }
    next_display:
        if (name) {
            free(name);
        }
        (void) IOObjectRelease(display);
    }
    (void) IOObjectRelease(iter);

    if (!isnan(brightness) && number_of_displays_set < 1) {
        (void) fprintf(stderr, "Error: No %sdisplays could be set\n",
                       (only_set_one_display ? "matching " : ""));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
