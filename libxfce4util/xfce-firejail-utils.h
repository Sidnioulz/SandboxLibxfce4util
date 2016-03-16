/*
 * Copyright (c) 2016 Steve Dodier-Lazaro <sidi@xfce.org>
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#if !defined(LIBXFCE4UTIL_INSIDE_LIBXFCE4UTIL_H) && !defined(LIBXFCE4UTIL_COMPILATION)
#error "Only <libxfce4util/libxfce4util.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __XFCE_FIREJAIL_UTILS_H__
#define __XFCE_FIREJAIL_UTILS_H__

#include <glib.h>
#include <firejail/exechelper.h>

G_BEGIN_DECLS

GList *    xfce_get_firejail_profile_names    (gboolean insert_generic);
gchar*     xfce_get_firejail_profile_for_name (const gchar *name);

G_END_DECLS

#endif /* !__XFCE_FIREJAIL_UTILS_H__ */
