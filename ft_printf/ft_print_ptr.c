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

static int	ft_ptr_len(unsigned long long ptr)
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

static int	ft_put_ptr(unsigned long long ptr)
{
	char	*hex;
	int		ret;

	hex = "0123456789abcdef";
	if (ptr >= 16)
	{
		ret = ft_put_ptr(ptr / 16);
		if (ret == -1)
			return (-1);
		ret = ft_put_ptr(ptr % 16);
		if (ret == -1)
			return (-1);
	}
	else
	{
		if (write(1, &hex[ptr], 1) == -1)
			return (-1);
	}
	return (0);
}

int	ft_print_ptr(unsigned long long ptr)
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
	if (ft_put_ptr(ptr) == -1)
		return (-1);
	count += ft_ptr_len(ptr);
	return (count);
}
