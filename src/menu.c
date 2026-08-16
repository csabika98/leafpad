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
#include <gdk/gdkkeysyms.h>
#include <string.h>

static GtkWidget *menu_item_save;
static GtkWidget *menu_item_cut;
static GtkWidget *menu_item_copy;
static GtkWidget *menu_item_paste;
static GtkWidget *menu_item_delete;

static GtkUIManager *ui_manager = NULL;

/*
 *  Action labels are kept as the old GtkItemFactory paths ("/File/_New") so
 *  that the existing message catalogs keep matching; menu_translate() drops
 *  everything up to the last '/' of the *translated* string, which is where
 *  the label proper begins.
 */
static GtkActionEntry action_entries[] =
{
	{ "File",   NULL, N_("/_File") },
	{ "FileNew",    GTK_STOCK_NEW,     N_("/File/_New"),       "<Primary>N",
		NULL, G_CALLBACK(on_file_new) },
	{ "FileOpen",   GTK_STOCK_OPEN,    N_("/File/_Open..."),   "<Primary>O",
		NULL, G_CALLBACK(on_file_open) },
	{ "FileSave",   GTK_STOCK_SAVE,    N_("/File/_Save"),      "<Primary>S",
		NULL, G_CALLBACK(on_file_save) },
	{ "FileSaveAs", GTK_STOCK_SAVE_AS, N_("/File/Save _As..."), "<shift><Primary>S",
		NULL, G_CALLBACK(on_file_save_as) },
#ifdef ENABLE_PRINT
	{ "FilePrintPreview", GTK_STOCK_PRINT_PREVIEW, N_("/File/Print Pre_view"),
		"<shift><Primary>P", NULL, G_CALLBACK(on_file_print_preview) },
	{ "FilePrint",  GTK_STOCK_PRINT,   N_("/File/_Print..."),  "<Primary>P",
		NULL, G_CALLBACK(on_file_print) },
#endif
	{ "FileQuit",   GTK_STOCK_QUIT,    N_("/File/_Quit"),      "<Primary>Q",
		NULL, G_CALLBACK(on_file_quit) },

	{ "Edit",   NULL, N_("/_Edit") },
	{ "EditUndo",   GTK_STOCK_UNDO,    N_("/Edit/_Undo"),      "<Primary>Z",
		NULL, G_CALLBACK(on_edit_undo) },
	{ "EditRedo",   GTK_STOCK_REDO,    N_("/Edit/_Redo"),      "<shift><Primary>Z",
		NULL, G_CALLBACK(on_edit_redo) },
	{ "EditCut",    GTK_STOCK_CUT,     N_("/Edit/Cu_t"),       "<Primary>X",
		NULL, G_CALLBACK(on_edit_cut) },
	{ "EditCopy",   GTK_STOCK_COPY,    N_("/Edit/_Copy"),      "<Primary>C",
		NULL, G_CALLBACK(on_edit_copy) },
	{ "EditPaste",  GTK_STOCK_PASTE,   N_("/Edit/_Paste"),     "<Primary>V",
		NULL, G_CALLBACK(on_edit_paste) },
	{ "EditDelete", GTK_STOCK_DELETE,  N_("/Edit/_Delete"),    NULL,
		NULL, G_CALLBACK(on_edit_delete) },
	{ "EditSelectAll", NULL,           N_("/Edit/Select _All"), "<Primary>A",
		NULL, G_CALLBACK(on_edit_select_all) },

	{ "Search", NULL, N_("/_Search") },
	{ "SearchFind", GTK_STOCK_FIND,    N_("/Search/_Find..."), "<Primary>F",
		NULL, G_CALLBACK(on_search_find) },
	{ "SearchFindNext", NULL,          N_("/Search/Find _Next"), "<Primary>G",
		NULL, G_CALLBACK(on_search_find_next) },
	{ "SearchFindPrevious", NULL,      N_("/Search/Find _Previous"), "<shift><Primary>G",
		NULL, G_CALLBACK(on_search_find_previous) },
	{ "SearchReplace", GTK_STOCK_FIND_AND_REPLACE, N_("/Search/_Replace..."), "<Primary>H",
		NULL, G_CALLBACK(on_search_replace) },
	{ "SearchJumpTo", GTK_STOCK_JUMP_TO, N_("/Search/_Jump To..."), "<Primary>J",
		NULL, G_CALLBACK(on_search_jump_to) },

	{ "Options", NULL, N_("/_Options") },
	{ "OptionsFont", GTK_STOCK_SELECT_FONT, N_("/Options/_Font..."), NULL,
		NULL, G_CALLBACK(on_option_font) },

	{ "Help",   NULL, N_("/_Help") },
	{ "HelpAbout",  GTK_STOCK_ABOUT,   N_("/Help/_About"),     NULL,
		NULL, G_CALLBACK(on_help_about) },
};

static GtkToggleActionEntry toggle_entries[] =
{
	{ "OptionsWordWrap", NULL,    N_("/Options/_Word Wrap"),    NULL,
		NULL, G_CALLBACK(on_option_word_wrap),    FALSE },
	{ "OptionsLineNumbers", NULL, N_("/Options/_Line Numbers"), NULL,
		NULL, G_CALLBACK(on_option_line_numbers), FALSE },
	{ "OptionsAutoIndent", NULL,  N_("/Options/_Auto Indent"),  NULL,
		NULL, G_CALLBACK(on_option_auto_indent),  FALSE },
};

static const gchar *ui_description =
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu action='File'>"
"      <menuitem action='FileNew'/>"
"      <menuitem action='FileOpen'/>"
"      <menuitem action='FileSave'/>"
"      <menuitem action='FileSaveAs'/>"
"      <separator/>"
#ifdef ENABLE_PRINT
"      <menuitem action='FilePrintPreview'/>"
"      <menuitem action='FilePrint'/>"
"      <separator/>"
#endif
"      <menuitem action='FileQuit'/>"
"    </menu>"
"    <menu action='Edit'>"
"      <menuitem action='EditUndo'/>"
"      <menuitem action='EditRedo'/>"
"      <separator/>"
"      <menuitem action='EditCut'/>"
"      <menuitem action='EditCopy'/>"
"      <menuitem action='EditPaste'/>"
"      <menuitem action='EditDelete'/>"
"      <separator/>"
"      <menuitem action='EditSelectAll'/>"
"    </menu>"
"    <menu action='Search'>"
"      <menuitem action='SearchFind'/>"
"      <menuitem action='SearchFindNext'/>"
"      <menuitem action='SearchFindPrevious'/>"
"      <menuitem action='SearchReplace'/>"
"      <separator/>"
"      <menuitem action='SearchJumpTo'/>"
"    </menu>"
"    <menu action='Options'>"
"      <menuitem action='OptionsFont'/>"
"      <menuitem action='OptionsWordWrap'/>"
"      <menuitem action='OptionsLineNumbers'/>"
"      <separator/>"
"      <menuitem action='OptionsAutoIndent'/>"
"    </menu>"
"    <menu action='Help'>"
"      <menuitem action='HelpAbout'/>"
"    </menu>"
"  </menubar>"
"</ui>";

static gchar *menu_translate(const gchar *path, gpointer data)
{
	gchar *str;
	gchar *sep;

	str = (gchar *)_(path);
	sep = strrchr(str, '/');

	return sep ? sep + 1 : str;
}

/*
 *  Look up a menu proxy widget by its GtkUIManager path, e.g.
 *  "/MenuBar/Search/SearchFindNext". Replaces the old
 *  gtk_item_factory_get_widget()/get_item() pair.
 */
/*
 *  The accelerators below are described the same way as the action table, so
 *  that "<Primary>" resolves identically -- Command on the Quartz backend,
 *  Control elsewhere. Building the modifier by hand with
 *  gtk_widget_get_modifier_mask() is not equivalent: on an unrealized window
 *  it yields 0, which silently registers the accelerator with no modifier at
 *  all and swallows the bare keystroke.
 */
static void connect_accel_closure(GtkAccelGroup *accel_group,
	const gchar *accel, GCallback callback)
{
	guint key;
	GdkModifierType mods;

	gtk_accelerator_parse(accel, &key, &mods);
	g_return_if_fail(key != 0 && mods != 0);

	gtk_accel_group_connect(accel_group, key, mods, 0,
		g_cclosure_new_swap(callback, NULL, NULL));
}

static void add_widget_accel(GtkWidget *widget,
	GtkAccelGroup *accel_group, const gchar *accel)
{
	guint key;
	GdkModifierType mods;

	gtk_accelerator_parse(accel, &key, &mods);
	g_return_if_fail(key != 0);

	gtk_widget_add_accelerator(widget, "activate", accel_group, key, mods, 0);
}

GtkWidget *menu_get_widget(const gchar *path)
{
	GtkWidget *widget;

	g_return_val_if_fail(ui_manager != NULL, NULL);

	widget = gtk_ui_manager_get_widget(ui_manager, path);
	if (!widget)
		g_warning("menu_get_widget: no widget at '%s'", path);

	return widget;
}

void menu_sensitivity_from_modified_flag(gboolean is_text_modified)
{
	gtk_widget_set_sensitive(menu_item_save,   is_text_modified);
}

void menu_sensitivity_from_selection_bound(gboolean is_bound_exist)
{
	gtk_widget_set_sensitive(menu_item_cut,    is_bound_exist);
	gtk_widget_set_sensitive(menu_item_copy,   is_bound_exist);
	gtk_widget_set_sensitive(menu_item_delete, is_bound_exist);
}

//void menu_sensitivity_from_clipboard(gboolean is_clipboard_exist)
void menu_sensitivity_from_clipboard(void)
{
//g_print("clip board checked.\n");
	gtk_widget_set_sensitive(menu_item_paste,
		gtk_clipboard_wait_is_text_available(
			gtk_clipboard_get(GDK_SELECTION_CLIPBOARD)));
}

GtkWidget *create_menu_bar(GtkWidget *window)
{
	GtkAccelGroup *accel_group;
	GtkActionGroup *action_group;
	GError *error = NULL;

	action_group = gtk_action_group_new("LeafpadActions");
	gtk_action_group_set_translate_func(action_group, menu_translate, NULL, NULL);
	gtk_action_group_add_actions(action_group,
		action_entries, G_N_ELEMENTS(action_entries), NULL);
	gtk_action_group_add_toggle_actions(action_group,
		toggle_entries, G_N_ELEMENTS(toggle_entries), NULL);

	ui_manager = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);
	if (!gtk_ui_manager_add_ui_from_string(ui_manager, ui_description, -1, &error)) {
		g_error("building menus failed: %s", error->message);
		g_error_free(error);
	}

	accel_group = gtk_ui_manager_get_accel_group(ui_manager);
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	/* hidden keybinds */
	connect_accel_closure(accel_group, "<Primary>w",
		G_CALLBACK(on_file_close));
	connect_accel_closure(accel_group, "<Primary>t",
		G_CALLBACK(on_option_always_on_top));
	add_widget_accel(menu_get_widget("/MenuBar/Edit/EditRedo"),
		accel_group, "<Primary>y");
	add_widget_accel(menu_get_widget("/MenuBar/Search/SearchFindNext"),
		accel_group, "F3");
	add_widget_accel(menu_get_widget("/MenuBar/Search/SearchFindPrevious"),
		accel_group, "<Shift>F3");
	add_widget_accel(menu_get_widget("/MenuBar/Search/SearchReplace"),
		accel_group, "<Primary>r");

	/* initialize sensitivities */
	gtk_widget_set_sensitive(
		menu_get_widget("/MenuBar/Search/SearchFindNext"),
		FALSE);
	gtk_widget_set_sensitive(
		menu_get_widget("/MenuBar/Search/SearchFindPrevious"),
		FALSE);

	menu_item_save   = menu_get_widget("/MenuBar/File/FileSave");
	menu_item_cut    = menu_get_widget("/MenuBar/Edit/EditCut");
	menu_item_copy   = menu_get_widget("/MenuBar/Edit/EditCopy");
	menu_item_paste  = menu_get_widget("/MenuBar/Edit/EditPaste");
	menu_item_delete = menu_get_widget("/MenuBar/Edit/EditDelete");
	menu_sensitivity_from_selection_bound(FALSE);

	return menu_get_widget("/MenuBar");
}
