/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_unsigned.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kaztakam <kaztakam@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by kaztakam          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by kaztakam         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

/**
 * @brief Count the decimal digits of an unsigned integer.
 * @param n The unsigned integer to measure.
 * @return The number of digits.
 */
static int	unumlen(unsigned int n)
{
	int	len;

	len = 0;
	if (n == 0)
		return (1);
	while (n > 0)
	{
		len++;
		n /= 10;
	}
	return (len);
}

/**
 * @brief Recursively write an unsigned integer to stdout.
 * @param n The unsigned integer to print.
 * @return 0 on success, -1 on write error.
 */
static int	put_unsigned(unsigned int n)
{
	char	c;

	if (n >= 10)
	{
		if (put_unsigned(n / 10) == -1)
			return (-1);
	}
	c = (n % 10) + '0';
	if (write(1, &c, 1) == -1)
		return (-1);
	return (0);
}

/**
 * @brief Print an unsigned integer and return char count.
 * @param n The unsigned integer to print.
 * @return Number of characters printed, or -1 on error.
 */
int	print_unsigned(unsigned int n)
{
	if (put_unsigned(n) == -1)
		return (-1);
	return (unumlen(n));
}
