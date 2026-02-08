/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_char.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kaztakam <kaztakam@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by kaztakam          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by kaztakam         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

/**
 * @brief Write a single character to stdout.
 * @param c The character to print.
 * @return 1 on success, -1 on write error.
 */
int	print_char(int c)
{
	unsigned char	ch;

	ch = (unsigned char)c;
	if (write(1, &ch, 1) == -1)
		return (-1);
	return (1);
}
