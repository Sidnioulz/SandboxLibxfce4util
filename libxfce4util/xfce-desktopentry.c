/* $Id$ */
/*-
 * Copyright (C) 2004 Jasper Huijsmans <jasper@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <glib.h>

#include <libxfce4util/xfce-desktopentry.h>

typedef struct
{
    char *key;
    char *value;
    char *translated_value;
    char *section;
}
entry_t;

struct _XfceDesktopEntryPrivate
{
    char *file;
    char *locale;

    entry_t *entries;
    int num_entries;
};

static void xfce_desktop_entry_class_init (XfceDesktopEntryClass * klass);

static void xfce_desktop_entry_init (XfceDesktopEntry * desktop_entry);

static void xfce_desktop_entry_finalize (GObject *object);

GObjectClass *parent_class = NULL;

#if TESTING /* set in header file */
static void
print_entry_info (entry_t *entry)
{
    g_print ("Key         : %s\n", entry->key);
    g_print ("Value       : %s\n", entry->value);
    if (entry->translated_value)
	g_print ("Translation : %s\n", entry->translated_value);
    if (entry->section)
	g_print ("Section     : %s\n", entry->section);
    g_print ("\n");
}

void
print_desktop_entry_info (XfceDesktopEntry *desktop_entry)
{
    entry_t *entry;
    int i;

    g_print ("[%s]\n", desktop_entry->priv->file);

    entry = &(desktop_entry->priv->entries[0]);

    for (i = 0; i < desktop_entry->priv->num_entries; ++i, entry++)
    {
	print_entry_info (entry);
    }
}
#endif /* TESTING */

GType
xfce_desktop_entry_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
	static const GTypeInfo type_info = {
	    sizeof (XfceDesktopEntryClass),
	    (GBaseInitFunc) NULL,
	    (GBaseFinalizeFunc) NULL,
	    (GClassInitFunc) xfce_desktop_entry_class_init,
	    (GClassFinalizeFunc) NULL,
	    NULL,
	    sizeof (XfceDesktopEntry),
	    0,			/* n_preallocs */
	    (GInstanceInitFunc) xfce_desktop_entry_init,
	};

	type = g_type_register_static (G_TYPE_OBJECT,
				       "XfceDesktopEntry", &type_info, 0);
    }

    return type;
}

static void
xfce_desktop_entry_class_init (XfceDesktopEntryClass * klass)
{
    parent_class = g_type_class_peek_parent (klass);

    G_OBJECT_CLASS (klass)->finalize = xfce_desktop_entry_finalize;
}

static void
xfce_desktop_entry_init (XfceDesktopEntry * desktop_entry)
{
    desktop_entry->priv = g_new0 (XfceDesktopEntryPrivate, 1);
}

static void
free_entry (entry_t * entry)
{
    g_free (entry->key);
    g_free (entry->value);
    g_free (entry->translated_value);
    g_free (entry->section);
}

static void
xfce_desktop_entry_finalize (GObject *object)
{
    XfceDesktopEntry * desktop_entry = XFCE_DESKTOP_ENTRY (object);
    XfceDesktopEntryPrivate *priv = desktop_entry->priv;
    int i;

    g_free (priv->file);

    for (i = 0; i < priv->num_entries; i++)
    {
	free_entry (&(priv->entries[i]));
    }

    g_free (priv->entries);
    g_free (priv);

    parent_class->finalize (G_OBJECT (desktop_entry));
}

G_CONST_RETURN char *
xfce_desktop_entry_get_file (XfceDesktopEntry * desktop_entry)
{
    g_return_val_if_fail (XFCE_IS_DESKTOP_ENTRY (desktop_entry), NULL);

    return desktop_entry->priv->file;
}

XfceDesktopEntry *
xfce_desktop_entry_new (const char *file, const char **categories,
			int num_categories)
{
    XfceDesktopEntry *desktop_entry;
    XfceDesktopEntryPrivate *priv;
    int i;
    entry_t *entry;

    g_return_val_if_fail (file != NULL, NULL);
    g_return_val_if_fail (categories != NULL, NULL);

    desktop_entry = g_object_new (XFCE_TYPE_DESKTOP_ENTRY, NULL);

    priv = desktop_entry->priv;

    priv->file = g_strdup (file);
    priv->entries = g_new0 (entry_t, num_categories);
    priv->num_entries = num_categories;

    entry = &(priv->entries[0]);

    for (i = 0; i < priv->num_entries; ++i, entry++)
    {
	entry->key = g_strdup (categories[i]);
    }

    return desktop_entry;
}

static gboolean
parse_desktop_entry_line (const char *line, char **section, 
			  char **key, char **value, char **locale)
{
    char *p, *q;

    p = (char *)line;
    
    /* initialize to NULL, so we don't have tho think about it anymore */
    *section = NULL;
    *key = NULL;
    *value = NULL;
    *locale = NULL;
    
    while (g_ascii_isspace (*p))
	++p;

    if (*p == '#' || *p == '\n' || *p == '\0')
	return FALSE;
    
    if (*p == '[')
    {
	++p;
	if ((q = strchr (p, ']')) == NULL)
	    return FALSE;

	*section = g_new (char, q - p + 1);
	strncpy (*section, p, q - p);
	(*section)[q - p] = '\0';
	
	return TRUE;
    }
    else
    {
	char *r;
	
	if ((q = strchr (p, '=')) == NULL)
	    return FALSE;
	
	r = q + 1;
	--q;

	while (g_ascii_isspace (*q))
	    --q;

	if (*q == ']')
	{
	    char *s;

	    if ((s = strchr (p, '[')) == NULL)
		return FALSE;
	    
	    *key = g_new (char, s - p + 1);
	    strncpy (*key, p, s - p);
	    (*key)[s - p] = '\0';

	    ++s;
	    *locale = g_new (char, q - s + 1);
	    strncpy (*locale, s, q - s);
	    (*locale)[q - s] = '\0';
	}
	else
	{
	    ++q;
	    *key = g_new (char, q - p + 1);
	    strncpy (*key, p, q - p);
	    (*key)[q - p] = '\0';
	}

	while (g_ascii_isspace (*r))
	    ++r;
	
	q = r + strlen (r);

	while (
	    q > r 
	&&
	    (
	        g_ascii_isspace (*(q-1)) 
	    || 
	        ((*(q-1)) == '\r')) /* kde... */
	)
	    --q;

	if (q > r) {
	    *value = g_new (char, q - r + 1);
	    strncpy (*value, r, q - r);
	    (*value) [q - r] = '\0';
	} else {
	    *value = g_new0 (char, 1);
	}
	
	return TRUE;
    }
}

/* @brief Match current locale against locale string.
 * 
 * @param current_locale : current locale value
 * @param locale	   : locale value to match against
 *
 * The locale is of the general form LANG_COUNTRY.ENCODING@MODIFIER.
 * Each of COUNTRY, ENCODING and MODIFIER can be absent.
 *
 * Matched by removing rightmost element one by one. This is not entirely
 * according to the freedesktop.org spec, but much easier. Should 
 * probably be fixed.
 *
 * @return : integer value ranging from 0 (no match) to 4 (full match).
 */
static int
match_locale (const char *current_locale, const char *locale)
{
    char *p;
    
    if (strcmp (current_locale, locale) == 0)
    {
	return 4;
    }

    if ((p = strchr (current_locale, '@')) != NULL &&
	strlen (locale) == p - current_locale &&
	strncmp (current_locale, locale, p - current_locale) == 0)
    {
	return 3;
    }
    
    if ((p = strchr (current_locale, '.')) != NULL &&
	strlen (locale) == p - current_locale &&
	strncmp (current_locale, locale, p - current_locale) == 0)
    {
	return 2;
    }
    
    if ((p = strchr (current_locale, '_')) != NULL &&
	strlen (locale) == p - current_locale &&
	strncmp (current_locale, locale, p - current_locale) == 0)
    {
	return 1;
    }

    return 0;
}

gboolean
xfce_desktop_entry_parse (XfceDesktopEntry * desktop_entry)
{
    char *current_locale;
    int locale_matched = 0;
    char *contents;
    char **lines, **p;
    char *current_section = NULL;
    gboolean result = FALSE;

    g_return_val_if_fail (XFCE_IS_DESKTOP_ENTRY (desktop_entry), FALSE);

    current_locale = g_strdup (setlocale (LC_MESSAGES, NULL));

    if (!g_file_get_contents (desktop_entry->priv->file, &contents, NULL, NULL))
	return FALSE;

    lines = g_strsplit (contents, "\n", -1);
    g_free (contents);
    
    for (p = lines; *p != NULL; ++p)
    {
	entry_t *entry;
	int i;
	char *section, *key, *value, *locale;
	
	if (!parse_desktop_entry_line (*p, &section, &key, &value, &locale))
	    continue;

	if (section != NULL)
	{
	    g_free (current_section);
	    current_section = section;
	    continue;
	}
	
	entry = &(desktop_entry->priv->entries[0]);

	for (i = 0; i < desktop_entry->priv->num_entries; ++i, ++entry)
	{
	    if (strcmp (key, entry->key) == 0)
	    {
		if (current_locale && locale)
		{
		    int match = match_locale (current_locale, locale);

		    if (match > locale_matched)
		    {
			g_free (entry->translated_value);
			entry->translated_value = g_strdup (value);
		    }
		}
		else
		{
		    g_free (entry->value);
		    entry->value = g_strdup (value);
		    result = TRUE;
		}

		if (current_section) {
            if (entry->section != NULL)
                g_free (entry->section);
		    entry->section = g_strdup (current_section);
        }
		
		break;
	    }
	}

	g_free (key);
	g_free (value);
    if (locale != NULL)
        g_free (locale);
    }

    if (current_locale != NULL) g_free (current_locale);
    g_free (current_section);
    g_strfreev (lines);

    return result;
}

static G_CONST_RETURN entry_t *
xfce_desktop_entry_get_entry (XfceDesktopEntry * desktop_entry,
			      const char *key)
{
    entry_t *entry;
    int i;

    entry = &(desktop_entry->priv->entries[0]);

    for (i = 0; i < desktop_entry->priv->num_entries; ++i, entry++)
    {
	if (strcmp (entry->key, key) == 0)
	{
	    return entry;
	}
    }

    return NULL;
}

gboolean
xfce_desktop_entry_get_string (XfceDesktopEntry * desktop_entry,
			       const char *key, gboolean translated,
			       char **value)
{
    const entry_t *entry;
    char *temp;

    g_return_val_if_fail (XFCE_IS_DESKTOP_ENTRY (desktop_entry), FALSE);
    g_return_val_if_fail (key != NULL, FALSE);
    g_return_val_if_fail (value != NULL, FALSE);

    if (!(entry = xfce_desktop_entry_get_entry (desktop_entry, key)))
	return FALSE;

    if (!entry->value || !strlen(entry->value))
	return FALSE;

    temp = entry->value;
    if (translated && entry->translated_value != NULL)
    {
	temp = entry->translated_value;
    }

    *value = g_strdup (temp);
    return TRUE;
}

gboolean
xfce_desktop_entry_get_int (XfceDesktopEntry * desktop_entry,
			    const char *key, int *value)
{
    const entry_t *entry;
    int temp;

    g_return_val_if_fail (XFCE_IS_DESKTOP_ENTRY (desktop_entry), FALSE);
    g_return_val_if_fail (key != NULL, FALSE);
    g_return_val_if_fail (value != NULL, FALSE);

    if (!(entry = xfce_desktop_entry_get_entry (desktop_entry, key)))
	return FALSE;

    if (!entry->value || !strlen(entry->value))
	return FALSE;

    temp = atoi (entry->value);

    if (temp >= 0)
    {
	*value = temp;
	return TRUE;
    }

    return FALSE;
}