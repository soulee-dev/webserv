#include "RedirectHandler.hpp"
#include "../../Client.hpp"
#include "Handler.hpp"
#include <chrono>
#include <map>
#include <string>
#include <vector>

void	HandleRedirect(Client &client)
{
	std::map<std::string, int> redirect = client.request.location.getRedirection();
	std::map<std::string, std::string> header;
	std::vector<unsigned char> emptyBody;

	if (redirect.size() == 0)
		return;

	std::map<std::string, int>::reverse_iterator rit = redirect.rbegin();
	header["Location"] = rit->first;

	SetResponse(client, rit->second, header, emptyBody);
}