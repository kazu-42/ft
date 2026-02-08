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

/**
 * @brief Dispatch a format specifier to the matching printer.
 * @param args The va_list of variadic arguments.
 * @param specifier The format character after '%'.
 * @return Number of characters printed, or -1 on error.
 */
static int	print_format(va_list args, char specifier)
{
	int	count;

	count = 0;
	if (specifier == 'c')
		count = print_char(va_arg(args, int));
	else if (specifier == 's')
		count = print_str(va_arg(args, char *));
	else if (specifier == 'p')
		count = print_ptr(va_arg(args, unsigned long long));
	else if (specifier == 'd' || specifier == 'i')
		count = print_nbr(va_arg(args, int));
	else if (specifier == 'u')
		count = print_unsigned(va_arg(args, unsigned int));
	else if (specifier == 'x')
		count = print_hex(va_arg(args, unsigned int), 0);
	else if (specifier == 'X')
		count = print_hex(va_arg(args, unsigned int), 1);
	else if (specifier == '%')
		count = print_char('%');
	return (count);
}

/**
 * @brief A simplified printf that handles cspdiuxX%%.
 * @param format The format string.
 * @return Total characters printed, or -1 on error.
 */
int	ft_printf(const char *format, ...)
{
	va_list	args;
	int		count;
	int		ret;

	if (!format)
		return (-1);
	va_start(args, format);
	count = 0;
	while (*format && count >= 0)
	{
		if (*format == '%')
		{
			format++;
			ret = print_format(args, *format);
		}
		else
			ret = print_char(*format);
		if (ret == -1)
			count = -1;
		else
			count += ret;
		format++;
	}
	va_end(args);
	return (count);
}
