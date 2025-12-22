/* 
 * This file is part of jsonfs.
 * jsonfs - File system for working with JSON.
 *
 * Copyright (C) 2025 Egorov Konstantin
 *
 * jsonfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * jsonfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with jsonfs. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief It contains the file_time structure and 
 *        declarations of functions for working with it.
 */

#ifndef FILE_TIME_H_SENTRY
#define FILE_TIME_H_SENTRY

/* ================================= */
/*               Types               */
/* ================================= */

/**
 * @struct file_time
 * @brief File time metadata structure.
 */
struct file_time {
    char *path;                 /**< File/directory path */
    time_t atime;               /**< Last access time */
    time_t mtime;               /**< Last modification time */
    time_t ctime;               /**< Last status change time */
    struct file_time *next_node;/**< Next node in singly-linked list */
};

/**
 * @enum set_time
 * @see add_node_to_list_ft 
 */
enum set_time {
	SET_ATIME = 1,
	SET_MTIME = 2,
	SET_CTIME = 4
};

/* ================================= */
/*            Declarations           */
/* ================================= */

/**
 * @brief Adds a new file time node to the linked list.
 *
 * Creates a new file_time node with the specified path and time flags.
 * If the node already exists in the list, returns NULL.
 * If the list is empty (root is NULL), creates and returns the first node.
 *
 * @param path File path for the new node (must not be NULL).
 * @param root Head of the file_time linked list (can be NULL for empty list).
 * @param flags Bitmask specifying which time fields to set to current time.
 *              See enum set_time for available flags.
 *
 * @return Pointer to the newly created node on success,
 *         NULL if node already exists or on allocation failure.
 */
struct file_time *add_node_to_list_ft(const char* path, struct file_time *root, 
									  enum set_time flags);
/**
 * @brief Removes a node from the list.
 * 
 * @param path File path for the node being deleted (must not be NULL).
 * @param root Head of the file_time linked list (must not be NULL).
 * 
 * @return 0 on success, -1 on failure.
 */
int remove_node_to_list_ft(const char* path, struct file_time *root);

/**
 * @brief Finds a file_time node by path in the linked list.
 *
 * Searches through the file_time linked list for a node with matching path.
 *
 * @param path File path to search for (must not be NULL).
 * @param root Head of the file_time linked list to search (must not be NULL).
 *
 * @return Pointer to the found file_time node, or NULL if not found.
 */
struct file_time *find_node_file_time(const char *path, struct file_time *root);

/**
 * @brief Free the struct file_time node in the list. 
 * @param ft The node that will be free. 
 */
void free_file_time(struct file_time *ft);

#endif /* FILE_TIME_H_SENTRY */
