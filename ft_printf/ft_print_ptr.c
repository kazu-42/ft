/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_ptr.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kaztakam <kaztakam@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by kaztakam          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by kaztakam         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

/**
 * @brief Count the number of hex digits in a pointer value.
 * @param ptr The pointer value.
 * @return Number of hexadecimal digits.
 */
static int	ptr_len(unsigned long long ptr)
{
	int	len;

	len = 0;
	if (ptr == 0)
		return (1);
	while (ptr > 0)
	{
		len++;
		ptr /= 16;
	}
	return (len);
}

/**
 * @brief Recursively print a pointer value in lowercase hex.
 * @param ptr The pointer value to print.
 * @return 0 on success, -1 on write error.
 */
static int	put_ptr(unsigned long long ptr)
{
	char	*hex;

	hex = "0123456789abcdef";
	if (ptr >= 16)
	{
		if (put_ptr(ptr / 16) == -1)
			return (-1);
	}
	if (write(1, &hex[ptr % 16], 1) == -1)
		return (-1);
	return (0);
}

/**
 * @brief Print a pointer with "0x" prefix, or "(nil)" if 0.
 * @param ptr The pointer value to print.
 * @return Total characters printed, or -1 on error.
 */
int	print_ptr(unsigned long long ptr)
{
	int	count;

	if (ptr == 0)
	{
		if (write(1, "(nil)", 5) == -1)
			return (-1);
		return (5);
	}
	if (write(1, "0x", 2) == -1)
		return (-1);
	count = 2;
	if (put_ptr(ptr) == -1)
		return (-1);
	count += ptr_len(ptr);
	return (count);
}
