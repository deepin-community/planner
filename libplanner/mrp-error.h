/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
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

#include <glib.h>

/**
 * MRP_ERROR:
 *
 * Error domain for libmrproject.
 */
#define MRP_ERROR mrp_error_quark ()

/**
 * MrpError:
 * @MRP_ERROR_TASK_RELATION_FAILED: task relation failed.
 * @MRP_ERROR_TASK_MOVE_FAILED: task move failed.
 * @MRP_ERROR_LOAD_FILE_DONT_EXIST: load file doesn't exist.
 * @MRP_ERROR_LOAD_FILE_INVALID: load file is invalid.
 * @MRP_ERROR_SAVE_FILE_EXIST: save file exists.
 * @MRP_ERROR_SAVE_FILE_CHANGED: save file changed.
 * @MRP_ERROR_EXPORT_UNSUPPORTED: export unsupported.
 * @MRP_ERROR_EXPORT_FAILED: export failed.
 * @MRP_ERROR_NO_FILE_MODULE: no file module.
 * @MRP_ERROR_SAVE_WRITE_FAILED: save write failed.
 * @MRP_ERROR_INVALID_URI: invalid URI.
 * @MRP_ERROR_FAILED: General error (should avoid using this).
 *
 * Error codes returned by libmrproject functions.
 */
typedef enum {
        MRP_ERROR_TASK_RELATION_FAILED,
        MRP_ERROR_TASK_MOVE_FAILED,

	MRP_ERROR_LOAD_FILE_DONT_EXIST,
	MRP_ERROR_LOAD_FILE_INVALID,
	MRP_ERROR_SAVE_FILE_EXIST,
	MRP_ERROR_SAVE_FILE_CHANGED,
	MRP_ERROR_EXPORT_UNSUPPORTED,
	MRP_ERROR_EXPORT_FAILED,
	MRP_ERROR_NO_FILE_MODULE,
	MRP_ERROR_SAVE_WRITE_FAILED,

	MRP_ERROR_INVALID_URI,

        MRP_ERROR_FAILED
} MrpError;

GQuark mrp_error_quark (void) G_GNUC_CONST;
