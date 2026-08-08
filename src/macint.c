/*
 *  Leafpad - GTK+ based simple text editor
 *  Copyright (C) 2004-2005 Tarot Osuji
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "leafpad.h"

#ifdef ENABLE_MAC_INTEGRATION

#include <gtkosxapplication.h>

static GtkosxApplication *osxapp = NULL;

/*
 *  Finder hands a double-clicked document to the app as an Apple Event rather
 *  than on argv, so it arrives here instead of through parse_args().
 */
static gboolean cb_open_file(GtkosxApplication *app, gchar *path, gpointer data)
{
	FileInfo *fi;

	if (check_text_modification())
		return TRUE;

	fi = g_malloc(sizeof(FileInfo));
	fi->filename = g_strdup(path);
	fi->charset = pub->fi->charset_flag ? g_strdup(pub->fi->charset) : NULL;
	fi->charset_flag = pub->fi->charset_flag;
	fi->lineend = LF;

	if (file_open_real(pub->mw->view, fi))
		g_free(fi);
	else {
		g_free(pub->fi);
		pub->fi = fi;
		undo_clear_all(pub->mw->buffer);
		set_main_window_title();
	}

	return TRUE;
}

/* Returning TRUE keeps the app alive, so Quit still offers to save. */
static gboolean cb_block_termination(GtkosxApplication *app, gpointer data)
{
	if (check_text_modification())
		return TRUE;

	save_config_file();

	return FALSE;
}

void macint_init(void)
{
	GtkWidget *item;

	osxapp = g_object_new(GTKOSX_TYPE_APPLICATION, NULL);

	gtkosx_application_set_menu_bar(osxapp,
		GTK_MENU_SHELL(pub->mw->menubar));

	/*
	 *  About and Quit belong in the application menu on macOS; Quit is
	 *  provided by the system, so the File entry is dropped rather than
	 *  moved.
	 */
	item = menu_get_widget("/MenuBar/Help/HelpAbout");
	if (item) {
		gtk_widget_hide(item);
		gtkosx_application_insert_app_menu_item(osxapp, item, 0);
	}
	item = menu_get_widget("/MenuBar/File/FileQuit");
	if (item)
		gtk_widget_hide(item);

	item = menu_get_widget("/MenuBar/Help");
	if (item)
		gtkosx_application_set_help_menu(osxapp, GTK_MENU_ITEM(item));

	g_signal_connect(osxapp, "NSApplicationOpenFile",
		G_CALLBACK(cb_open_file), NULL);
	g_signal_connect(osxapp, "NSApplicationBlockTermination",
		G_CALLBACK(cb_block_termination), NULL);
}

/*
 *  Called after gtk_widget_show_all(): the in-window menu bar has to be hidden
 *  again once it has been shown, or it stays visible under the system one.
 */
void macint_ready(void)
{
	gtk_widget_hide(pub->mw->menubar);
	gtkosx_application_ready(osxapp);
}

#endif /* ENABLE_MAC_INTEGRATION */
