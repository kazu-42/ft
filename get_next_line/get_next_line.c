/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kaztakam <kaztakam@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by kaztakam          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by kaztakam         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

/**
 * @brief Read from fd into stash until newline or EOF.
 * @param fd The file descriptor to read from.
 * @param stash The current accumulated buffer.
 * @return Updated stash, or NULL on error.
 */
static char	*read_to_stash(int fd, char *stash)
{
	char	*buf;
	int		bytes_read;

	buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));
	if (!buf)
		return (NULL);
	bytes_read = 1;
	while (!ft_strchr(stash, '\n') && bytes_read > 0)
	{
		bytes_read = read(fd, buf, BUFFER_SIZE);
		if (bytes_read == -1)
		{
			free(buf);
			free(stash);
			return (NULL);
		}
		buf[bytes_read] = '\0';
		stash = strjoin(stash, buf);
	}
	free(buf);
	return (stash);
}

/**
 * @brief Initialise stash to an empty string if NULL.
 * @param stash The current stash pointer.
 * @return The initialised stash, or NULL on malloc failure.
 */
static char	*init_stash(char *stash)
{
	if (!stash)
	{
		stash = malloc(sizeof(char) * 1);
		if (!stash)
			return (NULL);
		stash[0] = '\0';
	}
	return (stash);
}

/**
 * @brief Return the next line from fd, including newline.
 * @param fd The file descriptor to read from.
 * @return The next line, or NULL on EOF / error.
 */
char	*get_next_line(int fd)
{
	static char	*stash;
	char		*line;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	stash = init_stash(stash);
	if (!stash)
		return (NULL);
	stash = read_to_stash(fd, stash);
	if (!stash)
		return (NULL);
	if (stash[0] == '\0')
	{
		free(stash);
		stash = NULL;
		return (NULL);
	}
	line = get_line(stash);
	stash = trim_stash(stash);
	return (line);
}
