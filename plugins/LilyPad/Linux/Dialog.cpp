/*  LilyPad - Pad plugin for PS2 Emulator
 *  Copyright (C) 2002-2017  PCSX2 Dev Team/ChickenLiver
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU Lesser General Public License as published by the Free
 *  Software Found- ation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with PCSX2.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Linux/Dialog.h"
#include <cstring>

#if defined(__unix__)
#include <gtk/gtk.h>

void SysMessage(const char *fmt, ...)
{
    va_list list;
    char msg[512];

    va_start(list, fmt);
    vsprintf(msg, fmt, list);
    va_end(list);

    if (msg[strlen(msg) - 1] == '\n')
        msg[strlen(msg) - 1] = 0;

    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL,
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_OK,
                                    "%s", msg);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

#endif

void LinuxAboutDialog()
{
    SysMessage("Lilypad is a gamepad plugin for pcsx2 by ChickenLiver, ported to Linux by gregory, arcum42, and the PCSX2 Dev Team.");
}

GtkWidget *Create_General_Tab()
{
    GtkWidget *tab_box;
    GtkWidget *label;
    GtkWidget *input_apis_frame, *pads_frame, *diagnostics_frame, *hacks_frame;
    GtkWidget *input_apis_box, *pads_box, *diagnostics_hacks_box;

    tab_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    label = gtk_label_new("I am a configuration dialog box.");


	input_apis_frame = gtk_frame_new("Input APIs");
	pads_frame = gtk_frame_new("Pads");
	diagnostics_frame = gtk_frame_new("Device Diagnostics");
	hacks_frame = gtk_frame_new("Hacks and Advanced Features");

    input_apis_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    pads_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    diagnostics_hacks_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    gtk_box_pack_start (GTK_BOX (input_apis_box), input_apis_frame, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (pads_box), pads_frame, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (diagnostics_hacks_box), diagnostics_frame, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (diagnostics_hacks_box), hacks_frame, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(tab_box), input_apis_box);
    gtk_container_add(GTK_CONTAINER(tab_box), pads_box);
    gtk_container_add(GTK_CONTAINER(tab_box), diagnostics_hacks_box);

    gtk_container_add(GTK_CONTAINER(input_apis_frame), gtk_label_new("Input APIs here!"));
    gtk_container_add(GTK_CONTAINER(pads_frame), gtk_label_new("Pads here!"));
    gtk_container_add(GTK_CONTAINER(diagnostics_frame), gtk_label_new("Diagnostics here!"));
    gtk_container_add(GTK_CONTAINER(hacks_frame), gtk_label_new("Hacks here!"));

    return tab_box;
}

GtkWidget *Create_Controller_1_Tab()
{
    GtkWidget *tab_box;
    GtkWidget *label;

    tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    label = gtk_label_new("I am the second page of a configuration dialog box.");

    gtk_box_pack_start (GTK_BOX (tab_box), label, TRUE, FALSE, 0);

    return tab_box;
}

GtkWidget *Create_Controller_2_Tab()
{
    GtkWidget *tab_box;
    GtkWidget *label;

    tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    label = gtk_label_new("I am the third page of a configuration dialog box.");

    gtk_box_pack_start (GTK_BOX (tab_box), label, TRUE, FALSE, 0);

    return tab_box;
}

void ShowLinuxConfigDialog()
{
    GtkWidget *dialog;
    GtkWidget *main_box, *main_frame;
    GtkWidget *notebook;
    GtkWidget *general_tab_frame, *cont1_tab_frame, *cont2_tab_frame;

    int result = 0;

    dialog = gtk_dialog_new_with_buttons ("Lilypad",
                                        NULL,
                                        (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                        "_Cancel",
                                        GTK_RESPONSE_REJECT,
                                        "_OK",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);

    // Note: Currently just coding for Gtk 3. I'll worry about Gtk 2 later.
    notebook = gtk_notebook_new();
    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	main_frame = gtk_frame_new("Lilypad Config");
    general_tab_frame = Create_General_Tab();
    cont1_tab_frame = Create_Controller_1_Tab();
    cont2_tab_frame = Create_Controller_2_Tab();

    result = gtk_notebook_append_page_menu (GTK_NOTEBOOK(notebook), general_tab_frame, gtk_label_new("General"), NULL);
    result = gtk_notebook_append_page_menu (GTK_NOTEBOOK(notebook), cont1_tab_frame, gtk_label_new("Controller 1"), NULL);
    result = gtk_notebook_append_page_menu (GTK_NOTEBOOK(notebook), cont2_tab_frame, gtk_label_new("Controller 2"), NULL);

    gtk_box_pack_start (GTK_BOX (main_box), notebook, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(main_frame), main_box);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), main_frame);

    gtk_widget_show_all(dialog);
    result = gtk_dialog_run (GTK_DIALOG (dialog));
    switch (result)
    {
        case GTK_RESPONSE_ACCEPT:
        // do_application_specific_something ();
        break;
        default:
        // do_nothing_since_dialog_was_cancelled ();
        break;
    }
    gtk_widget_destroy (dialog);
}