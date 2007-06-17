/* Hey EMACS -*- linux-c -*- */
/* $Id: ctree.c 3250 2007-04-03 15:50:21Z roms $ */

/*  TiLP - Tilp Is a Linking Program
 *  Copyright (C) 1999-2007  Romain Lievin
 *
 *  This program is free software you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "gui.h"
#include "support.h"
#include "ctree.h"
#include "tilibs.h"
#include "groups.h"
#include "file.h"

static GtkTreeStore *tree;

#define FONT_NAME "courier"

/* Initialization */

static gint column2index(GtkTreeViewColumn* column)
{
	gint i;

	for (i = 0; i < CTREE_NVCOLS; i++) 
	{
		GtkTreeViewColumn *col;

		col = gtk_tree_view_get_column(GTK_TREE_VIEW(gfm_widget.tree), i);
		if (col == column)
			return i;
	}

	return -1;
}

static gboolean select_func(GtkTreeSelection * selection,
			    GtkTreeModel * model,
			    GtkTreePath * path,
			    gboolean path_currently_selected,
			    gpointer data)
{
	GtkTreeIter iter;
	VarEntry *ve;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, COLUMN_DATA, &ve, -1);

	if (ve == NULL)
		return FALSE;

	if (ve->type == tifiles_folder_type(GFile.model))
		return FALSE;

	return TRUE;
}

static void tree_selection_changed(GtkTreeSelection * selection,
				   gpointer user_data)
{
	GList *list;
	GtkTreeIter iter;
	GtkTreeModel *model;

	// destroy selection
	g_list_free(gfm_widget.sel1);
	g_list_free(gfm_widget.sel2);

	// create a new selection
	for (list = gtk_tree_selection_get_selected_rows(selection, &model);
	     list != NULL; list = list->next) 
	{
		GtkTreePath *path = list->data;
		VarEntry *ve;
		
		gtk_tree_model_get_iter(model, &iter, path);
		gtk_tree_model_get(model, &iter, COLUMN_DATA, &ve, -1);

		if (ve->type != tifiles_flash_type(GFile.model)) 
		{
			gfm_widget.sel1 = g_list_append(gfm_widget.sel1, ve);
		} 
		else 
		{
			gfm_widget.sel2 = g_list_append(gfm_widget.sel2, ve);
		}
	}

	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);
}

static void renderer_edited(GtkCellRendererText * cell,
			    const gchar * path_string,
			    const gchar * new_text, gpointer user_data)
{
	GtkTreeModel *model = GTK_TREE_MODEL(tree);
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	const char *old_text;
	VarEntry *ve;

	if (!gtk_tree_model_get_iter(model, &iter, path))
		return;

	gtk_tree_model_get(model, &iter, COLUMN_NAME, &old_text, -1);
	gtk_tree_model_get(model, &iter, COLUMN_DATA, &ve, -1);

	strncpy(ve->name, new_text, 8);
	ve->name[8] = '\0';
	gtk_tree_store_set(tree, &iter, COLUMN_NAME, ve->name, -1);

	gtk_tree_path_free(path);
}

void ctree_init(void)
{
	GtkTreeView *view = GTK_TREE_VIEW(gfm_widget.tree);
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	gint i;

	tree = gtk_tree_store_new(CTREE_NCOLS, G_TYPE_STRING,
				  GDK_TYPE_PIXBUF, G_TYPE_STRING,
				  G_TYPE_STRING, G_TYPE_POINTER,
				  G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_INT);
	model = GTK_TREE_MODEL(tree);

	gtk_tree_view_set_model(view, model);
	gtk_tree_view_set_headers_visible(view, TRUE);
	gtk_tree_view_set_headers_clickable(view, TRUE);
	gtk_tree_view_set_rules_hint(view, FALSE);

	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column(view, column);
	gtk_tree_view_column_set_title(column, _("Name"));

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column),
					renderer, FALSE);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column),
					    renderer, "pixbuf",
					    COLUMN_ICON, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column),
					renderer, FALSE);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column),
					    renderer, "text", COLUMN_NAME,
						"font", COLUMN_FONT,
						"editable", COLUMN_EDIT,
					    NULL);
	g_signal_connect(G_OBJECT(renderer), "edited",	 G_CALLBACK(renderer_edited), NULL);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_insert_column_with_attributes(view, -1, _("Attr"),
						    renderer, "pixbuf",
						    COLUMN_ATTR, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(view, -1, _("Type"),
						    renderer, "text",
						    COLUMN_TYPE, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(view, -1, _("Size"),
						    renderer, "text",
						    COLUMN_SIZE, NULL);

	for (i = 0; i < CTREE_NVCOLS; i++) 
	{
		GtkTreeViewColumn *col;
		col = gtk_tree_view_get_column(view, i);
		gtk_tree_view_column_set_resizable(col, TRUE);
	}

	selection = gtk_tree_view_get_selection(view);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	gtk_tree_selection_set_select_function(selection, select_func, NULL, NULL);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(tree_selection_changed), NULL);

	ctree_set_basetree();
}

static GtkTreeIter vars_node;
static GtkTreeIter apps_node;

void ctree_set_basetree(void)
{
	GtkTreeIter *top_node = NULL;

	// clear tree
	gtk_tree_store_clear(tree);

	gtk_tree_store_append(tree, &vars_node, top_node);
	gtk_tree_store_set(tree, &vars_node, COLUMN_NAME, NODE3,
			   COLUMN_DATA, (gpointer) NULL, -1);

	if (tifiles_is_flash(GFile.model)) 
	{
		gtk_tree_store_append(tree, &apps_node, top_node);
		gtk_tree_store_set(tree, &apps_node, COLUMN_NAME, NODE4,
				   COLUMN_DATA, (gpointer) NULL, -1);
	}

	gtk_tree_view_expand_all(GTK_TREE_VIEW(gfm_widget.tree));
}

/* Management */

void ctree_refresh(void)
{
	GtkTreeView *view = GTK_TREE_VIEW(gfm_widget.tree);
	GdkPixbuf *pix1, *pix2, *pix3, *pix4, *pix5, *pix6, *pix7;
	GdkPixbuf *pix9 = NULL;
	GtkTreeIter pareng_node;
	GtkTreeIter child_node;
	GNode *vars, *apps;
	int i, j;

	if (GFile.trees.vars == NULL)
		return;

	// place base nodes
	ctree_set_basetree();
	memcpy(&pareng_node, &vars_node, sizeof(GtkTreeIter));

	// load pixmaps
	pix1 = create_pixbuf("ctree_close_dir.xpm");
	pix2 = create_pixbuf("TIicon2.ico");
	pix3 = create_pixbuf("ctree_open_dir.xpm");
	pix4 = create_pixbuf("attr_locked.xpm");
	pix5 = create_pixbuf("attr_archived.xpm");
	pix6 = create_pixbuf("TIicon4.ico");
	pix7 = create_pixbuf("attr_none.xpm");

	// variables tree
	vars = GFile.trees.vars;
	for (i = 0; i < (int)g_node_n_children(vars); i++) 
	{
		GNode *parent = g_node_nth_child(vars, i);
		VarEntry *fe = (VarEntry *) (parent->data);

		if ((fe != NULL) || tifiles_calc_is_ti9x(GFile.model))
		{
			char *utf8 = ticonv_varname_to_utf8(GFile.model, fe->name);

			gtk_tree_store_append(tree, &pareng_node, &vars_node);
			gtk_tree_store_set(tree, &pareng_node, 
					   COLUMN_NAME, utf8, 
					   COLUMN_DATA, (gpointer) fe,
					   COLUMN_ICON, pix1, 
					   COLUMN_EDIT, FALSE,
					   -1);
			g_free(utf8);
		}

		for (j = 0; j < (int)g_node_n_children(parent); j++) 
		{
			GNode *node = g_node_nth_child(parent, j);
			gchar **row_text = g_malloc0((CTREE_NCOLS + 1) * sizeof(gchar *));
			VarEntry *ve = (VarEntry *) (node->data);
			char icon_name[256];

			row_text[0] = ticonv_varname_to_utf8(GFile.model, ve->name);
			row_text[2] = g_strdup_printf("%s", tifiles_vartype2string(GFile.model, ve->type));
			tilp_var_get_size(ve, &row_text[3]);

			strcpy(icon_name, tifiles_vartype2icon(GFile.model, ve->type));
			strcat(icon_name, ".ico");
			tilp_file_underscorize(icon_name);
			pix9 = create_pixbuf(icon_name);

			// ticonv wrapper
			tilp_vars_translate(row_text[0]);

			gtk_tree_store_append(tree, &child_node, &pareng_node);
			gtk_tree_store_set(tree, &child_node, COLUMN_NAME,
					   row_text[0],
					   COLUMN_TYPE,
					   row_text[2], COLUMN_SIZE,
					   row_text[3], COLUMN_DATA,
					   (gpointer) ve, COLUMN_ICON, pix9,
					   COLUMN_FONT, FONT_NAME,
					   COLUMN_EDIT, TRUE,
					   -1);

			switch (ve->attr) 
			{
			case ATTRB_NONE:
				gtk_tree_store_set(tree, &child_node, COLUMN_ATTR, pix7, -1);
				break;
			case ATTRB_LOCKED:
				gtk_tree_store_set(tree, &child_node, COLUMN_ATTR, pix4, -1);
				break;
			case ATTRB_ARCHIVED:
				gtk_tree_store_set(tree, &child_node, COLUMN_ATTR, pix5, -1);
				break;
			default:
				break;
			}
			g_object_unref(pix9);
			g_strfreev(row_text);
		}
	}

	// Appplications tree
	apps = GFile.trees.apps;
	for (i = 0; i < (int)g_node_n_children(apps) && tifiles_is_flash(GFile.model); i++) 
	{
		GNode *parent = g_node_nth_child(apps, i);

		for (j = 0; j < (int)g_node_n_children(parent); j++) 
		{
			GNode *node = g_node_nth_child(parent, j);
			gchar **row_text = g_malloc0((CTREE_NCOLS + 1) * sizeof(gchar *));
			VarEntry *ve = (VarEntry *) (node->data);
			char icon_name[256];

			row_text[0] = ticonv_varname_to_utf8(GFile.model, ve->name);
			row_text[2] = g_strdup_printf("%s", tifiles_vartype2string(GFile.model, ve->type));
			row_text[3] = g_strdup_printf("%u", (int) (ve->size));

			strcpy(icon_name, tifiles_vartype2icon(GFile.model, ve->type));
			strcat(icon_name, ".ico");
			tilp_file_underscorize(icon_name);
			pix9 = create_pixbuf(icon_name);

			gtk_tree_store_append(tree, &child_node, &apps_node);
			gtk_tree_store_set(tree, &child_node, 
					COLUMN_NAME, row_text[0], 
					COLUMN_TYPE, row_text[2],
					COLUMN_SIZE, row_text[3], 
					COLUMN_DATA, (gpointer) ve, 
					COLUMN_ICON, pix9,
					COLUMN_FONT, FONT_NAME,
					   -1);
			g_object_unref(pix9);
			g_strfreev(row_text);
		}
	}
	gtk_tree_view_expand_all(GTK_TREE_VIEW(gfm_widget.tree));

	g_object_unref(pix1);
	g_object_unref(pix2);
	g_object_unref(pix3);
	g_object_unref(pix4);
	g_object_unref(pix5);
	g_object_unref(pix6);

	//tilp_remote_selection_destroy();
}

/* Callbacks */

GLADE_CB gboolean
on_treeview1_button_press_event(GtkWidget * widget,
				GdkEventButton * event, gpointer user_data)
{
	GtkTreeView *view = GTK_TREE_VIEW(gfm_widget.tree);
	GtkTreeModel *model = GTK_TREE_MODEL(tree);
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	GtkTreeIter parent;
	VarEntry *ve;
	int col;
	
	gtk_tree_view_get_cursor(view, &path, &column);
	col = column2index(column);

	if (path == NULL)
		return FALSE;

	gtk_tree_model_get_iter(model, &parent, path);
	gtk_tree_model_get(model, &parent, COLUMN_DATA, &ve, -1);

	if (ve == NULL)
		return FALSE;

	if((event->type == GDK_2BUTTON_PRESS) && (col == COLUMN_ATTR))
	{
		if(ve->attr == ATTRB_NONE)
			ve->attr = ATTRB_LOCKED;
		if(ve->attr == ATTRB_LOCKED)
			ve->attr = ATTRB_ARCHIVED;
		if(ve->attr == ATTRB_ARCHIVED)
			ve->attr = ATTRB_NONE;
		/*
		switch (ve->attr) 
			{
			case ATTRB_NONE:
				gtk_tree_store_set(tree, &child_node, COLUMN_ATTR, pix4, -1);
				break;
			case ATTRB_LOCKED:
				gtk_tree_store_set(tree, &child_node, COLUMN_ATTR, pix4, -1);
				break;
			case ATTRB_ARCHIVED:
				gtk_tree_store_set(tree, &child_node, COLUMN_ATTR, pix5, -1);
				break;
			default:
				break;
			}
			*/
		printf("attr clicked !\n");
	}

	return FALSE;
}

#include <gdk/gdkkeysyms.h>


/* Key pressed */
GLADE_CB gboolean
on_treeview1_key_press_event(GtkWidget* widget, GdkEventKey* event,
								gpointer user_data)
{	
	return FALSE;
}
