/*-
 * Copyright (c) 2012-2014 Stephan Arts. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY [LICENSOR] "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE,DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <signal.h>

#include <pwd.h>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/un.h>

#include <dirent.h>

#include <pthread.h>

#include <unistd.h>
#include <string.h>

#include <openssl/sha.h>

#include <time.h>

#include <libargux/libargux.h>


#include "assert.h"
#include "util.h"
#include "settings.h"
#include "scheduler.h"
#include "db.h"
#include "fqdn.h"

#include "command.h"

#ifndef MAX_PLUGINS
#define MAX_PLUGINS 10
#endif

static ArguxPlugin *_plugins[MAX_PLUGINS];
static int n_plugins = 0;

static ArguxPluginDB *_db_plugin = NULL;

static int _init_db = 0;

#define USER_BUF_LEN 1024

static struct passwd _pwd;

static char _user_buf[USER_BUF_LEN];

enum
{
    OPTION_VERSION = 0,
    OPTION_VERBOSE,
    OPTION_HELP,
    OPTION_FATAL_WARNINGS,
    OPTION_INIT_DB,
    OPTION_CONFIG
};

/************************
 * Command-line options *
 ************************/
static struct option long_options[] = {
    {"version", no_argument, 0, 'V'},        /* OPTION_VERSION */
    {"verbose", no_argument, 0, 'v'},        /* OPTION_VERBOSE */
    {"help", no_argument, 0, 'h'},           /* OPTION_HELP    */
    {"fatal-warnings", 0, 0, 0},             /* OPTION_FATAL_WARNINGS */
    {"init-db", 0, 0, 0},                    /* OPTION_INIT_DB */
    {"config", required_argument, 0, 'c'},   /* OPTION_CONFIG */
    {0, 0, 0, 0}
};

static void
show_version ()
{
    printf ("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
    printf ("Copyright (c) 2012-2015 Stephan Arts\n");
    printf ("There is NO WARRANTY, to the extent permitted by law.\n");
    return;
}

static void
show_usage ()
{
    printf ("Usage: %s [options]\n", PACKAGE_NAME);
    printf ("\n");
    printf ("Options:\n");
    printf ("   --version  -V     Show version information\n");
    printf ("   --help     -h     Show usage information (this output)\n");
    printf ("              -v     Show verbose output\n");
    printf ("              -vv    Show very-verbose output\n");
    printf ("\n");
    printf ("   --init-db         Initialize DB schema\n");
    printf ("   --config=<file>   Use different config-file\n");
    printf ("              -c <file>\n");
    printf ("\n");
    printf ("   --fatal-warnings  Make all warnings fatal\n");
    return;
}

void
process_signal (int s)
{
    switch (s)
    {
    case SIGINT:
        /**
         * Let's ignore further instances of SIGINT.
         */
        signal (SIGINT, SIG_IGN);
        argux_scheduler_main_quit ();
        break;
    case SIGTERM:
        argux_scheduler_main_quit ();
        break;
    default:
        break;
    }
    return;
}

void
autoregister_host () {

    char *hostname = get_fqdn();
    void *host_ptr = NULL;
    ArguxError *error = NULL;

    if (strcmp(hostname, "localhost") == 0) {
        argux_log_warning (
                "Failed to register host, "
                "autoregister_host enabled, "
                "but fqdn resolves to localhost.");
        return;
    }

    _db_plugin->host.get(
            hostname,
            &host_ptr,
            &error); 
    if (host_ptr == NULL) {
        argux_log_info (
                "autoregister_host enabled, "
                "but host '%s' not found.",
                hostname);
        if (error) {
            argux_error_free(error);
            error = NULL;
        }
        _db_plugin->host.add (
            hostname,
            &error);
        if (error) {
            argux_log_error (
                    "%s",
                    argux_error_get_msg(error));
            argux_error_free (error);
            error = NULL;
        }
        _db_plugin->perm.host.set (
                "SYS",
                hostname,
                3,
                &error);
        if (error) {
            argux_log_error (
                    "%s",
                    argux_error_get_msg(error));
            argux_error_free (error);
            error = NULL;
        }
    }
}

/**
 * main
 * @argc: Number of elements in argv
 * @argv: Array of command-line options provided to the application
 *
 * @returns: Exit-code
 */
int
main (int argc, char **argv)
{
    int     option_index = 0;
    int     c = 0;
    int     verbosity = 0;
    int     log_level = 0;
    ArguxError *error = NULL;

    DIR    *plugin_dir = NULL;
    struct dirent *dirp;
    int     i = 0;

    int     n_workers = 2;

    int     port = 8888;

    char    plugin_path[1024];

    struct sigaction sa;

    struct passwd *_pwd_res = NULL;

    char *config_file = NULL;

    sa.sa_handler = process_signal;
    sa.sa_flags = SA_RESTART;
    sigemptyset (&sa.sa_mask);

    sigaction (SIGINT, &sa, NULL);
    sigaction (SIGTERM, &sa, NULL);

    ArguxSettings *settings = NULL;

    while (1)
    {
        c = getopt_long (argc, argv, "vVhc:",
                long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            switch (option_index)
            {
            case OPTION_VERSION:
                show_version ();
                exit (0);
                break;
            case OPTION_HELP:
                show_usage ();
                exit (0);
                break;
            case OPTION_FATAL_WARNINGS:
                argux_set_fatal_asserts (TRUE);
                break;
            case OPTION_INIT_DB:
                _init_db = 1;
                break;
            case OPTION_CONFIG:
                if (config_file == NULL) {
                    config_file = malloc (strlen(optarg)+1);
                    strcpy(config_file, optarg);
                }
                break;
            }
            break;
        case 'V':
            show_version ();
            exit (0);
            break;
        case 'h':
            show_usage ();
            exit (0);
            break;
        case 'v':
            verbosity = verbosity + 1;
            break;
        case 'c':
            if (config_file == NULL) {
                config_file = malloc (strlen(optarg)+1);
                strcpy(config_file, optarg);
            }
            break;
        default:
            fprintf (stderr, "Try '%s --help' for more information\n", PACKAGE_NAME);
            exit (1);
            break;
        }
    }

    if (verbosity == 0)
    {

#ifdef ENABLE_DEBUG
        argux_log_init (1);
#else
        argux_log_init (0);
#endif                          /* ENABLE_DEBUG */

        switch (log_level)
        {
        default:

#ifdef ENABLE_DEBUG
            argux_log_mask (ARGUX_LOG_DEBUG);
            argux_log_info ("Set logging level to 'DEBUG'");
#else
            argux_log_mask (log_level);
            argux_log_info ("Set logging level to '...'");
#endif                          /* ENABLE_DEBUG */
        }
    } else
    {
        argux_log_init (1);

        switch (verbosity)
        {
        case 1:
            argux_log_mask (ARGUX_LOG_INFO);
            argux_log_info ("Set logging level to 'INFO'");
            break;
        case 2:
            argux_log_mask (ARGUX_LOG_DEBUG);
            argux_log_info ("Set logging level to 'DEBUG'");
            break;
        }
    }

    /**
     * Initialise argux library
     */
    libargux_init ();

    /**
     * Load settings from file
     */
    if (config_file == NULL) {
        settings = argux_settings_load (
                SYSCONFDIR "/argux.conf",
                &error);
    }
    else {
        settings = argux_settings_load (
                config_file,
                &error);
    }
    if (settings == NULL) {
        if (error) {
            argux_log_error (
                    "%s",
                    argux_error_get_msg (error));
            argux_error_free (error);
        }
        exit(1);
    }

    const char *db_type = argux_settings_get (settings, "db_type");
    if (db_type == NULL) {
        argux_log_error ("DB-Type not specified in config-file");
        exit(1);
    }

    const char *http_port = argux_settings_get (settings, "http_port");
    if (http_port == NULL) {
        port = 8888; 
    } else {
        port = atoi(http_port);
    }

    const char *username = argux_settings_get (settings, "username");

    int euid = geteuid();
    if (euid == 0) {
        if (username == NULL) {
            argux_log_critical (
                "Cannot determine username of %s user.");

            exit(1);
        }

        if( getpwnam_r (
                username,
                &_pwd,
                _user_buf,
                USER_BUF_LEN,
                &_pwd_res)) {

            argux_log_critical (
                    "cannot determine uid for user %s",
                    username);
            exit(1);
        }
        if (_pwd_res == NULL) {
            argux_log_critical (
                    "cannot determine uid for user %s",
                    username);
            exit(1);
        }

        if (setuid(_pwd.pw_uid)) {
            argux_log_critical (
                    "Failed to execute setuid(%d)",
                    _pwd.pw_uid);
            exit(1);
        }

        argux_log_info (
                "%s is started by root, dropped privileges to %s",
                PACKAGE,
                username);
    }

    const char *plugin_d = argux_settings_get (settings, "plugin_dir");
    if (plugin_d == NULL || strlen(plugin_d) == 0) {
        plugin_d = PLUGINDIR;
    }


    /**
     * Maximum 10 items (Development value).
     */
    argux_items_init (10);

    /**
     * Load all plugins from PLUGIN_DIR
     */
    plugin_dir = opendir (plugin_d );
    if (plugin_dir != NULL)
    {
        while ((dirp = readdir (plugin_dir)) != NULL)
        {
            if (n_plugins == MAX_PLUGINS)
            {
                argux_log_warning (
                        "Max plugins reached (%d), "
                        "can not load more plugins.");
                break;
            }
            i = snprintf (plugin_path, 1024, "%s/%s", plugin_d, dirp->d_name);
            if (i < 0)
            {
                /* An error occurred */
            } else
            {
                if (i > 1023)
                {
                    /* Error, prevented buffer overflow... path too long */
                    argux_log_warning (
                            "Can not load plugin, "
                            "plugin-path exceeds 1024 bytes");
                } else
                {
                    /**
                     * Only load .so files, the rest can't be a plugin.
                     */
                    if (strcmp (&plugin_path[i - 3], ".so") == 0)
                    {
                        _plugins[n_plugins] = argux_plugin_load (
                                plugin_path,
                                &error);

                        if (_plugins[n_plugins] == NULL && error)
                        {
                            argux_log_warning (
                                    "%s",
                                    argux_error_get_msg (error));
                            argux_error_free (error);
                            error = NULL;
                        } else
                        {
                            argux_log_info (
                                    "Load plugin: %s\n",
                                    plugin_path);
                            //argux_plugin_register_types (plugin);
                            n_plugins++;
                        }
                    }
                }
            }
        }
    }

#ifdef ENABLE_DEBUG
    /** Do not allow further dynamic memory allocation */
    argux_set_allow_malloc (FALSE);
#endif                          /* ENABLE_DEBUG */

    for (i = 0; i < n_plugins; ++i)
    {
        if (_plugins[i]->type == ARGUX_PLUGIN_DB)
        {
            if (strcmp(
                    ((ArguxPluginDB *)_plugins[i])->db_name,
                    db_type) == 0)
            {
                if (_db_plugin != NULL) {
                    printf("Duplicate DB-Types\n");
                    exit(1);
                }

                _db_plugin = (ArguxPluginDB *)_plugins[i];
            }
        }
    }

    if (_db_plugin == NULL)
    {
        printf("Failed to initialize database '%s', no such plugin\n", db_type);
        exit(1);
    }

    /**
     * Set the properties to the plugin.
     */
    char **keys;
    int l = _db_plugin->db.listprop(&keys);
    for (i = 0; i < l; ++i) {
        const char *val = argux_settings_get (settings, keys[i]);
        if (val != NULL) {
            _db_plugin->db.setprop(keys[i], val);
        }
    }

    /**
     * Initialize a new database file.
     */
    if (_init_db != 0) {
        if (_db_plugin->db.init(&error) != 0)
        {
            printf("%s", argux_error_get_msg(error));
        }
        exit(0);
    }

    /* Open the Database */
    if(_db_plugin->db.connect(&error)) {
        argux_log_error("%s", argux_error_get_msg(error));
        exit(1);
    }

    /* Check if the host should be auto-rgistered */
    const char *reg_host = argux_settings_get (
            settings,
            "autoregister_host");
    if (reg_host != NULL) {

        if (strcmp(reg_host, "true") == 0) {
            autoregister_host();
        } else {
            argux_log_debug ("autoregister_host = '%s'", reg_host);
        }
    } else {
        argux_log_debug ("autoregister_host missing");
    }

    /** Start the main loop */
    argux_scheduler_main (port, n_workers);

    if(_db_plugin->db.disconnect(&error)) {
        argux_log_error("%s", argux_error_get_msg (error));
    }

    exit (0);
}
