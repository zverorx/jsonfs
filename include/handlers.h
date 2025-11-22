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
 * @brief JSONFS filesystem logic interface.
 *
 * Contains declarations of functions from handlers.c and specifications for them.
 */

#ifndef HANDLERS_H_SENTRY
#define HANDLERS_H_SENTRY

/**
 * @brief Sets attributes for special files. Used in getattr.
 * 
 * @param path The absolute path to the special file.
 * @param stat Structure to fill with file attributes.
 * @param pd Private filesystem data from FUSE context. 
 * @return return 0 if success, negative error code otherwise.
 * 
 * @see is_special_file()
 */
int getattr_special_file(const char *path, struct stat *st,
						struct jsonfs_private_data *pd);

/**
 * @brief Sets attributes for JSON files and directories. 
 * 
 * @param path Absolute path, must not be NULL. 
 * @param stat Structure to fill with file attributes.
 * @param pd Private filesystem data from FUSE context. 
 * @return return 0 if success, negative error code otherwize.
 */
int getattr_json_file(const char *path, struct stat *st,
						struct jsonfs_private_data *pd);

/**
 * @brief Reads content from special filesystem control files.
 * 
 * Implements read operation for virtual special files:
 * - /.status: displays filesystem status information 
 * - /.save: indicates whether the changes have been saved. 
 * 
 * @param path The absolute path to the special file.
 * @param buffer Buffer provided by FUSE for storing read data.
 * @param size Maximum number of bytes to read. 
 * @param offset Byte offset from which to start reading. 
 * @param pd Private filesystem data from FUSE context. 
 * 
 * @return Number of bytes read on success, negative error code on failure.
 */
int read_special_file(const char *path, char *buffer, size_t size,
					  off_t offset, struct jsonfs_private_data *pd);

/**
 * @brief Reads content from JSON file.
 * 
 * @param path The absolute path to the JSON node.
 * @param buffer Buffer provided by FUSE for storing read data.
 * @param size Maximum number of bytes to read.
 * @param offset Byte offset from which to start reading.
 * @param pd Private filesystem data from FUSE context.
 * 
 * @return Number of bytes read on success, negative error code on failure.
 */
int read_json_file(const char *path, char *buffer, size_t size,
				   off_t offset, struct jsonfs_private_data *pd);

/**
 * @brief Writes data to special filesystem control files.
 * 
 * @param path The absolute path to the special file.
 * @param buffer Buffer containing data to write.
 * @param size Number of bytes to write.
 * @param offset Byte offset where to start writing.
 * @param pd Private filesystem data from FUSE context.
 * 
 * @return Number of bytes written on success, negative error code on failure.
 */
int write_special_file(const char *path, const char *buffer, size_t size,
					   off_t offset, struct jsonfs_private_data *pd);

/**
 * @brief Writes data to JSON file nodes.
 * 
 * @param path The absolute path to the JSON node.
 * @param buffer Buffer containing data to write.
 * @param size Number of bytes to write.
 * @param offset Byte offset where to start writing.
 * @param pd Private filesystem data from FUSE context.
 * 
 * @return Number of bytes written on success, negative error code on failure.
 */					   
int write_json_file(const char *path, const char *buffer, size_t size,
					off_t offset, struct jsonfs_private_data *pd);
/**
 * @brief Deleting files in the file system.
 *
 * @param path Absolute path, must not be NULL. 
 * @param file_type Constants S_IFREG or S_IFDIR.
 * @param pd Private filesystem data from FUSE context.
 * 
 * @return 0 on success, negative error code on failure.
 */
int rm_file(const char *path, int file_type, struct jsonfs_private_data *pd);

#endif /* HANDLERS_H_SENTRY */