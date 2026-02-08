/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_hex.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kaztakam <kaztakam@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by kaztakam          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by kaztakam         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

/**
 * @brief Count the number of hex digits of an unsigned int.
 * @param n The value to measure.
 * @return The number of hexadecimal digits.
 */
static int	hexlen(unsigned int n)
{
	int	len;

	len = 0;
	if (n == 0)
		return (1);
	while (n > 0)
	{
		len++;
		n /= 16;
	}
	return (len);
}

/**
 * @brief Recursively print an unsigned int in hexadecimal.
 * @param n The value to print.
 * @param uppercase If nonzero, use uppercase hex digits.
 * @return 0 on success, -1 on write error.
 */
static int	put_hex(unsigned int n, int uppercase)
{
	char	*hex;

	if (uppercase)
		hex = "0123456789ABCDEF";
	else
		hex = "0123456789abcdef";
	if (n >= 16)
	{
		if (put_hex(n / 16, uppercase) == -1)
			return (-1);
	}
	if (write(1, &hex[n % 16], 1) == -1)
		return (-1);
	return (0);
}

/**
 * @brief Print an unsigned int in hex and return char count.
 * @param n The value to print.
 * @param uppercase If nonzero, use uppercase hex digits.
 * @return Number of characters printed, or -1 on error.
 */
int	print_hex(unsigned int n, int uppercase)
{
	if (put_hex(n, uppercase) == -1)
		return (-1);
	return (hexlen(n));
}
