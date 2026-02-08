/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kaztakam <kaztakam@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by kaztakam          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by kaztakam         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

static int	ft_print_format(va_list args, char specifier)
{
	int	count;

	count = 0;
	if (specifier == 'c')
		count = ft_print_char(va_arg(args, int));
	else if (specifier == 's')
		count = ft_print_str(va_arg(args, char *));
	else if (specifier == 'p')
		count = ft_print_ptr(va_arg(args, unsigned long long));
	else if (specifier == 'd' || specifier == 'i')
		count = ft_print_nbr(va_arg(args, int));
	else if (specifier == 'u')
		count = ft_print_unsigned(va_arg(args, unsigned int));
	else if (specifier == 'x')
		count = ft_print_hex(va_arg(args, unsigned int), 0);
	else if (specifier == 'X')
		count = ft_print_hex(va_arg(args, unsigned int), 1);
	else if (specifier == '%')
		count = ft_print_char('%');
	return (count);
}

int	ft_printf(const char *format, ...)
{
	va_list	args;
	int		count;
	int		ret;

	if (!format)
		return (-1);
	va_start(args, format);
	count = 0;
	while (*format)
	{
		if (*format == '%')
		{
			format++;
			ret = ft_print_format(args, *format);
			if (ret == -1)
				return (-1);
			count += ret;
		}
		else
		{
			ret = ft_print_char(*format);
			if (ret == -1)
				return (-1);
			count += ret;
		}
		format++;
	}
	va_end(args);
	return (count);
}
