/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_nbr.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kaztakam <kaztakam@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by kaztakam          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by kaztakam         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

/**
 * @brief Count the number of decimal digits including sign.
 * @param n The integer whose printed length to compute.
 * @return The number of characters needed to print n.
 */
static int	numlen(int n)
{
	int	len;

	len = 0;
	if (n <= 0)
		len = 1;
	while (n != 0)
	{
		len++;
		n /= 10;
	}
	return (len);
}

/**
 * @brief Recursively write a signed integer to stdout.
 * @param n The integer to print.
 * @return 0 on success, -1 on write error.
 */
static int	put_nbr(int n)
{
	char	c;

	if (n == -2147483648)
	{
		if (write(1, "-2147483648", 11) == -1)
			return (-1);
		return (0);
	}
	if (n < 0)
	{
		if (write(1, "-", 1) == -1)
			return (-1);
		n = -n;
	}
	if (n >= 10)
	{
		if (put_nbr(n / 10) == -1)
			return (-1);
	}
	c = (n % 10) + '0';
	if (write(1, &c, 1) == -1)
		return (-1);
	return (0);
}

/**
 * @brief Print a signed integer and return the char count.
 * @param n The integer to print.
 * @return Number of characters printed, or -1 on error.
 */
int	print_nbr(int n)
{
	if (put_nbr(n) == -1)
		return (-1);
	return (numlen(n));
}
