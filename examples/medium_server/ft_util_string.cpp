/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_util_string.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jinam <jinam@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/20 14:08:51 by jinam             #+#    #+#             */
/*   Updated: 2023/07/20 14:33:22 by jinam            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.hpp"

std::string& ltrim(std::string& s)
{
    std::string _white = " \n\r\t\f\v";
    size_t start = s.find_first_not_of(_white);
    s.erase(0, start);
    return s;
}
std::string& rtrim(std::string& s)
{
    std::string _white = " \n\r\t\f\v";
    size_t start = s.find_last_not_of(_white);
    s.erase(start + 1);
    return s;
}
std::string& trim(std::string& s)
{
    return ltrim(rtrim(s));
}
//
// #include <iostream>
//
// int main(void)
// {
//     std::string str = "               abcd\r\n";
//
//     std::cout << str << std::endl;
//     trim(str);
//     std::cout << str << std::endl;
// }
