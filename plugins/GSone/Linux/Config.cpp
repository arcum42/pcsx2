/*  GSOne
 *  Copyright (C) 2004-2018 PCSX2 Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <string>
//using namespace std;
#include <gtk/gtk.h>

#include "GS.h"
#include "Config.h"

extern std::string s_strIniPath;
PluginConf Ini;

void CreateDialog()
{
    GtkWidget *dialog;

    GtkWidget *log_check;

    GtkWidget *rend_label;
    GtkWidget *rend_combo;
    GtkWidget *rend_box;

    GtkWidget *main_box;
    GtkWidget *main_frame;

    int return_value = 0;

    /* Create the widgets */
    dialog = gtk_dialog_new_with_buttons(
        "GSOne Config",
        NULL, /* parent window*/
        (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        "OK", GTK_RESPONSE_ACCEPT,
        "Cancel", GTK_RESPONSE_REJECT,
        NULL);

    log_check = gtk_check_button_new_with_label("Logging");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(log_check), conf.Log);

    rend_label = gtk_label_new("Renderer:");
    rend_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rend_combo), "0 - Null");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rend_combo), "1 - OpenGL");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rend_combo), "2 - Vulkan");
    gtk_combo_box_set_active(GTK_COMBO_BOX(rend_combo), (int)conf.renderer);

#if GTK_MAJOR_VERSION < 3
    main_box = gtk_hbox_new(false, 5);
    rend_box = gtk_vbox_new(false, 2);
#else
    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    rend_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
#endif

    main_frame = gtk_frame_new("GSOne Config");
    gtk_container_add(GTK_CONTAINER(main_frame), main_box);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), main_frame);

    gtk_container_add(GTK_CONTAINER(rend_box), rend_label);
    gtk_container_add(GTK_CONTAINER(rend_box), rend_combo);

    gtk_container_add(GTK_CONTAINER(main_box), log_check);
    gtk_container_add(GTK_CONTAINER(main_box), rend_box);


    gtk_widget_show_all(dialog);


    return_value = gtk_dialog_run(GTK_DIALOG(dialog));

    if (return_value == GTK_RESPONSE_ACCEPT) 
    {
        conf.Log = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(log_check));
        if (gtk_combo_box_get_active(GTK_COMBO_BOX(rend_combo)) != -1)
            conf.renderer = (GS_ONE_RENDERER)gtk_combo_box_get_active(GTK_COMBO_BOX(rend_combo));
    }
    gtk_widget_destroy (dialog);
}
void CFGabout()
{
    SysMessage("GSOne: A simple plugin.");
}

void CFGconfigure()
{
    LoadConfig();
    //PluginNullConfigure("Since this is a null plugin, all that is really configurable is logging.", conf.Log);
    CreateDialog();
    SaveConfig();
}

void LoadConfig()
{
    const std::string iniFile(s_strIniPath + "/GSOne.ini");

    if (!Ini.Open(iniFile, READ_FILE)) {
        printf("failed to open %s\n", iniFile.c_str());
        SaveConfig(); //save and return
        return;
    }

    conf.Log = Ini.ReadInt("logging", 0);
    conf.renderer = (GS_ONE_RENDERER)Ini.ReadInt("Renderer", 0);
    Ini.Close();
}

void SaveConfig()
{
    const std::string iniFile(s_strIniPath + "/GSOne.ini");

    if (!Ini.Open(iniFile, WRITE_FILE)) {
        printf("failed to open %s\n", iniFile.c_str());
        return;
    }

    Ini.WriteInt("logging", conf.Log);
    Ini.WriteInt("Renderer", (int)conf.renderer);
    Ini.Close();
}
