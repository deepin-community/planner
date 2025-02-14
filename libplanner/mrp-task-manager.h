/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2004 Imendio AB
 * Copyright (C) 2001-2002 CodeFactory AB
 * Copyright (C) 2001-2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2001-2002 Mikael Hallendal <micke@imendio.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include <glib-object.h>
#include <libplanner/mrp-project.h>
#include <libplanner/mrp-task.h>

G_BEGIN_DECLS

#define MRP_TYPE_TASK_MANAGER         (mrp_task_manager_get_type ())

G_DECLARE_FINAL_TYPE (MrpTaskManager, mrp_task_manager, MRP, TASK_MANAGER, GObject)

MrpTaskManager *mrp_task_manager_new                        (MrpProject           *project);
GList          *mrp_task_manager_get_all_tasks              (MrpTaskManager       *manager);
void            mrp_task_manager_insert_task                (MrpTaskManager       *manager,
                                                             MrpTask              *parent,
                                                             gint                  position,
                                                             MrpTask              *task);
void            mrp_task_manager_remove_task                (MrpTaskManager       *manager,
                                                             MrpTask              *task);
gboolean        mrp_task_manager_move_task                  (MrpTaskManager       *manager,
                                                             MrpTask              *task,
                                                             MrpTask              *sibling,
                                                             MrpTask              *parent,
                                                             gboolean              before,
                                                             GError              **error);
void            mrp_task_manager_set_root                   (MrpTaskManager       *manager,
                                                             MrpTask              *task);
MrpTask        *mrp_task_manager_get_root                   (MrpTaskManager       *manager);
void            mrp_task_manager_traverse                   (MrpTaskManager       *manager,
                                                             MrpTask              *root,
                                                             MrpTaskTraverseFunc   func,
                                                             gpointer              user_data);
void            mrp_task_manager_set_block_scheduling       (MrpTaskManager       *manager,
                                                             gboolean              block);
gboolean        mrp_task_manager_get_block_scheduling       (MrpTaskManager       *manager);
void            mrp_task_manager_rebuild                    (MrpTaskManager       *manager);
void            mrp_task_manager_recalc                     (MrpTaskManager       *manager,
                                                             gboolean              force);
gint            mrp_task_manager_calculate_task_work        (MrpTaskManager       *manager,
                                                             MrpTask              *task,
                                                             mrptime               start,
                                                             mrptime               finish);
gint            mrp_task_manager_calculate_summary_duration (MrpTaskManager       *manager,
                                                             MrpTask              *task,
                                                             mrptime               start,
                                                             mrptime               finish);
void            mrp_task_manager_dump_task_tree             (MrpTaskManager       *manager);
void            mrp_task_manager_dump_task_list             (MrpTaskManager       *manager);

G_END_DECLS
