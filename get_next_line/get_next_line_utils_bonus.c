/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils_bonus.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kaztakam <kaztakam@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by kaztakam          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by kaztakam         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"

/**
 * @brief Return the length of a string, 0 if NULL.
 * @param s The string to measure.
 * @return The length in bytes.
 */
size_t	ft_strlen(const char *s)
{
	size_t	i;

	i = 0;
	if (!s)
		return (0);
	while (s[i])
		i++;
	return (i);
}

/**
 * @brief Locate the first occurrence of c in s.
 * @param s The string to search.
 * @param c The character to find.
 * @return Pointer to the match, or NULL if not found.
 */
char	*ft_strchr(const char *s, int c)
{
	if (!s)
		return (NULL);
	while (*s)
	{
		if (*s == (char)c)
			return ((char *)s);
		s++;
	}
	if ((char)c == '\0')
		return ((char *)s);
	return (NULL);
}

/**
 * @brief Join s1 and s2 into a new string; frees s1.
 * @param s1 The first string (freed after join).
 * @param s2 The second string.
 * @return The joined string, or NULL on error.
 */
char	*strjoin(char *s1, char *s2)
{
	size_t	i;
	size_t	j;
	char	*joined;

	if (!s1 || !s2)
		return (NULL);
	joined = malloc(sizeof(char) * (ft_strlen(s1) + ft_strlen(s2) + 1));
	if (!joined)
	{
		free(s1);
		return (NULL);
	}
	i = 0;
	while (s1[i])
	{
		joined[i] = s1[i];
		i++;
	}
	j = 0;
	while (s2[j])
		joined[i++] = s2[j++];
	joined[i] = '\0';
	free(s1);
	return (joined);
}

/**
 * @brief Extract the first line from stash (up to newline).
 * @param stash The accumulated read buffer.
 * @return A new string containing the line, or NULL.
 */
char	*get_line(char *stash)
{
	size_t	i;
	char	*line;

	i = 0;
	if (!stash || !stash[0])
		return (NULL);
	while (stash[i] && stash[i] != '\n')
		i++;
	if (stash[i] == '\n')
		i++;
	line = malloc(sizeof(char) * (i + 1));
	if (!line)
		return (NULL);
	i = 0;
	while (stash[i] && stash[i] != '\n')
	{
		line[i] = stash[i];
		i++;
	}
	if (stash[i] == '\n')
		line[i++] = '\n';
	line[i] = '\0';
	return (line);
}

/**
 * @brief Remove the first line from stash, return the rest.
 * @param stash The accumulated read buffer (freed).
 * @return Remaining content after the first line, or NULL.
 */
char	*trim_stash(char *stash)
{
	size_t	i;
	size_t	j;
	char	*trimmed;

	i = 0;
	while (stash[i] && stash[i] != '\n')
		i++;
	if (!stash[i])
	{
		free(stash);
		return (NULL);
	}
	trimmed = malloc(sizeof(char) * (ft_strlen(stash) - i));
	if (!trimmed)
	{
		free(stash);
		return (NULL);
	}
	i++;
	j = 0;
	while (stash[i])
		trimmed[j++] = stash[i++];
	trimmed[j] = '\0';
	free(stash);
	return (trimmed);
}
